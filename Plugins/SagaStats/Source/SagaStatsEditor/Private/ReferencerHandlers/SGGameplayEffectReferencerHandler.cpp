/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "SGGameplayEffectReferencerHandler.h"

#include "SGEditorLog.h"
#include "GameplayEffect.h"
#include "Engine/Blueprint.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/UObjectToken.h"
#include "SGEditorSubsystem.h"

#if UE_VERSION_NEWER_THAN(5, 3, -1)
#include "GameplayEffectComponents/ImmunityGameplayEffectComponent.h"
#include "GameplayEffectComponents/RemoveOtherGameplayEffectComponent.h"
#endif

#define LOCTEXT_NAMESPACE "SGGameplayEffectReferencerHandler"

TSharedPtr<ISGAttributeReferencerHandler> FSGGameplayEffectReferencerHandler::Create()
{
	return MakeShared<FSGGameplayEffectReferencerHandler>();
}

FSGGameplayEffectReferencerHandler::~FSGGameplayEffectReferencerHandler()
{
	AttributesCacheMap.Reset();
}

void FSGGameplayEffectReferencerHandler::OnPreCompile(const FString& InPackageName)
{
	AttributesCacheMap.Reset();
}

void FSGGameplayEffectReferencerHandler::OnPostCompile(const FString& InPackageName)
{
}

