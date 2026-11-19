// MIG-C-CommandGrid, lot G1 (SAURON-1119t, D830, D836, D839): the CONTENT of the 3 x 4 command grid, first half, in script. NOT COMPILED when written.
// Port of Source/HeroesOfLiberty/Player/HOLCommandGrid.cpp lines 38-237 (GetCommand, GetOrder, the Market's trades, the ages, the cursor), same tables,
// same decisions. The C++ bodies are gone (lot L3): HOLCommandGrid.cpp keeps only the bridge to this rule (UHOLCommandGridBoard), no C++ twin.
// The input is FHOLGridContextView and the output FHOLGridCommandView (the USTRUCTs of UHOLCommandGridReadLibrary): nothing else is called, except
//   - HOLCommandGridRead::GetSpecializationCell: the cell of the first specialization option is the cvar hol.UI.SpecializationCell, registered in C++
//     (HOLCommandGrid.cpp, default 6): a setting, not a rule; read through the library until the cvar moves (TYPES_A_EXPOSER, see the report);
//   - HOLHeroAbilityRead::IsUnlocked (the hero's rule, which stays in C++ for the server): the abilities' unlock level.
// NOT ported (data not exposed to script): SetSpecialization / GetOpenSpecializationTier (FHOLSpecializationView). The second half (PressKey, ClickAge,
// Check*, Describe, groups, MergeCommands) is lots G2a / G2b.
// Numbers of the hero abilities, as FHOLGridCommandView::Ability holds them: Aura 0, CloseRanks 1, Banner 2 (EHOLHeroAbility's order).

namespace HOLCommandGridRules
{
	// The grid's size and cells (HOLCommandGrid.h:278-320); a constant of a file is not seen by another file, hence the accessors below.
	const int NumCells = 12;
	const int GridColumns = 4;
	const int CancelCell = 11;
	const int ResearchFirstCell = 8;
	const int NumAges = 4;
	const int TradableGoods = 3;
	const int SpecializationOptions = 2;
	const int AttackMoveCell = 4;
	const int StopCell = 5;
	const int HoldCell = 11;
	const int DeployCell = 6;
	const int HeroAbilityCloseRanks = 1;
	const int HeroAbilityBanner = 2;
	const float LockedAgePanelOpacity = 0.55f;

	int GetNumCells() { return NumCells; }
	int GetGridColumns() { return GridColumns; }
	int GetCancelCell() { return CancelCell; }
	int GetResearchFirstCell() { return ResearchFirstCell; }
	int GetNumAges() { return NumAges; }
	int GetTradableGoods() { return TradableGoods; }
	int GetAttackMoveCell() { return AttackMoveCell; }
	int GetStopCell() { return StopCell; }
	int GetHoldCell() { return HoldCell; }
	int GetDeployCell() { return DeployCell; }
	float GetLockedAgePanelOpacity() { return LockedAgePanelOpacity; }

	// GetRageCell / GetHeroAbilityCell (cpp 136-143): the rage on E (cell 2); the cry on A, the banner on Z; the aura has no cell.
	int GetRageCell()
	{
		return 2;
	}

	int GetHeroAbilityCell(int Ability)
	{
		if (Ability == HeroAbilityCloseRanks)
		{
			return 0;
		}
		if (Ability == HeroAbilityBanner)
		{
			return 1;
		}
		return -1;
	}

	// GetCategoryRow (cpp 212-215): the row of a category on the panel's page.
	int GetCategoryRow(EHOLBuildingCategory Category)
	{
		if (Category == EHOLBuildingCategory::Economic)
		{
			return 0;
		}
		if (Category == EHOLBuildingCategory::Military)
		{
			return 1;
		}
		return 2;
	}

	// GetTradeCell (cpp 145-153): purchases on the top row (A Z E), sales on the second (Q S D).
	int GetTradeCell(EHOLResource Resource, bool bBuy)
	{
		int Good = int(Resource);
		if (Good < 0 || Good >= TradableGoods)
		{
			return -1;
		}
		return (bBuy ? 0 : GridColumns) + Good;
	}

	// GetTradeGain / GetTradeGainText (cpp 155-166, D781): "+Amount" and the logo (the resource bought, Gold when selling).
	FHOLTradeGainView GetTradeGain(EHOLResource Resource, bool bBuy, int Received)
	{
		FHOLTradeGainView Gain;
		Gain.Amount = Math::Max(Received, 0);
		Gain.Logo = bBuy ? Resource : EHOLResource::Gold;
		return Gain;
	}

	FString GetTradeGainText(const FHOLTradeGainView& Gain)
	{
		return f"+{Gain.Amount}";
	}

