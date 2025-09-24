/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitMeterFilled.h"

#include "SGAbilitySystemComponent.h"

UAbilityAsync_WaitMeterFilled* UAbilityAsync_WaitMeterFilled::WaitMeterFilled(AActor* TargetActor, TSubclassOf<UMeterBase> MeterClass)
{
	UAbilityAsync_WaitMeterFilled* Async = NewObject<UAbilityAsync_WaitMeterFilled>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterFilled::Activate()
{
	Super::Activate();
	if(USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		MeterFilledHandle = ASC->GetMeterFilledDelegate(MeterClass).AddUObject(this,&ThisClass::OnMeterFilledCallback);
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitMeterFilled::EndAction()
{
	if(USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterFilledDelegate(MeterClass).Remove(MeterFilledHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterFilled::OnMeterFilledCallback(UMeterBase* Meter)
{
	OnFilled.Broadcast(Meter);
}
