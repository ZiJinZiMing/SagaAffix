// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameplayAbilityTypes.h"

#include "GameplayTagsManager.h"


namespace UE::MyGAS
{
	FGameplayTag Tag_TryActivateAbility;

	struct FNativeGameplayTags : FGameplayTagNativeAdder
	{
		virtual ~FNativeGameplayTags() {}

		virtual void AddTags() override
		{
			UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
			Tag_TryActivateAbility = Manager.AddNativeGameplayTag(TEXT("Ability.TryActivateAbility"));
			//todo:	可以考虑OnFilterGameplayTag从面板中隐藏Tag
			// Manager.OnFilterGameplayTag
		}

		static const FNativeGameplayTags& Get()
		{
			return StaticInstance;
		}
		static FNativeGameplayTags StaticInstance;
	};
	FNativeGameplayTags FNativeGameplayTags::StaticInstance;

} // UE::SmartObject::EnabledReason


bool FMyGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	return Super::NetSerialize(Ar, Map, bOutSuccess) && TargetData.NetSerialize(Ar, Map, bOutSuccess);
}
