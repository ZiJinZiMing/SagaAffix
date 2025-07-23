/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/SagaIncreaseMeter.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

USagaIncreaseMeter::USagaIncreaseMeter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	InitDegeneration(0.f);
	InitDegenerationCooldown(0.f);
}

void USagaIncreaseMeter::InitFromMetaDataTable(const UDataTable* DataTable)
{
	Super::InitFromMetaDataTable(DataTable);
}

void USagaIncreaseMeter::OnAccumulate_Implementation(const FSagaAttributeSetExecutionData& Data)
{
	Super::OnAccumulate_Implementation(Data);
	
	if (GetImpactedAccumulate() > 0 && GetDegenerationCooldown() > 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(DegenerationCooldownTimer);
		GetWorld()->GetTimerManager().SetTimer(DegenerationCooldownTimer, FTimerDelegate::CreateUObject(this, &ThisClass::OnDegenerationCooldownTimerFinish), GetDegenerationCooldown(), false);
	}
}


void USagaIncreaseMeter::Tick(float DeltaTime)
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

void USagaIncreaseMeter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USagaIncreaseMeter, Degeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USagaIncreaseMeter, DegenerationCooldown, COND_None, REPNOTIFY_Always);
}

bool USagaIncreaseMeter::CanDegeneration() const
{
	return GetDegeneration() > 0 && !DegenerationCooldownTimer.IsValid();
}

void USagaIncreaseMeter::OnDegenerationCooldownTimerFinish()
{
	DegenerationCooldownTimer.Invalidate();

	if (GetDegeneration() <= 0.f)
	{
		SetAttributeValue(GetCurrentAttribute(),METER_MINIMUM);
	}
}
