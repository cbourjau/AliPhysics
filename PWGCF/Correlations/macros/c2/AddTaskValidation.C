#ifndef __CINT__
#include "TDirectory.h"
#include "TList.h"

#include "AliAnalysisManager.h"
#include "AliAnalysisDataContainer.h"
#include "AliAnalysisTaskValidation.h"
#include "AliVEvent.h"

#include <iostream>
#include <string>
#endif

using namespace std;

AliAnalysisTaskValidation *AddTaskValidation(const char* suffix = "recon") {
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    ::Error("AddTaskValidation", "No analysis manager to connect to.");
    return NULL;
  }
  
  AliAnalysisDataContainer *coutput1 =
    mgr->CreateContainer(Form("event_selection_%s", suffix),
			 TList::Class(),
			 AliAnalysisManager::kOutputContainer,
			 Form("%s", mgr->GetCommonFileName()));

  AliAnalysisDataContainer *cExchange =
    mgr->CreateContainer("event_selection_xchange",
			 AliAnalysisTaskValidation::Class(),
			 AliAnalysisManager::kExchangeContainer,
			 Form("%s", mgr->GetCommonFileName()));
  
  AliAnalysisTaskValidation *taskValidation = new AliAnalysisTaskValidation("TaskValidation");
  if (!taskValidation) {
      Error("CreateTasks", "Failed to add task!");
      return NULL;
  }
  mgr->AddTask(taskValidation);
  AliAnalysisDataContainer *inputContainer = mgr->GetCommonInputContainer();
  if(!inputContainer) {
      Error("CreateTasks", "No input container available. Failed to add task!");
      return NULL;
  }
  mgr->ConnectInput(taskValidation, 0, mgr->GetCommonInputContainer());
  mgr->ConnectOutput(taskValidation, 1, coutput1);
  mgr->ConnectOutput(taskValidation, 2, cExchange);
  return taskValidation;
}
