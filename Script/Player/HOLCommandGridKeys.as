// MIG-C-CommandGrid, lot G2a (SAURON-1119x, D830, D836, D839): the KEYS and the FEEDBACK of the 3 x 4 command grid, in script. NOT COMPILED when written.
// Port of Source/HeroesOfLiberty/Player/HOLCommandGrid.cpp: PressKey, ClickAge, GetChosenAgeAfter, FindMissingResource, CheckProduction, CheckBuild,
// Blocks, IsPopulationBlocked, Describe, GetCellFace, IsSelectedByCategory (lines 238-418), same decisions. The C++ file is NOT touched: until the
// bridge and the green agreement test (Script/Tests/HOLCommandGridKeys_Test.as) two copies exist.
// What it calls: HOLCommandGridRules:: (lot G1: GetCommand, IsBuildGrid, IsAgeOpen, GetAgeBuilding, the constants) and the USTRUCTs of
// UHOLCommandGridReadLibrary (FHOLGridContextView in, FHOLGridKeyResultView / FHOLFeedbackView / FHOLGridCellFaceView out). Nothing else.
// NOT ported (data not exposed to script, see TYPES_A_EXPOSER in the report):
//   - CheckResearch: needs FHOLTechnologyRow + HOLTechnology::Check (C++ only); the library's CheckResearch(FName, FHOLTechFactsView, ...) stays the way in.
//   - GetCycleRank: needs FHOLBuildingTypeRow::IsBuildable / BuildGridSlot; GetCycleRankOfType(FName) of the library stays the way in.
// Describe: the placement refusals and the technology blocks arrive as NUMBERS in FHOLFeedbackView; the numbers below are the C++ enums' order
// (EHOLPlacementRefusal in Buildings/HOLConstructionRules.h, HOLTechnology::EBlock in Technologies/HOLTechnologyRules.h), checked by the agreement test.

namespace HOLCommandGridKeys
{
	// EHOLPlacementRefusal as numbers (None 0, OutOfMap 1, Building 2, Source 3, EnemyCentre 4, NavMesh 5).
	const int RefusalOutOfMap = 1;
	const int RefusalBuilding = 2;
	const int RefusalSource = 3;
	const int RefusalEnemyCentre = 4;
	const int RefusalNavMesh = 5;

	// HOLTechnology::EBlock as numbers (None 0, NotImplemented 1, AlreadyResearched 2, AlreadyQueued 3, TierTooLow 4, HeroLevelTooLow 5,
	// RequiresMissing 6, NoBuilding 7, CannotPay 8).
	const int BlockNotImplemented = 1;
	const int BlockAlreadyResearched = 2;
	const int BlockAlreadyQueued = 3;
	const int BlockTierTooLow = 4;
	const int BlockHeroLevelTooLow = 5;
	const int BlockRequiresMissing = 6;
	const int BlockNoBuilding = 7;
	const int BlockCannotPay = 8;

