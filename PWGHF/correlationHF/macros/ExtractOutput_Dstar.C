//
// On AliRoot shell, call the following before loading the macro:
//
// gSystem->SetIncludePath("-I. -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include -I$ALICE_ROOT/ANALYSIS/macros -I$ROOTSYS/include");
// gROOT->LoadMacro("AliDhCorrelationExtraction.cxx++g"); // set the right path! - if not already in AliPhysics!!
//
// Modify:
// - The arguments of the macro
// - The names and paths in SetInputNames method
// - The SetSBRanges values in the ExtractXPt methods (if you use austoSB=kFALSE, otherwise they are dummy)
//
// NOTE FOR D*: If setting the sideband range externally, the (unique) sideband to be used is LSB (not RSB)
// NOTE FOR D+: If you need to integrate mass-pT bins in a single correlation bin BEFORE extracting the outputs, set plotter->IntegratePtBins(kFALSE) in the ExtractXPt methods 
// 
// setting plotter->SetDebugLevel(1) will print a series of additional plots for debugging; plotter->SetDebugLevel(2) will also be more verbose
//

void ExtractOutput(
   Bool_t treeSE=kFALSE, Bool_t treeME=kFALSE, //read TH2F from offline correlation framewrks (produced from the TTree) for SE, ME analysis instead of THnSparse
   Int_t specie=AliDhCorrelationExtraction::kDStarD0pi, //the D-meson decay channel (check the enumerator for the options)
   Int_t SandB=AliDhCorrelationExtraction::kBfromBinCount, //how to extract S and B (check the enumerator for the options) - kBfromBinCount is the paper approach
   Int_t SBscale=AliDhCorrelationExtraction::kBinCountScaling, //how to renormalize the sidebands (check the enumerator for the options) - kBfromBinCount is the paper approach
   Int_t rebin=1, //rebin the invariant mass plots - USE WITH CARE! (different bin width w.r.t. THnSparse)
   Double_t leftRng=0.14, Double_t rightRng=0.16, //invariant mass fit range -> use 1.7-2.1 for D0 and D+, 0.14-0.16 for D* (but 1.695-2.1 for D0 in pp for results before 1/5/2015)
   Int_t funcBkg=AliHFMassFitter::kPowEx, //background function used for the mass fit -> use kExpo for D0 and D+, kPowEx for D*
   Double_t nsigmaFitter=3, //number of sigma in which to extract S, B, S/B, signficance... (only for the spectra visualization, no influence on the correlations)
   Double_t nsigmaS=2, //number of sigma in which to define the signal region (for correlations)
   Bool_t autoSB=kFALSE, //kTRUE = define SB from the fit results, in the range insigma-outsigma (below); kFALSE = use ranges provided via SetSBRanges
   Double_t insigma=0, Double_t outsigma=0, //sideband ranges (in units of sigma). Valid only if autoSB=kTRUES, otherwise dummy
   Bool_t singleBinSB=kFALSE, //kTRUE=a single mass bin is used in the THnSparse for storing the sideband corlelations (used by D0 to save space)
   Int_t npools=9, //number of pools for the event-mixing
   Bool_t poolByPool=kTRUE, //kTRUE=pool-by-pool ME correction; kFALSE=merged-pools ME correction (set the options that you used in the online analysis)
   Double_t deltaEtaMin=-1., Double_t deltaEtaMax=1.) //deltaEta ranges for correlation distributions
{
 
  //Create and set the correlation plotter class
  AliDhCorrelationExtraction *plotter = new AliDhCorrelationExtraction();
  Bool_t flagSpecie = plotter->SetDmesonSpecie(specie);
  plotter->SetSandBextraction(SandB);
  plotter->SetSBscaling(SBscale);
  plotter->SetRebinMassPlots(rebin);
  plotter->SetFitRanges(leftRng,rightRng); //use 1.7-2.1 for D0 and D+, 0.14-0.16 for D*
  plotter->SetBkgFitFunction(funcBkg); //use kExpo for D0 and D+, kPowEx for D*
  plotter->SetNumberOfSigmasFitter(nsigmaFitter);
  plotter->SetSignalSigmas(nsigmaS);
  plotter->SetAutoSBRange(autoSB); //kTRUE = evaluate SB range automatically (give inner and outer sigma as 2° and 3° args); kFALSE = use ranges provided from outside (via SetSBRanges)
  plotter->SetSBSingleBin(singleBinSB);
  plotter->SetNpools(npools);
  plotter->SetCorrectPoolsSeparately(poolByPool); //kTRUE = pool.by-pool extraction and correction; kFALSE = merged ME pools
  plotter->SetDeltaEtaRange(deltaEtaMin,deltaEtaMax);
  plotter->SetSubtractSoftPiInMEdistr(kFALSE);
  plotter->ReadTTreeOutputFiles(treeSE,treeME);
  if(!flagSpecie) return;

  plotter->SetDebugLevel(0); //0 = get main results; 1 = get full list of plots; 2 = get debug printouts

  SetInputNames(plotter);  // check the names in the method!!

  Bool_t read = plotter->ReadInputs();
  if(!read) {
    printf("Error in reading the input file! Exiting...\n");
    return;
  }

  gSystem->Exec(Form("rm -r Output_Root"));
  gSystem->Exec(Form("rm -r Output_png")); 
  gSystem->Exec(Form("mkdir Output_Root"));
  gSystem->Exec(Form("mkdir Output_png"));
  
  ExtractLowPt(plotter);
  ExtractMidPt(plotter);
  ExtractHighPt(plotter);

  return;
}

