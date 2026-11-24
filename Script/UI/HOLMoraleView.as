// The morale pastille of the selected unit (S2-d, PLAN-MORAL-1120.md sub-step 10, script part), the first caller of HOLMoraleBlock.as.
// ALL the reading of a unit's morale state goes through ONE function, HOLMoraleView::Read. It returns an EMPTY view (nothing available) until the
// replicated effects mask exists (sub-step 9, C++ network: FHOLNetUnitState mask + FHOLUnitView); then only the body of Read changes, nothing else.
// The pastille is a line of the selection block's statistics host (SelectionStatsHost), so it hides with the statistics. Labels come from the
// effects table (HOLMoraleTable::Name: Triomphe, Demoralise, Hesitation, D866); the colours are the dossier's (green bonus, red malus, gold immunity).
// NOT COMPILED / NOT PLAYED (the binary of socle-ai is stale): the tests of HOLMoraleView_Test.as are to be run by the integration pass.
namespace HOLMoraleView
{
	// What the replicated state says about the shown unit: nothing available (the default, until sub-step 9), or an effects state.
	struct FHOLMoraleView
	{
		bool bAvailable = false;
		bool bOwn = false;
		bool bPeasant = false;
		FHOLMoraleStateData State;
	}

	// What the pastille draws: hidden, or a text and a colour.
	struct FHOLMoralePastille
	{
		bool bShown = false;
		FString Text;
		FLinearColor Color = FLinearColor(1.0, 1.0, 1.0, 1.0);
	}

	// THE one reading point. EffectMask (A2, S2-c step 9) is the mask the server replicated for the shown unit (bit R = the effect of table row R,
	// FHOLUnitSelView.EffectMask); 0 gives an empty view (the pastille stays hidden). Only the ids travel: no time left, no immunity on the client yet.
	FHOLMoraleView Read(int ShownSlot, bool bOwn, bool bPeasant, int EffectMask = 0)
	{
		FHOLMoraleView View;
		View.bOwn = bOwn;
		View.bPeasant = bPeasant;
		if (EffectMask != 0)
		{
			View.bAvailable = true;
			for (int Id = 0; Id < 16; Id++)
			{
				if ((EffectMask & (1 << Id)) != 0)
					View.State.Effects.Add(HOLMorale::NativeMakeEffectFromRow(Id));
			}
		}
		return View;
	}

	// The colour of the pastille: red when a malus is there (it is what the player must see), else green for a bonus, else gold for the immunity alone.
	FLinearColor ColorOf(const HOLMoraleBlock::FHOLMoraleBadge& Badge)
	{
		if (Badge.bMalus)
			return FLinearColor(0.9, 0.2, 0.15, 1.0);
		if (Badge.bBonus)
			return FLinearColor(0.3, 0.8, 0.3, 1.0);
		return FLinearColor(0.95, 0.8, 0.25, 1.0);
	}

	// The pure rule: the view in, what is drawn out. Lines come from HOLMoraleBlock (bonus first, then malus), each "<name>" or "<name> <n> s" (our units).
	FHOLMoralePastille Present(const FHOLMoraleView& View)
	{
		FHOLMoralePastille Result;
		if (!View.bAvailable)
			return Result;
		const HOLMoraleBlock::FHOLMoraleBadge Badge = HOLMoraleBlock::Badge(View.State, View.bPeasant);
		const TArray<HOLMoraleBlock::FHOLMoraleLine> Lines = HOLMoraleBlock::Lines(View.State, View.bOwn, View.bPeasant);
		if (Lines.Num() == 0 && !Badge.bImmune)
			return Result;
		FString Text = "";
		for (int Index = 0; Index < Lines.Num(); Index++)
		{
			if (Index > 0)
				Text += "  ";
			Text += "● " + HOLMoraleTable::Name(Lines[Index].Id).ToString();
			if (Lines[Index].SecondsLeft >= 0)
				Text += f" {Lines[Index].SecondsLeft} s";
		}
		if (Lines.Num() == 0)
			Text = "●";
		Result.bShown = true;
		Result.Text = Text;
		Result.Color = ColorOf(Badge);
		return Result;
	}

	// The pastille's text block in the host, made once (found again by its name).
	UCommonTextBlock FindOrBuild(UHOLScriptScreen Screen, UVerticalBox Host)
	{
		for (int Index = 0; Index < Host.GetChildrenCount(); Index++)
		{
			UWidget Child = Host.GetChildAt(Index);
			if (Child != nullptr && Child.GetName() == n"MoraleBadge")
				return Cast<UCommonTextBlock>(Child);
		}
		UCommonTextBlock Badge = Screen.MakeText(n"MoraleBadge", UHOLTextStyleHUD);
		Badge.SetVisibility(ESlateVisibility::Collapsed);
		Host.AddChildToVerticalBox(Badge);
		return Badge;
	}

	// The HUD's single hook (HOLGameScreenScript.as SelectionFrameTick, one unit selected): reads the view, draws or hides the pastille.
	void Refresh(UHOLScriptScreen Screen, UVerticalBox Host)
	{
		if (Screen == nullptr || Host == nullptr)
			return;
		UCommonTextBlock Badge = FindOrBuild(Screen, Host);
		if (Badge == nullptr)
			return;
		FHOLUnitSelView Sel;
		FHOLMoraleView View;
		if (HOLSelectionUnitRead::GetUnitSelectionView(Screen.GetShownSlot(), Sel) && Sel.UnitCount == 1)
		{
			// A worker is a Peasant for the morale (D705: it never reaches the Peasants).
			const bool bPeasant = Sel.TypeWorkers.Num() > 0 && Sel.TypeWorkers[0];
			View = Read(Screen.GetShownSlot(), Sel.bOwn, bPeasant, Sel.EffectMask);
		}
		const FHOLMoralePastille Drawn = Present(View);
		if (!Drawn.bShown)
		{
			Badge.SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
		Badge.SetText(FText::FromString(Drawn.Text));
		Badge.SetColorAndOpacity(Drawn.Color);
		Badge.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}
