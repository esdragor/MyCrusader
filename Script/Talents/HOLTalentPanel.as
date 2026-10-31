// The talent panel as a view (T5; GDD 9.6, D103-D105, D203): what the screen shows of the hero's talents, from his level and his picks
// alone (pure, no world, tested in HOLTalentPanel_Test.as); the widgets will apply it (the hero's block, F-key panel). The rules are
// HOLTalentRules.as's: this file only arranges them for the eye. The rows shown are those the prototype's maximum level reaches
// (HOLHero::GetMaxLevel: rows 2 to 6); the others (7 to 9) come with the levels.

// One row of the tree: the family of the talent taken (the column), or still to take.
struct FHOLTalentRowView
{
	int Row = 0;
	// The family of the talent taken in this row, -1 while none.
	int Family = -1;
	bool bTaken = false;
	// The next row to fill (rows fill in order); it can be taken when a point is in reserve.
	bool bNext = false;
	bool bCanPick = false;
}

struct FHOLTalentPanelView
{
	// Points in reserve (earned minus taken, D105), and the reminder on screen while one is unspent.
	int Points = 0;
	bool bReminder = false;
	TArray<FHOLTalentRowView> Rows;
	// The "advised choice" button (D105): the family most filled; on only with a point to spend and a first talent taken.
	int AdvisedFamily = -1;
	bool bAdviceEnabled = false;
	// Seals and key talents the sheet shows (and the adversary who scouts him, D203), and per family whether its key can still come (D103).
	int Seals = 0;
	int Keys = 0;
	TArray<bool> FamilyKeyReachable;
	// The stable line of the tests and hol.HUD.Report: "points 1 | 1 n . . . | advised 1 | seals 0 keys 0" (a family digit: taken;
	// n: next row, a point to take it; w: next row, waiting for a point; .: a later row).
	FString Describe;
}

namespace HOLTalentPanel
{
	// The panel of a hero of Level who took Picks (the families, row 2 first).
	FHOLTalentPanelView Make(int Level, const TArray<int>& Picks)
	{
		FHOLTalentPanelView View;
		View.Points = HOLTalent::PointsAvailable(Level, Picks);
		View.bReminder = View.Points > 0;
		const int MaxLevel = HOLHero::GetMaxLevel();
		const int RowsShown = HOLTalent::PointsEarned(MaxLevel);
		const int NextRow = HOLTalent::NextRow(Picks);
		FString Cells = "";
		for (int Index = 0; Index < RowsShown; Index++)
		{
			FHOLTalentRowView Row;
			Row.Row = HOLTalent::FirstRow + Index;
			Row.bTaken = Index < Picks.Num();
			Row.Family = Row.bTaken ? Picks[Index] : -1;
			Row.bNext = Row.Row == NextRow;
			Row.bCanPick = Row.bNext && View.Points > 0;
			View.Rows.Add(Row);
			if (Index > 0)
				Cells += " ";
			Cells += Row.bTaken ? f"{Row.Family}" : Row.bCanPick ? "n" : Row.bNext ? "w" : ".";
		}
		View.AdvisedFamily = HOLTalent::AdvisedFamily(Picks);
		View.bAdviceEnabled = View.Points > 0 && View.AdvisedFamily != HOLTalent::FamilyNone;
		View.Seals = HOLTalent::SealCount(Picks);
		View.Keys = HOLTalent::KeyCount(Picks);
		for (int Family = 0; Family < HOLTalent::FamilyCount; Family++)
			View.FamilyKeyReachable.Add(HOLTalent::CanStillReachKey(Picks, MaxLevel, Family));
		const FString Advised = View.AdvisedFamily == HOLTalent::FamilyNone ? "none" : f"{View.AdvisedFamily}";
		View.Describe = f"points {View.Points} | {Cells} | advised {Advised} | seals {View.Seals} keys {View.Keys}";
		return View;
	}
}
