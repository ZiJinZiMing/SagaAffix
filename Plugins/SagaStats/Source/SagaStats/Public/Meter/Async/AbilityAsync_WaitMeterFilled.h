/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitMeterFilled.generated.h"


class USagaMeterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitMeterFilledEvent, USagaMeterBase*, Meter);

/**
 * 
 */
UCLASS()
class SAGASTATS_API UAbilityAsync_WaitMeterFilled : public UAbilityAsync
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnWaitMeterFilledEvent OnFilled;

	UFUNCTION(BlueprintCallable, Category = "Ability|Async", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityAsync_WaitMeterFilled* WaitMeterFilled(AActor* TargetActor, TSubclassOf<USagaMeterBase> MeterClass);

protected:
	virtual void Activate() override;
	
	virtual void EndAction() override;
	
private:
	FDelegateHandle MeterFilledHandle;

	UPROPERTY()
	TSubclassOf<USagaMeterBase> MeterClass;

	UFUNCTION()
	void OnMeterFilledCallback(USagaMeterBase* Meter);
	
};

