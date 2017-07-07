#include <iostream>
#include <vector>

#include "TH1.h"
#include "THn.h"
#include "TList.h"
#include "TTree.h"
#include "TMath.h"

#include "AliAODEvent.h"
#include "AliAODMCParticle.h"
#include "AliHeader.h"
#include "AliGenEventHeader.h"
#include "AliMCEvent.h"
#include "AliMultSelection.h"
#include "AliStack.h"
#include "AliVEvent.h"

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
    fSettings()
{
  // Rely on validation task for event and track selection
  DefineInput(1, AliAnalysisTaskValidation::Class());
  DefineOutput(1, TList::Class());
  // DefineOutput(2, TTree::Class());
}

//________________________________________________________________________
AliESDToSaneTTreeTask::AliESDToSaneTTreeTask(const char *name)
  : AliAnalysisTaskSE(name),
    fOutputList(0),
    fSaneTree(0),
    fEtas(),
    fPhis(),
    fWeights(),
    fSettings()
{
  // Rely on validation task for event and track selection
  DefineInput(1, AliAnalysisTaskValidation::Class());
  DefineOutput(1, TList::Class());
  // DefineOutput(2, TTree::Class());
}

//________________________________________________________________________
void AliESDToSaneTTreeTask::UserCreateOutputObjects()
{
  // Setup output list if it was not done already
  if (!this->fOutputList){
    this->fOutputList = new TList();
    this->fOutputList->SetOwner();
  }

  // AliLog::SetGlobalLogLevel(AliLog::kError);

  this->fSaneTree = new TTree("sane_tree", "Why do sane trees need a damn title");

  // this->fSaneTree->Branch("tracks", &fTracksVector);
  Float_t dummy_float = 0;
  this->fSaneTree->Branch("centrality", &dummy_float);
  this->fSaneTree->Branch("zvtx", &dummy_float);
  this->fSaneTree->Branch("etas", &fEtas);
  this->fSaneTree->Branch("phis", &fPhis);
  this->fSaneTree->Branch("weights", &fWeights);

  PostData(1, fOutputList);
  PostData(2, fSaneTree);
}

//________________________________________________________________________
void AliESDToSaneTTreeTask::UserExec(Option_t *)
{
  // Get the event validation object
  AliAnalysisTaskValidation* ev_val = dynamic_cast<AliAnalysisTaskValidation*>(this->GetInputData(1));

  AliMCEvent*   mcEvent = this->MCEvent();

  if (!ev_val->IsValidEvent()){
    PostData(1, this->fOutputList);
    return;
  }
  Float_t centrality = this->GetEventClassifierValue();
  Float_t zvtx = (this->InputEvent()->GetPrimaryVertex())
    ? this->InputEvent()->GetPrimaryVertex()->GetZ()
    : -999;
  Double_t evWeight = (/*this->fSettings.kMCTRUTH == this->fSettings.fDataType*/ false)
    ? mcEvent->GenEventHeader()->EventWeight()
    : 1;
  // Load all valid tracks/hits used in the following
  auto tracks = this->GetValidTracks();
  for (auto v: {fEtas, fPhis, fWeights}) {
    v.clear();
    v.resize(tracks.size());
  }
  for (auto track: tracks) {
    fEtas.push_back(track.eta);
    fPhis.push_back(track.phi);
    fWeights.push_back(track.weight);
  }

  this->fSaneTree->SetBranchAddress("centrality", &centrality);
  this->fSaneTree->SetBranchAddress("zvtx", &zvtx);

  this->fSaneTree->Fill();

  PostData(1, this->fOutputList);
  PostData(2, this->fSaneTree);
}

//________________________________________________________________________
void AliESDToSaneTTreeTask::Terminate(Option_t *)
{
  // PostData(1, this->fOutputList);
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
    cout << multiplicity << endl;
    return multiplicity;
  }
}
