/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Styling/SGEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FSGEditorStyle::FSGEditorStyle()
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

FSGEditorStyle::~FSGEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}
