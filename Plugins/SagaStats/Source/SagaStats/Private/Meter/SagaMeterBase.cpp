/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/SagaMeterBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "SagaAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"


USagaMeterBase::USagaMeterBase(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	Current.MinValue.ClampType = ESagaClampingType::Float;
	Current.MinValue.Value = METER_MINIMUM;
	Current.MaxValue.ClampType = ESagaClampingType::AttributeBased;
	FGameplayAttribute MaximumAttribute = GetMaximumAttribute();
	Current.MaxValue.Attribute = MaximumAttribute;

	InitMaximum(1.f);
	InitAccumulate(0.f);

}


bool USagaMeterBase::IsFilled() const
{
	return GreaterOrNearlyEqual(GetCurrent(), GetMaximum());
}

bool USagaMeterBase::IsEmptied() const
{
	return LessOrNearlyEqual(GetCurrent(),METER_MINIMUM);
}


void USagaMeterBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
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

bool USagaMeterBase::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	bool Success = Super::PreGameplayEffectExecute(Data);
	if (Success)
	{
		//todo:Reset
	}
	return Success;
}

void USagaMeterBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetAccumulateAttribute())
	{
		const FSagaAttributeSetExecutionData ExecutionData(Data);
		OnAccumulate(ExecutionData);
		SetAccumulate(0);
	}
	else if (Data.EvaluatedData.Attribute == GetReduceAttribute())
	{
		const FSagaAttributeSetExecutionData ExecutionData(Data);
		OnReduce(ExecutionData);
		SetReduce(0);
	}
}


void USagaMeterBase::OnAccumulate_Implementation(const FSagaAttributeSetExecutionData& Data)
{
	float OldCurrent = GetCurrent();
	SetAttributeValue(GetCurrentAttribute(), GetAccumulate() + OldCurrent);
	SetImpactedAccumulate(GetCurrent() - OldCurrent);
}

void USagaMeterBase::OnReduce_Implementation(const FSagaAttributeSetExecutionData& Data)
{
	float OldCurrent = GetCurrent();
	SetAttributeValue(GetCurrentAttribute(), OldCurrent - GetReduce());
	SetImpactedReduce(OldCurrent - GetCurrent());
}

void USagaMeterBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	if (Attribute == GetCurrentAttribute())
	{
		PerformClampingForAttribute(Attribute, NewValue);
	}
}

float USagaMeterBase::GetPercent() const
{
	if (GetMaximum() == 0) return 0.f;
	return GetCurrent() / GetMaximum();
}

void USagaMeterBase::OnEmptied_Implementation()
{
	Cast<USagaAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterEmptiedDelegate(GetClass()).Broadcast(this);
}

void USagaMeterBase::OnFilled_Implementation()
{
	Cast<USagaAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterFilledDelegate(GetClass()).Broadcast(this);
}

void USagaMeterBase::OnMaximumChanged(float OldValue, float NewValue)
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

void USagaMeterBase::OnCurrentChanged(float OldValue, float NewValue)
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

bool USagaMeterBase::ShouldTick() const
{
	return GetOwningActor()->HasAuthority();
}

void USagaMeterBase::Tick(float DeltaTime)
{
}

void USagaMeterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USagaMeterBase, Current, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USagaMeterBase, Maximum, COND_None, REPNOTIFY_Always);
};


bool USagaMeterBase::GreaterOrNearlyEqual(float A, float B)
{
	return A >= B || FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
}

bool USagaMeterBase::LessOrNearlyEqual(float A, float B)
{
	return A <= B || FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
}

const USagaMeterBase* USagaMeterBase::GetMeter(AActor* Actor, TSubclassOf<USagaMeterBase> MeterClass)
{
	if(UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		return Cast<USagaMeterBase>(ASC->GetAttributeSet(MeterClass));
	}
	return nullptr;
}
