/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "SagaMeterBase.h"
#include "SagaIncreaseMeter.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class SAGASTATS_API USagaIncreaseMeter : public USagaMeterBase
{
	GENERATED_BODY()

public:
	
	explicit USagaIncreaseMeter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	
	SAGA_ATTRIBUTE_ACCESSORS(Degeneration);
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Degeneration)
	FGameplayAttributeData Degeneration;
	UFUNCTION()
	void OnRep_Degeneration(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(Degeneration, OldValue);
	}

	SAGA_ATTRIBUTE_ACCESSORS(DegenerationCooldown);
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_DegenerationCooldown)
	FGameplayAttributeData DegenerationCooldown;
	UFUNCTION()
	void OnRep_DegenerationCooldown(const FGameplayAttributeData& OldValue)
	{
		SAGA_GAMEPLAYATTRIBUTE_REPNOTIFY(DegenerationCooldown, OldValue);
	}
	
protected:

	virtual void InitFromMetaDataTable(const UDataTable* DataTable) override;
	
	virtual void OnAccumulate_Implementation(const FSagaAttributeSetExecutionData& Data) override;
	
	
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool CanDegeneration() const;

	UPROPERTY(transient,VisibleInstanceOnly,BlueprintReadOnly, Category="Runtime")
	FTimerHandle DegenerationCooldownTimer;

	UFUNCTION()
	void OnDegenerationCooldownTimerFinish();
	
};
