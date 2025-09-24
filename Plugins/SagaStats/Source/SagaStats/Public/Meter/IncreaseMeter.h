/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "MeterBase.h"
#include "IncreaseMeter.generated.h"

/**
 * @class UIncreaseMeter
 * @brief 一种随时间衰减的计量条 (A meter that degenerates over time).
 *
 * @details
 * 该类继承自 UMeterBase，并额外实现了一种“衰减”机制。
 * This class inherits from UMeterBase and adds a "degeneration" mechanism.
 *
 * 主要用于实现那些在没有持续获得时会随时间减少的数值，例如连击点数、临时的护盾或过热值等。
 * It is primarily used for values that should decrease over time if not actively maintained, such as combo points, temporary shields, or overheat values.
 *
 * 核心行为 (Core Behavior):
 * 1. 当通过 "Accumulate" 属性为该计量条增加数值时，会重置一个“衰减冷却”计时器。
 *    When the meter's value is increased via the "Accumulate" attribute, a "degeneration cooldown" timer is reset.
 * 2. 在冷却计时器激活期间，计量条的数值不会发生变化。
 *    The meter's value will not change while this cooldown timer is active.
 * 3. 当冷却计时器结束后，计量条会根据 "Degeneration" 属性设定的速率开始持续减少，直到归零。
 *    After the cooldown timer finishes, the meter will begin to decrease continuously at the rate defined by the "Degeneration" attribute, until it reaches zero.
 *
 * @see UMeterBase
 */
UCLASS(Abstract)
class SAGASTATS_API UIncreaseMeter : public UMeterBase
{
	GENERATED_BODY()

public:
	
	explicit UIncreaseMeter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	
	/**
	* 计量条的衰减速率（每秒）。
	* The rate of degeneration for the meter (per second).
	* - 如果 > 0，计量条将在冷却结束后，以该速率持续衰减，直到归零。
	* - If > 0, the meter will continuously decrease at this rate after the cooldown ends, until it reaches zero.
	* - 如果 <= 0，则没有持续衰减效果。但是，若 DegenerationCooldown > 0，则在冷却结束后，计量条的当前值会立即被重置为最小值。
	* - If <= 0, there is no continuous degeneration. However, if DegenerationCooldown > 0, the meter's current value will be immediately reset to its minimum value after the cooldown ends.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Degeneration)
	FGameplayAttributeData Degeneration;
	SAGA_ATTRIBUTE_ACCESSORS(Degeneration);
	UFUNCTION()
	void OnRep_Degeneration(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(Degeneration, OldValue);
	}

	/**
	* 距离上次累加后，开始衰减前的冷却时间（秒）。
	* The cooldown time in seconds before degeneration starts after the last accumulation.
	* - 如果 > 0，每次累加都会重置这个冷却计时器。在计时器激活期间，计量条不会衰减。
	* - If > 0, each accumulation will reset this cooldown timer. The meter will not degenerate while the timer is active.
	* - 如果 <= 0，则没有冷却效果，计量条会在满足条件时立即开始衰减。
	* - If <= 0, there is no cooldown effect, and the meter will start degenerating immediately when conditions are met.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_DegenerationCooldown)
	FGameplayAttributeData DegenerationCooldown;
	SAGA_ATTRIBUTE_ACCESSORS(DegenerationCooldown);
	UFUNCTION()
	void OnRep_DegenerationCooldown(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(DegenerationCooldown, OldValue);
	}
	
protected:
	friend class USGAbilitySystemComponent;

	virtual void InitFromMetaDataTable(const UDataTable* DataTable) override;
	
	virtual void OnAccumulate_Implementation(const FSGAttributeSetExecutionData& Data) override;

	virtual void PostAccumulate() override;
	
	
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool CanDegeneration() const;

	UPROPERTY(transient,VisibleInstanceOnly,BlueprintReadOnly, Category="Runtime")
	FTimerHandle DegenerationCooldownTimer;

	UFUNCTION()
	void OnDegenerationCooldownTimerFinish();
	
};
