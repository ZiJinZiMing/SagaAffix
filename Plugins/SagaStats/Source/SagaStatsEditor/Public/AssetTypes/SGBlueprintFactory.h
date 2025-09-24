/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "SGBlueprintFactory.generated.h"

class USGAttributeSet;

/**
 * 
 */
UCLASS(HideCategories = Object, MinimalAPI)
class USGBlueprintFactory : public UFactory
{
	GENERATED_BODY()

public:
	USGBlueprintFactory(const FObjectInitializer& ObjectInitializer);

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ End UFactory Interface	

public:

	/** The type of blueprint that will be created */
	UPROPERTY(EditAnywhere, Category = SagaEditor)
	TEnumAsByte<enum EBlueprintType> BlueprintType;

	/** The parent class of the created blueprint */
	UPROPERTY(EditAnywhere, Category = SagaEditor)
	TSubclassOf<USGAttributeSet> ParentClass;
	
};
