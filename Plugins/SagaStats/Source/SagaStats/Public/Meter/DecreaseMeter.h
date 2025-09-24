/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "MeterBase.h"
#include "DecreaseMeter.generated.h"



/*State of Meter*/
UENUM(BlueprintType)
enum class EMeterState : uint8
{
	Normal,
	Lock,
	Reset,
};
DECLARE_ENUM_TO_STRING(EMeterState);



/**
 * @class UDecreaseMeter
 * @brief 一种随时间恢复的、带有状态机的计量条 (A state machine-driven meter that regenerates over time).
 *
 * @details
 * 该类继承自 UMeterBase，用于实现那些通常处于满值，被消耗后会随时间恢复的数值，最典型的用例是“护盾”或“体力”。
 * This class inherits from UMeterBase and is designed for values that are typically full and regenerate over time after being depleted. The most common use cases are "Shields" or "Stamina".
 *
 * 它不仅仅是一个简单的恢复计量条，而是一个包含三种状态的状态机：
 * It is more than a simple regenerating meter; it is a state machine with three states:
 * - Normal: 正常状态。在该状态下，计量条在被消耗后，会经过一段冷却时间 (`RegenerationCooldown`)，然后以 `Regeneration` 的速率开始恢复。
 *   (Normal state. In this state, after being depleted, the meter will begin to regenerate at the `Regeneration` rate after a `RegenerationCooldown` period.)
 * - Lock: 锁定状态。当计量条完全耗尽时进入此状态。在该状态下，所有恢复都会暂停，持续时间由 `LockDuration` 决定。这模拟了“护盾被击破”后的惩罚时间。
 *   (Lock state. Entered when the meter is fully depleted. In this state, all regeneration is paused for a duration determined by `LockDuration`. This simulates the penalty time after a "shield break".)
 * - Reset: 重置状态。在锁定状态结束后进入。在该状态下，计量条会以一个独立的速率 (`ResetRate`) 开始恢复，并且在恢复到一定阈值 (`ImmuneThreshold`) 前可能免疫伤害。
 *   (Reset state. Entered after the Lock state ends. In this state, the meter recovers at a separate `ResetRate` and can be immune to damage until it passes the `ImmuneThreshold`.)
 *
 * @see UMeterBase
 */
UCLASS(Abstract)
class SAGASTATS_API UDecreaseMeter : public UMeterBase
{
	GENERATED_BODY()

public:
	explicit UDecreaseMeter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	/**
	* 计量条的最小值，Current值不能低于该值
	* The minimum value of the meter. 'Current' cannot go below this value.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_MinimumClamp)
	FGameplayAttributeData MinimumClamp;
	SAGA_ATTRIBUTE_ACCESSORS(MinimumClamp)
	UFUNCTION()
	void OnRep_MinimumClamp(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(MinimumClamp, OldValue);
	}

	/**
	* @brief [Normal]状态下的恢复速率 (The regeneration rate in the [Normal] state).
	*
	* @details
	* 定义了计量条的恢复速度和方式。
	* Defines the speed and manner of the meter's recovery.
	*
	* - 当 > 0: 作为每秒持续恢复的速率 (Heal Over Time)。
	*   When > 0: Acts as the rate of continuous regeneration per second (Heal Over Time).
	*
	* - 当 <= 0: 无持续恢复效果。但若 RegenerationCooldown > 0，则会在冷却结束后触发“立即回满” (Instant-Fill) 的效果。
	*   When <= 0: No continuous regeneration. However, if RegenerationCooldown > 0, it triggers an "Instant-Fill" effect after the cooldown ends.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Regeneration)
	FGameplayAttributeData Regeneration;
	SAGA_ATTRIBUTE_ACCESSORS(Regeneration)
	UFUNCTION()
	void OnRep_Regeneration(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(Regeneration, OldValue);
	}

	/**
	* @brief [Normal]状态下，开始恢复前的冷却时间 (The cooldown before regeneration begins in the [Normal] state).
	*
	* @details
	* 定义了从受到伤害到恢复开始之间的延迟时间（秒）。
	* Defines the delay in seconds between taking damage and the start of recovery.
	*
	* - 当 > 0: 启用延迟机制。每次受伤害都会重置此计时器，计时期间无法恢复。
	*   When > 0: Enables the delay mechanism. Each time damage is taken, this timer is reset, and regeneration is paused during this period.
	*
	* - 当 <= 0: 禁用延迟机制。恢复效果会在满足条件时立即开始，没有等待时间。
	*   When <= 0: Disables the delay mechanism. The recovery effect will begin immediately when conditions are met, with no waiting period.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_RegenerationCooldown)
	FGameplayAttributeData RegenerationCooldown;
	SAGA_ATTRIBUTE_ACCESSORS(RegenerationCooldown)
	UFUNCTION()
	void OnRep_RegenerationCooldown(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(RegenerationCooldown, OldValue);
	}

	/**
	* [Lock]状态的持续时间（秒）。
	* Duration of the [Lock] state in seconds.
	* - > 0: 锁定时长
	* - > 0: The duration of the lock.
	* - = 0: 锁定一帧
	* - = 0: Lock for one frame.
	* - < 0: 永久锁定（需要手动调用StopLockState）
	* - < 0: Permanent lock (requires manually calling StopLockState).
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_LockDuration)
	FGameplayAttributeData LockDuration;
	SAGA_ATTRIBUTE_ACCESSORS(LockDuration)
	UFUNCTION()
	void OnRep_LockDuration(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(LockDuration, OldValue);
	}


	/**
	* [Reset]状态下的恢复速率（每秒）。
	* The recovery rate per second in the [Reset] state.
	* - > 0: 在重置状态下的恢复速率
	* - > 0: The recovery rate in the reset state.
	* - <= 0: 立即恢复满
	* - <= 0: Instantly recovers to full.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_ResetRate)
	FGameplayAttributeData ResetRate;
	SAGA_ATTRIBUTE_ACCESSORS(ResetRate);
	UFUNCTION()
	void OnRep_ResetRate(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(ResetRate, OldValue);
	}

	/**
	* [Reset]状态下的免疫阈值。在当前值低于该阈值时免疫伤害。
	* The immunity threshold in the [Reset] state. Damage is ignored when the current value is below this threshold.
	* - > 0: 当前值低于该值时免疫
	* - > 0: Immune when the current value is below this threshold.
	* - = 0: (默认)没有免疫效果
	* - = 0: (Default) No immunity.
	* - < 0: 在整个[Reset]状态下都免疫
	* - < 0: Immune throughout the entire [Reset] state.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_ImmuneThreshold)
	FGameplayAttributeData ImmuneThreshold;
	SAGA_ATTRIBUTE_ACCESSORS(ImmuneThreshold);
	UFUNCTION()
	void OnRep_ImmuneThreshold(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(ImmuneThreshold, OldValue);
	}

	/*State of meter*/
	UPROPERTY(ReplicatedUsing=OnRep_MeterState, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Runtime")
	EMeterState MeterState;

public:
	
	UFUNCTION(BlueprintCallable, Category="Meter")
	void StopLockState();


	UFUNCTION(BlueprintCallable, Category="Meter")
	void StopResetState();

protected:
	friend class USGAbilitySystemComponent;
	
	UFUNCTION()
	void OnRep_MeterState(const EMeterState& OldValue);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnReduce_Implementation(const FSGAttributeSetExecutionData& Data) override;

	virtual void PostReduce() override;

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
