/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Styling/SSEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FSSEditorStyle::FSSEditorStyle()
	: FSlateStyleSet(TEXT("SSEditorStyleSet"))
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("SagaStats"));
	check(Plugin.IsValid());

	const FString PluginContentDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources/Icons")));
	FSlateStyleSet::SetContentRoot(PluginContentDir);

	const FVector2D Icon16X16 = FVector2D(16.0);
	Set(
		"Icons.HeaderView",
		new FSlateVectorImageBrush(FSlateStyleSet::RootToContentDir("BlueprintHeader_16", TEXT(".svg")), Icon16X16)
	);

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FSSEditorStyle::~FSSEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}
