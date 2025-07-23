/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#include "SagaEditorSubsystem.h"

#include "Editor.h"
#include "SagaStatsDelegates.h"
#include "SSEditorLog.h"
#include "GameplayEffect.h"
#include "K2Node.h"
#include "MessageLogModule.h"
#include "TimerManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Logging/MessageLog.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/UObjectToken.h"
#include "ReferencerHandlers/SSGameplayEffectReferencerHandler.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "UObject/UObjectIterator.h"

#define LOCTEXT_NAMESPACE "SagaEditorSubsystem"

void USagaEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowFilters = true;
	InitOptions.bShowPages = true;
	MessageLogModule.RegisterLogListing(LogName, LOCTEXT("SagaStatsLog", "SagaStats Log"), InitOptions);

	FSagaStatsDelegates::OnVariableRenamed.AddUObject(this, &USagaEditorSubsystem::HandleAttributeRename);
	FSagaStatsDelegates::OnPreCompile.AddUObject(this, &USagaEditorSubsystem::HandlePreCompile);

	// Seems like a ForceRefresh() on FGameplayAttribute Details avoids the need to re-open the GE BP to update the Attribute picker
	// but still getting occasional crash on Array export text
	FSagaStatsDelegates::OnPostCompile.AddUObject(this, &USagaEditorSubsystem::HandlePostCompile);

	// Register handlers
	RegisterReferencerHandler(TEXT("GameplayEffect"), FSSGameplayEffectReferencerHandler::Create());
}

void USagaEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();

	RegisteredHandlers.Reset();

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.UnregisterLogListing(LogName);

	FSagaStatsDelegates::OnPreCompile.RemoveAll(this);
	FSagaStatsDelegates::OnPostCompile.RemoveAll(this);
	FSagaStatsDelegates::OnVariableRenamed.RemoveAll(this);
}

USagaEditorSubsystem& USagaEditorSubsystem::Get()
{
	check(GEditor);
	return *GEditor->GetEditorSubsystem<USagaEditorSubsystem>();
}

void USagaEditorSubsystem::RegisterReferencerHandler(const FName& InClassName, const TSharedPtr<ISSAttributeReferencerHandler>& InHandler)
{
	RegisteredHandlers.Emplace(InClassName, InHandler);
}

void USagaEditorSubsystem::UnregisterReferencerHandler(const FName& InClassName)
{
	if (RegisteredHandlers.Contains(InClassName))
	{
		RegisteredHandlers.Remove(InClassName);
	}
}

TSharedPtr<ISSAttributeReferencerHandler> USagaEditorSubsystem::FindAssetDependencyHandler(const FName& InPackageName, TWeakObjectPtr<UObject>& OutDefaultObject)
{
	const FString PackageName = GetLoadClassPackagePath(InPackageName.ToString());

	// Try load class and access to CDO
	const UClass* LoadClass = StaticLoadClass(UObject::StaticClass(), nullptr, *PackageName);
	if (!LoadClass)
	{
		return nullptr;
	}

	UObject* CDO = LoadClass->GetDefaultObject(false);
	if (!CDO)
	{
		return nullptr;
	}

	OutDefaultObject = CDO;

	if (const UStruct* SuperStruct = CDO->GetClass()->GetSuperStruct())
	{
		const FName ClassIdentifier = SuperStruct->GetFName();
		if (RegisteredHandlers.Contains(ClassIdentifier))
		{
			return RegisteredHandlers.FindChecked(ClassIdentifier);
		}
	}
	
	return nullptr;
}

