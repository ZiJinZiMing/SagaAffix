/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "SagaMeterBase.h"
#include "SagaStatsType.h"
#include "SagaDecreaseMeter.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnDynamicGuardMeterSet, TSubclassOf<USagaDecreaseMeter> /*GuardMeterClass*/);


/**
 * 
 */
UCLASS(Abstract)
class SAGASTATS_API USagaDecreaseMeter : public USagaMeterBase
{
	GENERATED_BODY()

public:
	explicit USagaDecreaseMeter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	SAGA_ATTRIBUTE_ACCESSORS(MinimumClamp)
	/* The minimum clamp value for Current*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_MinimumClamp)
	FGameplayAttributeData MinimumClamp;
	UFUNCTION()
	void OnRep_MinimumClamp(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(MinimumClamp, OldValue);
	}

	SAGA_ATTRIBUTE_ACCESSORS(Regeneration)
	/* The minimum clamp value for Current*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Regeneration)
	FGameplayAttributeData Regeneration;
	UFUNCTION()
	void OnRep_Regeneration(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(Regeneration, OldValue);
	}

	SAGA_ATTRIBUTE_ACCESSORS(RegenerationCooldown)
	/* The minimum clamp value for Current*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_RegenerationCooldown)
	FGameplayAttributeData RegenerationCooldown;
	UFUNCTION()
	void OnRep_RegenerationCooldown(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(RegenerationCooldown, OldValue);
	}

	SAGA_ATTRIBUTE_ACCESSORS(LockDuration)
	/* Duration of lock state
	 * >0, keep lock state in seconds
	 * =0, keep lock state a frame
	 * <0, always keep lock state 
	 */
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_LockDuration)
	FGameplayAttributeData LockDuration;
	UFUNCTION()
	void OnRep_LockDuration(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(LockDuration, OldValue);
	}


	SAGA_ATTRIBUTE_ACCESSORS(ResetRate);
	/* Recovery rate in resetting state
	 * >0, recovery rate in reset state
	 * <=0, reset immediately
	 */
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_ResetRate)
	FGameplayAttributeData ResetRate;
	UFUNCTION()
	void OnRep_ResetRate(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(ResetRate, OldValue);
	}

	SAGA_ATTRIBUTE_ACCESSORS(ImmuneThreshold);
	/* Immune in resetting state
	 * >0, maintain immune until the threshold reached
	 * =0(default), no immune in recovery state
	 * <0, maintain immune throughout the resetting state
	 */
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_ImmuneThreshold)
	FGameplayAttributeData ImmuneThreshold;
	UFUNCTION()
	void OnRep_ImmuneThreshold(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(ImmuneThreshold, OldValue);
	}

	/*State of meter*/
	UPROPERTY(ReplicatedUsing=OnRep_MeterState, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Runtime")
	EMeterState MeterState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Meter|Guard")
	TSubclassOf<USagaDecreaseMeter> GuardMeterClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Meter|Guard")
	TSubclassOf<USagaDecreaseMeter> DynamicGuardMeter;

	FOnDynamicGuardMeterSet OnDynamicGuardMeterSet;
	
public:
	
	UFUNCTION(BlueprintCallable, Category="Meter")
	void StopLockState();


	UFUNCTION(BlueprintCallable, Category="Meter")
	void StopResetState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false, Category="Meter|Guard")
	void CalcGuardReduce(USagaDecreaseMeter* ProtectedMeter, float InReduce, float& OutGuardReduce, float& OutRemainReduce) const;
	virtual void CalcGuardReduce_Implementation(USagaDecreaseMeter* ProtectedMeter, float InReduce, float& OutGuardReduce, float& OutRemainReduce) const;

	UFUNCTION(BlueprintCallable, Category="Meter|Guard")
	void SetDynamicGuardMeter(TSubclassOf<USagaDecreaseMeter> GuardMeter);
	
protected:
	UFUNCTION()
	void OnRep_MeterState(const EMeterState& OldValue);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnReduce_Implementation(const FSagaAttributeSetExecutionData& Data) override;

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;

	virtual void Tick(float DeltaTime) override;

	bool CanRegeneration() const;

	virtual void InitFromMetaDataTable(const UDataTable* DataTable) override;
	
	UPROPERTY(transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Runtime")
	FTimerHandle RegenerationCooldownTimer;

	UFUNCTION()
	void OnRegenerationCooldownTimerFinish();

	UPROPERTY(transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Runtime")
	FTimerHandle LockStateTimer;

	UFUNCTION()
	void OnLockStateFinish();

	virtual void OnEmptied_Implementation() override;

	virtual void OnFilled_Implementation() override;

	bool IsInResetImmune() const;

	void SetMeterState(EMeterState NewState);
};