	// GetReportName (cpp 45-54): the short name of a specialization cell for hol.HUD.Report ("SpecializeT1A"); None for any other command.
	FName GetReportName(const FHOLGridCommandView& Command)
	{
		if (Command.Command != EHOLGridCommandKind::Specialize && Command.Command != EHOLGridCommandKind::CancelSpecialization)
		{
			return FName();
		}
		FString Verb = Command.Command == EHOLGridCommandKind::Specialize ? "Specialize" : "CancelSpecialization";
		FString Letter = Command.Option == 0 ? "A" : "B";
		return FName(f"{Verb}T{Command.Tier}{Letter}");
	}

	// One hero ability's cell (cpp 124-130): greyed when not unlocked at the hero's level or cooling down; the seconds left (0 when none told).
	FHOLGridCommandView MakeHeroAbilityCommand(const FHOLGridContextView& Context, int Ability)
	{
		FHOLGridCommandView Result;
		Result.Command = EHOLGridCommandKind::HeroAbility;
		Result.Ability = Ability;
		int Cooldown = Ability >= 0 && Ability < Context.HeroCooldownSeconds.Num() ? Context.HeroCooldownSeconds[Ability] : 0;
		Result.CooldownSeconds = Math::Max(0, Cooldown);
		Result.bGreyed = !HOLHeroAbilityRead::IsUnlocked(EHOLHeroAbility(Ability), Context.HeroLevel) || Result.CooldownSeconds > 0;
		return Result;
	}

	// GetCommand (cpp 55-134): what the cell of the grid on the right shows for this selection.
	FHOLGridCommandView GetCommand(const FHOLGridContextView& Context, int Cell)
	{
		FHOLGridCommandView Result;
		if (Cell < 0 || Cell >= NumCells)
		{
			return Result;
		}
		if (Context.bOwnBuilding)
		{
			// A main forum down produces nothing and is no site to cancel (V2.3a, D351).
			if (Context.bForumDown)
			{
				return Result;
			}
			// A building under construction produces nothing (D312); its site may be cancelled (D311), one at a time.
			if (!Context.bBuildingFinished)
			{
				if (Cell == CancelCell && !Context.bBuildingGroup)
				{
					Result.Command = EHOLGridCommandKind::CancelConstruction;
				}
				return Result;
			}
			// M5: a Market produces nothing; its cells are the trades (PURCHASES on the top row A Z E, SALES on the second Q S D).
			if (Context.bMarket)
			{
				int Good = Cell % GridColumns;
				if (Cell < 2 * GridColumns && Good < TradableGoods)
				{
					Result.Command = EHOLGridCommandKind::Trade;
					Result.Resource = EHOLResource(Good);
					Result.bBuy = Cell < GridColumns;
				}
				return Result;
			}
			int FirstOption = HOLCommandGridRead::GetSpecializationCell(0);
			if (Cell < Context.Produces.Num())
			{
				Result.Command = EHOLGridCommandKind::Produce;
				Result.TypeName = Context.Produces[Cell];
			}
			else if (Context.bPrimaryForum && !Context.bBuildingGroup && Context.SpecializationTier > 0
				&& Cell >= FirstOption && Cell < HOLCommandGridRead::GetSpecializationCell(SpecializationOptions))
			{
				// SP3 (D332, D684, D785): the open tier's two options; the running one cancels at no cost (its time left on the cell), the
				// other still starts (it replaces).
				Result.Option = Cell - FirstOption;
				Result.Tier = Context.SpecializationTier;
				bool bRunning = Result.Option == Context.SpecializationPendingOption;
				Result.Command = bRunning ? EHOLGridCommandKind::CancelSpecialization : EHOLGridCommandKind::Specialize;
				Result.RemainingMs = bRunning ? Context.SpecializationRemainingMs : 0;
			}
			else if (Cell >= ResearchFirstCell && Cell - ResearchFirstCell < Context.Researches.Num())
			{
				// The technology row (W X C V, D30): the building's technologies in table order, after its units.
				Result.Command = EHOLGridCommandKind::Research;
				Result.TypeName = Context.Researches[Cell - ResearchFirstCell];
			}
			return Result;
		}
		if (Context.bRageUnit && Cell == GetRageCell())
		{
			Result.Command = EHOLGridCommandKind::NecroticRage;
			return Result;
		}
		if (Context.bHero)
		{
			if (Cell == GetHeroAbilityCell(HeroAbilityCloseRanks))
			{
				Result = MakeHeroAbilityCommand(Context, HeroAbilityCloseRanks);
			}
			if (Cell == GetHeroAbilityCell(HeroAbilityBanner))
			{
				Result = MakeHeroAbilityCommand(Context, HeroAbilityBanner);
			}
		}
		return Result;
	}

