/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "VisualLogger/VisualLogger.h"
#include "AbilitySystemComponent.h"
#include "SagaAttributeSet.h"
#include "TickableAttributeSetInterface.h"
#include "SagaMeterBase.generated.h"

#define METER_MINIMUM 0


/**
 * 
 */
UCLASS(abstract, meta = (HideInDetailsView))
class SAGASTATS_API USagaMeterBase : public USagaAttributeSet, public ITickableAttributeSetInterface
{
	GENERATED_BODY()

public:
	USagaMeterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	SAGA_ATTRIBUTE_ACCESSORS(Current);
	/*Current value of meter*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Current, meta=(HideFromModifiers))
	FSagaClampedGameplayAttributeData Current;
	UFUNCTION()
	void OnRep_Current(const FSagaClampedGameplayAttributeData& OldValue)
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

	SAGA_ATTRIBUTE_ACCESSORS(Accumulate);
	UPROPERTY(EditDefaultsOnly, Category="Meter")
	FGameplayAttributeData Accumulate;

	SAGA_ATTRIBUTE_ACCESSORS(ImpactedAccumulate);
	UPROPERTY(EditDefaultsOnly, Category="Meter", meta=(HideFromModifiers))
	FGameplayAttributeData ImpactedAccumulate;

	SAGA_ATTRIBUTE_ACCESSORS(Reduce);
	UPROPERTY(EditDefaultsOnly, Category="Meter")
	FGameplayAttributeData Reduce;

	SAGA_ATTRIBUTE_ACCESSORS(ImpactedReduce);
	UPROPERTY(EditDefaultsOnly, Category="Meter", meta=(HideFromModifiers))
	FGameplayAttributeData ImpactedReduce;


public:
	static bool GreaterOrNearlyEqual(float A, float B);

	static bool LessOrNearlyEqual(float A, float B);

	UFUNCTION(BlueprintCallable, Category="Meter")
	static const USagaMeterBase* GetMeter(AActor* Actor, TSubclassOf<USagaMeterBase> MeterClass);

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
	void OnAccumulate(const FSagaAttributeSetExecutionData& Data);
	virtual void OnAccumulate_Implementation(const FSagaAttributeSetExecutionData& Data);

	UFUNCTION(BlueprintNativeEvent)
	void OnReduce(const FSagaAttributeSetExecutionData& Data);
	virtual void OnReduce_Implementation(const FSagaAttributeSetExecutionData& Data);

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