bool FSGGameplayEffectReferencerHandler::HandlePreCompile(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload)
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())
	
	UGameplayEffect* EffectCDO = Cast<UGameplayEffect>(InPayload.DefaultObject);
	if (!EffectCDO)
	{
		return false;
	}
	
	TArray<FAttributeReference> AttributesCache;
	AttributesCache.Reserve(EffectCDO->Modifiers.Num());

	// Duration
	FAttributeReference DurationAttributeReference;
	if (BuildModifierMagnitudeAttributeReference(EffectCDO->DurationMagnitude, DurationAttributeReference))
	{
		DurationAttributeReference.Index = 0;
		DurationAttributeReference.Type = EAttributeReferenceType::DurationMagnitude;
		AttributesCache.Add(DurationAttributeReference);
	}

	// Modifiers
	{
		int32 CurrentIndex = 0;
		for (const FGameplayModifierInfo& Modifier : EffectCDO->Modifiers)
		{
			CurrentIndex++;

			// Modifier attribute
			{
				FAttributeReference AttributeReference;
				if (BuildModifierInfoAttributeReference(Modifier, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::ModifierInfo_Attribute;
					AttributesCache.Add(AttributeReference);
				}
			}

			// Modifier attribute based magnitude
			{
				FAttributeReference AttributeReference;
				if (BuildModifierMagnitudeAttributeReference(Modifier.ModifierMagnitude, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::ModifierInfo_BackingAttribute_AttributeToCapture;
					AttributesCache.Add(AttributeReference);
				}
			}
		}
	}
	
	// Gameplay Cues > Magnitude Attribute
	{
		int32 CurrentIndex = 0;
		for (const FGameplayEffectCue& GameplayCue : EffectCDO->GameplayCues)
		{
			CurrentIndex++;

			FAttributeReference AttributeReference;
			if (BuildEffectCueMagnitudeAttributeReference(GameplayCue, AttributeReference))
			{
				AttributeReference.Index = CurrentIndex - 1;
				AttributeReference.Type = EAttributeReferenceType::GameplayCue_MagnitudeAttribute;
				AttributesCache.Add(AttributeReference);
			}
		}
	}

	// Tags > RemoveGameplayEffectQuery
	{

		// TODO: Check everything working fine in 5.3 and GEComponents
		
#if UE_VERSION_NEWER_THAN(5, 3, -1)
		if (const URemoveOtherGameplayEffectComponent* GameplayEffectComponent = EffectCDO->FindComponent<URemoveOtherGameplayEffectComponent>())
		{
			int32 CurrentIndex = 0;
			for (const FGameplayEffectQuery& Query : GameplayEffectComponent->RemoveGameplayEffectQueries)
			{
				CurrentIndex++;
				
				FAttributeReference AttributeReference;
				if (BuildEffectQueryAttributeReference(Query, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute;
					AttributesCache.Add(AttributeReference);
				}
			}
		}
#else
		FAttributeReference AttributeReference;
		if (BuildEffectQueryAttributeReference(EffectCDO->RemoveGameplayEffectQuery, AttributeReference))
		{
			AttributeReference.Index = 0;
			AttributeReference.Type = EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute;
			AttributesCache.Add(AttributeReference);
		}
#endif
	}

	// Immunity > GrantedApplicationImmunityQuery
	{
		// TODO: Check everything working fine in 5.3 and GEComponents
		
#if UE_VERSION_NEWER_THAN(5, 3, -1)
		if (const UImmunityGameplayEffectComponent* GameplayEffectComponent = EffectCDO->FindComponent<UImmunityGameplayEffectComponent>())
		{
			int32 CurrentIndex = 0;
			for (const FGameplayEffectQuery& Query : GameplayEffectComponent->ImmunityQueries)
			{
				CurrentIndex++;
				
				FAttributeReference AttributeReference;
				if (BuildEffectQueryAttributeReference(Query, AttributeReference))
				{
					AttributeReference.Index = CurrentIndex - 1;
					AttributeReference.Type = EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute;
					AttributesCache.Add(AttributeReference);
				}
			}
		}
#else
		FAttributeReference AttributeReference;
		if (BuildEffectQueryAttributeReference(EffectCDO->GrantedApplicationImmunityQuery, AttributeReference))
		{
			AttributeReference.Index = 0;
			AttributeReference.Type = EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute;
			AttributesCache.Add(AttributeReference);
		}
#endif
	}

	AttributesCacheMap.Add(InAssetIdentifier, AttributesCache);
	return true;
}

bool FSGGameplayEffectReferencerHandler::HandleAttributeRename(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())

	UGameplayEffect* EffectCDO = Cast<UGameplayEffect>(InPayload.DefaultObject);
	if (!EffectCDO)
	{
		return false;
	}

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InPayload.PackageName);
	if (!Blueprint)
	{
		SG_EDITOR_NS_LOG(Warning, TEXT("Failed to update modifiers because of invalid Blueprint for %s"), *InPayload.PackageName)
		return false;
	}

	bool bModified = UpdateModifiers(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateModifiersBackingAttribute(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateDuration(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateCues(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateRemoveGameplayEffectQuery(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateImmunityEffectQuery(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	
	return bModified;
}


bool FSGGameplayEffectReferencerHandler::HandleAttributeRemoved(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())

	UGameplayEffect* EffectCDO = Cast<UGameplayEffect>(InPayload.DefaultObject);
	if (!EffectCDO)
	{
		return false;
	}

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InPayload.PackageName);
	if (!Blueprint)
	{
		SG_EDITOR_NS_LOG(Warning, TEXT("Failed to update modifiers because of invalid Blueprint for %s"), *InPayload.PackageName)
		return false;
	}

	bool bModified = UpdateModifiers(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateModifiersBackingAttribute(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateDuration(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateCues(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateRemoveGameplayEffectQuery(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	bModified |= UpdateImmunityEffectQuery(EffectCDO, Blueprint, InAssetIdentifier, InPayload, OutMessages);
	
	return bModified;
}

bool FSGGameplayEffectReferencerHandler::BuildModifierInfoAttributeReference(const FGameplayModifierInfo& InModifier, FAttributeReference& OutAttributeReference)
{
	if (!InModifier.Attribute.IsValid())
	{
		SG_EDITOR_NS_LOG(Verbose, TEXT("Invalid modifier"))
		return false;
	}
	
	const FGameplayAttribute& Attribute = InModifier.Attribute;
	const FString AttributeName = Attribute.GetName();
	const FProperty* Property = Attribute.GetUProperty();
	const FString PropertyPathName = Property ? Property->GetPathName() : TEXT("");

	FAttributeReference Reference;
	const FString FinalValue = FString::Printf(TEXT("(AttributeName=\"%s\",Attribute=%s)"), *AttributeName, *PropertyPathName);
	if (!USGEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
	{
		return false;
	}

	OutAttributeReference = MoveTemp(Reference);
	return true;
}

bool FSGGameplayEffectReferencerHandler::BuildModifierMagnitudeAttributeReference(const FGameplayEffectModifierMagnitude& InMagnitude, FAttributeReference& OutAttributeReference)
{
	const EGameplayEffectMagnitudeCalculation CalculationType = InMagnitude.GetMagnitudeCalculationType();
	
	TArray<FGameplayEffectAttributeCaptureDefinition> Definitions;
	InMagnitude.GetAttributeCaptureDefinitions(Definitions);

	TArray<FAttributeReference> AttributeReferences;
	AttributeReferences.Reserve(Definitions.Num());
	
	if (CalculationType == EGameplayEffectMagnitudeCalculation::AttributeBased && Definitions.Num() == 1 && Definitions.IsValidIndex(0))
	{
		const FGameplayEffectAttributeCaptureDefinition CaptureDefinition = Definitions[0];

		const FGameplayAttribute& AttributeToCapture = CaptureDefinition.AttributeToCapture;
		
		const FString AttributeName = AttributeToCapture.GetName();
		const FProperty* Property = AttributeToCapture.GetUProperty();
		const FString PropertyPathName = Property ? Property->GetPathName() : TEXT("");


		FAttributeReference Reference;
		const FString FinalValue = FString::Printf(TEXT("(AttributeName=\"%s\",Attribute=%s)"), *AttributeName, *PropertyPathName);
		if (!USGEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
		{
			return false;
		}

		OutAttributeReference = MoveTemp(Reference);
		return true;
	}
	
	return false;
}

bool FSGGameplayEffectReferencerHandler::BuildEffectCueMagnitudeAttributeReference(const FGameplayEffectCue& InEffectCue, FAttributeReference& OutAttributeReference)
{
	const FGameplayAttribute& MagnitudeAttribute = InEffectCue.MagnitudeAttribute;
	const FString AttributeName = MagnitudeAttribute.GetName();
	const FProperty* Property = MagnitudeAttribute.GetUProperty();
	const FString PropertyPathName = Property ? Property->GetPathName() : TEXT("");


	FAttributeReference Reference;
	const FString FinalValue = FString::Printf(TEXT("(AttributeName=\"%s\",Attribute=%s)"), *AttributeName, *PropertyPathName);
	if (!USGEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
	{
		return false;
	}

	OutAttributeReference = MoveTemp(Reference);
	return true;
}

bool FSGGameplayEffectReferencerHandler::BuildEffectQueryAttributeReference(const FGameplayEffectQuery& InEffectQuery, FAttributeReference& OutAttributeReference)
{
	const FGameplayAttribute& ModifyingAttribute = InEffectQuery.ModifyingAttribute;
	const FString AttributeName = ModifyingAttribute.GetName();
	const FProperty* Property = ModifyingAttribute.GetUProperty();
	const FString PropertyPathName = Property ? Property->GetPathName() : TEXT("");


	FAttributeReference Reference;
	const FString FinalValue = FString::Printf(TEXT("(AttributeName=\"%s\",Attribute=%s)"), *AttributeName, *PropertyPathName);
	if (!USGEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
	{
		return false;
	}

	OutAttributeReference = MoveTemp(Reference);
	return true;
}

bool FSGGameplayEffectReferencerHandler::UpdateModifiers(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(Display, TEXT("InEffectCDO: %s, InBlueprint: %s, InOldPropertyName: %s, InNewPropertyName: %s"),
		*GetNameSafe(InEffectCDO),
		*GetNameSafe(InBlueprint),
		*InPayload.OldPropertyName,
		*InPayload.NewPropertyName
	)

	check(InEffectCDO);
	check(InBlueprint);

	SG_EDITOR_LOG(VeryVerbose, TEXT("USGEditorSubsystem::UpdateGameplayEffectModifiers Blueprint: %s"), *GetNameSafe(InBlueprint))
	int32 CurrentIndex = 0;

	// List of modifiers pending a rename
	TArray<FAttributeModifierToReplace> ModifiersToReplace;

	// List of modifiers pending a reset - e.g. set to back to None and not the attribute that was removed in GBA Blueprint
	TArray<FAttributeModifierToReplace> ModifiersToReset;
	
	for (const FGameplayModifierInfo& Modifier : InEffectCDO->Modifiers)
	{
		FAttributeReference CachedModifier;
		const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedModifier, [CurrentIndex](const FAttributeReference& Item)
		{
			return Item.Type == EAttributeReferenceType::ModifierInfo_Attribute && Item.Index == CurrentIndex;
		});

		if (!bHasCachedAttribute)
		{
			CurrentIndex++;
			continue;
		}
		
		// THIS LINE
		//
		// if (!Modifier.Attribute.IsValid() && CachedModifier.AttributeName == InPayload.OldPropertyName)
		//
		// On 5.4, Modifier.Attribute used to return invalid as expected
		// On 5.5, the `TFieldPath<FProperty> Attribute` is considered valid, which is likely why the rename doesn't happen
		// and why the bits in `FGameplayAttribute::PostSerialize` to mark the attribute as searchable is executed (and can result
		// in a crash when for instance compiling / saving a GE, when OwnerVariant is accessed)

		// Handle rename
		if (CachedModifier.AttributeName == InPayload.OldPropertyName)
		{
			if (!InBlueprint->GeneratedClass)
			{
				CurrentIndex++;
				continue;
			}

			if (FProperty* Prop = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
			{
				ModifiersToReplace.Add(FAttributeModifierToReplace(CurrentIndex, Prop,CachedModifier.AttributeOwnerClass));
			}
		}
		// Handle removed
		else if (CachedModifier.AttributeName == InPayload.RemovedPropertyName)
		{
			ModifiersToReset.Add(FAttributeModifierToReplace(CurrentIndex, nullptr, nullptr));
		}

		CurrentIndex++;
	}

	// Now that we have the list of modifiers that needs an update, update modifiers
	SG_EDITOR_LOG(Verbose, TEXT("USGEditorSubsystem::UpdateGameplayEffectModifiers Update CDO modifiers now from gathered props to replace: %d"), ModifiersToReplace.Num())
	for (const FAttributeModifierToReplace& ModifierToReplace : ModifiersToReplace)
	{
		const int32 Index = ModifierToReplace.Index;

		if (!InEffectCDO->Modifiers.IsValidIndex(Index))
		{
			SG_EDITOR_LOG(Verbose, TEXT("Invalid index %d for CDO modifiers"), Index)
			continue;
		}

		FGameplayModifierInfo& ModifierInfo = InEffectCDO->Modifiers[Index];
		ModifierInfo.Attribute = FGameplayAttribute(ModifierToReplace.Property.Get());

		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);

		Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));
		if (InPayload.ReferencerBlueprint.IsValid())
		{
			Message->AddToken(
				FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USGEditorSubsystem::HandleMessageLogLinkActivated))
			);
		}
		
		Message->AddToken(FTextToken::Create(FText::Format(
			LOCTEXT("ChangedModifierFromTo", "Updated Gameplay Effect > Modifiers at Index {0} from {1} to {2}"),
			FText::AsNumber(Index),
			FText::FromString(InPayload.OldPropertyName),
			FText::FromString(ModifierInfo.Attribute.GetName())
		)));
		
		OutMessages.Add(Message);
	}
	
	SG_EDITOR_LOG(Verbose, TEXT("USGEditorSubsystem::UpdateGameplayEffectModifiers Update CDO modifiers now from gathered props to reset: %d"), ModifiersToReset.Num())
	for (const FAttributeModifierToReplace& ModifierToReset : ModifiersToReset)
	{
		const int32 Index = ModifierToReset.Index;
		if (!InEffectCDO->Modifiers.IsValidIndex(Index))
		{
			SG_EDITOR_LOG(Verbose, TEXT("Invalid index %d for CDO modifiers"), Index)
			continue;
		}
		
		FGameplayModifierInfo& ModifierInfo = InEffectCDO->Modifiers[Index];
		ModifierInfo.Attribute = FGameplayAttribute();

		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);

		Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));
		if (InPayload.ReferencerBlueprint.IsValid())
		{
			Message->AddToken(
				FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USGEditorSubsystem::HandleMessageLogLinkActivated))
			);
		}
		
		Message->AddToken(FTextToken::Create(FText::Format(
			LOCTEXT("ChangedModifierFromTo", "Updated Gameplay Effect > Modifiers at Index {0} from {1} to {2} because {1} was removed"),
			FText::AsNumber(Index),
			FText::FromString(InPayload.RemovedPropertyName),
			FText::FromString(ModifierInfo.Attribute.GetName())
		)));
		
		OutMessages.Add(Message);
	}

	return !ModifiersToReplace.IsEmpty() || !ModifiersToReset.IsEmpty();
}

bool FSGGameplayEffectReferencerHandler::UpdateGameplayEffectModifierMagnitude(FGameplayModifierInfo& InOutModifier, const FAttributeReference& CachedAttribute, const UBlueprint* InBlueprint, const FSGAttributeReferencerPayload& InPayload)
{
	const EGameplayEffectMagnitudeCalculation CalculationType = InOutModifier.ModifierMagnitude.GetMagnitudeCalculationType();

	TArray<FGameplayEffectAttributeCaptureDefinition> Definitions;
	InOutModifier.ModifierMagnitude.GetAttributeCaptureDefinitions(Definitions);

	if (CalculationType != EGameplayEffectMagnitudeCalculation::AttributeBased || !Definitions.IsValidIndex(0))
	{
		return false;
	}

	const bool bIsSameOwner = CachedAttribute.PackageNameOwner == InPayload.PackageName;

	// Start with default attribute (None), handles case of removal
	FGameplayAttribute NewAttribute = FGameplayAttribute();

	bool bShouldHandle = false;

	// Handle renamed
	if (bIsSameOwner && CachedAttribute.AttributeName == InPayload.OldPropertyName)
	{
		if (FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
		{
			SG_EDITOR_NS_LOG(VeryVerbose, TEXT("Replacing modifier magnitude backing attribute with %s"), *GetNameSafe(NewAttributeProp))
			NewAttribute = FGameplayAttribute(NewAttributeProp);
			bShouldHandle = true;
		}
	}
	// Handle removed
	else if (bIsSameOwner && CachedAttribute.AttributeName == InPayload.RemovedPropertyName)
	{
		// ModifiersToReset.Add(FAttributeModifierToReplace(CurrentIndex, nullptr));
		bShouldHandle = true;
	}
	
	bool bWasHandled = false;

	// Now make the replacement, if it was a proper rename or removal
	if (bShouldHandle)
	{
		FAttributeBasedFloat AttributeBasedFloat = FAttributeBasedFloat();
		if (const FAttributeBasedFloat* PrevAttributeBased = GetAttributeBasedFloatFromContainer(&InOutModifier.ModifierMagnitude))
		{
			// This case is to retain any member previously set by user for the backing attribute definition
			AttributeBasedFloat.Coefficient = PrevAttributeBased->Coefficient;
			AttributeBasedFloat.PreMultiplyAdditiveValue = PrevAttributeBased->PreMultiplyAdditiveValue;
			AttributeBasedFloat.PostMultiplyAdditiveValue = PrevAttributeBased->PostMultiplyAdditiveValue;
			AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
				NewAttribute,
				PrevAttributeBased->BackingAttribute.AttributeSource,
				PrevAttributeBased->BackingAttribute.bSnapshot
			);
			AttributeBasedFloat.AttributeCurve = PrevAttributeBased->AttributeCurve;
			AttributeBasedFloat.AttributeCalculationType = PrevAttributeBased->AttributeCalculationType;
			AttributeBasedFloat.FinalChannel = PrevAttributeBased->FinalChannel;
			AttributeBasedFloat.SourceTagFilter = PrevAttributeBased->SourceTagFilter;
			AttributeBasedFloat.TargetTagFilter = PrevAttributeBased->TargetTagFilter;
		}
		else
		{
			AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(NewAttribute, EGameplayEffectAttributeCaptureSource::Source, false);
		}

		InOutModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(AttributeBasedFloat);
		bWasHandled = true;
	}

	return bWasHandled;
}

bool FSGGameplayEffectReferencerHandler::UpdateGameplayEffectModifierMagnitude(FGameplayEffectModifierMagnitude& InOutModifierMagnitude, const FAttributeReference& CachedAttribute, const UBlueprint* InBlueprint, const FSGAttributeReferencerPayload& InPayload)
{
	const EGameplayEffectMagnitudeCalculation CalculationType = InOutModifierMagnitude.GetMagnitudeCalculationType();

	TArray<FGameplayEffectAttributeCaptureDefinition> Definitions;
	InOutModifierMagnitude.GetAttributeCaptureDefinitions(Definitions);

	if (CalculationType != EGameplayEffectMagnitudeCalculation::AttributeBased || !Definitions.IsValidIndex(0))
	{
		return false;
	}

	const bool bIsSameOwner = CachedAttribute.PackageNameOwner == InPayload.PackageName;

	// Start with default attribute (None), handles case of removal
	FGameplayAttribute NewAttribute = FGameplayAttribute();

	bool bShouldHandle = false;

	// Handle renamed
	if (bIsSameOwner && CachedAttribute.AttributeName == InPayload.OldPropertyName)
	{
		if (FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
		{
			SG_EDITOR_NS_LOG(VeryVerbose, TEXT("Replacing modifier magnitude backing attribute with %s"), *GetNameSafe(NewAttributeProp))
			NewAttribute = FGameplayAttribute(NewAttributeProp);
			bShouldHandle = true;
		}
	}
	// Handle removed
	else if (bIsSameOwner && CachedAttribute.AttributeName == InPayload.RemovedPropertyName)
	{
		// ModifiersToReset.Add(FAttributeModifierToReplace(CurrentIndex, nullptr));
		bShouldHandle = true;
	}
	
	bool bWasHandled = false;

	// Now make the replacement, if it was a proper rename or removal
	if (bShouldHandle)
	{
		FAttributeBasedFloat AttributeBasedFloat = FAttributeBasedFloat();
		if (const FAttributeBasedFloat* PrevAttributeBased = GetAttributeBasedFloatFromContainer(&InOutModifierMagnitude))
		{
			// This case is to retain any member previously set by user for the backing attribute definition
			AttributeBasedFloat.Coefficient = PrevAttributeBased->Coefficient;
			AttributeBasedFloat.PreMultiplyAdditiveValue = PrevAttributeBased->PreMultiplyAdditiveValue;
			AttributeBasedFloat.PostMultiplyAdditiveValue = PrevAttributeBased->PostMultiplyAdditiveValue;
			AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
				NewAttribute,
				PrevAttributeBased->BackingAttribute.AttributeSource,
				PrevAttributeBased->BackingAttribute.bSnapshot
			);
			AttributeBasedFloat.AttributeCurve = PrevAttributeBased->AttributeCurve;
			AttributeBasedFloat.AttributeCalculationType = PrevAttributeBased->AttributeCalculationType;
			AttributeBasedFloat.FinalChannel = PrevAttributeBased->FinalChannel;
			AttributeBasedFloat.SourceTagFilter = PrevAttributeBased->SourceTagFilter;
			AttributeBasedFloat.TargetTagFilter = PrevAttributeBased->TargetTagFilter;
		}
		else
		{
			AttributeBasedFloat.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(NewAttribute, EGameplayEffectAttributeCaptureSource::Source, false);
		}

		InOutModifierMagnitude = FGameplayEffectModifierMagnitude(AttributeBasedFloat);
		bWasHandled = true;
	}

	return bWasHandled;
}

bool FSGGameplayEffectReferencerHandler::UpdateModifiersBackingAttribute(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(Display, TEXT("InEffectCDO: %s, InBlueprint: %s, InOldPropertyName: %s, InNewPropertyName: %s"),
		*GetNameSafe(InEffectCDO),
		*GetNameSafe(InBlueprint),
		*InPayload.OldPropertyName,
		*InPayload.NewPropertyName
	)

	check(InEffectCDO);
	check(InBlueprint);

	bool bWasHandled = false;

	int32 CurrentIndex = 0;
	for (FGameplayModifierInfo& Modifier : InEffectCDO->Modifiers)
	{
		CurrentIndex++;
		
		FAttributeReference CachedAttribute;
		const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [CurrentIndex](const FAttributeReference& Item)
		{
			return Item.Type == EAttributeReferenceType::ModifierInfo_BackingAttribute_AttributeToCapture && Item.Index == CurrentIndex - 1;
		});
		
		if (!bHasCachedAttribute)
		{
			continue;
		}

		const bool bModifierHandled = UpdateGameplayEffectModifierMagnitude(Modifier.ModifierMagnitude, CachedAttribute, InBlueprint, InPayload);
		if (bModifierHandled) {
			const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
			Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));
			if (InPayload.ReferencerBlueprint.IsValid())
			{
				Message->AddToken(
					FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
					->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USGEditorSubsystem::HandleMessageLogLinkActivated))
				);
			}

			Message->AddToken(FTextToken::Create(FText::Format(
				LOCTEXT("ChangedModifiersAttributeBaseFromTo", "Updated Gameplay Effect > Modifiers > Attribute Based Magnitude at Index {0} from {1} to {2}"),
				FText::AsNumber(CurrentIndex - 1),
				FText::FromString(InPayload.OldPropertyName),
				FText::FromString(InPayload.NewPropertyName)
			)));

			OutMessages.Add(Message);
			bWasHandled = true;
		}
	}

	return bWasHandled;
}

