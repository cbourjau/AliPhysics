AliAnalysisTask *AddTask_cjahnke_JPsi(char* period = "11d",  Int_t trigger_index=0, Bool_t isMC, TString cfg = "ConfigJpsi_cj_pp", Bool_t alienconf = kFALSE, Bool_t localconf = kFALSE){
  //get the current analysis manager
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    ::Error("AddTask_cjahnke_JPsi", "No analysis manager found.");
    return NULL;
  }
  if (!mgr->GetInputEventHandler()) {
    ::Error("AddTask_cjahnke_JPsi", "This task requires an input event handler");
    return NULL;
  }

  //Do we have an MC handler?
  Bool_t hasMC=(mgr->GetMCtruthEventHandler()!=0x0);

	
  Bool_t isAOD=mgr->GetInputEventHandler()->IsA()==AliAODInputHandler::Class();

  //create task and add it to the manager
  AliAnalysisTaskMultiDielectron *task=new AliAnalysisTaskMultiDielectron("MultiDie");
  mgr->AddTask(task);
	
///======
// set config file name
	
	
	TString configFile("");
	if(cfg.IsNull()) cfg="ConfigJpsi_cj_pp";
	
	// the different paths
	TString alienPath("alien:///alice/cern.ch/user/c/cjahnke/MacrosJPsi");
	TString alirootPath("$ALICE_PHYSICS/PWGDQ/dielectron/macrosJPSI");
	
	// >>> local config
	if(localconf){
		configFile="ConfigJpsi_cj_pp.C";
	}
	
		// >>> aliroot config
	else if(!alienconf){
		configFile=alirootPath.Data();
	}
		// >>> alien config
	else{
		if(!gSystem->Exec(Form("alien_cp %s/%s.C .",alienPath.Data(),cfg.Data()))) {
			configFile=gSystem->pwd();
			printf("Not sure if copy or not from alien!!!\n");
		}
		else {
			printf("ERROR: couldn't copy file %s/%s.C from grid \n", alienPath.Data(),cfg.Data() );
			return;
		}
	}
		// add config to path
	if(!localconf){
		configFile+="/";
		configFile+=cfg.Data();
		configFile+=".C";
	}
	
		// load dielectron configuration file (only once)
	if (!gROOT->GetListOfGlobalFunctions()->FindObject("ConfigJpsi_cj_pp")){
		gROOT->LoadMacro(configFile.Data());
	}

	
	
	  
//add dielectron analysis with different cuts to the task
  for (Int_t i=0; i<nDie; ++i){ //nDie defined in config file
    AliDielectron *jpsi=ConfigJpsi_cj_pp(i,isAOD, trigger_index, isMC);
    if (isAOD) jpsi->SetHasMC(isMC);
	  if (jpsi){
		  task->AddDielectron(jpsi);
  
	  }
  }

	
	
	 
  //Add event filter
  AliDielectronEventCuts *eventCuts=new AliDielectronEventCuts("eventCuts","Vertex Track && |vtxZ|<10 && ncontrib>0");
  if(isAOD) eventCuts->SetVertexType(AliDielectronEventCuts::kVtxAny);
  eventCuts->SetRequireVertex();
  eventCuts->SetMinVtxContributors(1);
  eventCuts->SetVertexZ(-10.,10.);
  //eventCuts->SetCentralityRange(0.0,80.0);
  task->SetEventFilter(eventCuts);

	
  //pileup rejection
  //task->SetRejectPileup();
	
if(!isMC){
		//if(trigger_index == 0)task->SetTriggerMask(AliVEvent::kINT7);
	if(trigger_index == 0)task->SelectCollisionCandidates(AliVEvent::kINT7);
	
		//if(trigger_index == 1)task->SetTriggerMask(AliVEvent::kEMC7);
	if(trigger_index == 1)task->SelectCollisionCandidates(AliVEvent::kEMC7);
	if(trigger_index == 2 )task->SetTriggerMask(AliVEvent::kEMCEGA);
	
   if(trigger_index == 3 )
   {
	   task->SetTriggerMask(AliVEvent::kEMCEGA);
	   task->SetFiredTriggerName("EG1");
   }
   if(trigger_index == 4 )
   {
		task->SetTriggerMask(AliVEvent::kEMCEGA);
		task->SetFiredTriggerName("EG2");
   }
	
	//for 16k which has a different threshold:
   if(trigger_index == 6 )
   {
		task->SetTriggerMask(AliVEvent::kEMCEGA);
		task->SetFiredTriggerName("EG1");
   }

//=============================================================================
//=============================================================================
//=============================================================================
//DCal triggers
	
    if(trigger_index == 30 )
	{
		task->SetTriggerMask(AliVEvent::kEMCEGA);
		task->SetFiredTriggerName("DG1");
	}
	if(trigger_index == 40 )
	{
		task->SetTriggerMask(AliVEvent::kEMCEGA);
		task->SetFiredTriggerName("DG2");
	}
	
	//for 16k which has a different threshold:
	if(trigger_index == 60 )
	{
		task->SetTriggerMask(AliVEvent::kEMCEGA);
		task->SetFiredTriggerName("DG1");
	}

//=============================================================================
//=============================================================================
//=============================================================================
	
	
	
	
	if(trigger_index == 5 ){
		 task->SetTriggerMask(AliVEvent::kAny);
		 TString triggerClass = "kHighMultV0";
		 if(! triggerClass.IsNull() ) task->SetFiredTriggerName(triggerClass.Data() );
		//task->SetFiredTriggerName("HMV0");
	}
}
	//task->UsePhysicsSelection();
	
		
  //----------------------
  //create data containers
  //----------------------
  
  TString containerName = mgr->GetCommonFileName();
  containerName += ":PWGDQ_dielectron_EMCal";
    
  //create output container
  
  AliAnalysisDataContainer *cOutputHist1 =
    mgr->CreateContainer(Form("cjahnke_QA_%d", trigger_index), TList::Class(), AliAnalysisManager::kOutputContainer,
                         containerName.Data());
  
  AliAnalysisDataContainer *cOutputHist2 =
    mgr->CreateContainer(Form("cjahnke_CF_%d", trigger_index), TList::Class(), AliAnalysisManager::kOutputContainer,
                         containerName.Data());

  AliAnalysisDataContainer *cOutputHist3 =
    mgr->CreateContainer(Form("cjahnke_EventStat_%d", trigger_index), TH1D::Class(), AliAnalysisManager::kOutputContainer,
                         containerName.Data());

	
  mgr->ConnectInput(task,  0, mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task, 1, cOutputHist1);
  mgr->ConnectOutput(task, 2, cOutputHist2);
  mgr->ConnectOutput(task, 3, cOutputHist3);
	

  return task;
	
	
}
