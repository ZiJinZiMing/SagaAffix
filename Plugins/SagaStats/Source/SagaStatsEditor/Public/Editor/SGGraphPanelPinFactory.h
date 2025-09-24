/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "AttributeSet.h"
#include "EdGraphUtilities.h"
#include "EdGraphSchema_K2.h"
#include "SSGGameplayAttributeGraphPin.h"
#include "SGraphPin.h"

class FSGGraphPanelPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* InPin) const override
	{
		// TODO: Pass down containing owner to SSGGameplayAttributeGraphPin to be able to list only owned attributes
		// FBlueprintEditorUtils::FindBlueprintForNode(UEdGraphNode*)
		if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct
			&& InPin->PinType.PinSubCategoryObject == FGameplayAttribute::StaticStruct()
			&& InPin->Direction == EGPD_Input)
		{
			return SNew(SSGGameplayAttributeGraphPin, InPin);
		}
		
		return nullptr;
	}
};
