/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "AttributeSet/SGBlueprintLibrary.h"

#include "AttributeSet/SGAttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"
#include "SagaStatsLog.h"


FString USGBlueprintLibrary::GetDebugStringFromExecutionData(const FSGAttributeSetExecutionData& ExecutionData)
{
	return ExecutionData.ToString(LINE_TERMINATOR);
}

FString USGBlueprintLibrary::GetDebugStringFromExecutionDataSeparator(const FSGAttributeSetExecutionData& ExecutionData, const FString& Separator)
{
	return ExecutionData.ToString(Separator);
}

FString USGBlueprintLibrary::GetDebugStringFromAttribute(const FGameplayAttribute& Attribute)
{
	return Attribute.GetName();
}

bool USGBlueprintLibrary::NotEqual_GameplayAttributeGameplayAttribute(const FGameplayAttribute A, const FString B)
{
	return A.GetName() != B;
}

FText USGBlueprintLibrary::GetAttributeDisplayNameText(const FGameplayAttribute& InAttribute)
{
	return FText::FromString(InAttribute.GetName());
}

bool USGBlueprintLibrary::GetAttributeByPropertyName(const TSubclassOf<UAttributeSet>& AttributeSetClass, const FName& PropertyName, FGameplayAttribute& OutAttribute)
{
	if (FProperty* Property = FindFieldChecked<FProperty>(AttributeSetClass, PropertyName))
	{
		OutAttribute = FGameplayAttribute(Property, AttributeSetClass);
		return true;
	}
	return false;
}



UAbilitySystemComponent* USGBlueprintLibrary::GetInstigatorAbilitySystemComponentFromGEContext(FGameplayEffectContextHandle EffectContextHandle)
{
	// 检查上下文句柄是否有效 / Check if the context handle is valid
	if (!EffectContextHandle.IsValid())
	{
		return nullptr;
	}

	// 获取上下文 / Get the context
	const FGameplayEffectContext* Context = EffectContextHandle.Get();
	if (!Context)
	{
		return nullptr;
	}

	// 获取发起者的能力系统组件 / Get the instigator's ability system component
	return Context->GetInstigatorAbilitySystemComponent();
}





const FGameplayEffectSpec& USGBlueprintLibrary::GetOwningSpec(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
{
	return InExecutionParams.GetOwningSpec();
}

FGameplayEffectContextHandle USGBlueprintLibrary::GetEffectContext(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
{
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	return Spec.GetContext();
}

const FGameplayTagContainer& USGBlueprintLibrary::GetSourceTags(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
{
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	if (!SourceTags)
	{
		static FGameplayTagContainer DummyContainer;
		return DummyContainer;
	}
	
	return *SourceTags;
}

const FGameplayTagContainer& USGBlueprintLibrary::GetTargetTags(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
{
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	if (!TargetTags)
	{
		static FGameplayTagContainer DummyContainer;
		return DummyContainer;
	}
	
	return *TargetTags;
}

bool USGBlueprintLibrary::AttemptCalculateCapturedAttributeMagnitude(const FGameplayEffectCustomExecutionParameters& InExecutionParams, const TArray<FGameplayEffectAttributeCaptureDefinition>& InRelevantAttributesToCapture, const FGameplayAttribute InAttribute, float& OutMagnitude)
{
	// First, figure out which capture definition to use - This assumes InRelevantAttributesToCapture is properly populated and passed in by user
	const FGameplayEffectAttributeCaptureDefinition* FoundCapture = InRelevantAttributesToCapture.FindByPredicate([InAttribute](const FGameplayEffectAttributeCaptureDefinition& Entry)
	{
		return Entry.AttributeToCapture == InAttribute;
	});

	if (!FoundCapture)
	{
		SG_NS_LOG(Warning, TEXT("Unable to retrieve a valid Capture Definition from passed in RelevantAttributesToCapture and Attribute: %s"), *InAttribute.GetName())
		return false;
	}

	const FGameplayEffectAttributeCaptureDefinition CaptureDefinition = *FoundCapture;
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	return InExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDefinition, EvaluateParameters, OutMagnitude);
}

bool USGBlueprintLibrary::AttemptCalculateCapturedAttributeMagnitudeWithBase(const FGameplayEffectCustomExecutionParameters& InExecutionParams, const TArray<FGameplayEffectAttributeCaptureDefinition>& InRelevantAttributesToCapture, const FGameplayAttribute InAttribute, const float InBaseValue, float& OutMagnitude)
{
	// First, figure out which capture definition to use - This assumes InRelevantAttributesToCapture is properly populated and passed in by user
	const FGameplayEffectAttributeCaptureDefinition* FoundCapture = InRelevantAttributesToCapture.FindByPredicate([InAttribute](const FGameplayEffectAttributeCaptureDefinition& Entry)
	{
		return Entry.AttributeToCapture == InAttribute;
	});

	if (!FoundCapture)
	{
		SG_NS_LOG(Warning, TEXT("Unable to retrieve a valid Capture Definition from passed in RelevantAttributesToCapture and Attribute: %s"), *InAttribute.GetName())
		return false;
	}

	const FGameplayEffectAttributeCaptureDefinition CaptureDefinition = *FoundCapture;
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	return InExecutionParams.AttemptCalculateCapturedAttributeMagnitudeWithBase(CaptureDefinition, EvaluateParameters, InBaseValue, OutMagnitude);
}

const FGameplayEffectCustomExecutionOutput& USGBlueprintLibrary::AddOutputModifier(FGameplayEffectCustomExecutionOutput& InExecutionOutput, const FGameplayAttribute InAttribute, const EGameplayModOp::Type InModOp, const float InMagnitude)
{
	InExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(InAttribute, InModOp, InMagnitude));
	return MoveTemp(InExecutionOutput);
}

bool USGBlueprintLibrary::AttemptCalculateTransientAggregatorMagnitude(const FGameplayEffectCustomExecutionParameters& ExecutionParameters, FGameplayTag InAggregatorIdentifier, float& OutMagnitude)
{
	const FGameplayEffectSpec& Spec = ExecutionParameters.GetOwningSpec();
	auto SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	auto TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	return ExecutionParameters.AttemptCalculateTransientAggregatorMagnitude(InAggregatorIdentifier, EvaluateParameters, OutMagnitude);
}
