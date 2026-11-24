// P1a (lot P1 of the C++ -> Angelscript migration, step 5): the player's orders, first half. The rules of HOLPlayerOrders.cpp:16-124 (IsAccepted, Move,
// Attack, AttackBuilding, AttackMove, Stop, ToggleDeploy) in script. NOT CALLED BY THE GAME YET: HOLPlayerOrders.cpp still decides until the C++ bridge
// (P1b) replaces its bodies; until then the rule exists twice, and HOLPlayerOrders_Test.as covers the same cases as the C++ (agreement to prove in P1b).
// SendNonWorkersTo stays C++ (it reads HOLUnitView). NOT COMPILED at the time of writing.
// Every emitter takes the command subsystem (GetCommands(World) in the game, MakeDetachedForTest() in a test), gates the order, then calls the
// Issue*Player binding, which opens the player-order scope in C++ (the chain of the order is drawn, D328). Returns false when the order is refused.

namespace HOLPlayerOrders
{
	// The most units one order may name (HOLPlayerOrders.h MaxOrderedUnits, 200). A single source: the C++ constant goes in P1b.
	const int MaxOrderedUnitsValue = 200;

	int GetMaxOrderedUnits()
	{
		return MaxOrderedUnitsValue;
	}

	// An order of PlayerIndex naming NumUnits units may enter the queue: 0 <= PlayerIndex < MaxSlots and 0 <= NumUnits <= MaxOrderedUnits.
	bool IsAccepted(int PlayerIndex, int NumUnits)
	{
		return PlayerIndex >= 0 && PlayerIndex < HOLCommandScript::GetMaxSlots() && NumUnits >= 0 && NumUnits <= MaxOrderedUnitsValue;
	}

