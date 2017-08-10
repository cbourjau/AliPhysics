#include <iostream>
#include <vector>

#include "TH1.h"
#include "THn.h"
#include "TList.h"
#include "TTree.h"
#include "TChain.h"
#include "TMath.h"
#include <TFile.h>

#include "AliAODEvent.h"
#include "AliAODMCParticle.h"
#include "AliHeader.h"
#include "AliGenEventHeader.h"
#include "AliMCEvent.h"
#include "AliMultSelection.h"
#include "AliStack.h"
#include "AliVEvent.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisDataContainer.h"
#include "AliAnalysisDataSlot.h"

#include "AliESDToSaneTTreeTask.h"
#include "AliAnalysisC2Utils.h"

using std::cout;
using std::endl;
using std::vector;

//________________________________________________________________________
AliESDToSaneTTreeTask::AliESDToSaneTTreeTask()
  : AliAnalysisTaskSE(),
    fOutputList(0),
    fSaneTree(0),
    fEtas(),
    fPhis(),
    fWeights(),
    fSettings(),
    fFile(0)
{
  // Rely on validation task for event and track selection
  DefineInput(1, AliAnalysisTaskValidation::Class());
  DefineOutput(1, TTree::Class());
}

//________________________________________________________________________
AliESDToSaneTTreeTask::AliESDToSaneTTreeTask(const char *name)
  : AliAnalysisTaskSE(name),
    fOutputList(0),
    fSaneTree(0),
    fEtas(),
    fPhis(),
    fWeights(),
    fSettings(),
    fFile(0)
{
  // Rely on validation task for event and track selection
  DefineInput(0, TChain::Class());
  DefineInput(1, AliAnalysisTaskValidation::Class());
  DefineOutput(1, TTree::Class());
}


AliESDToSaneTTreeTask *AliESDToSaneTTreeTask::ConnectTask(AliAnalysisTaskValidation *validation_task) {
  AliESDToSaneTTreeTask *task = new AliESDToSaneTTreeTask("SaneTask");
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    ::Error("AddTaskC2", "No analysis manager to connect to.");
  };

  // Note: It is important to set the output class to TTree here or
  // all goes to shit - without a warning, of course :P
  AliAnalysisDataContainer *cout_tree =
    mgr->CreateContainer("sane_tree",
			 TTree::Class(),
			 AliAnalysisManager::kOutputContainer,
			 Form("%s", mgr->GetCommonFileName()));

  mgr->AddTask(task);
  // Boiler plate code for connecting the input
  mgr->ConnectInput(task, 0, mgr->GetCommonInputContainer());
  // Connect the output tree
  mgr->ConnectOutput(task, 1, cout_tree);
  // Connect the validation task
  task->ConnectInput(1, validation_task->GetOutputSlot(2)->GetContainer());
  return task;
}
//________________________________________________________________________
void AliESDToSaneTTreeTask::UserCreateOutputObjects()
{
  AliAnalysisManager* mgr = AliAnalysisManager::GetAnalysisManager();
  TObject* oc = mgr->GetOutputs()->FindObject("sane_tree");
  if (!oc) {
    Warning("CreateTree", "No output container %sTree",GetName());
    return;
  }
  AliAnalysisDataContainer* c = static_cast<AliAnalysisDataContainer*>(oc);
  c->SetSpecialOutput();

  Printf("Opening analysis file for container %s", c->GetName());
  fFile = AliAnalysisManager::OpenFile(c, "RECREATE");
  if (!fFile) {
    Warning("CreateTree", "Failed to open container file");
    return;
  }
  

  // Setup output list if it was not done already
  if (!this->fOutputList){
    this->fOutputList = new TList();
    this->fOutputList->SetOwner();
  }

  // AliLog::SetGlobalLogLevel(AliLog::kError);
  if (!this->fSaneTree){
    this->fSaneTree = new TTree("sane_tree", "Why do sane trees need a damn title");
    this->fSaneTree->SetDirectory(fFile);
  }
  // this->fSaneTree->Branch("tracks", &fTracksVector);
  Float_t dummy_float = 0;
  this->fSaneTree->Branch("centrality", &dummy_float);
  this->fSaneTree->Branch("zvtx", &dummy_float);
  this->fSaneTree->Branch("etas", &fEtas);
  this->fSaneTree->Branch("phis", &fPhis);
  this->fSaneTree->Branch("weights", &fWeights);

  PostData(1, fSaneTree);
}

