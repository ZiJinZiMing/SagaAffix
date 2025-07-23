/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "SagaStatsDelegates.h"

FSagaStatsDelegates::FSSOnVariableAddedOrRemoved FSagaStatsDelegates::OnVariableAdded;
FSagaStatsDelegates::FSSOnVariableAddedOrRemoved FSagaStatsDelegates::OnVariableRemoved;
FSagaStatsDelegates::FSSOnVariableRenamed FSagaStatsDelegates::OnVariableRenamed;
FSagaStatsDelegates::FSSOnVariableTypeChanged FSagaStatsDelegates::OnVariableTypeChanged;
FSagaStatsDelegates::FSSOnPreCompile FSagaStatsDelegates::OnPreCompile;
FSagaStatsDelegates::FSSOnPostCompile FSagaStatsDelegates::OnPostCompile;
FSagaStatsDelegates::FSSOnRequestDetailsRefresh FSagaStatsDelegates::OnRequestDetailsRefresh;
