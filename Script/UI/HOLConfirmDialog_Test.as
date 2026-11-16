// The confirm dialog's widget in script (MIG-CPP bascule ConfirmDialog): UHOLConfirmDialogScreen::Ask (the title screen's quit, the in-game
// menu's surrender) pushes the class FindScriptClass finds, the one of Script/UI/HOLConfirmDialog.as; its default labels are Oui / Non.

void Test_ConfirmDialogScreenScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLConfirmDialogScreen> Found = UHOLConfirmDialogScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the dialog opened by Ask has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLConfirmDialogScreenScript", "the dialog opened by Ask is HOLConfirmDialog.as's");
	T.AssertEquals(UHOLConfirmDialogScreen::YesLabel().ToString(), "Oui", "the confirm button's default label");
	T.AssertEquals(UHOLConfirmDialogScreen::NoLabel().ToString(), "Non", "the cancel button's default label");
}
