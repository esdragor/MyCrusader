// SAURON-1127ar1 (ArtistesInProgress.md, Reprise, defect 2): the sound of a selection of units. The local player's screen
// (HOLGameScreenScript.RingUnitSelection) reads his own selected units at each refresh; when the selection is NEW, the Selection sound of
// the type that rings (rules below) plays by the buildings' path (UHOLAudioPlayer.PlayTyped, Slot Selection): the art asset's
// SelectionSound for the local faction, else silence (no table cue: the game had no unit selection sound and the user chose none).
// The rules are pure (tested by HOLUnitSelectSound_Test.as).
namespace HOLUnitSelectSound
{
	// True when Current holds a unit Previous did not (a click, a box, a double click, a group recalled): a new selection rings. The
	// same selection reaffirmed (the same group recalled again, the same box) or one that only lost units (a death, a shift-click off)
	// stays silent; an empty selection never rings.
	bool IsNewSelection(const TArray<int>& Previous, const TArray<int>& Current)
	{
		return false;
	}

	// The type that rings for a selection whose units are of these types: the most numerous, the first met on a tie; None for none.
	FName SoundingType(const TArray<FName>& Types)
	{
		return NAME_None;
	}
}
