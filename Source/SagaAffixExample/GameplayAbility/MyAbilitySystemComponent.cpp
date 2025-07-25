// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAbilitySystemComponent.h"


#include "MyGameplayAbilityTargetData.h"
#include "MyGameplayAbilityTypes.h"






// Sets default values for this component's properties
UMyAbilitySystemComponent::UMyAbilitySystemComponent()
{
}

bool UMyAbilitySystemComponent::TryActivateAbilityByClassWithTargetData(TSubclassOf<UGameplayAbility> InAbilityToActivate, const FGameplayAbilityTargetDataHandle& TargetData)
{
	UGameplayAbility* InAbilityCDO = InAbilityToActivate.GetDefaultObject();

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability.Get() == InAbilityCDO)
		{
			// UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("TryActivateAbilityByClassWithTargetData|Key:%s"),*ScopedPredictionKey.ToString()));

			FGameplayEventData EventData;
			EventData.EventTag = UE::MyGAS::Tag_TryActivateAbility;
			// EventData.EventTag = FGameplayTag::EmptyTag;
			EventData.TargetData = TargetData;
			// EventData.ContextHandle = MakeEffectContext();
			return InternalTryActivateAbility(Spec.Handle, ScopedPredictionKey, nullptr, nullptr, &EventData);
		}
	}
	return false;
}

FGameplayAbilityActorInfo UMyAbilitySystemComponent::GetGameplayAbilityActorInfo() const
{
	return *(AbilityActorInfo.Get());
}

