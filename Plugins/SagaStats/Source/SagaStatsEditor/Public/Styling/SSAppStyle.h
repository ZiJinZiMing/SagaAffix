/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"

struct FSlateBrush;
struct FSlateFontInfo;

/**
 * AppStyle class specific to this module
 *
 * Wraps access to either FAppStyle or FEditorStyle based on engine version.
 */
class SAGASTATSEDITOR_API FSSAppStyle
{
public:
	static const FSlateBrush* GetBrush(const FName& InPropertyName, const ANSICHAR* InSpecifier = nullptr);
	static FSlateFontInfo GetFontStyle(const FName& InPropertyName, const ANSICHAR* InSpecifier = nullptr);
};
