/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include "ViewModel/ISGSlateViewModel.h"


/**
 * Implements a view model for the new attribute BP widget.
 */
class FSGNewAttributeViewModel : public ISGSlateViewModel
{
public:
	/** The name of the variable to create */
	DECLARE_VIEWMODEL_PROPERTY(FString, VariableName);

	/** Extra information about this variable, shown when cursor is over it */
	DECLARE_VIEWMODEL_PROPERTY(FString, Description);

	/**
	 * Should this Variable be replicated over the network?
	 *
	 * You may want to turn this off if you're dealing with "meta" attributes.
	 */
	DECLARE_VIEWMODEL_PROPERTY(bool, bIsReplicated);

	/** The asset currently being displayed by the header view */
	DECLARE_VIEWMODEL_PROPERTY(TWeakObjectPtr<UBlueprint>, CustomizedBlueprint);

	/** The pin type to choose from the available Gameplay Attribute Data allowed types */
	DECLARE_VIEWMODEL_PROPERTY(FEdGraphPinType, PinType)
	
	/** Default constructor */
	FSGNewAttributeViewModel();

	virtual void Initialize() override;

	FString ToString() const;
};
