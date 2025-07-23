/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/SagaDecreaseMeter.h"
#include "GameplayEffectExtension.h"
#include "SagaAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"



USagaDecreaseMeter::USagaDecreaseMeter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	Current.MinValue.ClampType = ESagaClampingType::AttributeBased;
	Current.MinValue.Attribute = GetMinimumClampAttribute();
	Current.MaxValue.ClampType = ESagaClampingType::AttributeBased;
	Current.MaxValue.Attribute = GetMaximumAttribute();

	SetMeterState(EMeterState::Normal);
}

void USagaDecreaseMeter::StopLockState()
{
	check(MeterState == EMeterState::Lock);
	OnLockStateFinish();
}

void USagaDecreaseMeter::StopResetState()
{
	check(MeterState == EMeterState::Reset);
	SetMeterState(EMeterState::Normal);
}

void USagaDecreaseMeter::OnRep_MeterState(const EMeterState& OldValue)
{
	Cast<USagaAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterStateChangeDelegate(GetClass()).Broadcast(this,OldValue);
}


void USagaDecreaseMeter::CalcGuardReduce_Implementation(USagaDecreaseMeter* ProtectedMeter,float InReduce, float& OutGuardReduce, float& OutRemainReduce) const
{
	OutGuardReduce = FMath::Min(InReduce, GetCurrent());
	OutRemainReduce = InReduce - OutGuardReduce;
}

void USagaDecreaseMeter::SetDynamicGuardMeter(TSubclassOf<USagaDecreaseMeter> GuardMeter)
{
	DynamicGuardMeter = GuardMeter;
	OnDynamicGuardMeterSet.Broadcast(GuardMeter);
}

void USagaDecreaseMeter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Regeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, RegenerationCooldown, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, LockDuration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ResetRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ImmuneThreshold, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MeterState, COND_None, REPNOTIFY_OnChanged);
}

void USagaDecreaseMeter::OnReduce_Implementation(const FSagaAttributeSetExecutionData& Data)
{
	Super::OnReduce_Implementation(Data);

	if (GetImpactedReduce() > 0 && GetRegenerationCooldown() > 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RegenerationCooldownTimer);
		GetWorld()->GetTimerManager().SetTimer(RegenerationCooldownTimer, FTimerDelegate::CreateUObject(this, &ThisClass::OnRegenerationCooldownTimerFinish), GetRegenerationCooldown(), false);
	}
}

bool USagaDecreaseMeter::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))return false;
	if (IsInResetImmune())return false;

	return true;
}

void USagaDecreaseMeter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MeterState == EMeterState::Normal)
	{
		if (CanRegeneration())
		{
			if (!IsFilled())
			{
				float NewValue = GetCurrent() + GetRegeneration() * DeltaTime;
				SetAttributeValue(GetCurrentAttribute(), NewValue);
			}
		}
	}
	else if (MeterState == EMeterState::Reset)
	{
		if (!IsFilled())
		{
			float NewValue = GetCurrent() + GetResetRate() * DeltaTime;
			SetAttributeValue(GetCurrentAttribute(), NewValue);
		}
	}
}

bool USagaDecreaseMeter::CanRegeneration() const
{
	return GetRegeneration() > 0 && !RegenerationCooldownTimer.IsValid();
}

void USagaDecreaseMeter::InitFromMetaDataTable(const UDataTable* DataTable)
{
	Super::InitFromMetaDataTable(DataTable);
	SetCurrent(GetMaximum());
}

void USagaDecreaseMeter::OnRegenerationCooldownTimerFinish()
{
	RegenerationCooldownTimer.Invalidate();

	if (GetRegeneration() <= 0.f)
	{
		SetAttributeValue(GetCurrentAttribute(), GetMaximum());
	}
}

void USagaDecreaseMeter::OnLockStateFinish()
{
	LockStateTimer.Invalidate();

	if (GetResetRate() > 0.f)
	{
		SetMeterState(EMeterState::Reset);
	}
	else
	{
		SetAttributeValue(GetCurrentAttribute(), GetMaximum());
		SetMeterState(EMeterState::Normal);
	}
}

void USagaDecreaseMeter::OnEmptied_Implementation()
{
	Super::OnEmptied_Implementation();

	if (MeterState == EMeterState::Normal)
	{
		SetMeterState(EMeterState::Lock);

		if (GetLockDuration() > 0)
		{
			GetWorld()->GetTimerManager().SetTimer(LockStateTimer, FTimerDelegate::CreateUObject(this, &ThisClass::OnLockStateFinish), GetLockDuration(), false);
		}
		else if (GetLockDuration() == 0)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::OnLockStateFinish));
		}
	}
}

void USagaDecreaseMeter::OnFilled_Implementation()
{
	Super::OnFilled_Implementation();

	if (MeterState == EMeterState::Reset)
	{
		SetMeterState(EMeterState::Normal);
	}
}

bool USagaDecreaseMeter::IsInResetImmune() const
{
	if (MeterState == EMeterState::Reset)
	{
		if (GetImmuneThreshold() < 0)
		{
			return true;
		}
		if (GetImmuneThreshold() > 0 && GetCurrent() < GetImmuneThreshold())
		{
			return true;
		}
	}
	return false;
}

void USagaDecreaseMeter::SetMeterState(EMeterState NewState)
{
	if (MeterState != NewState)
	{
		EMeterState OldState = MeterState;
		MeterState = NewState;
		Cast<USagaAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterStateChangeDelegate(GetClass()).Broadcast(this,OldState);
	}
}
