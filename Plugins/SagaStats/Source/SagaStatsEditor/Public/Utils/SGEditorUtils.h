/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"

class FSGAttributeSetBlueprintEditor;
class IDetailLayoutBuilder;
class UAttributeSet;
class UBlueprint;

namespace SGEditorUtils
{
	UAttributeSet* GetAttributeBeingCustomized(const IDetailLayoutBuilder& InDetailLayout);
	UBlueprint* GetBlueprintFromClass(const UClass* InClass);
	TWeakPtr<FSGAttributeSetBlueprintEditor> FindBlueprintEditorForAsset(UObject* InObject);
}