//________________________________________
void SetInputNames(AliDhCorrelationExtraction *plotter){
/*
  plotter->SetInputFilename("./../AnalysisResults_InvMass_TRAIN_FINALE_pp.root");
  plotter->SetListMassName("coutputmassD0MassOutput0100");
  plotter->SetDirNameSE("PWG3_D2H_D0InvMassOutput");
  plotter->SetListNameSE("correlationsOutput0100");
  plotter->SetDirNameME("PWG3_D2H_D0InvMassOutput_ME");
  plotter->SetListNameME("correlationsOutput_ME0100");

  plotter->SetMassHistoName("histMass_WeigD0Eff_");
  plotter->SetSECorrelHistoName("hPhi_Charg_Bin");
  plotter->SetMECorrelHistoNameSuffix("_EvMix");
*/

 //Dstar paths
  plotter->SetInputFilenameMass("./../AnalysisResults_InvMass_pPb_SlowTrain.root");
  plotter->SetInputFilenameSE("./../AnalysisResults_InvMass_pPb_SlowTrain.root");
  plotter->SetInputFilenameME("./../AnalysisResults_InvMass_pPb_SlowTrain.root");
  plotter->SetDirNameMass("PWGHF_HFCJ_SE_EffY_DEffY_vsPtMult_32_bins_SE_reco_2_348_sigmas");
  plotter->SetListNameMass("OutputCorrelationsSE");  
  plotter->SetDirNameSE("PWGHF_HFCJ_SE_EffY_DEffY_vsPtMult_32_bins_SE_reco_2_348_sigmas");
  plotter->SetListNameSE("OutputCorrelationsSE");
  plotter->SetDirNameME("PWGHF_HFCJ_ME_EffY_DEffY_vsPtMult_32_bins_ME_reco_2_348_sigmas");
  plotter->SetListNameME("OutputCorrelationsME");

  plotter->SetMassHistoName("histDStarMassWeight_");
  plotter->SetSECorrelHistoName("CorrelationsDStarHadron_");
  plotter->SetSECorrelHistoName_DstarBkg("CorrelationsBkgFromDStarSBHadron_");
  plotter->SetMECorrelHistoNameSuffix("");

/*
 //Dplus paths
  plotter->SetInputFilename("./../AnalysisResults_DplusNewPools.root");
  plotter->SetListMassName("coutHistos_PP_DplusHadCorr_SE_wTrkEff_wDkEff_PoolByPoolON"); //old prefix: _PPPass4SE
  plotter->SetDirNameSE("PP_DplusHadCorr_SE_wTrkEff_wDkEff_PoolByPoolON");
  plotter->SetListNameSE("coutHistos_PP_DplusHadCorr_SE_wTrkEff_wDkEff_PoolByPoolON");
  plotter->SetDirNameME("PP_DplusHadCorr_ME_wTrkEff_wDkEff_PoolByPoolON");
  plotter->SetListNameME("coutHistos_PP_DplusHadCorr_ME_wTrkEff_wDkEff_PoolByPoolON");

  plotter->SetMassHistoName("hnSparseM_Hdron_Data_Bin");
  plotter->SetSECorrelHistoName("hnSparseC_Hdron_Data_Bin");
  plotter->SetMECorrelHistoNameSuffix("_evMix");
*/

  return;
}

//________________________________________
void ExtractLowPt(AliDhCorrelationExtraction *plotter){

  plotter->SetNpTbins(2);
  plotter->SetFirstpTbin(4);
  plotter->SetLastpTbin(5);

  //Limits of sidebands passed from outside. Warning: use limits of the mass bin edges to avoid further adjustments in the normalization!
  Double_t LSBLowLim[2] = {0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3)}; //IMPORTANT!! to be filled accordingly to those set in the task
  Double_t LSBUppLim[2] = {0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3)};
  Double_t RSBLowLim[2] = {0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3)};
  Double_t RSBUppLim[2] = {0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3)};

  plotter->SetSBRanges(LSBLowLim,LSBUppLim,RSBLowLim,RSBUppLim);

  //fit invariant mass distribution and extract S and B for normalizations
  Bool_t massfit = plotter->FitInvariantMass();
  if(!massfit) {
    printf("Error in the fitting of the mass plots! Exiting...\n");
    return;
  }

  plotter->PrintRanges();
  plotter->PrintSandBForNormal();

  //extract correlation distributions
  printf("*** Extracting correlations in 3<pT(D)<5 GeV/c, 0.3<pT(assoc)<99 GeV/c ***\n");
  Bool_t corrExtrThr1 = plotter->ExtractCorrelations(0.3,99.);
  printf("*** Extracting correlations in 3<pT(D)<5 GeV/c, 0.3<pT(assoc)<1 GeV/c ***\n");
  Bool_t corrExtrThr2 = plotter->ExtractCorrelations(0.3,1.);
  printf("*** Extracting correlations in 3<pT(D)<5 GeV/c, 1<pT(assoc)<99 GeV/c ***\n");
  Bool_t corrExtrThr3 = plotter->ExtractCorrelations(1.,99.);
  if(!corrExtrThr1 || !corrExtrThr2 || !corrExtrThr3) {
    printf("Error in the extraction of the correlation distributions! Exiting...\n");
    return;
  }

  plotter->ClearObjects(); //important! Call it after each wide-pT range

}


