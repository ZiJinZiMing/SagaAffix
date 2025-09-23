/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "SGDelegates.h"

FSGDelegates::FOnVariableAddedOrRemoved FSGDelegates::OnVariableAdded;
FSGDelegates::FOnVariableAddedOrRemoved FSGDelegates::OnVariableRemoved;
FSGDelegates::FOnVariableRenamed FSGDelegates::OnVariableRenamed;
FSGDelegates::FOnVariableTypeChanged FSGDelegates::OnVariableTypeChanged;
FSGDelegates::FOnPreCompile FSGDelegates::OnPreCompile;
FSGDelegates::FOnPostCompile FSGDelegates::OnPostCompile;
FSGDelegates::FOnRequestDetailsRefresh FSGDelegates::OnRequestDetailsRefresh;
