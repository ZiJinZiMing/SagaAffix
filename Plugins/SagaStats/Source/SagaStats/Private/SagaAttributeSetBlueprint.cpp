/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "SagaAttributeSetBlueprint.h"

#include "SagaStatsDelegates.h"
#include "SagaStatsLog.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/UObjectGlobals.h"

#if WITH_EDITOR
#if UE_VERSION_NEWER_THAN(5, 1, -1)
#include "RigVMDeveloperTypeUtils.h"
#else
#include "RigVMTypeUtils.h"
#endif
#endif

USagaAttributeSetBlueprint::~USagaAttributeSetBlueprint()
{
	SS_LOG(VeryVerbose, TEXT("USagaAttributeSetBlueprint::~USagaAttributeSetBlueprint - Destructor"))
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectModified.RemoveAll(this);
	OnChanged().RemoveAll(this);
	OnCompiled().RemoveAll(this);
#endif
}

void USagaAttributeSetBlueprint::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	RegisterDelegates();
#endif
}

#if WITH_EDITOR

void USagaAttributeSetBlueprint::RegisterDelegates()
{
	if (IsTemplate())
	{
		return;
	}
	
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::RegisterDelegates - Setup delegates for %s"), *GetName())

	FCoreUObjectDelegates::OnObjectModified.RemoveAll(this);
	OnChanged().RemoveAll(this);
	OnCompiled().RemoveAll(this);

	FCoreUObjectDelegates::OnObjectModified.AddUObject(this, &USagaAttributeSetBlueprint::OnPreVariableChange);
	OnChanged().AddUObject(this, &USagaAttributeSetBlueprint::OnPostVariableChange);
	OnCompiled().AddUObject(this, &USagaAttributeSetBlueprint::OnPostCompiled);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USagaAttributeSetBlueprint::OnPreVariableChange(UObject* InObject)
{
	if (InObject != this)
	{
		return;
	}

	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPreVariableChange %s"), *GetNameSafe(InObject))

	LastNewVariables = NewVariables;
}

void USagaAttributeSetBlueprint::HandleVariableChanges(const UBlueprint* InBlueprint)
{
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPostVariableChange %s"), *GetNameSafe(InBlueprint))

	TMap<FGuid, int32> NewVariablesByGuid;
	for (int32 VarIndex = 0; VarIndex < NewVariables.Num(); VarIndex++)
	{
		NewVariablesByGuid.Add(NewVariables[VarIndex].VarGuid, VarIndex);
	}

	TMap<FGuid, int32> OldVariablesByGuid;
	for (int32 VarIndex = 0; VarIndex < LastNewVariables.Num(); VarIndex++)
	{
		OldVariablesByGuid.Add(LastNewVariables[VarIndex].VarGuid, VarIndex);
	}

	for (const FBPVariableDescription& OldVariable : LastNewVariables)
	{
		if (!NewVariablesByGuid.Contains(OldVariable.VarGuid))
		{
			OnVariableRemoved(OldVariable.VarName);
		}
	}

	for (const FBPVariableDescription& NewVariable : NewVariables)
	{
		if (!OldVariablesByGuid.Contains(NewVariable.VarGuid))
		{
			OnVariableAdded(NewVariable.VarName);
			continue;
		}

		const int32 OldVarIndex = OldVariablesByGuid.FindChecked(NewVariable.VarGuid);
		const FBPVariableDescription& OldVariable = LastNewVariables[OldVarIndex];
		if (OldVariable.VarName != NewVariable.VarName)
		{
			OnVariableRenamed(OldVariable.VarName, NewVariable.VarName);
		}

		if (OldVariable.VarType != NewVariable.VarType)
		{
			OnVariableTypeChanged(NewVariable.VarName, OldVariable.VarType, NewVariable.VarType);
		}
	}

	LastNewVariables = NewVariables;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USagaAttributeSetBlueprint::OnPostVariableChange(UBlueprint* InBlueprint) const
{
	if (InBlueprint != this)
	{
		return;
	}

	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPostVariableChange - %s"), *GetNameSafe(InBlueprint))
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPostVariableChange - IsPossiblyDirty: %s"), IsPossiblyDirty() ? TEXT("true") : TEXT("false"))
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPostVariableChange - IsUpToDate: %s"), IsUpToDate() ? TEXT("true") : TEXT("false"))
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USagaAttributeSetBlueprint::OnPostCompiled(UBlueprint* InBlueprint)
{
	if (InBlueprint != this)
	{
		return;
	}


	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPostCompiled - %s"), *GetNameSafe(InBlueprint))
	HandleVariableChanges(InBlueprint);

	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPostCompiled - IsPossiblyDirty: %s"), IsPossiblyDirty() ? TEXT("true") : TEXT("false"))
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnPostCompiled - IsUpToDate: %s"), IsUpToDate() ? TEXT("true") : TEXT("false"))

	if (const UPackage* Package = GetPackage())
	{
		FSagaStatsDelegates::OnPostCompile.Broadcast(Package->GetFName());
	}
}

void USagaAttributeSetBlueprint::OnVariableAdded(const FName& Name)
{
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnVariableAdded Name: %s"), *Name.ToString())
	FSagaStatsDelegates::OnVariableAdded.Broadcast(Name);
}

void USagaAttributeSetBlueprint::OnVariableRemoved(const FName& Name)
{
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnVariableRemoved Name: %s"), *Name.ToString())
	FSagaStatsDelegates::OnVariableRemoved.Broadcast(Name);
}

void USagaAttributeSetBlueprint::OnVariableRenamed(const FName& InOldVarName, const FName& InNewVarName) const
{
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnVariableRenamed InOldVarName: %s, InNewVarName: %s"), *InOldVarName.ToString(), *InNewVarName.ToString())

	const UPackage* Package = GetPackage();
	if (!Package)
	{
		SS_LOG(Warning, TEXT("USagaAttributeSetBlueprint::OnVariableRenamed SelfPackage nullptr"))
		return;
	}

	// Broadcast renamed event, the subsystem will handle the rest
	FSagaStatsDelegates::OnVariableRenamed.Broadcast(Package->GetFName(), InOldVarName, InNewVarName);
}

void USagaAttributeSetBlueprint::OnVariableTypeChanged(const FName& InVarName, const FEdGraphPinType& InOldPinType, const FEdGraphPinType& InNewPinType) const
{
	const FString OldVarTypeStr = FString::Printf(TEXT("Category: %s, SubCategory: %s"), *InOldPinType.PinCategory.ToString(), *InOldPinType.PinSubCategory.ToString());
	const FString NewVarTypeStr = FString::Printf(TEXT("Category: %s, SubCategory: %s"), *InNewPinType.PinCategory.ToString(), *InNewPinType.PinSubCategory.ToString());

	FString CPPType;
	UObject* CPPTypeObject = nullptr;
	RigVMTypeUtils::CPPTypeFromPinType(InNewPinType, CPPType, &CPPTypeObject);

	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnVariableTypeChanged InVarName: %s || InOldPinType: %s || InNewPinType: %s"), *InVarName.ToString(), *OldVarTypeStr, *NewVarTypeStr)
	SS_LOG(Verbose, TEXT("USagaAttributeSetBlueprint::OnVariableTypeChanged ==> CPPType: %s || CPPTypeObject: %s"), *CPPType, *GetNameSafe(CPPTypeObject))

	FSagaStatsDelegates::OnVariableTypeChanged.Broadcast(InVarName, CPPType, CPPTypeObject);
}

#endif