void USagaEditorSubsystem::HandleAttributeRename(const FName& InPackageName, const FName& InOldPropertyName, const FName& InNewPropertyName)
{
	if (bIsHandlingRename)
	{
		SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::HandleAttributeRename - Handling rename already. PackageName: %s, OldName: %s, NewName: %s"), *InPackageName.ToString(), *InOldPropertyName.ToString(), *InNewPropertyName.ToString())
		return;
	}
	
	PendingMessages.Reset();

	// Keep a copy of old name to display with the notif and message log
	const FName OldPropertyName = InOldPropertyName;
	
	FScopedSlowTask Progress(3.f, LOCTEXT("SlowTask_UpdateReferencers", "Rename attribute - Gather referencers"));
	Progress.MakeDialog();

	// First figure out the referencers for this package
	TArray<FAssetData> ReferencerAssets;
	TArray<FAssetDependency> Referencers;
	GetReferencers(InPackageName, Referencers, ReferencerAssets);

	// Handle Gameplay Effect Referencers ...
	Progress.EnterProgressFrame(1.f, FText::Format(LOCTEXT("SlowTask_UpdateReferencers", "Rename attribute - Update {0} referencers"), FText::AsNumber(Referencers.Num())));
	
	// Now, update any referencers with FGameplayAttribute properties to point to the new one, from the previously renamed prop
	UpdateReferencers(ReferencerAssets, InPackageName, InOldPropertyName, InNewPropertyName);

	Progress.EnterProgressFrame(1.f, LOCTEXT("SlowTask_UpdateK2Referencers", "Rename attribute - Update K2 Nodes referencers"));
	
	// Handle K2Node Referencers ...
	UpdateK2NodeReferencers(InPackageName, InOldPropertyName, InNewPropertyName);
	
	Progress.EnterProgressFrame(1.f);

	if (!PendingMessages.IsEmpty())
	{
		FMessageLog MessageLog(LogName);
		const FText PageText = FText::Format(LOCTEXT("PageInfo", "Renamed attribute from {0} to {1}"), FText::FromName(OldPropertyName), FText::FromName(InNewPropertyName));
		MessageLog.NewPage(FText::Format(LOCTEXT("NewPage", "{0} ({1})"), PageText, FText::AsDateTime(FDateTime::Now())));

		const FText NotifyText = FText::Format(LOCTEXT("NotifyText", "Blueprint Attributes: {0} ({1} updates)"), PageText, FText::AsNumber(PendingMessages.Num()));
		MessageLog.Info(NotifyText);
		MessageLog.AddMessages(PendingMessages);

		MessageLog.Notify(NotifyText, EMessageSeverity::Info, true);
	}
}

void USagaEditorSubsystem::HandlePreCompile(const FName& InPackageName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::HandlePreCompile InPackageName: %s"), *InPackageName.ToString())
	
	// First figure out the referencers for this package
	TArray<FAssetData> ReferencerAssets;
	TArray<FAssetDependency> Referencers;
	GetReferencers(InPackageName, Referencers, ReferencerAssets);

	for (const TPair<FName, TSharedPtr<ISSAttributeReferencerHandler>>& RegisteredHandler : RegisteredHandlers)
	{
		TSharedPtr<ISSAttributeReferencerHandler> Handler = RegisteredHandler.Value;
		if (Handler.IsValid())
		{
			Handler->OnPreCompile(InPackageName.ToString());
		}
	}

	// Then check if we have a registered handler
	for (const FAssetData& Referencer : ReferencerAssets)
	{
		FSSAttributeReferencerPayload Payload;
		Payload.PackageName = InPackageName.ToString();
		TSharedPtr<ISSAttributeReferencerHandler> ReferencerHandler = FindAssetDependencyHandler(Referencer.PackageName, Payload.DefaultObject);
		if (ReferencerHandler.IsValid() && Payload.DefaultObject.IsValid())
		{
			ReferencerHandler->HandlePreCompile(Referencer.PackageName, Payload);
		}
	}

	for (TObjectIterator<UK2Node> NodeIt; NodeIt; ++NodeIt)
	{
		UK2Node* Node = *NodeIt;
		if (!Node)
		{
			continue;
		}

		FString NodeName = Node->GetName();
		const UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(Node);
		if (!Blueprint)
		{
			continue;
		}

		// If we have an handler for this K2 Node, delegate any custom handling of an attribute rename (for instance, custom K2 Node switch with array property of FGameplayAttribute)
		const UClass* NodeClass = Node->GetClass();
		if (NodeClass && RegisteredHandlers.Contains(NodeClass->GetFName()))
		{
			TSharedPtr<ISSAttributeReferencerHandler> Handler = RegisteredHandlers.FindChecked(NodeClass->GetFName());
			if (Handler.IsValid() && Blueprint->GetPackage() != GetTransientPackage())
			{
				FSSAttributeReferencerPayload Payload;
				Payload.DefaultObject = Node;
				Payload.PackageName = InPackageName.ToString();
				
				FAssetIdentifier AssetIdentifier(Node, NAME_None);
				Handler->HandlePreCompile(AssetIdentifier, Payload);
			}
		}
	}
}

