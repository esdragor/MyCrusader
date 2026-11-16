#include "Screens/HOLConfirmDialogScreen.h"

#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "HeroesOfLibertyUILog.h"
#include "Style/HOLUIStyles.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLConfirmDialog"

UHOLConfirmDialogScreen::UHOLConfirmDialogScreen()
{
	bIsModal = true;
}

UHOLConfirmDialogScreen* UHOLConfirmDialogScreen::Ask(APlayerController* PlayerController, const FText& InQuestion, const FText& ConfirmLabel, TFunction<void()> InOnConfirm)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout)
	{
		return nullptr;
	}
	const TSubclassOf<UHOLConfirmDialogScreen> ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLUI, Warning, TEXT("Confirm dialog: no script class (Script/UI/HOLConfirmDialog.as)"));
		return nullptr;
	}
	UHOLConfirmDialogScreen* Dialog = Cast<UHOLConfirmDialogScreen>(Layout->PushModal(ScriptClass.Get()));
	if (Dialog)
	{
		Dialog->SetUp(InQuestion, ConfirmLabel, MoveTemp(InOnConfirm));
	}
	return Dialog;
}

TSubclassOf<UHOLConfirmDialogScreen> UHOLConfirmDialogScreen::FindScriptClass()
{
	// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			return *It;
		}
	}
	return nullptr;
}

FText UHOLConfirmDialogScreen::YesLabel()
{
	return LOCTEXT("Yes", "Oui");
}

FText UHOLConfirmDialogScreen::NoLabel()
{
	return LOCTEXT("No", "Non");
}

UVerticalBox* UHOLConfirmDialogScreen::MakeConfirmColumn()
{
	return MakeDialogColumn();
}

UCommonTextBlock* UHOLConfirmDialogScreen::AddConfirmText(UVerticalBox* Column, const FName Name, const FText& Text)
{
	return Column ? AddText(Column, Name, Text, UHOLTextStyleHeading::StaticClass(), 8.f) : nullptr;
}

UHorizontalBox* UHOLConfirmDialogScreen::AddConfirmRow(UVerticalBox* Column)
{
	return Column ? AddButtonRow(Column) : nullptr;
}

UHOLButton* UHOLConfirmDialogScreen::AddConfirmButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	return Row ? AddButton(Row, Name, Label) : nullptr;
}

void UHOLConfirmDialogScreen::BuildLayout()
{
	BP_BuildDialog();
}

UWidget* UHOLConfirmDialogScreen::NativeGetDesiredFocusTarget() const
{
	return CancelButton;
}

void UHOLConfirmDialogScreen::SetUp(const FText& InQuestion, const FText& ConfirmLabel, TFunction<void()> InOnConfirm)
{
	if (Question)
	{
		Question->SetText(InQuestion);
	}
	if (ConfirmButton && !ConfirmLabel.IsEmpty())
	{
		ConfirmButton->SetLabel(ConfirmLabel);
	}
	OnConfirm = MoveTemp(InOnConfirm);
}

void UHOLConfirmDialogScreen::Confirm()
{
	TFunction<void()> Action = MoveTemp(OnConfirm);
	Close();
	if (Action)
	{
		Action();
	}
}

void UHOLConfirmDialogScreen::Cancel()
{
	Close();
}

#undef LOCTEXT_NAMESPACE
