/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Styling/SSAppStyle.h"

#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_NEWER_THAN(5, 1, -1)
#include "Styling/AppStyle.h"
#else
#include "EditorStyleSet.h"
#endif

const FSlateBrush* FSSAppStyle::GetBrush(const FName& InPropertyName, const ANSICHAR* InSpecifier)
{
#if UE_VERSION_NEWER_THAN(5, 1, -1)
	return FAppStyle::GetBrush(InPropertyName, InSpecifier);
#else
	return FEditorStyle::GetBrush(InPropertyName, InSpecifier);
#endif
}

FSlateFontInfo FSSAppStyle::GetFontStyle(const FName& InPropertyName, const ANSICHAR* InSpecifier)
{
#if UE_VERSION_NEWER_THAN(5, 1, -1)
	return FAppStyle::GetFontStyle(InPropertyName, InSpecifier);
#else
	return FEditorStyle::GetFontStyle(InPropertyName, InSpecifier);
#endif
}