void USagaEditorSubsystem::GetReferencers(const FName& InPackageName, TArray<FAssetDependency>& OutReferencers, TArray<FAssetData>& OutAssetsData)
{
	const TArray<UClass*> AllowedClasses;
	GetReferencers(InPackageName, OutReferencers, OutAssetsData, AllowedClasses);
}

void USagaEditorSubsystem::GetReferencers(const FName& InPackageName, TArray<FAssetDependency>& OutReferencers, TArray<FAssetData>& OutAssetsData, const TArray<UClass*>& InAllowedClasses)
{
	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	
	// First figure out the referencers for this package
	const FAssetIdentifier Identifier = FAssetIdentifier(InPackageName);
	
	TArray<FAssetDependency> Referencers;
	AssetRegistry.GetReferencers(Identifier, Referencers);

	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::GetReferencers Referencers: %d"), Referencers.Num())

	// Gather the list of FAssetData based on referencers
	for (const FAssetDependency& Referencer : Referencers)
	{
		// Fill in the asset data representing the asset editors to close later on
		const FName ReferencerName = Referencer.AssetId.PackageName;
		SS_EDITOR_LOG(Verbose, TEXT("\t Referencer: %s"), *ReferencerName.ToString())

		// No filter class, return everything
		if (InAllowedClasses.IsEmpty())
		{
			OutReferencers.AddUnique(Referencer);
			AssetRegistry.GetAssetsByPackageName(ReferencerName, OutAssetsData);
			continue;
		}

		const FString LoadPath = GetLoadClassPackagePath(ReferencerName.ToString());
		const UClass* LoadedClass = StaticLoadClass(UObject::StaticClass(), nullptr, *LoadPath);

		if (!LoadedClass)
		{
			return;
		}

		const bool bIsAllowed = InAllowedClasses.ContainsByPredicate([LoadedClass](const UClass* AllowedClass)
		{
			return LoadedClass->IsChildOf(AllowedClass);
		});

		if (bIsAllowed)
		{
			OutReferencers.AddUnique(Referencer);
			AssetRegistry.GetAssetsByPackageName(ReferencerName, OutAssetsData);
		}
	}
}

TArray<FAssetDependency> USagaEditorSubsystem::GetReferencersOfClass(const TArray<FAssetDependency>& InReferencers, const UClass* InFilterClass)
{
	return InReferencers.FilterByPredicate([InFilterClass](const FAssetDependency& Referencer)
	{
		const FName ReferencerName = Referencer.AssetId.PackageName;
		const FString LoadPath = GetLoadClassPackagePath(ReferencerName.ToString());
		const UClass* LoadedClass = StaticLoadClass(UObject::StaticClass(), nullptr, *LoadPath);
		return LoadedClass && LoadedClass->IsChildOf(InFilterClass);
	});
}

void USagaEditorSubsystem::GetGameplayEffectReferencers(const FName& InPackageName, TArray<FAssetDependency>& OutReferencers, TArray<FAssetData>& OutAssetsData)
{
	GetReferencers(InPackageName, OutReferencers, OutAssetsData, { UGameplayEffect::StaticClass() });
}

UGameplayEffect* USagaEditorSubsystem::GetGameplayEffectCDO(const FString& InPackageName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::GetGameplayEffectCDO - Try load with %s"), *InPackageName)

	// Try load class and access to CDO
	const UClass* LoadClass = StaticLoadClass(UObject::StaticClass(), nullptr, *InPackageName);
	SS_EDITOR_LOG(Verbose, TEXT("\t LoadClass: %s"), *GetNameSafe(LoadClass))
	if (!LoadClass)
	{
		return nullptr;
	}

	const UObject* CDO = LoadClass->GetDefaultObject(false);
	SS_EDITOR_LOG(Verbose, TEXT("\t LoadClass CDO: %s (Class: %s)"), *GetNameSafe(CDO), *GetNameSafe(CDO->GetClass()))
	if (UGameplayEffect* EffectCDO = Cast<UGameplayEffect>(LoadClass->GetDefaultObject(false)))
	{
		return EffectCDO;
	}

	return nullptr;
}

