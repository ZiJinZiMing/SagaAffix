/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "SGDelegates.h"

FSGDelegates::FOnVariableAddedOrRemoved FSGDelegates::OnVariableAdded;
FSGDelegates::FOnVariableAddedOrRemoved FSGDelegates::OnVariableRemoved;
FSGDelegates::FOnVariableRenamed FSGDelegates::OnVariableRenamed;
FSGDelegates::FOnVariableTypeChanged FSGDelegates::OnVariableTypeChanged;
FSGDelegates::FOnPreCompile FSGDelegates::OnPreCompile;
FSGDelegates::FOnPostCompile FSGDelegates::OnPostCompile;
FSGDelegates::FOnRequestDetailsRefresh FSGDelegates::OnRequestDetailsRefresh;
