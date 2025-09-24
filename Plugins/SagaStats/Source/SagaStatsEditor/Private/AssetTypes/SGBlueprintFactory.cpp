/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#include "AssetTypes/SGBlueprintFactory.h"
#include "Misc/MessageDialog.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Editor.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "BlueprintEditorSettings.h"
#include "AttributeSet/SGAttributeSet.h"
#include "AttributeSet/SGAttributeSetBlueprint.h"


#define LOCTEXT_NAMESPACE "USGBlueprintFactory"

// ------------------------------------------------------------------------------
// Dialog to configure creation properties
// ------------------------------------------------------------------------------

class SSGBlueprintCreateDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSGBlueprintCreateDialog)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs)
	{
		bOkClicked = false;
		ParentClass = USGAttributeSet::StaticClass();

		ChildSlot
		[
			SNew(SBorder)
			.Visibility(EVisibility::Visible)
			.BorderImage(FAppStyle::GetBrush("Menu.Background"))
			[
				SNew(SBox)
				.Visibility(EVisibility::Visible)
				.WidthOverride(500.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						.Content()
						[
							SAssignNew(ParentClassContainer, SVerticalBox)
						]
					]

					// Ok/Cancel buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(8)
					[
						SNew(SUniformGridPanel)
						.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))
						.MinDesiredSlotWidth(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
						.MinDesiredSlotHeight(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
							.OnClicked(this, &SSGBlueprintCreateDialog::OkClicked)
							.Text(LOCTEXT("CreateSGBlueprintOk", "OK"))
						]
						+ SUniformGridPanel::Slot(1, 0)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
							.OnClicked(this, &SSGBlueprintCreateDialog::CancelClicked)
							.Text(LOCTEXT("CreateSGBlueprintCancel", "Cancel"))
						]
					]
				]
			]
		];

		MakeParentClassPicker();
	}

	/** Sets properties for the supplied SSBlueprintFactory */
	bool ConfigureProperties(TWeakObjectPtr<USGBlueprintFactory> InSSBlueprintFactory)
	{
		SSBlueprintFactory = InSSBlueprintFactory;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("CreateSagaBlueprintOptions", "Create SGAttributeSet"))
			.ClientSize(FVector2D(400, 700))
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			[
				AsShared()
			];

		PickerWindow = Window;

		GEditor->EditorAddModalWindow(Window);
		SSBlueprintFactory.Reset();

		return bOkClicked;
	}

private:
	class FSSBlueprintParentFilter : public IClassViewerFilter
	{
	public:
		/** All children of these classes will be included unless filtered out by another setting. */
		TSet<const UClass*> AllowedChildrenOfClasses;

		FSSBlueprintParentFilter()
		{
		}

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			// If it appears on the allowed child-of classes list (or there is nothing on that list)
			return InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			// If it appears on the allowed child-of classes list (or there is nothing on that list)
			return InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
		}
	};

	/** Creates the combo menu for the parent class */
	void MakeParentClassPicker()
	{
		// Load the classviewer module to display a class picker
		FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

		// Fill in options
		FClassViewerInitializationOptions Options;
		Options.Mode = EClassViewerMode::ClassPicker;

		// Only allow parenting to base blueprints.
		Options.bIsBlueprintBaseOnly = true;

		TSharedPtr<FSSBlueprintParentFilter> Filter = MakeShareable(new FSSBlueprintParentFilter);
		Options.ClassFilters.Add(Filter.ToSharedRef());

		// All child child classes of USS are valid.
		Filter->AllowedChildrenOfClasses.Add(USGAttributeSet::StaticClass());

		ParentClassContainer->ClearChildren();
		ParentClassContainer->AddSlot()
		                    .AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ParentClass", "Parent Class:"))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		];

		ParentClassContainer->AddSlot()
		[
			ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SSGBlueprintCreateDialog::OnClassPicked))
		];
	}

	/** Handler for when a parent class is selected */
	void OnClassPicked(UClass* ChosenClass)
	{
		ParentClass = ChosenClass;
	}

	/** Handler for when ok is clicked */
	FReply OkClicked()
	{
		if (SSBlueprintFactory.IsValid())
		{
			SSBlueprintFactory->BlueprintType = BPTYPE_Normal;
			SSBlueprintFactory->ParentClass = ParentClass.Get();
		}

		CloseDialog(true);

		return FReply::Handled();
	}

	void CloseDialog(bool bWasPicked = false)
	{
		bOkClicked = bWasPicked;
		if (PickerWindow.IsValid())
		{
			PickerWindow.Pin()->RequestDestroyWindow();
		}
	}

	/** Handler for when cancel is clicked */
	FReply CancelClicked()
	{
		CloseDialog();
		return FReply::Handled();
	}

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			CloseDialog();
			return FReply::Handled();
		}
		return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}

