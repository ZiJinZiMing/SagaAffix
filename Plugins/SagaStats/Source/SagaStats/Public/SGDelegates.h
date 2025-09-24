/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"

struct SAGASTATS_API FSGDelegates
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnVariableAddedOrRemoved, const FName&, const FName&);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnVariableRenamed, const FName&, const FName&, const FName&);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnVariableTypeChanged, const FName&, FString, UObject*);
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostCompile, const FName&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPreCompile, const FName&);

	DECLARE_MULTICAST_DELEGATE(FOnRequestDetailsRefresh)
	/**
	 * Triggered whenever a variable is added to a GBA Blueprint
	 *
	 * @param PackageName the package FName of the Blueprint where the variable was added
	 * @param PropertyName the property name that was added
	 */
	static FOnVariableAddedOrRemoved OnVariableAdded;
	/**
	 * Triggered whenever a variable is removed from a GBA Blueprint
	 *
	 * @param PackageName the package FName of the Blueprint from which the variable was removed
	 * @param PropertyName the property name that was removed
	 */
	static FOnVariableAddedOrRemoved OnVariableRemoved;
	/**
     * Triggered whenever a variable is removed from a GBA Blueprint
     *
     * @param PackageName the package FName of the Blueprint from which the variable was renamed
     * @param OldPropertyName the old property name
     * @param NewPropertyName the new property name after rename
     */
	static FOnVariableRenamed OnVariableRenamed;
	static FOnVariableTypeChanged OnVariableTypeChanged;
	
	static FOnPreCompile OnPreCompile;
	static FOnPostCompile OnPostCompile;
	
	static FOnRequestDetailsRefresh OnRequestDetailsRefresh;	
};