bool FSGGameplayEffectReferencerHandler::UpdateDuration(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(
		Verbose,
		TEXT("InEffectCDO: %s, InAssetIdentifier: %s, InPackageName: %s, InOldPropertyName: %s, InNewPropertyName: %s"),
		*GetNameSafe(InEffectCDO),
		*InAssetIdentifier.ToString(),
		*InPayload.PackageName,
		*InPayload.OldPropertyName,
		*InPayload.NewPropertyName
	)
	
	check(InEffectCDO);
	check(InBlueprint);
	
	bool bWasHandled = false;

	FAttributeReference CachedAttribute;
	const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [](const FAttributeReference& Item)
	{
		return Item.Type == EAttributeReferenceType::DurationMagnitude;
	});

	if (!bHasCachedAttribute)
	{
		return false;
	}

	bWasHandled = UpdateGameplayEffectModifierMagnitude(InEffectCDO->DurationMagnitude, CachedAttribute, InBlueprint, InPayload);
	if (bWasHandled)
	{
		const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
		Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));

		if (InPayload.ReferencerBlueprint.IsValid())
		{
			Message->AddToken(
				FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USGEditorSubsystem::HandleMessageLogLinkActivated))
			);
		}

		Message->AddToken(FTextToken::Create(FText::Format(
			LOCTEXT("ChangedDurationFromTo", "Updated Gameplay Effect > Duration Magnitude > Attribute Based Magnitude > Backing Attribute from {0} to {1}"),
			FText::FromString(InPayload.OldPropertyName),
			FText::FromString(InPayload.NewPropertyName)
		)));

		OutMessages.Add(Message);
	}

	return bWasHandled;
}