private:
	/** The factory for which we are setting up properties */
	TWeakObjectPtr<USGBlueprintFactory> SSBlueprintFactory;

	/** A pointer to the window that is asking the user to select a parent class */
	TWeakPtr<SWindow> PickerWindow;

	/** The container for the Parent Class picker */
	TSharedPtr<SVerticalBox> ParentClassContainer;

	/** The selected class */
	TWeakObjectPtr<UClass> ParentClass;

	/** True if Ok was clicked */
	bool bOkClicked = false;
};

/*------------------------------------------------------------------------------
USGBlueprintFactory implementation.
------------------------------------------------------------------------------*/

USGBlueprintFactory::USGBlueprintFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), BlueprintType()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = USGAttributeSetBlueprint::StaticClass();
	ParentClass = USGAttributeSet::StaticClass();
}

bool USGBlueprintFactory::ConfigureProperties()
{
	TSharedRef<SSGBlueprintCreateDialog> Dialog = SNew(SSGBlueprintCreateDialog);
	return Dialog->ConfigureProperties(this);
};

UObject* USGBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	// Make sure we are trying to factory a Agile FSM blueprint, then create and init one
	check(Class->IsChildOf(USGAttributeSetBlueprint::StaticClass()));

	// If they selected an interface, force the parent class to be UInterface
	if (BlueprintType == BPTYPE_Interface)
	{
		ParentClass = UInterface::StaticClass();
	}

	if ((ParentClass == nullptr) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass) || !ParentClass->IsChildOf(USGAttributeSet::StaticClass()))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString(ParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateSagaBlueprint", "Cannot create a SagaAttributeSet based on the class '{ClassName}'."), Args));
		return nullptr;
	}
	else
	{
		USGAttributeSetBlueprint* NewBP = CastChecked<USGAttributeSetBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BlueprintType, USGAttributeSetBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext));

		if (NewBP)
		{
			const UBlueprintEditorSettings* Settings = GetMutableDefault<UBlueprintEditorSettings>();
			if (Settings && Settings->bSpawnDefaultBlueprintNodes)
			{
				int32 NodePositionY = 0;
				UEdGraph* EventGraph = NewBP->UbergraphPages[0];

				// Those are the 3 non-const, no return values events. They appear in event graph
				FKismetEditorUtilities::AddDefaultEventNode(NewBP, EventGraph, FName(TEXT("K2_PostGameplayEffectExecute")), USGAttributeSet::StaticClass(), NodePositionY);
				FKismetEditorUtilities::AddDefaultEventNode(NewBP, EventGraph, FName(TEXT("K2_PostAttributeChange")), USGAttributeSet::StaticClass(), NodePositionY);

				// Those are implemented as overridable functions, cause they're either const or return values
				// And AddDefaultEventNode won't work for them (well the nodes are created but will complain on the first compilation if wired
				// FKismetEditorUtilities::AddDefaultEventNode(Blueprint, EventGraph, FName(TEXT("K2_PreAttributeChange")), USGAttributeSet::StaticClass(), NodePositionY);
				// FKismetEditorUtilities::AddDefaultEventNode(Blueprint, EventGraph, FName(TEXT("K2_PreAttributeBaseChange")), USGAttributeSet::StaticClass(), NodePositionY);
				// FKismetEditorUtilities::AddDefaultEventNode(Blueprint, EventGraph, FName(TEXT("K2_PreAttributeChange")), USGAttributeSet::StaticClass(), NodePositionY);
				// FKismetEditorUtilities::AddDefaultEventNode(Blueprint, EventGraph, FName(TEXT("K2_PostAttributeBaseChange")), USGAttributeSet::StaticClass(), NodePositionY);
			}
		}
		return NewBP;
	}
}

UObject* USGBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE
