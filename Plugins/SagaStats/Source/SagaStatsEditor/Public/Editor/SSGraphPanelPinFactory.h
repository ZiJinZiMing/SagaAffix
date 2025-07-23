/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "AttributeSet.h"
#include "EdGraphUtilities.h"
#include "EdGraphSchema_K2.h"
#include "SSSGameplayAttributeGraphPin.h"
#include "SGraphPin.h"

class FSSGraphPanelPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* InPin) const override
	{
		// TODO: Pass down containing owner to SSSGameplayAttributeGraphPin to be able to list only owned attributes
		// FBlueprintEditorUtils::FindBlueprintForNode(UEdGraphNode*)
		if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct
			&& InPin->PinType.PinSubCategoryObject == FGameplayAttribute::StaticStruct()
			&& InPin->Direction == EGPD_Input)
		{
			return SNew(SSSGameplayAttributeGraphPin, InPin);
		}
		
		return nullptr;
	}
};