UGameplayEffect* USagaEditorSubsystem::GetGameplayEffectCDO(const FAssetDependency& InAssetDependency)
{
	// Figure out asset name and correct loading path
	const FString PackageName = GetLoadClassPackagePath(InAssetDependency.AssetId.ToString());
	SS_EDITOR_LOG(VeryVerbose, TEXT("\t Try load with %s"), *PackageName)
	return GetGameplayEffectCDO(PackageName);
}

FString USagaEditorSubsystem::GetLoadClassPackagePath(const FString& InPackageName)
{
	FString Result;

	FString Left;
	FString Right;
	if (!InPackageName.Split(TEXT("/"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		SS_EDITOR_LOG(Warning, TEXT("USagaEditorSubsystem::GetGameplayEffectCDO Unable to split %s by /"), *InPackageName)
		return Result;
	}

	if (Right.IsEmpty())
	{
		SS_EDITOR_LOG(Warning, TEXT("USagaEditorSubsystem::GetGameplayEffectCDO Empty right"), *InPackageName)
		return Result;
	}

	return FString::Printf(TEXT("%s.%s_C"), *InPackageName, *Right);
}

void USagaEditorSubsystem::UpdateK2NodeReferencers(const FName& InPackageName, const FName& InOldPropertyName, const FName& InNewPropertyName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::UpdateK2NodeReferencers InPackageName: %s, InOldPropertyName: %s, InNewPropertyName: %s"), *InPackageName.ToString(), *InOldPropertyName.ToString(), *InNewPropertyName.ToString())
	
	UObject* OwnerAttributeSet = LoadObject<UObject>(nullptr, *InPackageName.ToString());
	if (!OwnerAttributeSet)
	{
		SS_EDITOR_LOG(Warning, TEXT("USagaEditorSubsystem::UpdateK2NodeReferencers - Cannot load AttributeSet class from %s"), *InPackageName.ToString())
		return;
	}
	
	const UBlueprint* OwnerAttributeSetBP = Cast<UBlueprint>(OwnerAttributeSet);
	if (!OwnerAttributeSetBP)
	{
		SS_EDITOR_LOG(Warning, TEXT("USagaEditorSubsystem::UpdateK2NodeReferencers - Cannot load AttributeSet class Blueprint from %s"), *InPackageName.ToString())
		return;
	}

	// Gather the list of Blueprints with K2Node using a Pin Attribute parameter
	const TArray<FPinToModify> PinsToModify = GetPinsToModify(InPackageName.ToString(), InOldPropertyName.ToString(), InNewPropertyName.ToString());
	
	// Actually update the list of pins to point to the new attribute property
	if (!PinsToModify.IsEmpty())
	{
		HandlePins(PinsToModify, OwnerAttributeSetBP, InNewPropertyName);
	}
}

TArray<USagaEditorSubsystem::FPinToModify> USagaEditorSubsystem::GetPinsToModify(const FString& InPackageName, const FString& InOldPropertyName, const FString& InNewPropertyName)
{
	TArray<FPinToModify> PinsToModify;

	for (TObjectIterator<UK2Node> NodeIt; NodeIt; ++NodeIt)
	{
		UK2Node* Node = *NodeIt;
		if (!Node)
		{
			continue;
		}

		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(Node);
		if (!Blueprint)
		{
			continue;
		}

		if (Blueprint->GetPackage() == GetTransientPackage())
		{
			continue;
		}

		// If we have an handler for this K2 Node, delegate any custom handling of an attribute rename (for instance, custom K2 Node switch with array property of FGameplayAttribute)
		UClass* NodeClass = Node->GetClass();
		if (NodeClass && RegisteredHandlers.Contains(NodeClass->GetFName()))
		{
			TSharedPtr<ISSAttributeReferencerHandler> Handler = RegisteredHandlers.FindChecked(NodeClass->GetFName());
			if (Handler.IsValid())
			{
				FSSAttributeReferencerPayload Payload;
				Payload.DefaultObject = Node;
				Payload.PackageName = InPackageName;
				Payload.OldPropertyName = InOldPropertyName;
				Payload.NewPropertyName = InNewPropertyName;
				
				TArray<TSharedRef<FTokenizedMessage>> Messages;
				FAssetIdentifier AssetIdentifier(Node, NAME_None);
				Handler->HandleAttributeRename(AssetIdentifier, Payload, Messages);
				PendingMessages.Append(Messages);
			}
		}
		
		for (UEdGraphPin* Pin : Node->Pins)
		{
			FEdGraphPinType PinType = Pin->PinType;
			const bool bIsAttributePin = PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && PinType.PinSubCategoryObject == FGameplayAttribute::StaticStruct();
			if (!bIsAttributePin)
			{
				continue;
			}
			
			if (Pin->Direction == EGPD_Input)
			{
				FString DefaultValue = Pin->GetDefaultAsString();
				if (DefaultValue.IsEmpty())
				{
					continue;
				}

				FString PackageName;
				FString AttributeName;
				if (!ParseAttributeFromDefaultValue(DefaultValue, PackageName, AttributeName))
				{
					SS_EDITOR_LOG(
						VeryVerbose,
						TEXT("USagaEditorSubsystem::UpdateK2NodeReferencers - Cannot parse attribute from DefaultValue: %s (Pin: %s, Node: %s, Graph: %s)"),
						*DefaultValue,
						*Pin->GetName(),
						*GetNameSafe(Node),
						*GetNameSafe(Node->GetGraph())
					)
					continue;
				}

				if (PackageName == InPackageName && AttributeName == InOldPropertyName)
				{
					PinsToModify.AddUnique(FPinToModify(Pin, Node, AttributeName));
				}
			}
		}
	}

	return MoveTemp(PinsToModify);
}

void USagaEditorSubsystem::HandlePins(const TArray<FPinToModify>& InPinsToModify, const UBlueprint* InOwnerAttributeSetBP, const FName& InNewPropertyName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::UpdateK2NodeReferencers - Test InPinsToModify: %d"), InPinsToModify.Num())
	TSet<UBlueprint*> BlueprintsModified;
	for (const FPinToModify& PinToModify : InPinsToModify)
	{
		if (!PinToModify.Pin || !PinToModify.BlueprintNode.IsValid())
		{
			continue;
		}

		if (!InOwnerAttributeSetBP->GeneratedClass)
		{
			SS_EDITOR_LOG(Warning, TEXT("USagaEditorSubsystem::UpdateK2NodeReferencers - NO GENERATED CLASS (InOwnerAttributeSetBP: %s"), *GetNameSafe(InOwnerAttributeSetBP))
			continue;
		}

		UEdGraphPin* Pin = PinToModify.Pin;
		TWeakObjectPtr<UK2Node> BlueprintNode = PinToModify.BlueprintNode;
		UBlueprint* Blueprint = BlueprintNode->GetBlueprint();
		const UEdGraph* OwningGraph = BlueprintNode->GetGraph();
		FString DefaultValue = Pin->GetDefaultAsString();

		//Feature Begin Attribute In subclass of AttributeSet
		FString AttributeOwnerClassName;
		ParseAttributeOwnerClassNameFromDefaultValue(DefaultValue, AttributeOwnerClassName);
		UClass* AttributeOwnerClass = LoadObject<UClass>(NULL, *AttributeOwnerClassName);
		//Feature End

		SS_EDITOR_LOG(Verbose, TEXT("\t USagaEditorSubsystem::UpdateK2NodeReferencers - PinToModify Blueprint: %s"), *GetNameSafe(Blueprint))
		SS_EDITOR_LOG(Verbose, TEXT("\t USagaEditorSubsystem::UpdateK2NodeReferencers - PinToModify Pin: %s"), *Pin->GetName())

		if (FProperty* Prop = FindFProperty<FProperty>(InOwnerAttributeSetBP->GeneratedClass, InNewPropertyName))
		{
			SS_EDITOR_LOG(Verbose, TEXT("\t Prop: %s (Owner: %s)"), *GetNameSafe(Prop), *InOwnerAttributeSetBP->GeneratedClass->GetName())

			FString FinalValue;
			//Feature Begin Attribute In subclass of AttributeSet
			FGameplayAttribute NewAttributeStruct(Prop, AttributeOwnerClass);
			//Feature End

			FGameplayAttribute::StaticStruct()->ExportText(FinalValue, &NewAttributeStruct, &NewAttributeStruct, nullptr, PPF_SerializedAsImportText, nullptr);

			if (FinalValue != DefaultValue && Pin->GetSchema())
			{
				// const FScopedTransaction Transaction(LOCTEXT("ChangePinValueTransaction", "Change Pin Value"));
				Pin->Modify();
				Pin->GetSchema()->TrySetDefaultValue(*Pin, FinalValue);

				TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
				Message->AddToken(FTextToken::Create(LOCTEXT("ChangedPinProperty", "K2 Node Pin: Changed property value of ")));
				if (Blueprint)
				{
					Message->AddToken(
						FUObjectToken::Create(Blueprint, FText::FromString(Blueprint->GetName()))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&HandleMessageLogLinkActivated))
					);
				}

				if (OwningGraph)
				{
					Message->AddToken(FTextToken::Create(LOCTEXT("Separator", " > ")));
					Message->AddToken(
						FUObjectToken::Create(OwningGraph, FText::FromString(OwningGraph->GetName()))
						->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&HandleMessageLogLinkActivated))
					);
				}
					
				Message->AddToken(FTextToken::Create(LOCTEXT("Separator", " > ")));
				Message->AddToken(
					FUObjectToken::Create(BlueprintNode.Get(), BlueprintNode->GetNodeTitle(ENodeTitleType::ListView))
					->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&HandleMessageLogLinkActivated))
				);

				FText MessageText = FText::Format(
					LOCTEXT("ChangedPinFromTo", "from {0} to {1}"),
					FText::FromString(PinToModify.OldPropertyName),
					FText::FromString(Prop->GetName())
				);
				Message->AddToken(FTextToken::Create(MessageText));

				PendingMessages.Add(Message);
				SS_EDITOR_LOG(Verbose, TEXT("\t %s"), *Message->ToText().ToString())

				BlueprintsModified.Emplace(Blueprint);
			}
		}
	}

	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::UpdateK2NodeReferencers - BlueprintsModified: %d"), BlueprintsModified.Num())
	for (UBlueprint* Blueprint : BlueprintsModified)
	{
		if (!Blueprint)
		{
			continue;
		}

		SS_EDITOR_LOG(Verbose, TEXT("\t MarkBlueprintAsStructurallyModified Blueprint: %s"), *Blueprint->GetName())
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

		FString PathName = Blueprint->GetPathName();
		SS_EDITOR_LOG(Verbose, TEXT("\t PathName: %s"), *PathName)

		// Handle case of blueprint modified being the same as original blueprint that was compiled
		// and avoid user having to compile twice to un-dirty the BP, eg. Hit Compile once triggers the whole rename and update K2 node code path,
		// if pins were matching in this BP, pin are changed and BP marked as modified, needs compilation / save again
		//
		// For other assets, make sense to let the users decide whether it wants to save modified assets, but for the one it just compiled,
		// we'd like to avoid this confusion.
		if (Blueprint == InOwnerAttributeSetBP)
		{
			SS_EDITOR_LOG(Verbose, TEXT("\t Blueprint: %s is the same. Needs compile again."), *Blueprint->GetName())
			bIsHandlingRename = true;
			FKismetEditorUtilities::CompileBlueprint(Blueprint);
			bIsHandlingRename = false;

			if (Blueprint->Status == BS_UpToDate)
			{
				SS_EDITOR_LOG(Display, TEXT("Blueprint compiled successfully (%s)"), *Blueprint->GetName());
			}
			else if (Blueprint->Status == BS_UpToDateWithWarnings)
			{
				SS_EDITOR_LOG(Warning, TEXT("Blueprint compiled successfully with warnings(%s)"), *Blueprint->GetName());
			}
			else if (Blueprint->Status == BS_Error)
			{
				SS_EDITOR_LOG(Error, TEXT("Blueprint failed to compile (%s)"), *Blueprint->GetName());
			}
			else
			{
				SS_EDITOR_LOG(Error, TEXT("Blueprint is in an unexpected state after compiling (%s)"), *Blueprint->GetName());
			}
		}
	}
}

