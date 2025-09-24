/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "AttributeSet/Async/AbilityAsync_WaitAttributeSetAddOrRemove.h"

#include "SGAbilitySystemComponent.h"

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
	if(USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
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
	if(USGAbilitySystemComponent* ASC = Cast<USGAbilitySystemComponent>(GetAbilitySystemComponent()))
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
