/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "SGraphPin.h"
#include "UObject/WeakFieldPtr.h"

class SSGGameplayAttributeGraphPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SSGGameplayAttributeGraphPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	//~ Begin SGraphPin Interface
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
	//~ End SGraphPin Interface

	//Feature Begin Attribute In subclass of AttributeSet
	void OnAttributeChanged(FProperty* SelectedAttribute, UClass* InAttributeOwnerClass);
	//Feature End

	TWeakFieldPtr<FProperty> LastSelectedProperty;

private:
	bool GetDefaultValueIsEnabled() const
	{
		return !GraphPinObj->bDefaultValueIsReadOnly;
	}
};
