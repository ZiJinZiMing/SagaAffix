/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "SagaStatsType.h"
#include "AbilitySystemComponent.h"
#include "SagaAbilitySystemComponent.generated.h"

class UDamageProcessUnit;
class USagaMeterBase;
class USagaDecreaseMeter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttributeSetAddOrRemoveEvent, UAttributeSet* /*AttributeSet*/, bool /*IsAdd*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeterEmptiedEvent, USagaMeterBase* /*Meter*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeterFilledEvent, USagaMeterBase* /*Meter*/)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMeterStateChangeEvent, USagaDecreaseMeter* /*Meter*/, EMeterState /*OldState*/)


UCLASS(ClassGroup=(AbilitySystem), meta=(BlueprintSpawnableComponent))
class SAGASTATS_API USagaAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Attribute")
	void RemoveAttributeSet(UAttributeSet* AttributeSet);

	UFUNCTION(BlueprintCallable, Category="Attribute")
	void RemoveAttributeSetByClass(TSubclassOf<UAttributeSet> AttributeSetClass);

	FOnAttributeSetAddOrRemoveEvent& GetAttributeSetAddOrRemoveDelegate(TSubclassOf<UAttributeSet> SetClass);
	
	FOnMeterEmptiedEvent& GetMeterEmptiedDelegate(TSubclassOf<USagaMeterBase> MeterClass);
	
	FOnMeterFilledEvent& GetMeterFilledDelegate(TSubclassOf<USagaMeterBase> MeterClass);
	
	FOnMeterStateChangeEvent& GetMeterStateChangeDelegate(TSubclassOf<USagaDecreaseMeter> MeterClass);
	
protected:
	TMap<TSubclassOf<UAttributeSet>, FOnAttributeSetAddOrRemoveEvent> AttributeSetAddOrRemoveDelegates;
	
	TMap<TSubclassOf<USagaMeterBase>, FOnMeterEmptiedEvent> MeterEmptiedDelegates;

	TMap<TSubclassOf<USagaMeterBase>, FOnMeterFilledEvent> MeterFilledDelegates;
	
	TMap<TSubclassOf<USagaDecreaseMeter>, FOnMeterStateChangeEvent> MeterStateChangeDelegates;
	
protected:
	/** Add a new attribute set */
	virtual void AddSpawnedAttribute(UAttributeSet* AttributeSet) override;

	/** Remove an existing attribute set */
	virtual void RemoveSpawnedAttribute(UAttributeSet* AttributeSet) override;

	/** Remove all attribute sets */
	virtual void RemoveAllSpawnedAttributes() override;

	virtual void OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes) override;


	// ----------------------------------------------------------------------------------------------------------------
	//	Affix
	// ----------------------------------------------------------------------------------------------------------------

public:
	const TArray<TObjectPtr<UDamageProcessUnit>>& GetDamageProcessUnits() const { return DamageProcessUnits; };

	TArray<TObjectPtr<UDamageProcessUnit>>& GetDamageProcessUnits_Mutable() { return DamageProcessUnits; };
	
	
protected:
	/*动态DamageProcessUnit*/
	UPROPERTY(Transient,BlueprintReadOnly, Category="Damage")
	TArray<TObjectPtr<UDamageProcessUnit>> DamageProcessUnits;


	// ----------------------------------------------------------------------------------------------------------------
	//	Abilities
	// ----------------------------------------------------------------------------------------------------------------

	
};
