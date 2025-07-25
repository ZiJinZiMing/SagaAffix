/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Utils/SSExecutionCalculationBlueprintLibrary.h"

#include "SagaStatsLog.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Kismet/KismetSystemLibrary.h"

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

bool USSExecutionCalculationBlueprintLibrary::RequirementsMet(const FGameplayTagContainer& TagContainer, const FGameplayTagRequirements& Requirements)
{
	// 调用引擎内置的标签需求检查方法 / Call engine's built-in tag requirement check method
	return Requirements.RequirementsMet(TagContainer);
}

void USSExecutionCalculationBlueprintLibrary::PrintExecutionOutputModifiers(const FGameplayEffectCustomExecutionOutput& ExecutionOutput)
{
	// 获取输出修改器数组 / Get the output modifiers array
	const TArray<FGameplayModifierEvaluatedData>& OutputModifiers = ExecutionOutput.GetOutputModifiers();
	
	// 打印修改器总数 / Print total number of modifiers
	UE_LOG(LogTemp, Log, TEXT("ExecutionOutput包含 %d 个修改器 / ExecutionOutput contains %d modifiers"), OutputModifiers.Num(), OutputModifiers.Num());
	
	// 遍历并打印每个修改器的信息 / Iterate through and print information for each modifier
	for (int32 Index = 0; Index < OutputModifiers.Num(); ++Index)
	{
		const FGameplayModifierEvaluatedData& Modifier = OutputModifiers[Index];
		
		// 打印修改器详细信息 / Print modifier details
		UE_LOG(LogTemp, Log, TEXT("Modifier %d: 属性=%s, 数值=%.3f"), 
			Index,
			*Modifier.Attribute.GetName(), 
			Modifier.Magnitude);
	}
}


