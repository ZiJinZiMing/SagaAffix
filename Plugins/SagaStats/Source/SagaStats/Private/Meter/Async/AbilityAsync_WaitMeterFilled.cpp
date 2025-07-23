/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitMeterFilled.h"

#include "SagaAbilitySystemComponent.h"

UAbilityAsync_WaitMeterFilled* UAbilityAsync_WaitMeterFilled::WaitMeterFilled(AActor* TargetActor, TSubclassOf<USagaMeterBase> MeterClass)
{
	UAbilityAsync_WaitMeterFilled* Async = NewObject<UAbilityAsync_WaitMeterFilled>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterFilled::Activate()
{
	Super::Activate();
	if(USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		MeterFilledHandle = ASC->GetMeterFilledDelegate(MeterClass).AddUObject(this,&ThisClass::OnMeterFilledCallback);
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitMeterFilled::EndAction()
{
	if(USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterFilledDelegate(MeterClass).Remove(MeterFilledHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterFilled::OnMeterFilledCallback(USagaMeterBase* Meter)
{
	OnFilled.Broadcast(Meter);
}
