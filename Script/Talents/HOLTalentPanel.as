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
	// TQ4 (PROVISIONAL, not decided, hol.UI.TalentFirstChoice): the row the next talent fills, and the three family buttons (B) shown while a point is kept.
	// In C the advised button takes a default family when none is taken yet; in A (a tree window, not built) the buttons stand in.
	int NextRow = 0;
	bool bFamilyButtons = false;
	int FirstChoiceMode = 1;
	// The stable line of the tests and hol.HUD.Report: "points 1 | 1 n . . . | advised 1 | seals 0 keys 0" (a family digit: taken;
	// n: next row, a point to take it; w: next row, waiting for a point; .: a later row).
	FString Describe;
}

namespace HOLTalentPanel
{
	// The options of TQ4 (where the first talent is chosen): A a tree window (not built), B three family buttons (default), C a default family.
	const int ModeTree = 0;
	const int ModeButtons = 1;
	const int ModeDefault = 2;

	// The panel in the default option (B).
	FHOLTalentPanelView Make(int Level, const TArray<int>& Picks)
	{
		return MakeFor(Level, Picks, ModeButtons);
	}

	// The family option C advises when none is taken yet: the first with an effect in the game (the Captain in the prototype).
	int DefaultFamily()
	{
		const TArray<int> Families = HOLAITalent::EffectFamilies();
		return Families.Num() > 0 ? Families[0] : HOLTalent::FamilyNone;
	}

	// The panel of a hero of Level who took Picks (the families, row 2 first), for one option of TQ4.
	FHOLTalentPanelView MakeFor(int Level, const TArray<int>& Picks, int Mode)
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
		View.FirstChoiceMode = Mode;
		View.NextRow = NextRow;
		if (Mode == ModeDefault && View.AdvisedFamily == HOLTalent::FamilyNone)
			View.AdvisedFamily = DefaultFamily();
		View.bFamilyButtons = View.Points > 0 && Mode != ModeDefault;
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

// The panel's texts by key, in French (source of the gathered texts, GatherText; English and Japanese in Tools/Localization/fill_po.py, keys
// "HOLTalentPanel,<Name>"). Literals only in these calls. {0} is the points kept. The talents' own names and effects come from ONE table,
// HOLTalentNames (HOLTalentNames.as): no text here spells a talent's name (HOLTalentNames_Test.as keeps it so).
namespace HOLTalentText
{
	FText ReminderPattern(bool bAdvised)
	{
		if (bAdvised)
			return NSLOCTEXT("HOLTalentPanel", "ReminderAdvised", "Talents à dépenser : {0} — choix conseillé");
		return NSLOCTEXT("HOLTalentPanel", "Reminder", "Talents à dépenser : {0}");
	}

	FText TooltipPattern(bool bAdvised)
	{
		if (bAdvised)
			return NSLOCTEXT("HOLTalentPanel", "TooltipAdvisedTalent", "Prend le talent de la famille la plus remplie : {0} — {1}");
		return NSLOCTEXT("HOLTalentPanel", "TooltipFirst", "Choisissez d'abord un premier talent : l'arbre arrive");
	}

	// The reminder's line: "Talents à dépenser : 2 — choix conseillé".
	FString ReminderText(FHOLTalentPanelView View)
	{
		return ReminderPattern(View.bAdviceEnabled).ToString().Replace("{0}", f"{View.Points}");
	}

	FText FamilyPattern(int Family)
	{
		if (Family == 0)
			return NSLOCTEXT("HOLTalentPanel", "FamilyGuardian", "Gardien");
		if (Family == 1)
			return NSLOCTEXT("HOLTalentPanel", "FamilyCaptain", "Capitaine");
		return NSLOCTEXT("HOLTalentPanel", "FamilyChampion", "Champion");
	}

	// The name of a family on its button.
	FString FamilyName(int Family)
	{
		return FamilyPattern(Family).ToString();
	}

	// What a talent does, as the screen says it: its effect, and when the prototype cannot show it yet, that too (D194).
	FString EffectText(int Row, int Family)
	{
		FString Effect = HOLTalentNames::Effect(Row, Family).ToString();
		if (!HOLHero::TalentHasEffect(Row, Family))
			Effect += " (" + HOLTalentNames::NoEffectYet().ToString() + ")";
		return Effect;
	}

	FText FamilyTipPattern()
	{
		return NSLOCTEXT("HOLTalentPanel", "FamilyTipTalent", "{0}, rangée {1} : {2} — {3}");
	}

	// The family button's tooltip: the talent the click takes (the next row of that family), by its name and its effect.
	FString FamilyTooltip(FHOLTalentPanelView View, int Family)
	{
		return FamilyTipPattern().ToString().Replace("{0}", FamilyName(Family)).Replace("{1}", f"{View.NextRow}")
			.Replace("{2}", HOLTalentNames::Name(View.NextRow, Family).ToString()).Replace("{3}", EffectText(View.NextRow, Family));
	}

	// The advised button's tooltip: the talent it takes, or why it cannot yet.
	FString TooltipText(FHOLTalentPanelView View)
	{
		if (!View.bAdviceEnabled)
			return TooltipPattern(false).ToString();
		return TooltipPattern(true).ToString().Replace("{0}", HOLTalentNames::Name(View.NextRow, View.AdvisedFamily).ToString())
			.Replace("{1}", EffectText(View.NextRow, View.AdvisedFamily));
	}

	FText NoticePattern()
	{
		return NSLOCTEXT("HOLTalentPanel", "Notice", "Talent pris : {0} — {1}");
	}

	// The notice that shows for a few seconds when a point is spent: "Talent pris : Boucliers levés — +1 armure ...".
	FString NoticeText(int Row, int Family)
	{
		return NoticePattern().ToString().Replace("{0}", HOLTalentNames::Name(Row, Family).ToString()).Replace("{1}", EffectText(Row, Family));
	}

	FText ListPattern()
	{
		return NSLOCTEXT("HOLTalentPanel", "List", "Talents pris ({0}) : {1}");
	}

	// The talents taken, by name, in the order of the rows (row 2 first); empty with none.
	FString ListText(const TArray<int>& Picks)
	{
		if (Picks.Num() == 0)
			return "";
		FString Names = "";
		for (int Index = 0; Index < Picks.Num(); Index++)
		{
			if (Index > 0)
				Names += ", ";
			Names += HOLTalentNames::Name(HOLTalent::FirstRow + Index, Picks[Index]).ToString();
		}
		return ListPattern().ToString().Replace("{0}", f"{Picks.Num()}").Replace("{1}", Names);
	}
}