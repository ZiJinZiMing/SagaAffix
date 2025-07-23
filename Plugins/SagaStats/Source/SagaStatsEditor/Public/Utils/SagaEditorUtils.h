/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"

class FSagaAttributeSetBlueprintEditor;
class IDetailLayoutBuilder;
class UAttributeSet;
class UBlueprint;

namespace SagaEditorUtils
{
	UAttributeSet* GetAttributeBeingCustomized(const IDetailLayoutBuilder& InDetailLayout);
	UBlueprint* GetBlueprintFromClass(const UClass* InClass);
	TWeakPtr<FSagaAttributeSetBlueprintEditor> FindBlueprintEditorForAsset(UObject* InObject);
}