void USagaEditorSubsystem::HandleMessageLogLinkActivated(const TSharedRef<IMessageToken>& InToken)
{
	if (InToken->GetType() == EMessageToken::Object)
	{
		const TSharedRef<FUObjectToken> UObjectToken = StaticCastSharedRef<FUObjectToken>(InToken);
		if (UObjectToken->GetObject().IsValid())
		{
			FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(UObjectToken->GetObject().Get());
		}
	}
}

bool USagaEditorSubsystem::ParseAttributeFromDefaultValue(const FString& InDefaultValue, FString& OutPackageName, FString& OutAttributeName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::ParsePackageNameFromDefaultValue InDefaultValue: %s"), *InDefaultValue)
	FString DefaultValue = InDefaultValue;

	DefaultValue.RemoveFromStart(TEXT("("));
	DefaultValue.RemoveFromEnd(TEXT(")"));

	TArray<FString> Segments;
	DefaultValue.ParseIntoArray(Segments, TEXT(","));

	if (Segments.IsEmpty())
	{
		return false;
	}

	for (const FString& Segment : Segments)
	{
		FString Key;
		FString Value;
		Segment.Split(TEXT("="), &Key, &Value);

		if (Key != TEXT("Attribute") || Value.IsEmpty())
		{
			continue;
		}

		FString LongPackageName;
		FString AttributeName;
		Value.Split(TEXT(":"), &LongPackageName, &AttributeName);

		FString Right;
		FString PackageName;
		LongPackageName.Split(TEXT("."), &PackageName, &Right);

		if (!PackageName.IsEmpty() && !AttributeName.IsEmpty())
		{
			OutPackageName = PackageName;
			OutAttributeName = AttributeName;
			return true;
		}
	}

	return false;
}