	// GetOrder (cpp 168-183): the orders' panel left of the grid: Q attack-move (soldiers, no king alone), S stop, V hold, D deploy / pack.
	FHOLGridCommandView GetOrder(const FHOLGridContextView& Context, int Cell)
	{
		FHOLGridCommandView Result;
		if (Context.bOwnUnits && !Context.bOwnBuilding)
		{
			if (Cell == AttackMoveCell && Context.bSoldiers && !Context.bOnlyKings)
			{
				Result.Command = EHOLGridCommandKind::AttackMove;
			}
			else if (Cell == StopCell)
			{
				Result.Command = EHOLGridCommandKind::Stop;
			}
			else if (Cell == HoldCell && !Context.bOnlyKings)
			{
				Result.Command = EHOLGridCommandKind::Hold;
			}
			if (Cell == DeployCell && Context.bDeployables)
			{
				Result.Command = EHOLGridCommandKind::Deploy;
				Result.bPack = Context.bDeployedAny;
			}
		}
		return Result;
	}

	// GetMouseCursor (cpp 184-188): the attack cursor (crosshairs) while Q waits for its point and no building is being placed; else the arrow.
	// Returns the decision as a bool: EMouseCursor is not read in script (the bridge maps true to Crosshairs, false to Default).
	bool IsAttackCursor(bool bAttackMoveArmed, bool bPlacing)
	{
		return bAttackMoveArmed && !bPlacing;
	}

	// IsBuildGrid (cpp 189-192): the builders' grid, shown when no building is selected and the builders are alone.
	bool IsBuildGrid(const FHOLGridContextView& Context)
	{
		return !Context.bOwnBuilding && Context.bBuilders && !Context.bSoldiers;
	}

	// The buildable types are three parallel arrays (FHOLGridContext::Buildable is built from the shortest, cpp library ToRule 39-48).
	int GetBuildableCount(const FHOLGridContextView& Context)
	{
		return Math::Min(Math::Min(Context.BuildableTypes.Num(), Context.BuildableCells.Num()), Context.BuildableAges.Num());
	}

	bool IsBuildableAlwaysOpen(const FHOLGridContextView& Context, int Index)
	{
		return Index >= 0 && Index < Context.BuildableAlwaysOpen.Num() && Context.BuildableAlwaysOpen[Index];
	}

	// IsAgeReached / IsAgeOpen / GetAgePanelOpacity (cpp 194-210).
	bool IsAgeReached(const FHOLGridContextView& Context, int Age)
	{
		return Age >= 0 && Age < NumAges && Age <= Context.ReachedAge;
	}

	bool IsAgeOpen(const FHOLGridContextView& Context, int Age)
	{
		if (IsAgeReached(Context, Age))
		{
			return true;
		}
		if (Age < 0 || Age >= NumAges)
		{
			return false;
		}
		int Count = GetBuildableCount(Context);
		for (int Index = 0; Index < Count; Index++)
		{
			if (Context.BuildableAges[Index] == Age && IsBuildableAlwaysOpen(Context, Index))
			{
				return true;
			}
		}
		return false;
	}

	float GetAgePanelOpacity(const FHOLGridContextView& Context, int Age)
	{
		// A page faded only while nothing on it can be built (a Nomad's first forum opens its page, D797).
		return IsAgeOpen(Context, Age) ? 1.0f : LockedAgePanelOpacity;
	}

	// GetAgeBuilding (cpp 217-237): the cell of a building on an age's panel (builders' grid only); greyed while its age is not reached.
	FHOLGridCommandView GetAgeBuilding(const FHOLGridContextView& Context, int Age, int Cell)
	{
		FHOLGridCommandView Result;
		if (!IsBuildGrid(Context) || Age < 0 || Age >= NumAges || Cell < 0 || Cell >= NumCells)
		{
			return Result;
		}
		FName NoName;
		int Count = GetBuildableCount(Context);
		for (int Index = 0; Index < Count; Index++)
		{
			if (Context.BuildableCells[Index] == Cell && Context.BuildableAges[Index] == Age && Context.BuildableTypes[Index] != NoName)
			{
				Result.Command = EHOLGridCommandKind::Build;
				Result.TypeName = Context.BuildableTypes[Index];
				Result.Age = Age;
				Result.bGreyed = !IsAgeReached(Context, Age) && !IsBuildableAlwaysOpen(Context, Index);
				break;
			}
		}
		return Result;
	}
}
