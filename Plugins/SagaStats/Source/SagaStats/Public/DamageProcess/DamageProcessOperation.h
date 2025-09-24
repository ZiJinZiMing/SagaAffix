/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "DAG/DAGNode.h"
#include "DamageProcessOperation.generated.h"


UENUM(BlueprintType)
namespace EDamageProcessModOp
{
	/**
	 * Defines the ways that mods will modify attributes. Values of the same type are aggregated, and then applied in the following equation:
	 * ((BaseValue + AddBase) * MultiplyAdditive / DivideAdditive * MultiplyCompound) + AddFinal
	 */
	enum Type : int
	{
		BaseValue = 0 UMETA(DisplayName="BaseValue"),

		/** Adds to the Base value. This happens first, before all other mods are considered. */
		AddBase UMETA(DisplayName="Add (Base)"),

		/** Multipliers are added together first, then multiplied against prev result. E.g. 50% + 50% = 100% in values is 1.5 + 1.5 = 2.0. */
		MultiplyAdditive UMETA(DisplayName = "Multiply (Additive)"),

		/** Multiply the prev result by this value. E.g. two values of 1.5 compounded: 1.5 * 1.5 = 2.25. */
		MultiplyCompound UMETA(DisplayName = "Multiply (Compound)"),

		/** Add this value to the final computed result. */
		AddFinal UMETA(DisplayName = "Add (Final)"),

		/** This must always be the last value (used in iteration code). */
		Max UMETA(Hidden, DisplayName="Invalid"),
	};
}


USTRUCT(BlueprintType)
struct SAGASTATS_API FDamageProcessModifierInfo
{
	GENERATED_USTRUCT_BODY()

	/** The Attribute we modify or the GE we modify modifies. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute Attribute;

	/**
	 * The numeric operation of this modifier: Override, Add, Multiply, etc
	 * When multiple modifiers aggregate together, the equation is:
	 * ((BaseValue + AddBase) * MultiplyAdditive / DivideAdditive * MultiplyCompound) + AddFinal
	 */
	UPROPERTY(EditDefaultsOnly, Category=GameplayModifier)
	TEnumAsByte<EDamageProcessModOp::Type> ModifierOp = EDamageProcessModOp::BaseValue;

	/** Equality/Inequality operators */
	bool operator==(const FDamageProcessModifierInfo& Other) const;
	bool operator!=(const FDamageProcessModifierInfo& Other) const;
};


/**
 * 
 */
UCLASS()
class SAGASTATS_API UDamageProcessOperation : public UDAGNode
{
	GENERATED_BODY()

public:

	//todo：需要保证Attribute唯一
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDamageProcessModifierInfo> Modifiers;

	UPROPERTY(visibleAnywhere, BlueprintReadOnly)
	TArray<float> ModifierMagnitudes;

	/*Index决定执行顺序*/
	UPROPERTY(editAnywhere, BlueprintReadOnly)
	TArray<FGameplayAttribute> BackingAttributes;

	UPROPERTY(EditAnywhere)
	FString OperationName;
	
	UFUNCTION(BlueprintCallable)
	int GetAttributeMagnitude(FGameplayAttribute Attribute,float& OutMagnitude) const;

	UFUNCTION(BlueprintCallable)
	int SetAttributeMagnitude(FGameplayAttribute Attribute, float InMagnitude);

	UPROPERTY(BlueprintReadOnly)
	AActor* OwnerActor;
	
protected:
	// 蓝图事件 / Blueprint events
	UFUNCTION(BlueprintImplementableEvent, Category = "DAG")
	void OnNodeExecuted();

	virtual void PostInitProperties() override;
	
	virtual void GetOrderedChildren(TArray<UDAGNode*>& OutOrderedChildren) const override;

	virtual FString GetNodeDisplayName() const override;

	virtual FString GetMermaidNodeShape(const FString& NodeLabel) const override;
	
	virtual void Execute() override;
};