bool FSGGameplayEffectReferencerHandler::UpdateCues(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(
		Verbose,
		TEXT("InEffectCDO: %s, InBlueprint: %s, InAssetIdentifier: %s, InPayload: %s"),
		*GetNameSafe(InEffectCDO),
		*GetNameSafe(InBlueprint),
		*InAssetIdentifier.ToString(),
		*InPayload.ToString()
	)

	check(InEffectCDO);
	check(InBlueprint);
	
	bool bWasModified = false;
	
	int32 CurrentIndex = 0;
	for (FGameplayEffectCue& GameplayCue : InEffectCDO->GameplayCues)
	{
		CurrentIndex++;
		
		FAttributeReference CachedAttribute;
		const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [CurrentIndex](const FAttributeReference& Item)
		{
			return Item.Type == EAttributeReferenceType::GameplayCue_MagnitudeAttribute && Item.Index == CurrentIndex - 1;
		});

		if (!bHasCachedAttribute)
		{
			continue;
		}


		bool bWasHandled = false;

		// Cached attribute for this index is matching the old name, replace
		if (CachedAttribute.AttributeName == InPayload.OldPropertyName)
		{
			if (FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
			{
				GameplayCue.MagnitudeAttribute = FGameplayAttribute(NewAttributeProp);
				bWasHandled = true;
			}	
		}
		// Handle removal
		else if (CachedAttribute.AttributeName == InPayload.RemovedPropertyName)
		{
			GameplayCue.MagnitudeAttribute =  FGameplayAttribute();
			bWasHandled = true;
		}

		if (bWasHandled)
		{
			const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
			Message->AddToken(FTextToken::Create(LOCTEXT("ChangedEffect", "Gameplay Effect: ")));

			if (InPayload.ReferencerBlueprint.IsValid())
			{
				Message->AddToken(
					FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
					->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USGEditorSubsystem::HandleMessageLogLinkActivated))
				);
			}

			Message->AddToken(FTextToken::Create(FText::Format(
				LOCTEXT("ChangedGameplayCuesFromToWithIndex", "Updated Display > Gameplay Cues > Magnitude Attribute at Index {0} from {1} to {2}"),
				FText::AsNumber(CurrentIndex - 1),
				FText::FromString(InPayload.OldPropertyName),
				FText::FromString(InPayload.NewPropertyName)
			)));

			OutMessages.Add(Message);
			bWasModified = true;
		}
	}
	
	return bWasModified;
}