//Feature Begin Attribute In subclass of AttributeSet
bool USagaEditorSubsystem::ParseAttributeOwnerClassNameFromDefaultValue(const FString& InDefaultValue, FString& OutAttributeOwnerClassName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::ParseAttributeOwnerClassNameFromDefaultValue InDefaultValue: %s"), *InDefaultValue)
	FString DefaultValue = InDefaultValue;

	DefaultValue.RemoveFromStart(TEXT("("));
	DefaultValue.RemoveFromEnd(TEXT(")"));

	TArray<FString> Segments;
	DefaultValue.ParseIntoArray(Segments, TEXT(","));

	if (Segments.IsEmpty())
	{
		return false;
	}

	for (const FString& Segment : Segments)
	{
		FString Key;
		FString Value;
		Segment.Split(TEXT("="), &Key, &Value);

		if (Key != TEXT("AttributeOwnerClass") || Value.IsEmpty())
		{
			continue;
		}
		OutAttributeOwnerClassName = Value;
		return true;
	}	

	return false;
}
//Feature End


void USagaEditorSubsystem::UpdateReferencers(TArray<FAssetData> InReferencers, const FName& InPackageName, const FName& InOldPropertyName, const FName& InNewPropertyName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::UpdateReferencers Referencer Assets: %d"), InReferencers.Num())

	FScopedSlowTask Progress(InReferencers.Num(), FText::Format(LOCTEXT("SlowTask_UpdateReferencers", "Rename attribute - Update {0} referencers"), FText::AsNumber(InReferencers.Num())));
	Progress.MakeDialog();

	for (const FAssetData& Referencer : InReferencers)
	{
		Progress.EnterProgressFrame(1.f);

		FSSAttributeReferencerPayload Payload;
		Payload.PackageName = InPackageName.ToString();
		Payload.OldPropertyName = InOldPropertyName.ToString();
		Payload.NewPropertyName = InNewPropertyName.ToString();
		Payload.ReferencerBlueprint = Cast<UBlueprint>(Referencer.GetAsset());
		TSharedPtr<ISSAttributeReferencerHandler> Handler = FindAssetDependencyHandler(Referencer.PackageName, Payload.DefaultObject);

		if (Handler.IsValid() && Payload.DefaultObject.IsValid())
		{
			TArray<TSharedRef<FTokenizedMessage>> Messages;
			const bool bHandledRename = Handler->HandleAttributeRename(Referencer.PackageName, Payload, Messages);
			PendingMessages.Append(Messages);
			
			if (bHandledRename)
			{
				Payload.DefaultObject->Modify();
				Payload.DefaultObject->PostEditChange();
				Payload.DefaultObject->MarkPackageDirty();

				FSagaStatsDelegates::OnRequestDetailsRefresh.Broadcast();
			}
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void USagaEditorSubsystem::HandlePostCompile(const FName& InPackageName)
{
	SS_EDITOR_LOG(Verbose, TEXT("USagaEditorSubsystem::HandlePostCompile InPackageName: %s"), *InPackageName.ToString())

	for (const TPair<FName, TSharedPtr<ISSAttributeReferencerHandler>>& RegisteredHandler : RegisteredHandlers)
	{
		TSharedPtr<ISSAttributeReferencerHandler> Handler = RegisteredHandler.Value;
		if (Handler.IsValid())
		{
			Handler->OnPostCompile(InPackageName.ToString());
		}
	}

	// First figure out the referencers for this package
	TArray<FAssetData> ReferencerAssets;
	TArray<FAssetDependency> Referencers;
	GetGameplayEffectReferencers(InPackageName, Referencers, ReferencerAssets);

	// Second, close the editors. It fixes a crash when the Slate details customizations for Gameplay Effects and Attributes tries
	// to access a now invalid property
	TArray<FAssetData> ClosedAssets;
	CloseEditors(ReferencerAssets, ClosedAssets);
	
	// Next, build up the list of package names that is going to be passed down to re-open closed assets on next frame
	TArray<FString> ClosedPackageNames;
	Algo::Transform(ClosedAssets, ClosedPackageNames, [](const FAssetData& AssetData)
	{
		return AssetData.PackageName.ToString();
	});
	
	// Delay for one frame so that editor can finish shut down.
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateStatic(&USagaEditorSubsystem::HandleNextTick, InPackageName, ClosedPackageNames));
	}
	
	OpenClosedEditors(ClosedPackageNames);
}

void USagaEditorSubsystem::CloseEditors(const TArray<FAssetData>& InAssets, TArray<FAssetData>& OutClosedAssets)
{
	for (const FAssetData& AssetData : InAssets)
	{
		const UObject* Asset = AssetData.FastGetAsset();
		SS_EDITOR_LOG(Verbose, TEXT("\tClosing asset: %s (UObject: %s)"), *AssetData.GetFullName(), *GetNameSafe(Asset));

		if (!Asset)
		{
			continue;
		}

		const TSharedPtr<IToolkit> AssetEditor = FToolkitManager::Get().FindEditorForAsset(Asset);
		if (AssetEditor.IsValid() && IsAssetCurrentlyBeingEdited(AssetEditor, Asset))
		{
			FToolkitManager::Get().CloseToolkit(AssetEditor.ToSharedRef());

			// Store assets to delete and reopen
			OutClosedAssets.Add(AssetData);
		}
	}
}

void USagaEditorSubsystem::OpenClosedEditors(const TArray<FString>& InClosedAssets)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		SS_EDITOR_LOG(Error, TEXT("USagaEditorSubsystem::OpenClosedEditors failed because AssetEditorSubsystem is null"));
		return;
	}
	
	TArray<UObject*> ObjectsToReopen;
	for (const FString& ClosedAsset : InClosedAssets)
	{
		if (UObject* Object = StaticLoadObject(UObject::StaticClass(), nullptr, *ClosedAsset))
		{
			ObjectsToReopen.Add(Object);
		}
	}

	for (UObject* Asset : ObjectsToReopen)
	{
		AssetEditorSubsystem->OpenEditorForAsset(Asset);
	}
}

bool USagaEditorSubsystem::IsAssetCurrentlyBeingEdited(const TSharedPtr<IToolkit>& InAssetEditor, const UObject* InAsset)
{
	if (!InAsset)
	{
		return false;
	}

	if (InAssetEditor.IsValid() && InAssetEditor->IsAssetEditor())
	{
		if (const TArray<UObject*>* EditedObjects = InAssetEditor->GetObjectsCurrentlyBeingEdited())
		{
			for (const UObject* EditedObject : *EditedObjects)
			{
				if (EditedObject == InAsset)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void USagaEditorSubsystem::HandleNextTick(FName InPackageName, const TArray<FString> InClosedAssets)
{
	OpenClosedEditors(InClosedAssets);
	
	if (const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InPackageName.ToString()))
	{
		const TSharedPtr<IToolkit> AssetEditor = FToolkitManager::Get().FindEditorForAsset(Blueprint);
		if (const TSharedPtr<FAssetEditorToolkit> EditorToolkit = StaticCastSharedPtr<FAssetEditorToolkit>(AssetEditor))
		{
			EditorToolkit->FocusWindow();
		}
	}
}

#undef LOCTEXT_NAMESPACE
