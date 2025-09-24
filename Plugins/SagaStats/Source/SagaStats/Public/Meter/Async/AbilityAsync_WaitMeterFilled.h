/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitMeterFilled.generated.h"


class UMeterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitMeterFilledEvent, UMeterBase*, Meter);

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
	static UAbilityAsync_WaitMeterFilled* WaitMeterFilled(AActor* TargetActor, TSubclassOf<UMeterBase> MeterClass);

protected:
	virtual void Activate() override;
	
	virtual void EndAction() override;
	
private:
	FDelegateHandle MeterFilledHandle;

	UPROPERTY()
	TSubclassOf<UMeterBase> MeterClass;

	UFUNCTION()
	void OnMeterFilledCallback(UMeterBase* Meter);
	
};

