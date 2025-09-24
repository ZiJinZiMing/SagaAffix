/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "Meter/MeterBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "SGAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"


UMeterBase::UMeterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bManualUpdateCurrent(false)
{
	Current.MinValue.ClampType = ESGClampingType::Float;
	Current.MinValue.Value = METER_MINIMUM;
	Current.MaxValue.ClampType = ESGClampingType::AttributeBased;
	FGameplayAttribute MaximumAttribute = GetMaximumAttribute();
	Current.MaxValue.Attribute = MaximumAttribute;

	InitMaximum(1.f);
	InitAccumulate(0.f);
}


bool UMeterBase::IsFilled() const
{
	return GreaterOrNearlyEqual(GetCurrent(), GetMaximum());
}

bool UMeterBase::IsEmptied() const
{
	return LessOrNearlyEqual(GetCurrent(),METER_MINIMUM);
}


void UMeterBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaximumAttribute())
	{
		OnMaximumChanged(OldValue, NewValue);
	}
	else if (Attribute == GetCurrentAttribute())
	{
		OnCurrentChanged(OldValue, NewValue);
	}
}

bool UMeterBase::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	bool Success = Super::PreGameplayEffectExecute(Data);
	if (Success)
	{
		//todo:Reset
	}
	return Success;
}

void UMeterBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (!bManualUpdateCurrent)
	{
		if (Data.EvaluatedData.Attribute == GetAccumulateAttribute())
		{
			const FSGAttributeSetExecutionData ExecutionData(Data);
			OnAccumulate(ExecutionData);
			SetAccumulate(0);

			PostAccumulate();
		}
		else if (Data.EvaluatedData.Attribute == GetReduceAttribute())
		{
			const FSGAttributeSetExecutionData ExecutionData(Data);
			OnReduce(ExecutionData);
			SetReduce(0);

			PostReduce();
		}
	}
}


void UMeterBase::OnAccumulate_Implementation(const FSGAttributeSetExecutionData& Data)
{
	float OldCurrent = GetCurrent();
	SetAttributeValue(GetCurrentAttribute(), GetAccumulate() + OldCurrent);
	SetImpactedAccumulate(GetCurrent() - OldCurrent);
}

void UMeterBase::OnReduce_Implementation(const FSGAttributeSetExecutionData& Data)
{
	float OldCurrent = GetCurrent();
	SetAttributeValue(GetCurrentAttribute(), OldCurrent - GetReduce());
	SetImpactedReduce(OldCurrent - GetCurrent());
}

void UMeterBase::PostAccumulate()
{
}

void UMeterBase::PostReduce()
{
}

void UMeterBase::PostDamageProcess(float CurrentValue, float PrevCurrentValue, const FSGAttributeSetExecutionData& Data)
{
	if (CurrentValue > PrevCurrentValue)
	{
		SetImpactedAccumulate(CurrentValue - PrevCurrentValue);
		PostAccumulate();
	}
	else if (CurrentValue < PrevCurrentValue)
	{
		SetImpactedReduce(PrevCurrentValue - CurrentValue);
		PostReduce();
	}
}

void UMeterBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	if (Attribute == GetCurrentAttribute())
	{
		PerformClampingForAttribute(Attribute, NewValue);
	}
}

float UMeterBase::GetPercent() const
{
	if (GetMaximum() == 0) return 0.f;
	return GetCurrent() / GetMaximum();
}

void UMeterBase::OnEmptied_Implementation()
{
	Cast<USGAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterEmptiedDelegate(GetClass()).Broadcast(this);
}

void UMeterBase::OnFilled_Implementation()
{
	Cast<USGAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterFilledDelegate(GetClass()).Broadcast(this);
}

void UMeterBase::OnMaximumChanged(float OldValue, float NewValue)
{
	//Current is only clamped by the Maximum value and does not increase with the Maximum value
	float NewCurrent = GetCurrent();
	if (PerformClampingForAttribute(GetCurrentAttribute(), NewCurrent))
	{
		bool ClampedFilled = GetCurrent() < OldValue && GetCurrent() >= NewValue;
		SetAttributeValue(GetCurrentAttribute(), NewCurrent);
		if (ClampedFilled)
		{
			OnFilled();
		}
	}
}

void UMeterBase::OnCurrentChanged(float OldValue, float NewValue)
{
	if (IsEmptied())
	{
		if (!LessOrNearlyEqual(OldValue,METER_MINIMUM))
		{
			OnEmptied();
		}
	}
	else if (IsFilled())
	{
		if (!GreaterOrNearlyEqual(OldValue, GetMaximum()))
		{
			OnFilled();
		}
	}
}

bool UMeterBase::ShouldTick() const
{
	return GetOwningActor()->HasAuthority();
}

void UMeterBase::Tick(float DeltaTime)
{
}

void UMeterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMeterBase, Current, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMeterBase, Maximum, COND_None, REPNOTIFY_Always);
};


bool UMeterBase::GreaterOrNearlyEqual(float A, float B)
{
	return A >= B || FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
}

bool UMeterBase::LessOrNearlyEqual(float A, float B)
{
	return A <= B || FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
}

const UMeterBase* UMeterBase::GetMeter(AActor* Actor, TSubclassOf<UMeterBase> MeterClass)
{
	if(UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		return Cast<UMeterBase>(ASC->GetAttributeSet(MeterClass));
	}
	return nullptr;
}
