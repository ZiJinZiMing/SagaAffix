/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "SSSVariableRowWidget.h"

#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

// void SSSVariableRowWidget::Construct(const FArguments& InArgs, const TSharedPtr<SWidget>& InNameContent, const TSharedPtr<SWidget>& InValueContent)
void SSSVariableRowWidget::Construct(const FArguments& InArgs)
{
	const TSharedPtr<SHorizontalBox> Content = SNew(SHorizontalBox);

	// Name content
	Content->AddSlot()
		.MaxWidth(InArgs._NameMaxWidth)
		.Padding(InArgs._NamePadding)
		.VAlign(InArgs._VAlign)
		.HAlign(InArgs._HAlign)
		[
			InArgs._NameContent.Widget
		];

	// Value content
	SHorizontalBox::FScopedWidgetSlotArguments Slot = Content->AddSlot();
	Slot.Padding(InArgs._ValuePadding);
	if (InArgs._FillWidth.IsSet())
	{
		Slot.FillWidth(InArgs._FillWidth);
	}

	Slot
	[
		SNew(SBox)
		.HeightOverride(InArgs._EditableTextHeight)
		[
			InArgs._ValueContent.Widget
		]
	];
	

	ChildSlot[
		Content.ToSharedRef()
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
