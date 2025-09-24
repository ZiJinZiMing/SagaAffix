/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Styling/SGAppStyle.h"

#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_NEWER_THAN(5, 1, -1)
#include "Styling/AppStyle.h"
#else
#include "EditorStyleSet.h"
#endif

const FSlateBrush* FSGAppStyle::GetBrush(const FName& InPropertyName, const ANSICHAR* InSpecifier)
{
#if UE_VERSION_NEWER_THAN(5, 1, -1)
	return FAppStyle::GetBrush(InPropertyName, InSpecifier);
#else
	return FEditorStyle::GetBrush(InPropertyName, InSpecifier);
#endif
}

FSlateFontInfo FSGAppStyle::GetFontStyle(const FName& InPropertyName, const ANSICHAR* InSpecifier)
{
#if UE_VERSION_NEWER_THAN(5, 1, -1)
	return FAppStyle::GetFontStyle(InPropertyName, InSpecifier);
#else
	return FEditorStyle::GetFontStyle(InPropertyName, InSpecifier);
#endif
}
