/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "Meter/DecreaseMeter.h"
#include "GameplayEffectExtension.h"
#include "SGAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_ENUM_TO_STRING(EMeterState, "/Script/SagaStats")


UDecreaseMeter::UDecreaseMeter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	Current.MinValue.ClampType = ESGClampingType::AttributeBased;
	Current.MinValue.Attribute = GetMinimumClampAttribute();
	Current.MaxValue.ClampType = ESGClampingType::AttributeBased;
	Current.MaxValue.Attribute = GetMaximumAttribute();

	SetMeterState(EMeterState::Normal);
}

void UDecreaseMeter::StopLockState()
{
	check(MeterState == EMeterState::Lock);
	OnLockStateFinish();
}

void UDecreaseMeter::StopResetState()
{
	check(MeterState == EMeterState::Reset);
	SetMeterState(EMeterState::Normal);
}

void UDecreaseMeter::OnRep_MeterState(const EMeterState& OldValue)
{
	Cast<USGAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterStateChangeDelegate(GetClass()).Broadcast(this,OldValue);
}


void UDecreaseMeter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MinimumClamp, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Regeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, RegenerationCooldown, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, LockDuration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ResetRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ImmuneThreshold, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MeterState, COND_None, REPNOTIFY_OnChanged);
}

void UDecreaseMeter::OnReduce_Implementation(const FSGAttributeSetExecutionData& Data)
{
	Super::OnReduce_Implementation(Data);
}

void UDecreaseMeter::PostReduce()
{
	Super::PostReduce();

	if (GetImpactedReduce() > 0 && GetRegenerationCooldown() > 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RegenerationCooldownTimer);
		GetWorld()->GetTimerManager().SetTimer(RegenerationCooldownTimer, FTimerDelegate::CreateUObject(this, &ThisClass::OnRegenerationCooldownTimerFinish), GetRegenerationCooldown(), false);
	}
}

bool UDecreaseMeter::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))return false;
	if (IsInResetImmune())return false;

	return true;
}

void UDecreaseMeter::Tick(float DeltaTime)
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

bool UDecreaseMeter::CanRegeneration() const
{
	return GetRegeneration() > 0 && !RegenerationCooldownTimer.IsValid();
}

void UDecreaseMeter::InitFromMetaDataTable(const UDataTable* DataTable)
{
	Super::InitFromMetaDataTable(DataTable);
	SetCurrent(GetMaximum());
}

void UDecreaseMeter::OnRegenerationCooldownTimerFinish()
{
	RegenerationCooldownTimer.Invalidate();

	if (GetRegeneration() <= 0.f)
	{
		SetAttributeValue(GetCurrentAttribute(), GetMaximum());
	}
}

void UDecreaseMeter::OnLockStateFinish()
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

void UDecreaseMeter::OnEmptied_Implementation()
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

void UDecreaseMeter::OnFilled_Implementation()
{
	Super::OnFilled_Implementation();

	if (MeterState == EMeterState::Reset)
	{
		SetMeterState(EMeterState::Normal);
	}
}

bool UDecreaseMeter::IsInResetImmune() const
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

void UDecreaseMeter::SetMeterState(EMeterState NewState)
{
	if (MeterState != NewState)
	{
		EMeterState OldState = MeterState;
		MeterState = NewState;
		Cast<USGAbilitySystemComponent>(GetOwningAbilitySystemComponent())->GetMeterStateChangeDelegate(GetClass()).Broadcast(this,OldState);
	}
}