//________________________________________________________________________
void AliESDToSaneTTreeTask::UserExec(Option_t *)
{
  // Get the event validation object
  AliAnalysisTaskValidation* ev_val = dynamic_cast<AliAnalysisTaskValidation*>(this->GetInputData(1));
  
  if (!ev_val->IsValidEvent()){
    return;
  }
  Float_t centrality = this->GetEventClassifierValue();
  Float_t zvtx = (this->InputEvent()->GetPrimaryVertex())
    ? this->InputEvent()->GetPrimaryVertex()->GetZ()
    : -999;
  Double_t evWeight = (/*this->fSettings.kMCTRUTH == this->fSettings.fDataType*/ false)
    ? this->MCEvent()->GenEventHeader()->EventWeight()
    : 1;

  // Load all valid tracks/hits used in the following
  auto tracks = this->GetValidTracks();

  // Write the current event as a python dictionary to stderr. Skrew you ROOT, you piece of $#!7!
  std::cerr << "{ ";
  std::cerr << "\"centrality\": " << centrality << ",";
  std::cerr << "\"zvtx\": " << zvtx << ",";
  std::cerr << "\"etas\": [";
  for (auto track: tracks) {std::cerr << track.eta << ",";}
  std::cerr << "], ";
  std::cerr << "\"phis\": [";
  for (auto track: tracks) {std::cerr << track.phi << ",";}
  std::cerr << "],";
  std::cerr << "\"weights\": [";
  for (auto track: tracks) {std::cerr << track.weight << ",";}
  std::cerr << "],";
  std::cerr << "}" <<std::endl;
}

//________________________________________________________________________
void AliESDToSaneTTreeTask::Terminate(Option_t *)
{
  // if (!fFile) return;
  // fFile->Write();
  // fFile->Close();
  // delete fFile;
}

AliAnalysisTaskValidation::Tracks AliESDToSaneTTreeTask::GetValidTracks() {
  // Get the event validation object
  AliAnalysisTaskValidation* ev_val = dynamic_cast<AliAnalysisTaskValidation*>(this->GetInputData(1));
  ev_val->GetFMDhits();

  AliAnalysisTaskValidation::Tracks ret_vector;
  // Append central tracklets
  if (this->kRECON == this->fSettings.fDataType) {
    // Are we running on SPD clusters? If so add them to our track vector
    if (this->fSettings.fUseSPDclusters) {
      AliFatal("SPD clusters not yet implemented");
    }
    else if (this->fSettings.fUseSPDtracklets) {
      auto spdhits = ev_val->GetSPDtracklets();
      ret_vector.insert(ret_vector.end(), spdhits.begin(), spdhits.end());
    }
    // Append the fmd hits to this vector if we are looking at reconstructed data,
    // All hits on the FMD (above the internally used threshold) are "valid"
    if (this->fSettings.fUseFMD) {
      auto fmdhits = ev_val->GetFMDhits();
      ret_vector.insert(ret_vector.end(), fmdhits.begin(), fmdhits.end());
    }
  }
  // MC truth case:
  else {
    AliError("MC truth is not yet implemented");
    // auto allMCtracks = this->GetValidCentralTracks();
    // this->fValidTracks.insert(this->fValidTracks.end(), allMCtracks.begin(), allMCtracks.end());
  }
  return ret_vector;
}

Float_t AliESDToSaneTTreeTask::GetEventClassifierValue() {
  if (this->fSettings.fMultEstimator == "ValidTracks"){
    return this->GetValidTracks().size();
  }
  else {
    // Event is invalid if no multselection is present; ie. tested in IsValidEvent() already
    AliMultEstimator *multEstimator =
      (dynamic_cast< AliMultSelection* >(this->InputEvent()->FindListObject("MultSelection")))
      ->GetEstimator(this->fSettings.fMultEstimator);
    // const Float_t multiplicity = ((Float_t)multEstimator->GetValue()) / multEstimator->GetMean();
    const Float_t multiplicity = multEstimator->GetPercentile();
    return multiplicity;
  }
}
