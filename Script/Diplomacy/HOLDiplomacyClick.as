// A click on one control of the Diplomacy panel's lines (D7 of DiplomacyInProgress.md): a button's click does not say which one, so each
// control gets one of these small objects (the line's slot and the control's kind) bound to its OnClicked; they all go to the screen's
// PanelClick, the same function hol.HUD.Click reaches by the control's name.
class UHOLDiplomacyClick : UObject
{
	UHOLGameScreenScript Screen;
	int Slot = -1;
	int Kind = 0;

	UFUNCTION()
	void Clicked()
	{
		if (Screen != nullptr)
			Screen.PanelClick(Slot, Kind);
	}
}