	// PressKey (cpp 238-275): where a key goes. The decision tree, in the C++ order.
	FHOLGridKeyResultView PressKey(const FHOLGridContextView& Context, int ChosenAge, int Cell)
	{
		FHOLGridKeyResultView Result;
		if (Cell < 0 || Cell >= HOLCommandGridRules::GetNumCells())
		{
			Result.ChosenAge = ChosenAge;
			return Result;
		}
		if (Context.bOwnBuilding)
		{
			Result.Target = EHOLGridKeyTargetKind::Grid;
			return Result;
		}
		if (!HOLCommandGridRules::IsBuildGrid(Context))
		{
			// The hero's cells (A Z) and the rage's are the grid's; the other keys the orders'.
			FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Context, Cell);
			bool bGridKey = Command.Command == EHOLGridCommandKind::HeroAbility || Command.Command == EHOLGridCommandKind::NecroticRage;
			Result.Target = bGridKey ? EHOLGridKeyTargetKind::Grid : EHOLGridKeyTargetKind::Orders;
			return Result;
		}
		// No age chosen: A Z E R choose one (one not reached: nothing), the others are the orders'.
		if (ChosenAge == -1)
		{
			bool bAgeKey = Cell < HOLCommandGridRules::GetNumAges();
			bool bReached = bAgeKey && HOLCommandGridRules::IsAgeOpen(Context, Cell);
			if (bReached)
			{
				Result.Target = EHOLGridKeyTargetKind::Age;
				Result.ChosenAge = Cell;
			}
			else
			{
				Result.Target = bAgeKey ? EHOLGridKeyTargetKind::None : EHOLGridKeyTargetKind::Orders;
				Result.ChosenAge = -1;
			}
			return Result;
		}
		// An age chosen: every key is its buildings' (an empty cell: nothing, never an order nor another age), the age kept.
		Result.ChosenAge = ChosenAge;
		FHOLGridCommandView Building = HOLCommandGridRules::GetAgeBuilding(Context, ChosenAge, Cell);
		if (Building.Command != EHOLGridCommandKind::None && !Building.bGreyed)
		{
			Result.Target = EHOLGridKeyTargetKind::Build;
			Result.TypeName = Building.TypeName;
		}
		return Result;
	}

	// ClickAge (cpp 277-293): a click on an age's header (Cell -1) or on one of its cells.
	FHOLGridKeyResultView ClickAge(const FHOLGridContextView& Context, int ChosenAge, int Age, int Cell)
	{
		if (!HOLCommandGridRules::IsBuildGrid(Context) || !HOLCommandGridRules::IsAgeOpen(Context, Age))
		{
			FHOLGridKeyResultView Nothing;
			Nothing.ChosenAge = ChosenAge;
			return Nothing;
		}
		if (Cell == -1)
		{
			FHOLGridKeyResultView Header;
			Header.Target = EHOLGridKeyTargetKind::Age;
			Header.ChosenAge = Age;
			return Header;
		}
		return PressKey(Context, Age, Cell);
	}

	// GetChosenAgeAfter (cpp 295-298): the age stays open after a placement with Shift, closes on every other event.
	int GetChosenAgeAfter(int ChosenAge, EHOLAgeEventKind Event)
	{
		return Event == EHOLAgeEventKind::PlacedKept ? ChosenAge : -1;
	}

	// FindMissingResource (cpp 300-311), without the out parameter: the first resource (Food, Wood, Stone, Gold) the stock does not cover, or -1.
	int FindMissingResource(const FHOLResources& Stock, const FHOLResources& Cost)
	{
		if (Stock.Food < Cost.Food)
		{
			return int(EHOLResource::Food);
		}
		if (Stock.Wood < Cost.Wood)
		{
			return int(EHOLResource::Wood);
		}
		if (Stock.Stone < Cost.Stone)
		{
			return int(EHOLResource::Stone);
		}
		if (Stock.Gold < Cost.Gold)
		{
			return int(EHOLResource::Gold);
		}
		return -1;
	}

	bool HasMissingResource(const FHOLResources& Stock, const FHOLResources& Cost)
	{
		return FindMissingResource(Stock, Cost) >= 0;
	}

	// IsPopulationBlocked (cpp 371-374).
	bool IsPopulationBlocked(int PopulationUsed, int PopulationCap, int Places)
	{
		return PopulationUsed + Math::Max(Places, 0) > PopulationCap;
	}

	// CheckProduction (cpp 313-330): a full queue first, then the cost (the first resource missing), then the population (which only waits, see Blocks).
	FHOLFeedbackView CheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, int QueueLength, int MaxQueue, int PopulationUsed, int PopulationCap, int Places)
	{
		FHOLFeedbackView Feedback;
		int Missing = FindMissingResource(Stock, Cost);
		if (QueueLength >= MaxQueue)
		{
			Feedback.Kind = EHOLCommandFeedbackKind::QueueFull;
		}
		else if (Missing >= 0)
		{
			Feedback.Resource = EHOLResource(Missing);
			Feedback.Kind = EHOLCommandFeedbackKind::NotEnoughResources;
		}
		else if (IsPopulationBlocked(PopulationUsed, PopulationCap, Places))
		{
			Feedback.Kind = EHOLCommandFeedbackKind::PopulationFull;
		}
		return Feedback;
	}

	// CheckBuild (cpp 355-363).
	FHOLFeedbackView CheckBuild(const FHOLResources& Stock, const FHOLResources& Cost)
	{
		FHOLFeedbackView Feedback;
		int Missing = FindMissingResource(Stock, Cost);
		if (Missing >= 0)
		{
			Feedback.Resource = EHOLResource(Missing);
			Feedback.Kind = EHOLCommandFeedbackKind::NotEnoughResources;
		}
		return Feedback;
	}

	// Blocks (cpp 365-369): the feedbacks that stop the command (a full population only waits).
	bool Blocks(const FHOLFeedbackView& Feedback)
	{
		return Feedback.Kind == EHOLCommandFeedbackKind::NotEnoughResources || Feedback.Kind == EHOLCommandFeedbackKind::QueueFull
			|| Feedback.Kind == EHOLCommandFeedbackKind::PlacementRefused || Feedback.Kind == EHOLCommandFeedbackKind::ResearchRefused;
	}

	// The names StaticEnum<>::GetNameStringByValue gives in the C++ Describe.
	FString GetResourceName(EHOLResource Resource)
	{
		if (Resource == EHOLResource::Food)
		{
			return "Food";
		}
		if (Resource == EHOLResource::Wood)
		{
			return "Wood";
		}
		if (Resource == EHOLResource::Stone)
		{
			return "Stone";
		}
		if (Resource == EHOLResource::Gold)
		{
			return "Gold";
		}
		return "";
	}

	FString GetCategoryName(EHOLBuildingCategory Category)
	{
		if (Category == EHOLBuildingCategory::Economic)
		{
			return "Economic";
		}
		if (Category == EHOLBuildingCategory::Military)
		{
			return "Military";
		}
		if (Category == EHOLBuildingCategory::Technological)
		{
			return "Technological";
		}
		return "";
	}

	// HOLTechnology::DescribeBlock (HOLTechnologyRules.cpp): the words of a research refusal, from the block's number.
	FString DescribeBlock(int Block)
	{
		if (Block == BlockNotImplemented)
		{
			return "soon";
		}
		if (Block == BlockAlreadyResearched)
		{
			return "already researched";
		}
		if (Block == BlockAlreadyQueued)
		{
			return "already queued";
		}
		if (Block == BlockTierTooLow)
		{
			return "tier too low";
		}
		if (Block == BlockHeroLevelTooLow)
		{
			return "hero level too low";
		}
		if (Block == BlockRequiresMissing)
		{
			return "requires missing";
		}
		if (Block == BlockNoBuilding)
		{
			return "no building";
		}
		if (Block == BlockCannotPay)
		{
			return "cannot pay";
		}
		return "none";
	}

	// Describe (cpp 376-400): the words of the log for a feedback (the words of UHOLConstructionSubsystem::DescribeRefusal for a placement).
	FString Describe(const FHOLFeedbackView& Feedback)
	{
		if (Feedback.Kind == EHOLCommandFeedbackKind::NotEnoughResources)
		{
			return f"not enough {GetResourceName(Feedback.Resource)}";
		}
		if (Feedback.Kind == EHOLCommandFeedbackKind::QueueFull)
		{
			return "queue full";
		}
		if (Feedback.Kind == EHOLCommandFeedbackKind::PopulationFull)
		{
			return "population full";
		}
		if (Feedback.Kind == EHOLCommandFeedbackKind::ResearchRefused)
		{
			return f"research refused ({DescribeBlock(Feedback.Block)})";
		}
		if (Feedback.Kind == EHOLCommandFeedbackKind::PlacementRefused)
		{
			if (Feedback.Refusal == RefusalOutOfMap)
			{
				return "placement refused (out of the map)";
			}
			if (Feedback.Refusal == RefusalBuilding)
			{
				return "placement refused (on a building)";
			}
			if (Feedback.Refusal == RefusalSource)
			{
				return "placement refused (on a resource source)";
			}
			if (Feedback.Refusal == RefusalEnemyCentre)
			{
				return "placement refused (too near an enemy main centre)";
			}
			if (Feedback.Refusal == RefusalNavMesh)
			{
				return "placement refused (off the navmesh)";
			}
			return "placement refused";
		}
		if (Feedback.Kind == EHOLCommandFeedbackKind::NoBuilding)
		{
			return f"no {GetCategoryName(Feedback.Category)} building";
		}
		return "none";
	}

	// GetCellFace (cpp 402-408): a cell shows its icon, or its name while it has none.
	FHOLGridCellFaceView GetCellFace(bool bHasIcon)
	{
		FHOLGridCellFaceView Face;
		Face.bIcon = bHasIcon;
		Face.bName = !bHasIcon;
		return Face;
	}

	// IsSelectedByCategory (cpp 415-418), on the category of the type (the row itself is not exposed): the library's IsCategorySelectedByCategory.
	bool IsCategorySelectedByCategory(EHOLBuildingCategory TypeCategory, EHOLBuildingCategory Category)
	{
		return TypeCategory == Category;
	}
}
