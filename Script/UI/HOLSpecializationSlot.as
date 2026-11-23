// SP3 (ARWEN-1123c, D823 Q-specialization-ui = B, D201): the permanent specialization button in the faction slot, as a view and two click rules
// (pure: no world, no widget). The button is lit while a tier is open; a click shows the small panel of the tier's options (2, 3 for the Templars);
// while an unlock runs, a bar under it counts down and a click on the running option cancels it. The D / F cells of the Forum grid stay a shortcut
// (same server rule: HOLSpecialization::Start / Cancel, through ServerStartSpecialization / ServerCancelSpecialization). Texts are the game's text
// table's (HOLGameNames.cpp GetSpecializationName "Voie A du palier N", GetSpecializationCellText "N s" / "Annuler"), none invented here.
// Tested in Script/Tests/HOLSpecializationSlot_Test.as.

enum EHOLSpecializationAction
{
	Nothing,
	Start,
	Cancel,
}

struct FHOLSpecializationSlotView
{
	// The button is lit: a tier is open (reached, the tier before applied, not applied yet, Forum complete).
	bool bLit = false;
	// The tier the button is about (1..3), 0 when none is open.
	int OpenTier = 0;
	// An unlock runs: PendingOption is its option, RemainingMs the match time left, Fraction how far the bar is (0..1).
	bool bRunning = false;
	int PendingOption = -1;
	int RemainingMs = 0;
	float Fraction = 0.0;
	// The panel's lines, one per option of the open tier ("Voie A du palier 1").
	TArray<FString> OptionTexts;
	// The bar's text, "23 s" while an unlock runs, empty otherwise.
	FString BarText;
	// "spec dark", "spec lit tier 1 options 2", "spec running tier 1 option 0 23 s".
	FString Describe;
}

struct FHOLSpecializationClick
{
	EHOLSpecializationAction Action = EHOLSpecializationAction::Nothing;
	int Tier = 0;
	int Option = -1;
	FString Describe;
}

namespace HOLFactionSlot
{
	// "FactionBox<n>" (the faction slot's power buttons) gives n, -1 for any other name (FactionBoxLabel1, FactionBoxColumn, GridCell01...).
	int BoxIndexOfName(FName Name)
	{
		const FString Text = Name.ToString();
		const FString Prefix = "FactionBox";
		if (!Text.StartsWith(Prefix))
			return -1;
		const FString Rest = Text.RightChop(Prefix.Len());
		if (Rest.IsEmpty())
			return -1;
		const FString Digits = "0123456789";
		int Value = 0;
		for (int Index = 0; Index < Rest.Len(); Index++)
		{
			const int Digit = Digits.Find(Rest.Mid(Index, 1));
			if (Digit < 0)
				return -1;
			Value = Value * 10 + Digit;
		}
		return Value;
	}
}

namespace HOLSpecializationSlot
{
	// The lowest tier not applied yet, when it is open (reached by the faction tier, Forum complete); 0 otherwise. The tier before is applied by construction.
	int OpenTierOf(FHOLSpecializationView View, int PlayerTier, bool bForumComplete)
	{
		for (int Tier = HOLSpecialization::FirstTier; Tier <= HOLSpecialization::LastTier; Tier++)
		{
			const int Applied = Tier < View.Chosen.Num() ? View.Chosen[Tier] : HOLSpecialization::None;
			if (Applied != HOLSpecialization::None)
				continue;
			return (bForumComplete && PlayerTier >= Tier) ? Tier : 0;
		}
		return 0;
	}

	// The slot's specialization view from what the owner reads (FHOLSpecializationView), the faction tier, whether the Forum is complete
	// (HOLNomad::HasCompleteForum) and the number of options of the faction (HOLSpecialization::OptionCountOf).
	FHOLSpecializationSlotView Make(FHOLSpecializationView View, int PlayerTier, bool bForumComplete, int OptionCount)
	{
		FHOLSpecializationSlotView Result;
		Result.OpenTier = OpenTierOf(View, PlayerTier, bForumComplete);
		Result.bLit = Result.OpenTier > 0;
		if (!Result.bLit)
		{
			Result.Describe = "spec dark";
			return Result;
		}
		const FString OptionNames = "ABC";
		for (int Option = 0; Option < OptionCount && Option < 3; Option++)
			Result.OptionTexts.Add(f"Voie {OptionNames.Mid(Option, 1)} du palier {Result.OpenTier}");
		Result.bRunning = View.PendingTier == Result.OpenTier && View.PendingOption >= 0;
		if (!Result.bRunning)
		{
			Result.Describe = f"spec lit tier {Result.OpenTier} options {Result.OptionTexts.Num()}";
			return Result;
		}
		Result.PendingOption = View.PendingOption;
		Result.RemainingMs = View.RemainingMs;
		const int Delay = HOLSpecialization::DelayMs(Result.OpenTier);
		Result.Fraction = Delay > 0 ? Math::Clamp(1.0 - float(View.RemainingMs) / float(Delay), 0.0, 1.0) : 0.0;
		Result.BarText = f"{(View.RemainingMs + 999) / 1000} s";
		Result.Describe = f"spec running tier {Result.OpenTier} option {Result.PendingOption} {Result.BarText}";
		return Result;
	}

	// A click on option Option of the panel: starts it, cancels it when it is the running one, nothing otherwise (dark, or another option runs).
	FHOLSpecializationClick ClickOption(FHOLSpecializationSlotView View, int Option)
	{
		FHOLSpecializationClick Result;
		if (!View.bLit || Option < 0 || Option >= View.OptionTexts.Num())
			return Result;
		if (!View.bRunning)
		{
			Result.Action = EHOLSpecializationAction::Start;
			Result.Tier = View.OpenTier;
			Result.Option = Option;
			Result.Describe = f"start {Result.Tier} {Result.Option}";
		}
		else if (Option == View.PendingOption)
		{
			Result.Action = EHOLSpecializationAction::Cancel;
			Result.Tier = View.OpenTier;
			Result.Option = Option;
			Result.Describe = f"cancel {Result.Tier}";
		}
		return Result;
	}
}
