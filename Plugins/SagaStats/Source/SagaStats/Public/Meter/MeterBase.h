/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "VisualLogger/VisualLogger.h"
#include "AbilitySystemComponent.h"
#include "TickableAttributeSetInterface.h"
#include "AttributeSet/SGAttributeSet.h"
#include "MeterBase.generated.h"

#define METER_MINIMUM 0


/**
 * @class UMeterBase
 * @brief 通用的计量条属性集基类 (The base AttributeSet for meters like Health, Mana, etc.).
 *
 * @details
 * 该类为所有“计量条”类型（如生命值、法力值、体力值）的属性集提供了基础功能。
 * This class provides the basic functionality for all "meter" type AttributeSets (e.g., Health, Mana, Stamina).
 *
 * 核心属性 (Core Attributes):
 * - Current: 计量条的当前值，其值会被钳制在 [0, Maximum] 范围内。
 *   (The current value of the meter, clamped between [0, Maximum]).
 * - Maximum: 计量条的最大值。
 *   (The maximum value of the meter).
 *
 * 设计模式 (Design Pattern):
 * 本类的核心设计是使用 "Accumulate" 和 "Reduce" 作为瞬时输入属性。外部GameplayEffect应该修改这两个属性，而不是直接修改 "Current"。
 * The core design uses "Accumulate" and "Reduce" as transient input attributes. External GameplayEffects should modify these attributes instead of "Current" directly.
 *
 * 在 PostGameplayEffectExecute 中，当检测到 Accumulate 或 Reduce 属性被修改后，会触发相应的蓝图事件 OnAccumulate / OnReduce，然后立刻将这两个属性清零。
 * In PostGameplayEffectExecute, after detecting a modification to Accumulate or Reduce, the corresponding Blueprint events (OnAccumulate / OnReduce) are fired,
 * and these attributes are then immediately reset to zero.
 *
 * 使用方法 (Usage):
 * 1. 创建一个 GameplayEffect 来修改 Accumulate (用于增加) 或 Reduce (用于减少) 的值。
 *    Create a GameplayEffect to modify the value of Accumulate (for increases) or Reduce (for decreases).
 * 2. 在该类的蓝图或C++子类中实现 OnAccumulate / OnReduce 事件，以定义如何根据输入值最终修改 Current 值。
 *    Implement the OnAccumulate / OnReduce events in a Blueprint or C++ subclass to define how the Current value is ultimately modified based on the input.
 *
 * @see UIncreaseMeter, UDecreaseMeter
 */
UCLASS(Abstract, meta = (HideInDetailsView))
class SAGASTATS_API UMeterBase : public USGAttributeSet, public ITickableAttributeSetInterface
{
	GENERATED_BODY()

public:
	UMeterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	SAGA_ATTRIBUTE_ACCESSORS(Current);
	/*Current value of meter*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Current, meta=(HideFromModifiers))
	FSGClampedGameplayAttributeData Current;
	UFUNCTION()
	void OnRep_Current(const FSGClampedGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(Current, OldValue);
	}

	SAGA_ATTRIBUTE_ACCESSORS(Maximum);
	/* Maximum of meter*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Maximum)
	FGameplayAttributeData Maximum;
	UFUNCTION()
	void OnRep_Maximum(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(Maximum, OldValue);
	}

	UPROPERTY(EditDefaultsOnly, Category="Meter")
	FGameplayAttributeData Accumulate;
	SAGA_ATTRIBUTE_ACCESSORS(Accumulate);
	
	UPROPERTY(EditDefaultsOnly, Category="Meter", meta=(HideFromModifiers))
	FGameplayAttributeData ImpactedAccumulate;
	SAGA_ATTRIBUTE_ACCESSORS(ImpactedAccumulate);

	UPROPERTY(EditDefaultsOnly, Category="Meter")
	FGameplayAttributeData Reduce;
	SAGA_ATTRIBUTE_ACCESSORS(Reduce);

	UPROPERTY(EditDefaultsOnly, Category="Meter", meta=(HideFromModifiers))
	FGameplayAttributeData ImpactedReduce;
	SAGA_ATTRIBUTE_ACCESSORS(ImpactedReduce);

	/*在PostGameplayEffectExecute中执行结算*/
	/*todo:ManualUpdateCurrent*/
	UPROPERTY(Transient, BlueprintReadWrite, VisibleAnywhere, Category="Meter")
	uint8 bManualUpdateCurrent : 1;
	
public:
	static bool GreaterOrNearlyEqual(float A, float B);

	static bool LessOrNearlyEqual(float A, float B);

	UFUNCTION(BlueprintCallable, Category="Meter")
	static const UMeterBase* GetMeter(AActor* Actor, TSubclassOf<UMeterBase> MeterClass);

	UFUNCTION(BlueprintPure, Category="Meter")
	bool IsFilled() const;

	UFUNCTION(BlueprintPure, Category="Meter")
	bool IsEmptied() const;

protected:
	//~begin UAttributeSet interface

	/** Called just after any modification happens to an attribute. */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	//~end UAttributeSet interface

	UFUNCTION(BlueprintPure, Category="Meter")
	float GetPercent() const;

	UFUNCTION(BlueprintNativeEvent, Category="Meter")
	void OnEmptied();
	virtual void OnEmptied_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category="Meter")
	void OnFilled();
	virtual void OnFilled_Implementation();

	virtual void OnMaximumChanged(float OldValue, float NewValue);

	virtual void OnCurrentChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintNativeEvent)
	void OnAccumulate(const FSGAttributeSetExecutionData& Data);
	virtual void OnAccumulate_Implementation(const FSGAttributeSetExecutionData& Data);

	UFUNCTION(BlueprintNativeEvent)
	void OnReduce(const FSGAttributeSetExecutionData& Data);
	virtual void OnReduce_Implementation(const FSGAttributeSetExecutionData& Data);

	/** Called after an Accumulate effect has been executed. */
	virtual void PostAccumulate();

	/** Called after a Reduce effect has been executed. */
	virtual void PostReduce();

	/**/
	UFUNCTION(BlueprintCallable)
	virtual void PostDamageProcess(float CurrentValue, float PrevCurrentValue, const FSGAttributeSetExecutionData& Data);

	//~begin ITickableAttributeSetInterface interface

	/**
	* Does this attribute set need to tick?
	*
	* @return true if this attribute set should currently be ticking, false otherwise.
	*/
	virtual bool ShouldTick() const override;
	/**
	 * Ticks the attribute set by DeltaTime seconds
	 * 
	 * @param DeltaTime Size of the time step in seconds.
	 */
	virtual void Tick(float DeltaTime) override;
	//~end ITickableAttributeSetInterface interface


	//~ Begin UObject interface
	/** Returns properties that are replicated for the lifetime of the actor channel */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ End UObject interface
};