bool FSGGameplayEffectReferencerHandler::UpdateRemoveGameplayEffectQuery(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
#if UE_VERSION_NEWER_THAN(5, 3, -1)

	bool bModified = false;
	if (URemoveOtherGameplayEffectComponent* GameplayEffectComponent = const_cast<URemoveOtherGameplayEffectComponent*>(InEffectCDO->FindComponent<URemoveOtherGameplayEffectComponent>()))
	{
		int32 CurrentIndex = 0;
		for (FGameplayEffectQuery& Query : GameplayEffectComponent->RemoveGameplayEffectQueries)
		{
			const FText MessageText = FText::Format(
				LOCTEXT("RemoveGameplayEffectQuery_ModifyingAttribute_WithIndex", "Tags > Remove Gameplay Effect Query [{0}] > ModifyingAttribute"),
				CurrentIndex
			);


			bModified |= UpdateEffectQuery(
				InBlueprint,
				InAssetIdentifier,
				EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute,
				InPayload,
				MessageText,
				Query.ModifyingAttribute,
				OutMessages
			);
			
			CurrentIndex++;
		}

		// Keep backwards compatibility (at least in terms of reading from the data)
		if (bModified && GameplayEffectComponent && !GameplayEffectComponent->RemoveGameplayEffectQueries.IsEmpty())
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			InEffectCDO->RemoveGameplayEffectQuery = GameplayEffectComponent->RemoveGameplayEffectQueries.Last();
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
		}
	}

	return bModified;
	
