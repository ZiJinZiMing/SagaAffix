/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitAttributeSetAddOrRemove.h"

#include "SagaAbilitySystemComponent.h"

UAbilityAsync_WaitAttributeSetAddOrRemove* UAbilityAsync_WaitAttributeSetAddOrRemove::WaitAttributeSetAddOrRemove(AActor* TargetActor, TSubclassOf<UAttributeSet> SetClass,bool TriggerAddWhenActive)
{
	UAbilityAsync_WaitAttributeSetAddOrRemove* Async = NewObject<UAbilityAsync_WaitAttributeSetAddOrRemove>();
	Async->SetAbilityActor(TargetActor);
	Async->SetClass = SetClass;
	Async->TriggerAddWhenActive = TriggerAddWhenActive;
	return Async;
}

void UAbilityAsync_WaitAttributeSetAddOrRemove::Activate()
{
	Super::Activate();
	if(USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		AttributeSetAddOrRemoveHandle = ASC->GetAttributeSetAddOrRemoveDelegate(SetClass).AddUObject(this,&ThisClass::OnAttributeSetAddOrRemoveCallback);
		if (TriggerAddWhenActive)
		{
			if (const UAttributeSet* Set = ASC->GetAttributeSet(SetClass))
			{
				OnAdd.Broadcast(const_cast<UAttributeSet*>(Set));
			}
		}
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitAttributeSetAddOrRemove::EndAction()
{
	if(USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetAttributeSetAddOrRemoveDelegate(SetClass).Remove(AttributeSetAddOrRemoveHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitAttributeSetAddOrRemove::OnAttributeSetAddOrRemoveCallback(UAttributeSet* AttributeSet, bool IsAdd)
{
	if (IsAdd)
	{
		OnAdd.Broadcast(AttributeSet);
	}
	else
	{
		OnRemove.Broadcast(AttributeSet);
	}
}