//________________________________________
void ExtractMidPt(AliDhCorrelationExtraction *plotter){

  plotter->SetNpTbins(3);
  plotter->SetFirstpTbin(6);
  plotter->SetLastpTbin(8);

  //Limits of sidebands passed from outside. Warning: use limits of the mass bin edges to avoid further adjustments in the normalization!
  Double_t LSBLowLim[3] = {0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3)}; //IMPORTANT!! to be filled accordingly to those set in the task
  Double_t LSBUppLim[3] = {0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3)};
  Double_t RSBLowLim[3] = {0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3)};
  Double_t RSBUppLim[3] = {0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3)};

  plotter->SetSBRanges(LSBLowLim,LSBUppLim,RSBLowLim,RSBUppLim);

  //fit invariant mass distribution and extract S and B for normalizations
  Bool_t massfit = plotter->FitInvariantMass();
  if(!massfit) {
    printf("Error in the fitting of the mass plots! Exiting...\n");
    return;
  }

  plotter->PrintRanges();
  plotter->PrintSandBForNormal();

  //extract correlation distributions
  printf("*** Extracting correlations in 5<pT(D)<8 GeV/c, 0.3<pT(assoc)<99 GeV/c ***\n");
  Bool_t corrExtrThr1 = plotter->ExtractCorrelations(0.3,99.);
  printf("*** Extracting correlations in 5<pT(D)<8 GeV/c, 0.3<pT(assoc)<1 GeV/c ***\n");
  Bool_t corrExtrThr2 = plotter->ExtractCorrelations(0.3,1.);
  printf("*** Extracting correlations in 5<pT(D)<8 GeV/c, 1<pT(assoc)<99 GeV/c ***\n");
  Bool_t corrExtrThr3 = plotter->ExtractCorrelations(1.,99.);
  if(!corrExtrThr1 || !corrExtrThr2 || !corrExtrThr3) {
    printf("Error in the extraction of the correlation distributions! Exiting...\n");
    return;
  }

  plotter->ClearObjects(); //important! Call it after each wide-pT range

}


//________________________________________
void ExtractHighPt(AliDhCorrelationExtraction *plotter){

  plotter->SetNpTbins(3);
  plotter->SetFirstpTbin(9);
  plotter->SetLastpTbin(11);

  //Limits of sidebands passed from outside. Warning: use limits of the mass bin edges to avoid bias in the normalization!
  Double_t LSBLowLim[3] = {0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3)}; //IMPORTANT!! to be filled accordingly to those set in the task
  Double_t LSBUppLim[3] = {0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3)};
  Double_t RSBLowLim[3] = {0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3),0.1454 + 5*(0.0019/3)};
  Double_t RSBUppLim[3] = {0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3),0.1454 + 10*(0.0019/3)};

// 0.1485-0.15
// 0.1485-0.1525
// 0.1485-0.155
// 0.1485-0.16

  plotter->SetSBRanges(LSBLowLim,LSBUppLim,RSBLowLim,RSBUppLim);

  //fit invariant mass distribution and extract S and B for normalizations
  Bool_t massfit = plotter->FitInvariantMass();
  if(!massfit) {
    printf("Error in the fitting of the mass plots! Exiting...\n");
    return;
  }

  plotter->PrintRanges();
  plotter->PrintSandBForNormal();

  //extract correlation distributions
  printf("*** Extracting correlations in 8<pT(D)<16 GeV/c, 0.3<pT(assoc)<99 GeV/c ***\n");
  Bool_t corrExtrThr1 = plotter->ExtractCorrelations(0.3,99.);
  printf("*** Extracting correlations in 8<pT(D)<16 GeV/c, 0.3<pT(assoc)<1 GeV/c ***\n");
  Bool_t corrExtrThr2 = plotter->ExtractCorrelations(0.3,1.);
  printf("*** Extracting correlations in 8<pT(D)<16 GeV/c, 1<pT(assoc)<99 GeV/c ***\n");
  Bool_t corrExtrThr3 = plotter->ExtractCorrelations(1.,99.);
  if(!corrExtrThr1 || !corrExtrThr2 || !corrExtrThr3) {
    printf("Error in the extraction of the correlation distributions! Exiting...\n");
    return;
  }

  plotter->ClearObjects(); //important! Call it after each wide-pT range

}
