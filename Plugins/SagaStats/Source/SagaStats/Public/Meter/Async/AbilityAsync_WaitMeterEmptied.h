/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitMeterEmptied.generated.h"


class USagaMeterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitMeterEmptiedEvent, USagaMeterBase*, Meter);

/**
 * 
 */
UCLASS()
class SAGASTATS_API UAbilityAsync_WaitMeterEmptied : public UAbilityAsync
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnWaitMeterEmptiedEvent OnEmptied;

	UFUNCTION(BlueprintCallable, Category = "Ability|Async", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityAsync_WaitMeterEmptied* WaitMeterEmptied(AActor* TargetActor, TSubclassOf<USagaMeterBase> MeterClass);

protected:
	virtual void Activate() override;
	
	virtual void EndAction() override;
	
private:
	FDelegateHandle MeterEmptiedHandle;

	UPROPERTY()
	TSubclassOf<USagaMeterBase> MeterClass;

	UFUNCTION()
	void OnMeterEmptiedCallback(USagaMeterBase* Meter);
	
};