#else
	const FText MessageText = LOCTEXT("RemoveGameplayEffectQuery_ModifyingAttribute", "Tags > Remove Gameplay Effect Query > ModifyingAttribute");
	return UpdateEffectQuery(
		InBlueprint,
		InAssetIdentifier,
		EAttributeReferenceType::RemoveGameplayEffectQuery_ModifyingAttribute,
		InPayload,
		MessageText,
		InEffectCDO->RemoveGameplayEffectQuery.ModifyingAttribute,
		OutMessages
	);
#endif
}

bool FSGGameplayEffectReferencerHandler::UpdateImmunityEffectQuery(UGameplayEffect* InEffectCDO, const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
#if UE_VERSION_NEWER_THAN(5, 3, -1)

	bool bModified = false;
	if (UImmunityGameplayEffectComponent* GameplayEffectComponent = const_cast<UImmunityGameplayEffectComponent*>(InEffectCDO->FindComponent<UImmunityGameplayEffectComponent>()))
	{
		int32 CurrentIndex = 0;
		for (FGameplayEffectQuery& Query : GameplayEffectComponent->ImmunityQueries)
		{
			const FText MessageText = FText::Format(
				LOCTEXT("GrantedApplicationImmunityQuery_ModifyingAttribute_WithIndex", "Immunity > Granted Application Immunity Query [{0}] > ModifyingAttribute"),
				CurrentIndex
			);
			
			bModified |= UpdateEffectQuery(
				InBlueprint,
				InAssetIdentifier,
				EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute,
				InPayload,
				MessageText,
				Query.ModifyingAttribute,
				OutMessages
			);
			
			CurrentIndex++;
		}
		
		// Keep backwards compatibility (at least in terms of reading from the data)
		if (bModified && GameplayEffectComponent && !GameplayEffectComponent->ImmunityQueries.IsEmpty())
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			InEffectCDO->GrantedApplicationImmunityQuery = GameplayEffectComponent->ImmunityQueries.Last();
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
		}
	}

	return bModified;
	
