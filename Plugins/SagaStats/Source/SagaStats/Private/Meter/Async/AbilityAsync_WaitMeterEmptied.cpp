/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitMeterEmptied.h"

#include "SagaAbilitySystemComponent.h"

UAbilityAsync_WaitMeterEmptied* UAbilityAsync_WaitMeterEmptied::WaitMeterEmptied(AActor* TargetActor, TSubclassOf<USagaMeterBase> MeterClass)
{
	UAbilityAsync_WaitMeterEmptied* Async = NewObject<UAbilityAsync_WaitMeterEmptied>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterEmptied::Activate()
{
	Super::Activate();
	if(USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		MeterEmptiedHandle = ASC->GetMeterEmptiedDelegate(MeterClass).AddUObject(this,&ThisClass::OnMeterEmptiedCallback);
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitMeterEmptied::EndAction()
{
	if(USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterEmptiedDelegate(MeterClass).Remove(MeterEmptiedHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterEmptied::OnMeterEmptiedCallback(USagaMeterBase* Meter)
{
	OnEmptied.Broadcast(Meter);
}
