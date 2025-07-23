/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "SagaAbilitySystemComponent.h"
#include "Meter/SagaMeterBase.h"
#include "Meter/SagaDecreaseMeter.h"


void USagaAbilitySystemComponent::RemoveAttributeSet(UAttributeSet* AttributeSet)
{
	RemoveSpawnedAttribute(AttributeSet);
}

void USagaAbilitySystemComponent::RemoveAttributeSetByClass(TSubclassOf<UAttributeSet> AttributeSetClass)
{
	if (UAttributeSet* Set = const_cast<UAttributeSet*>(GetAttributeSet(AttributeSetClass)))
	{
		RemoveAttributeSet(Set);
	}
}

FOnAttributeSetAddOrRemoveEvent& USagaAbilitySystemComponent::GetAttributeSetAddOrRemoveDelegate(TSubclassOf<UAttributeSet> SetClass)
{
	return AttributeSetAddOrRemoveDelegates.FindOrAdd(SetClass);
}

FOnMeterEmptiedEvent& USagaAbilitySystemComponent::GetMeterEmptiedDelegate(TSubclassOf<USagaMeterBase> MeterClass)
{
	return MeterEmptiedDelegates.FindOrAdd(MeterClass);
}

FOnMeterFilledEvent& USagaAbilitySystemComponent::GetMeterFilledDelegate(TSubclassOf<USagaMeterBase> MeterClass)
{
	return MeterFilledDelegates.FindOrAdd(MeterClass);
}

FOnMeterStateChangeEvent& USagaAbilitySystemComponent::GetMeterStateChangeDelegate(
	TSubclassOf<USagaDecreaseMeter> MeterClass)
{
	return MeterStateChangeDelegates.FindOrAdd(MeterClass);
}

void USagaAbilitySystemComponent::AddSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if (!IsValid(AttributeSet))
	{
		return;
	}

	if (GetSpawnedAttributes().Find(AttributeSet) == INDEX_NONE)
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, true);
	}

	Super::AddSpawnedAttribute(AttributeSet);

	UpdateShouldTick();
}

void USagaAbilitySystemComponent::RemoveSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if(GetSpawnedAttributes().Contains(AttributeSet))
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);
	}
	Super::RemoveSpawnedAttribute(AttributeSet);
	
	UpdateShouldTick();
}

void USagaAbilitySystemComponent::RemoveAllSpawnedAttributes()
{
	for (UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);

	}
	Super::RemoveAllSpawnedAttributes();

	UpdateShouldTick();
}

void USagaAbilitySystemComponent::OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes)
{
	if (IsUsingRegisteredSubObjectList())
	{
		// Find the attributes that got removed
		for (UAttributeSet* PreviousAttributeSet : PreviousSpawnedAttributes)
		{
			if (PreviousAttributeSet)
			{
				const bool bWasRemoved = GetSpawnedAttributes().Find(PreviousAttributeSet) == INDEX_NONE;
				if (bWasRemoved)
				{
					GetAttributeSetAddOrRemoveDelegate(PreviousAttributeSet->GetClass()).Broadcast(PreviousAttributeSet, false);
					UpdateShouldTick();
				}
			}
		}

		// Find the attributes that got added
		for (UAttributeSet* NewAttributeSet : GetSpawnedAttributes())
		{
			if (IsValid(NewAttributeSet))
			{
				const bool bIsAdded = PreviousSpawnedAttributes.Find(NewAttributeSet) == INDEX_NONE;
				if (bIsAdded)
				{
					GetAttributeSetAddOrRemoveDelegate(NewAttributeSet->GetClass()).Broadcast(NewAttributeSet, true);
					UpdateShouldTick();
				}
			}
		}
	}
	
	Super::OnRep_SpawnedAttributes(PreviousSpawnedAttributes);

}
