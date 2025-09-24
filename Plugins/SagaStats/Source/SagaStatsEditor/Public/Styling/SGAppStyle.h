/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"

struct FSlateBrush;
struct FSlateFontInfo;

/**
 * AppStyle class specific to this module
 *
 * Wraps access to either FAppStyle or FEditorStyle based on engine version.
 */
class SAGASTATSEDITOR_API FSGAppStyle
{
public:
	static const FSlateBrush* GetBrush(const FName& InPropertyName, const ANSICHAR* InSpecifier = nullptr);
	static FSlateFontInfo GetFontStyle(const FName& InPropertyName, const ANSICHAR* InSpecifier = nullptr);
};
