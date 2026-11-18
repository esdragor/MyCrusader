// The yes / no question in script (MIG-CPP bascule ConfirmDialog, D830/D835): UHOLConfirmDialogScreen::Ask (the title screen's quit question,
// the in-game menu's surrender question) pushes the class FindScriptClass finds, the one of this file, in the modal layer. Its column,
// question and two buttons are built here (BuildDialog, in place of BuildLayout); Yes runs the action the base holds (Confirm), No
// closes (Cancel). No has the focus (a stray Enter never confirms). The labels are the base's (localised in C++, namespace HOLConfirmDialog).
class UHOLConfirmDialogScreenScript : UHOLConfirmDialogScreen
{
	UFUNCTION(BlueprintOverride)
	void BuildDialog()
	{
		UVerticalBox Column = MakeConfirmColumn();
		Question = AddConfirmText(Column, n"Question", FText());
		UHorizontalBox Buttons = AddConfirmRow(Column);
		ConfirmButton = AddConfirmButtonNamed(Buttons, n"ConfirmButton", UHOLConfirmDialogScreen::YesLabel());
		CancelButton = AddConfirmButtonNamed(Buttons, n"CancelButton", UHOLConfirmDialogScreen::NoLabel());
	}

	UHOLButton AddConfirmButtonNamed(UHorizontalBox Row, FName Name, FText Label)
	{
		UHOLButton Button = AddConfirmButton(Row, Name, Label);
		if (Button != nullptr)
			Button.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		return Button;
	}

	UFUNCTION()
	void HandleButtonClicked(UCommonButtonBase Button)
	{
		if (Button == nullptr)
			return;
		if (Button == ConfirmButton)
			Confirm();
		else
			Cancel();
	}
}
