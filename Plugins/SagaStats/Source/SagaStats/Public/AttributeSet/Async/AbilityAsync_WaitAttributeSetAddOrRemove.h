/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitAttributeSetAddOrRemove.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitAttributeSetAddOrRemoveEvent, UAttributeSet*, AttributeSet);

/**
 * 
 */
UCLASS()
class SAGASTATS_API UAbilityAsync_WaitAttributeSetAddOrRemove : public UAbilityAsync
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnWaitAttributeSetAddOrRemoveEvent OnAdd;

	UPROPERTY(BlueprintAssignable)
	FOnWaitAttributeSetAddOrRemoveEvent OnRemove;

	UFUNCTION(BlueprintCallable, Category = "Ability|Async", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityAsync_WaitAttributeSetAddOrRemove* WaitAttributeSetAddOrRemove(AActor* TargetActor, TSubclassOf<UAttributeSet> SetClass,bool TriggerAddWhenActive = true);

	uint8 TriggerAddWhenActive : 1;

protected:
	virtual void Activate() override;

	virtual void EndAction() override;
	
private:
	FDelegateHandle AttributeSetAddOrRemoveHandle;

	UPROPERTY()
	TSubclassOf<UAttributeSet> SetClass;

	UFUNCTION()
	void OnAttributeSetAddOrRemoveCallback(UAttributeSet* AttributeSet, bool IsAdd);
	
};

