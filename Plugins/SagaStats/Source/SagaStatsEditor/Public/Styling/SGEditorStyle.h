/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class SAGASTATSEDITOR_API FSGEditorStyle : public FSlateStyleSet
{
public:
	FSGEditorStyle();
	virtual ~FSGEditorStyle() override;

	static FSGEditorStyle& Get()
	{
		static FSGEditorStyle Inst;
		return Inst;
	}
};
