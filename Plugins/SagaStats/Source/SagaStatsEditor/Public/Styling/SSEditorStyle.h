/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class SAGASTATSEDITOR_API FSSEditorStyle : public FSlateStyleSet
{
public:
	FSSEditorStyle();
	virtual ~FSSEditorStyle() override;

	static FSSEditorStyle& Get()
	{
		static FSSEditorStyle Inst;
		return Inst;
	}
};
