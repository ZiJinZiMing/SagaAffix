/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitMeterStateChange.h"

#include "SGAbilitySystemComponent.h"
#include "Meter/DecreaseMeter.h"

UAbilityAsync_WaitMeterStateChange* UAbilityAsync_WaitMeterStateChange::WaitMeterStateChange(AActor* TargetActor, TSubclassOf<UDecreaseMeter> MeterClass)
{
	UAbilityAsync_WaitMeterStateChange* Async = NewObject<UAbilityAsync_WaitMeterStateChange>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterStateChange::Activate()
{
	Super::Activate();
	if (USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
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
	if (USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterStateChangeDelegate(MeterClass).Remove(MeterStateChangeHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterStateChange::OnMeterStateChangeCallback(UDecreaseMeter* Meter, EMeterState OldState)
{
	OnStateChange.Broadcast(Meter, Meter->MeterState, OldState);
}
