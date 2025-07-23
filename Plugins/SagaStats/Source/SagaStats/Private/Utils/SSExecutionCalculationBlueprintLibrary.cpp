/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Utils/SSExecutionCalculationBlueprintLibrary.h"

#include "SagaStatsLog.h"
#include "GameplayEffectExecutionCalculation.h"

const FGameplayEffectSpec& USSExecutionCalculationBlueprintLibrary::GetOwningSpec(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
{
	return InExecutionParams.GetOwningSpec();
}

FGameplayEffectContextHandle USSExecutionCalculationBlueprintLibrary::GetEffectContext(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
{
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	return Spec.GetContext();
}

const FGameplayTagContainer& USSExecutionCalculationBlueprintLibrary::GetSourceTags(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
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

const FGameplayTagContainer& USSExecutionCalculationBlueprintLibrary::GetTargetTags(const FGameplayEffectCustomExecutionParameters& InExecutionParams)
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

bool USSExecutionCalculationBlueprintLibrary::AttemptCalculateCapturedAttributeMagnitude(const FGameplayEffectCustomExecutionParameters& InExecutionParams, const TArray<FGameplayEffectAttributeCaptureDefinition>& InRelevantAttributesToCapture, const FGameplayAttribute InAttribute, float& OutMagnitude)
{
	// First, figure out which capture definition to use - This assumes InRelevantAttributesToCapture is properly populated and passed in by user
	const FGameplayEffectAttributeCaptureDefinition* FoundCapture = InRelevantAttributesToCapture.FindByPredicate([InAttribute](const FGameplayEffectAttributeCaptureDefinition& Entry)
	{
		return Entry.AttributeToCapture == InAttribute;
	});

	if (!FoundCapture)
	{
		SS_NS_LOG(Warning, TEXT("Unable to retrieve a valid Capture Definition from passed in RelevantAttributesToCapture and Attribute: %s"), *InAttribute.GetName())
		return false;
	}

	const FGameplayEffectAttributeCaptureDefinition CaptureDefinition = *FoundCapture;
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	return InExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDefinition, EvaluateParameters, OutMagnitude);
}

bool USSExecutionCalculationBlueprintLibrary::AttemptCalculateCapturedAttributeMagnitudeWithBase(const FGameplayEffectCustomExecutionParameters& InExecutionParams, const TArray<FGameplayEffectAttributeCaptureDefinition>& InRelevantAttributesToCapture, const FGameplayAttribute InAttribute, const float InBaseValue, float& OutMagnitude)
{
	// First, figure out which capture definition to use - This assumes InRelevantAttributesToCapture is properly populated and passed in by user
	const FGameplayEffectAttributeCaptureDefinition* FoundCapture = InRelevantAttributesToCapture.FindByPredicate([InAttribute](const FGameplayEffectAttributeCaptureDefinition& Entry)
	{
		return Entry.AttributeToCapture == InAttribute;
	});

	if (!FoundCapture)
	{
		SS_NS_LOG(Warning, TEXT("Unable to retrieve a valid Capture Definition from passed in RelevantAttributesToCapture and Attribute: %s"), *InAttribute.GetName())
		return false;
	}

	const FGameplayEffectAttributeCaptureDefinition CaptureDefinition = *FoundCapture;
	const FGameplayEffectSpec& Spec = InExecutionParams.GetOwningSpec();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	return InExecutionParams.AttemptCalculateCapturedAttributeMagnitudeWithBase(CaptureDefinition, EvaluateParameters, InBaseValue, OutMagnitude);
}

const FGameplayEffectCustomExecutionOutput& USSExecutionCalculationBlueprintLibrary::AddOutputModifier(FGameplayEffectCustomExecutionOutput& InExecutionOutput, const FGameplayAttribute InAttribute, const EGameplayModOp::Type InModOp, const float InMagnitude)
{
	InExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(InAttribute, InModOp, InMagnitude));
	return MoveTemp(InExecutionOutput);
}

bool USSExecutionCalculationBlueprintLibrary::AttemptCalculateTransientAggregatorMagnitude(const FGameplayEffectCustomExecutionParameters& ExecutionParameters, FGameplayTag InAggregatorIdentifier, float& OutMagnitude)
{
	const FGameplayEffectSpec& Spec = ExecutionParameters.GetOwningSpec();
	auto SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	auto TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	return ExecutionParameters.AttemptCalculateTransientAggregatorMagnitude(InAggregatorIdentifier, EvaluateParameters, OutMagnitude);
}