#else
	const FText MessageText = LOCTEXT("GrantedApplicationImmunityQuery_ModifyingAttribute", "Immunity > Granted Application Immunity Query > ModifyingAttribute");
	return UpdateEffectQuery(
		InBlueprint,
		InAssetIdentifier,
		EAttributeReferenceType::GrantedApplicationImmunityQuery_ModifyingAttribute,
		InPayload,
		MessageText,
		InEffectCDO->GrantedApplicationImmunityQuery.ModifyingAttribute,
		OutMessages
	);
#endif
}

bool FSGGameplayEffectReferencerHandler::UpdateEffectQuery(const UBlueprint* InBlueprint, const FAssetIdentifier& InAssetIdentifier, EAttributeReferenceType InReferenceType, const FSGAttributeReferencerPayload& InPayload, const FText& InMessageText, FGameplayAttribute& OutModifyingAttribute, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("InBlueprint: %s, InAssetIdentifier: %s, InPayload: %s"), *GetNameSafe(InBlueprint), *InAssetIdentifier.ToString(), *InPayload.ToString())

	check(InBlueprint);

	FAttributeReference CachedAttribute;
	const bool bHasCachedAttribute = GetCachedAttributeByPredicate(InAssetIdentifier, CachedAttribute, [InReferenceType](const FAttributeReference& Item)
	{
		return Item.Type == InReferenceType;
	});

	if (!bHasCachedAttribute)
	{
		return false;
	}

	// not same owner, e.g. not the same class name
	if (CachedAttribute.PackageNameOwner != InPayload.PackageName)
	{
		return false;
	}

	bool bWasHandled = false;

	// Handle renamed
	if (CachedAttribute.AttributeName == InPayload.OldPropertyName)
	{
		if (FProperty* NewAttributeProp = FindFProperty<FProperty>(InBlueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
		{
			SG_EDITOR_NS_LOG(VeryVerbose, TEXT("Replacing %s with %s"), *InMessageText.ToString(), *GetNameSafe(NewAttributeProp))

			// Update it to the new attribute
			OutModifyingAttribute = FGameplayAttribute(NewAttributeProp);
			bWasHandled = true;
		}
	}
	else if (CachedAttribute.AttributeName == InPayload.RemovedPropertyName)
	{
		// Reset it back to default attribute (None)
		OutModifyingAttribute = FGameplayAttribute();
		bWasHandled = true;
	}

	if (bWasHandled)
	{
		// Build up a message for message log
		const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
		Message->AddToken(FTextToken::Create(FText::Format(LOCTEXT("ChangedEffectQuery_ModifyingAttribute", "Gameplay Effect: "), InMessageText)));

		if (InPayload.ReferencerBlueprint.IsValid())
		{
			Message->AddToken(
				FUObjectToken::Create(InPayload.ReferencerBlueprint.Get(), FText::FromString(InPayload.ReferencerBlueprint->GetName()))
				->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&USGEditorSubsystem::HandleMessageLogLinkActivated))
			);
		}
		
		Message->AddToken(FTextToken::Create(FText::Format(
			LOCTEXT("ChangedFromTo", "Updated {0} from {1} to {2}"),
			InMessageText,
			FText::FromString(InPayload.OldPropertyName),
			FText::FromString(InPayload.NewPropertyName)
		)));

		OutMessages.Add(Message);
	}

	return bWasHandled;
}

FAttributeBasedFloat* FSGGameplayEffectReferencerHandler::GetAttributeBasedFloatFromContainer(FGameplayEffectModifierMagnitude* InModifierMagnitude)
{
	if (const FProperty* AttributeBasedMagnitudeProp = FindFProperty<FProperty>(FGameplayEffectModifierMagnitude::StaticStruct(), TEXT("AttributeBasedMagnitude")))
	{
		uint8* ClassDefaults = reinterpret_cast<uint8*>(InModifierMagnitude);
		return AttributeBasedMagnitudeProp->ContainerPtrToValuePtr<FAttributeBasedFloat>(ClassDefaults);
	}

	return nullptr;
}

FString FSGGameplayEffectReferencerHandler::GetClassDefaultName(const FString& InName)
{
	FString Name = InName;
	Name.RemoveFromStart(TEXT("Default__"));
	Name.RemoveFromEnd(TEXT("_C"));
	return Name;
}

#undef LOCTEXT_NAMESPACE