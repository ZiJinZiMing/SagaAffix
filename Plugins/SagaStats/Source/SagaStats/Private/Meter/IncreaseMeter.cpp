/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "Meter/IncreaseMeter.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UIncreaseMeter::UIncreaseMeter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	InitDegeneration(0.f);
	InitDegenerationCooldown(0.f);
}

void UIncreaseMeter::InitFromMetaDataTable(const UDataTable* DataTable)
{
	Super::InitFromMetaDataTable(DataTable);
}

void UIncreaseMeter::OnAccumulate_Implementation(const FSGAttributeSetExecutionData& Data)
{
	Super::OnAccumulate_Implementation(Data);
}

void UIncreaseMeter::PostAccumulate()
{
	Super::PostAccumulate();

	if (GetImpactedAccumulate() > 0 && GetDegenerationCooldown() > 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(DegenerationCooldownTimer);
		GetWorld()->GetTimerManager().SetTimer(DegenerationCooldownTimer, FTimerDelegate::CreateUObject(this, &ThisClass::OnDegenerationCooldownTimerFinish), GetDegenerationCooldown(), false);
	}
}


void UIncreaseMeter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanDegeneration())
	{
		if (!IsEmptied())
		{
			float NewValue = GetCurrent() - GetDegeneration() * DeltaTime;
			SetAttributeValue(GetCurrentAttribute(), NewValue);
		}
	}
}

void UIncreaseMeter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UIncreaseMeter, Degeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIncreaseMeter, DegenerationCooldown, COND_None, REPNOTIFY_Always);
}

bool UIncreaseMeter::CanDegeneration() const
{
	return GetDegeneration() > 0 && !DegenerationCooldownTimer.IsValid();
}

void UIncreaseMeter::OnDegenerationCooldownTimerFinish()
{
	DegenerationCooldownTimer.Invalidate();

	if (GetDegeneration() <= 0.f)
	{
		SetAttributeValue(GetCurrentAttribute(),METER_MINIMUM);
	}
}
