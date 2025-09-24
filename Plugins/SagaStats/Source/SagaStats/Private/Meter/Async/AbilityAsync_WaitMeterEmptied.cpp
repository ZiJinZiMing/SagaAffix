/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitMeterEmptied.h"

#include "SGAbilitySystemComponent.h"

UAbilityAsync_WaitMeterEmptied* UAbilityAsync_WaitMeterEmptied::WaitMeterEmptied(AActor* TargetActor, TSubclassOf<UMeterBase> MeterClass)
{
	UAbilityAsync_WaitMeterEmptied* Async = NewObject<UAbilityAsync_WaitMeterEmptied>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterEmptied::Activate()
{
	Super::Activate();
	if(USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		MeterEmptiedHandle = ASC->GetMeterEmptiedDelegate(MeterClass).AddUObject(this,&ThisClass::OnMeterEmptiedCallback);
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitMeterEmptied::EndAction()
{
	if(USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterEmptiedDelegate(MeterClass).Remove(MeterEmptiedHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterEmptied::OnMeterEmptiedCallback(UMeterBase* Meter)
{
	OnEmptied.Broadcast(Meter);
}