	bool Move(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, const FVector& Target, bool bQueue, const FVector2D& FrontLine, bool bFrontWidth)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueMovePlayer(Commands, UnitIds, Target, PlayerIndex, bQueue, FrontLine, bFrontWidth);
		HOLFactionPowerServer::NoteHeroOrder(PlayerIndex, UnitIds); // D909 (GLORFINDEL-1124sc): a new order naming the hero cancels his pending cast
		return true;
	}

	bool Attack(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, int TargetUnitId, bool bQueue)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueAttackPlayer(Commands, UnitIds, TargetUnitId, PlayerIndex, bQueue);
		HOLFactionPowerServer::NoteHeroOrder(PlayerIndex, UnitIds);
		return true;
	}

	bool AttackBuilding(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, bool bQueue)
	{
		if (Commands == nullptr || Building == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueAttackBuildingPlayer(Commands, UnitIds, Building, PlayerIndex, bQueue);
		HOLFactionPowerServer::NoteHeroOrder(PlayerIndex, UnitIds);
		return true;
	}

	bool AttackMove(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, const FVector& Target, bool bQueue)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueAttackMovePlayer(Commands, UnitIds, Target, PlayerIndex, bQueue);
		HOLFactionPowerServer::NoteHeroOrder(PlayerIndex, UnitIds);
		return true;
	}

	// bHold: Hold, else Stop. No queue flag (a stop empties the queue).
	bool Stop(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, bool bHold)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueStopPlayer(Commands, UnitIds, PlayerIndex, bHold);
		HOLFactionPowerServer::NoteHeroOrder(PlayerIndex, UnitIds);
		return true;
	}

	bool ToggleDeploy(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds)
	{
		UHOLFactionPendingHolder Holder = UHOLFactionPendingHolder::Get();
		if (Holder == nullptr)
		{
			TArray<FHOLPendingCast> NoPending;
			return ToggleDeployFor(Commands, PlayerIndex, UnitIds, NoPending, -1);
		}
		return ToggleDeployFor(Commands, PlayerIndex, UnitIds, Holder.Pending, HOLHero::GetHeroUnitId(PlayerIndex));
	}

	// D909 seam (GLORFINDEL-1124se): the pending list and the slot's hero id are arguments (the wrapper above reads the world's; a detached test passes its own).
	bool ToggleDeployFor(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, TArray<FHOLPendingCast>& Pending, int HeroId)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		HOLCommandScript::IssueToggleDeployPlayer(Commands, UnitIds, PlayerIndex);
		return true;
	}

	// ---------------- P2: second half of HOLPlayerOrders.cpp (Gather, Build, DropOff, Place, QueueProduction, MarketTrade, SetRally, Garrison, UngarrisonAll) ----------------
	// The workers of Gather / Build / DropOff arrive ALREADY FILTERED by the C++ SendNonWorkersTo (it reads HOLUnitView, not exposed: it walks the soldiers and
	// removes them from the list, then asks this rule). Game data the rule needs but the script cannot read comes as an argument computed by C++: bSource and
	// SourceLocation (UHOLSourceSubsystem), bDeposit (building type row: Accepts, TakesDeliveries, owner), bSourceStanding (SetRally).

	// Gather: with Shift and a source standing, through the queue of moves (waits for the walks, EtapeDefini L.6); else the gather queue. Accepted even with no worker left.
	bool Gather(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& Workers, int SourceIndex, bool bQueue, bool bSource, const FVector& SourceLocation)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, Workers.Num()))
		{
			return false;
		}
		if (Workers.Num() > 0)
		{
			if (bQueue && bSource)
			{
				AHOLBuilding NoSite = nullptr;
				HOLCommandScript::IssueQueuedWorkPlayer(Commands, Workers, SourceIndex, NoSite, SourceLocation, PlayerIndex);
			}
			else
			{
				HOLCommandScript::IssueGatherPlayer(Commands, Workers, SourceIndex, PlayerIndex, bQueue);
			}
		}
		return true;
	}

	// Build: a site to help; with Shift through the queue of moves (Shift + Pb then Shift + a site), else the build queue.
	bool Build(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& Workers, AHOLBuilding Building, bool bQueue)
	{
		if (Commands == nullptr || Building == nullptr || !IsAccepted(PlayerIndex, Workers.Num()))
		{
			return false;
		}
		if (Workers.Num() > 0)
		{
			if (bQueue)
			{
				HOLCommandScript::IssueQueuedWorkPlayer(Commands, Workers, -1, Building, Building.GetActorLocation(), PlayerIndex);
			}
			else
			{
				HOLCommandScript::IssueBuildPlayer(Commands, Workers, Building, PlayerIndex, bQueue);
			}
		}
		return true;
	}

	// DropOff: not one of the player's deposits (bDeposit false): all walk to Point; else the drop-off move (carriers unload, the others walk), Workers filtered by C++ with Shift.
	bool DropOff(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, const FVector& Point, bool bQueue, bool bDeposit)
	{
		UHOLFactionPendingHolder Holder = UHOLFactionPendingHolder::Get();
		if (Holder == nullptr)
		{
			TArray<FHOLPendingCast> NoPending;
			return DropOffFor(Commands, PlayerIndex, UnitIds, Building, Point, bQueue, bDeposit, NoPending, -1);
		}
		return DropOffFor(Commands, PlayerIndex, UnitIds, Building, Point, bQueue, bDeposit, Holder.Pending, HOLHero::GetHeroUnitId(PlayerIndex));
	}

	// D909 seam (GLORFINDEL-1124se): same as ToggleDeployFor.
	bool DropOffFor(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building, const FVector& Point, bool bQueue, bool bDeposit, TArray<FHOLPendingCast>& Pending, int HeroId)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, UnitIds.Num()))
		{
			return false;
		}
		if (!bDeposit)
		{
			HOLCommandScript::IssueMovePlayer(Commands, UnitIds, Point, PlayerIndex, bQueue, FVector2D(0.0, 0.0), false);
			return true;
		}
		if (UnitIds.Num() > 0)
		{
			HOLCommandScript::IssueDropOffMovePlayer(Commands, UnitIds, Building, Point, PlayerIndex, bQueue);
		}
		return true;
	}

	// Place: too many builders: the foundation is placed all the same, without them (the slot is checked with 0 units).
	bool Place(UHOLCommandSubsystem Commands, int PlayerIndex, FName TypeName, const FVector& Point, const TArray<int>& UnitIds, bool bQueue)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, 0))
		{
			return false;
		}
		TArray<int> Builders;
		if (UnitIds.Num() <= MaxOrderedUnitsValue)
		{
			Builders = UnitIds;
		}
		HOLCommandScript::IssuePlace(Commands, PlayerIndex, TypeName, Point, Builders, bQueue);
		return true;
	}

	bool QueueProduction(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building, FName TypeName)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, 0) || Building == nullptr)
		{
			return false;
		}
		HOLCommandScript::IssueProductionQueue(Commands, PlayerIndex, Building, TypeName, false);
		return true;
	}

	// D893 step 3: the server's production order of a seat whose specialization read is Spec and faction Faction (AHOLPlayerState::GetSpecialization, GetSlotFaction):
	// a lane unit the seat has not unlocked is refused (false, nothing queued); every other unit goes through QueueProduction unchanged.
	bool QueueProductionFor(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building, FName TypeName, const FHOLSpecializationView& Spec, EHOLFaction Faction)
	{
		if (!HOLSpecUnlock::CanProduce(TypeName, Spec, Faction))
		{
			return false;
		}
		return QueueProduction(Commands, PlayerIndex, Building, TypeName);
	}

	bool MarketTrade(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building, int Resource, int Lots, bool bBuy)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, 0) || Building == nullptr)
		{
			return false;
		}
		HOLCommandScript::IssueProductionTrade(Commands, PlayerIndex, Building, Resource, Lots, bBuy);
		return true;
	}

	// SetRally: a source that is not standing (any more) is the ground: bSourceStanding comes from C++ (UHOLSourceSubsystem).
	bool SetRally(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building, const FVector& Point, int SourceIndex, bool bSourceStanding, AHOLBuilding Site, bool bQueue)
	{
		if (Commands == nullptr || !IsAccepted(PlayerIndex, 0) || Building == nullptr)
		{
			return false;
		}
		int Source = bSourceStanding ? SourceIndex : -1;
		HOLCommandScript::IssueRallyOrder(Commands, PlayerIndex, Building, Point, Source, Site, bQueue);
		return true;
	}

	// Garrison: the building takes the order (C++ TakesGarrisonOrder: CVar, owner, finished, places) and at least one unit.
	bool Garrison(UHOLCommandSubsystem Commands, int PlayerIndex, const TArray<int>& UnitIds, AHOLBuilding Building)
	{
		if (Commands == nullptr || Building == nullptr || UnitIds.Num() == 0 || !IsAccepted(PlayerIndex, UnitIds.Num()) || !HOLCommandScript::TakesGarrisonOrder(Building, PlayerIndex))
		{
			return false;
		}
		HOLCommandScript::IssueGarrison(Commands, PlayerIndex, UnitIds, Building);
		return true;
	}

	bool UngarrisonAll(UHOLCommandSubsystem Commands, int PlayerIndex, AHOLBuilding Building)
	{
		if (Commands == nullptr || Building == nullptr || !IsAccepted(PlayerIndex, 1) || !HOLCommandScript::OwnsBuilding(Building, PlayerIndex))
		{
			return false;
		}
		HOLCommandScript::IssueUngarrisonAll(Commands, PlayerIndex, Building);
		return true;
	}}