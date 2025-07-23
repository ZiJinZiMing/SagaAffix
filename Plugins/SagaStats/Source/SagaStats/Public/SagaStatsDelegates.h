/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"

struct SAGASTATS_API FSagaStatsDelegates
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FSSOnVariableAddedOrRemoved, const FName&);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FSSOnVariableRenamed, const FName&, const FName&, const FName&);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FSSOnVariableTypeChanged, const FName&, FString, UObject*);
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FSSOnPostCompile, const FName&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FSSOnPreCompile, const FName&);

	DECLARE_MULTICAST_DELEGATE(FSSOnRequestDetailsRefresh)

	static FSSOnVariableAddedOrRemoved OnVariableAdded;
	static FSSOnVariableAddedOrRemoved OnVariableRemoved;
	static FSSOnVariableRenamed OnVariableRenamed;
	static FSSOnVariableTypeChanged OnVariableTypeChanged;
	
	static FSSOnPreCompile OnPreCompile;
	static FSSOnPostCompile OnPostCompile;
	
	static FSSOnRequestDetailsRefresh OnRequestDetailsRefresh;
};
