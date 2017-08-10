#ifndef AliESDToSaneTTreeTask_cxx
#define AliESDToSaneTTreeTask_cxx

#include "AliAnalysisC2Settings.h"
#include "AliAnalysisTaskValidation.h"

class TTree;

class AliAODTrack;
class AliVEvent;

class AliESDToSaneTTreeTask : public AliAnalysisTaskSE {
 public:
  AliESDToSaneTTreeTask();
  AliESDToSaneTTreeTask(const char *name);

  // Function acting as "AddTask" macro
  // Takes a reference to the event an track selection task
  static AliESDToSaneTTreeTask ConnectTask(AliAnalysisTaskValidation &validation_tas);
  virtual ~AliESDToSaneTTreeTask() {};

  virtual void   UserCreateOutputObjects();
  virtual void   UserExec(Option_t *option);
  virtual void   Terminate(Option_t *);


 private:
  enum {
    kRECON,
    kMC
  };
  struct Settings {
    Int_t fDataType;
    Bool_t fUseSPDclusters;
    Bool_t fUseSPDtracklets;
    Bool_t fUseFMD;
    TString fMultEstimator;

    // Constructor
    Settings()
      :fDataType(kRECON), fUseSPDclusters(false), fUseSPDtracklets(true),
       fUseFMD(true), fMultEstimator("V0M") {};
  };
  // Helper function to load all tracks/hits used in the analysis
  AliAnalysisTaskValidation::Tracks GetValidTracks();
  // Helper function to find the event classifier value (e.g. the
  // multiplicity in the estimator set in fSettings
  Float_t GetEventClassifierValue();

  // Declaring these shuts up warnings from Weffc++
  AliESDToSaneTTreeTask(const AliESDToSaneTTreeTask&); // not implemented
  AliESDToSaneTTreeTask& operator=(const AliESDToSaneTTreeTask&); // not implemented

  Settings fSettings; //!
  TList *fOutputList;  //!

  TTree* fSaneTree; //!  A nice and sane tree for the analysis
  TFile* fFile;     //!  Output file. Keep as member so that we can close in Terminate

  std::vector<float> fEtas; //!
  std::vector<float> fPhis; //!
  std::vector<float> fWeights; //!

  ClassDef(AliESDToSaneTTreeTask, 1); // example of analysis
};

#endif
