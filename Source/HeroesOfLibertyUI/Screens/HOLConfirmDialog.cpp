#include "Screens/HOLConfirmDialog.h"

#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"

#define LOCTEXT_NAMESPACE "HOLConfirmDialog"

UHOLConfirmDialog::UHOLConfirmDialog()
{
	bIsModal = true;
}

void UHOLConfirmDialog::BuildLayout()
{
	UVerticalBox* Column = MakeDialogColumn();
	Question = AddText(Column, TEXT("Question"), FText::GetEmpty(), UHOLTextStyleHeading::StaticClass(), 8.f);
	UHorizontalBox* Buttons = AddButtonRow(Column);
	ConfirmButton = AddButton(Buttons, TEXT("ConfirmButton"), LOCTEXT("Yes", "Oui"));
	CancelButton = AddButton(Buttons, TEXT("CancelButton"), LOCTEXT("No", "Non"));
}

void UHOLConfirmDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked().AddUObject(this, &UHOLConfirmDialog::HandleConfirm);
	}
	if (CancelButton)
	{
		CancelButton->OnClicked().AddUObject(this, &UHOLConfirmDialog::Close);
	}
}

UWidget* UHOLConfirmDialog::NativeGetDesiredFocusTarget() const
{
	return CancelButton;
}

void UHOLConfirmDialog::SetUp(const FText& InQuestion, const FText& ConfirmLabel, TFunction<void()> InOnConfirm)
{
	if (Question)
	{
		Question->SetText(InQuestion);
	}
	if (ConfirmButton)
	{
		ConfirmButton->SetLabel(ConfirmLabel);
	}
	OnConfirm = MoveTemp(InOnConfirm);
}

void UHOLConfirmDialog::HandleConfirm()
{
	TFunction<void()> Action = MoveTemp(OnConfirm);
	Close();
	if (Action)
	{
		Action();
	}
}

#undef LOCTEXT_NAMESPACE
