/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "AttributeSet/SGAttributeSetBlueprint.h"

#include "SagaStatsLog.h"
#include "SGDelegates.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/UObjectGlobals.h"

#if WITH_EDITOR
#if UE_VERSION_NEWER_THAN(5, 1, -1)
#include "RigVMDeveloperTypeUtils.h"
#else
#include "RigVMTypeUtils.h"
#endif
#endif

USGAttributeSetBlueprint::~USGAttributeSetBlueprint()
{
	SG_LOG(VeryVerbose, TEXT("USGAttributeSetBlueprint::~USGAttributeSetBlueprint - Destructor"))
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectModified.RemoveAll(this);
	OnChanged().RemoveAll(this);
	OnCompiled().RemoveAll(this);
#endif
}

void USGAttributeSetBlueprint::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	RegisterDelegates();
#endif
}

#if WITH_EDITOR

void USGAttributeSetBlueprint::RegisterDelegates()
{
	if (IsTemplate())
	{
		return;
	}
	
	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::RegisterDelegates - Setup delegates for %s"), *GetName())

	FCoreUObjectDelegates::OnObjectModified.RemoveAll(this);
	OnChanged().RemoveAll(this);
	OnCompiled().RemoveAll(this);

	FCoreUObjectDelegates::OnObjectModified.AddUObject(this, &USGAttributeSetBlueprint::OnPreVariableChange);
	OnChanged().AddUObject(this, &USGAttributeSetBlueprint::OnPostVariableChange);
	OnCompiled().AddUObject(this, &USGAttributeSetBlueprint::OnPostCompiled);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USGAttributeSetBlueprint::OnPreVariableChange(UObject* InObject)
{
	if (InObject != this)
	{
		return;
	}

	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPreVariableChange %s"), *GetNameSafe(InObject))

	LastNewVariables = NewVariables;
}

void USGAttributeSetBlueprint::HandleVariableChanges(const UBlueprint* InBlueprint)
{
	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPostVariableChange %s"), *GetNameSafe(InBlueprint))

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
void USGAttributeSetBlueprint::OnPostVariableChange(UBlueprint* InBlueprint) const
{
	if (InBlueprint != this)
	{
		return;
	}

	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPostVariableChange - %s"), *GetNameSafe(InBlueprint))
	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPostVariableChange - IsPossiblyDirty: %s"), IsPossiblyDirty() ? TEXT("true") : TEXT("false"))
	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPostVariableChange - IsUpToDate: %s"), IsUpToDate() ? TEXT("true") : TEXT("false"))
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USGAttributeSetBlueprint::OnPostCompiled(UBlueprint* InBlueprint)
{
	if (InBlueprint != this)
	{
		return;
	}


	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPostCompiled - %s"), *GetNameSafe(InBlueprint))
	HandleVariableChanges(InBlueprint);

	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPostCompiled - IsPossiblyDirty: %s"), IsPossiblyDirty() ? TEXT("true") : TEXT("false"))
	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnPostCompiled - IsUpToDate: %s"), IsUpToDate() ? TEXT("true") : TEXT("false"))

	if (const UPackage* Package = GetPackage())
	{
		FSGDelegates::OnPostCompile.Broadcast(Package->GetFName());
	}
}

void USGAttributeSetBlueprint::OnVariableAdded(const FName& Name) const
{
	SG_NS_LOG(Verbose, TEXT("Name: %s"), *Name.ToString());
	const UPackage* Package = GetPackage();
	if (!Package)
	{
		SG_NS_LOG(Warning, TEXT("SelfPackage nullptr"))
		return;
	}
	FSGDelegates::OnVariableAdded.Broadcast(Package->GetFName(),Name);
}

void USGAttributeSetBlueprint::OnVariableRemoved(const FName& Name) const
{
	SG_NS_LOG(Verbose, TEXT("Name: %s"), *Name.ToString());
	const UPackage* Package = GetPackage();
	if (!Package)
	{
		SG_NS_LOG(Warning, TEXT("SelfPackage nullptr"))
		return;
	}
	FSGDelegates::OnVariableRemoved.Broadcast(Package->GetFName(),Name);
}

void USGAttributeSetBlueprint::OnVariableRenamed(const FName& InOldVarName, const FName& InNewVarName) const
{
	SG_NS_LOG(Verbose, TEXT("Name: %s -> %s"), *InOldVarName.ToString(), *InNewVarName.ToString());

	const UPackage* Package = GetPackage();
	if (!Package)
	{
		SG_NS_LOG(Warning, TEXT("SelfPackage nullptr"))
		return;
	}

	// Broadcast renamed event, the subsystem will handle the rest
	FSGDelegates::OnVariableRenamed.Broadcast(Package->GetFName(), InOldVarName, InNewVarName);
}

void USGAttributeSetBlueprint::OnVariableTypeChanged(const FName& InVarName, const FEdGraphPinType& InOldPinType, const FEdGraphPinType& InNewPinType) const
{
	const FString OldVarTypeStr = FString::Printf(TEXT("Category: %s, SubCategory: %s"), *InOldPinType.PinCategory.ToString(), *InOldPinType.PinSubCategory.ToString());
	const FString NewVarTypeStr = FString::Printf(TEXT("Category: %s, SubCategory: %s"), *InNewPinType.PinCategory.ToString(), *InNewPinType.PinSubCategory.ToString());

	FString CPPType;
	UObject* CPPTypeObject = nullptr;
	RigVMTypeUtils::CPPTypeFromPinType(InNewPinType, CPPType, &CPPTypeObject);

	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnVariableTypeChanged InVarName: %s || InOldPinType: %s || InNewPinType: %s"), *InVarName.ToString(), *OldVarTypeStr, *NewVarTypeStr)
	SG_LOG(Verbose, TEXT("USGAttributeSetBlueprint::OnVariableTypeChanged ==> CPPType: %s || CPPTypeObject: %s"), *CPPType, *GetNameSafe(CPPTypeObject))

	FSGDelegates::OnVariableTypeChanged.Broadcast(InVarName, CPPType, CPPTypeObject);
}

#endif