/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitMeterStateChange.generated.h"


class UDecreaseMeter;
enum class EMeterState : uint8;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWaitMeterStateChangeEvent, UDecreaseMeter*, Meter, EMeterState, NewState,EMeterState, OldState);

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
	static UAbilityAsync_WaitMeterStateChange* WaitMeterStateChange(AActor* TargetActor, TSubclassOf<UDecreaseMeter> MeterClass);

protected:
	virtual void Activate() override;

	virtual void EndAction() override;

private:
	FDelegateHandle MeterStateChangeHandle;

	UPROPERTY()
	TSubclassOf<UDecreaseMeter> MeterClass;

	UFUNCTION()
	void OnMeterStateChangeCallback(UDecreaseMeter* Meter,EMeterState OldState);
};
