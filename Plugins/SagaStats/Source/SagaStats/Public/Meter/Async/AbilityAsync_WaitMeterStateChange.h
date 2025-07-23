/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "SagaStatsType.h"
#include "AbilityAsync_WaitMeterStateChange.generated.h"


class USagaDecreaseMeter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWaitMeterStateChangeEvent, USagaDecreaseMeter*, Meter, EMeterState, NewState,EMeterState, OldState);

/**
 * 
 */
UCLASS()
class SAGASTATS_API UAbilityAsync_WaitMeterStateChange : public UAbilityAsync
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnWaitMeterStateChangeEvent OnStateChange;

	UFUNCTION(BlueprintCallable, Category = "Ability|Async", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityAsync_WaitMeterStateChange* WaitMeterStateChange(AActor* TargetActor, TSubclassOf<USagaDecreaseMeter> MeterClass);

protected:
	virtual void Activate() override;

	virtual void EndAction() override;

private:
	FDelegateHandle MeterStateChangeHandle;

	UPROPERTY()
	TSubclassOf<USagaDecreaseMeter> MeterClass;

	UFUNCTION()
	void OnMeterStateChangeCallback(USagaDecreaseMeter* Meter,EMeterState OldState);
};
