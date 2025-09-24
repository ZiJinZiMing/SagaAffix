/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitMeterEmptied.generated.h"


class UMeterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitMeterEmptiedEvent, UMeterBase*, Meter);

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
	static UAbilityAsync_WaitMeterEmptied* WaitMeterEmptied(AActor* TargetActor, TSubclassOf<UMeterBase> MeterClass);

protected:
	virtual void Activate() override;
	
	virtual void EndAction() override;
	
private:
	FDelegateHandle MeterEmptiedHandle;

	UPROPERTY()
	TSubclassOf<UMeterBase> MeterClass;

	UFUNCTION()
	void OnMeterEmptiedCallback(UMeterBase* Meter);
	
};

