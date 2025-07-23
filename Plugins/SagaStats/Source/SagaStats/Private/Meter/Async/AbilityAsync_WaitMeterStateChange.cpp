/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitMeterStateChange.h"

#include "SagaAbilitySystemComponent.h"
#include "Meter/SagaDecreaseMeter.h"

UAbilityAsync_WaitMeterStateChange* UAbilityAsync_WaitMeterStateChange::WaitMeterStateChange(AActor* TargetActor, TSubclassOf<USagaDecreaseMeter> MeterClass)
{
	UAbilityAsync_WaitMeterStateChange* Async = NewObject<UAbilityAsync_WaitMeterStateChange>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterStateChange::Activate()
{
	Super::Activate();
	if (USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		MeterStateChangeHandle = ASC->GetMeterStateChangeDelegate(MeterClass).AddUObject(this, &ThisClass::OnMeterStateChangeCallback);
	}
	else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitMeterStateChange::EndAction()
{
	if (USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterStateChangeDelegate(MeterClass).Remove(MeterStateChangeHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterStateChange::OnMeterStateChangeCallback(USagaDecreaseMeter* Meter, EMeterState OldState)
{
	OnStateChange.Broadcast(Meter, Meter->MeterState, OldState);
}
