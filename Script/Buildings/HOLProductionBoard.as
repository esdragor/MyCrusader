// The order of the gates of a production order and the cancel of a queued item (MIG-CPP step 5, lot R2a; D839: the C++ keeps the world, the replicated
// queue, the stock and the journal; the verdict is the script's). Pure with parameters: nothing here reads the world, an actor or the player state.
// UHOLProductionSubsystem::ApplyOrders (Buildings/HOLProductionSubsystem.cpp) reads the facts, asks UHOLProductionBoardScript and acts on the
// whole-number answer, writing the same journal lines as before. The formula of the cancel is NOT copied: the cost given back is the exposed C++ rule
// (HOLProductionRead::Cancel, HOLProduction::Cancel), called here; only the ORDER of the gates is new.
// Gates, in the C++ order: no player state (Warning line), not its building, no type row (silent), building not finished unless an early rally (a
// rally of a building that will produce, kept from its ghost on), main forum in ruin unless a rally (its rally point stays its owner's to move).
// NOT in a Dev / Developers / Examples / Editor folder: the dedicated server runs it.

/** The verdict of the gates of a production order, in the order of the gates (the bridge sends it as a whole number; the C++ fallback uses the same order). */
enum EHOLOrderGateVerdict
{
	Pass,
	NoPlayerState,
	NotItsBuilding,
	NoType,
	NotFinished,
	ForumDown
}

/** The cancel of a queued item: carried out (cost back, item removed), or ignored without a line (an index a client may name freely). */
enum EHOLProductionCancelVerdict
{
	CancelItem,
	IgnoreOrder
}

/** The verdict of a queue order (a unit, a siege engine or a research), as a whole number for the bridge; the codes of the refusals 1 to 3 are those of EHOLProductionRefusal. */
enum EHOLQueueVerdict
{
	Enqueue,
	NotProduced,
	QueueFull,
	NotEnoughResources,
	TechFaction,
	TechBlocked,
	SiegeBlocked,
	TierTooLow,
	UnitFaction
}

/** The outcome of a Rally order once the gates passed (lot R3): the C++ effect of each, and its journal line, stay in ApplyOrders. */
enum EHOLRallyOutcome
{
	NoStep,
	Clear,
	AddStep,
	Set
}

namespace HOLProductionBoard
{
	/**
	 * bHasState = the player has a state (a stock to pay with); bOwner = the building exists and its owner sent the order; bHasType = the building has a
	 * type row; bFinished = the building is finished; bEarlyRally = the order is a Rally of a building that produces something; bForumDown = the main
	 * forum is in ruin or rebuilding; bRally = the order is a Rally. The C++ computes the facts; here is only their order.
	 */
	EHOLOrderGateVerdict OrderGateVerdict(bool bHasState, bool bOwner, bool bHasType, bool bFinished, bool bEarlyRally, bool bForumDown, bool bRally)
	{
		if (!bHasState)
			return EHOLOrderGateVerdict::NoPlayerState;
		if (!bOwner)
			return EHOLOrderGateVerdict::NotItsBuilding;
		if (!bHasType)
			return EHOLOrderGateVerdict::NoType;
		if (!bFinished && !bEarlyRally)
			return EHOLOrderGateVerdict::NotFinished;
		if (bForumDown && !bRally)
			return EHOLOrderGateVerdict::ForumDown;
		return EHOLOrderGateVerdict::Pass;
	}

	/**
	 * The outcome of a Rally order (EtapeDefini N.1, AoE4 Shift). On the building's own body: Shift with a rally point adds nothing (NoStep), otherwise
	 * the rally point is cleared (the units are born at its foot, D283). Elsewhere: Shift with a rally point adds a step (AddStep: the C++ effect may
	 * still refuse it at MaxRallySteps and says so), otherwise the rally point is set (the first step is the rally point itself).
	 */
	EHOLRallyOutcome RallyOutcome(bool bOnBody, bool bShift, bool bHasRally)
	{
		if (bShift && bHasRally && bOnBody)
			return EHOLRallyOutcome::NoStep;
		if (bOnBody)
			return EHOLRallyOutcome::Clear;
		if (bShift && bHasRally)
			return EHOLRallyOutcome::AddStep;
		return EHOLRallyOutcome::Set;
	}

	/**
	 * A named site of a Rally order is kept only if it is another building than the one that rallies, the player's own (a client could name any, D288),
	 * and a site still to build or a camp (finished or not). Dropped, the flag stays on the ground. The C++ computes the four facts of the actor.
	 */
	bool RallySiteKept(bool bOtherBuilding, bool bOwnedByPlayer, bool bNeedsBuilders, bool bIsCamp)
	{
		return bOtherBuilding && bOwnedByPlayer && (bNeedsBuilders || bIsCamp);
	}

	/** The cancel is carried out for an index inside the queue, ignored otherwise (HOLProduction::Cancel answers false for a wrong index, silently). */
	EHOLProductionCancelVerdict CancelVerdict(bool bValidIndex)
	{
		if (!bValidIndex)
			return EHOLProductionCancelVerdict::IgnoreOrder;
		return EHOLProductionCancelVerdict::CancelItem;
	}

	/**
	 * The verdict of a queue order, in the order of ApplyOrders. A RESEARCH (bTechnology: a known technology, not in the building's Produces): its faction
	 * (4), then its technology check (5: it says "cannot pay" itself, so a research with no resources AND a full queue stops here, not at 2), then the
	 * queue. A UNIT or siege engine: produced here (1), siege tier (6), age (7), faction (8), then the queue: full (2) BEFORE no resources (3). The queue
	 * and the cost are NOT recomputed here: the exposed C++ rule HOLProductionRead::TryEnqueue answers on a scratch queue of QueueLength items. The facts
	 * (bTechBlocked, bSiegeBlocked, bTierTooLow, the faction) come from HOLTechnology::Check, HOLSiege::Check, CheckUnit, IsUnitOpenTo.
	 */
	EHOLQueueVerdict QueueVerdict(bool bProduces, bool bTechnology, bool bTechFactionOpen, bool bTechBlocked, bool bSiegeBlocked, bool bTierTooLow, bool bUnitFactionOpen, int QueueLength, int MaxQueue, FHOLResources Stock, FHOLResources Cost)
	{
		if (bTechnology)
		{
			if (!bTechFactionOpen)
				return EHOLQueueVerdict::TechFaction;
			if (bTechBlocked)
				return EHOLQueueVerdict::TechBlocked;
		}
		else
		{
			if (!bProduces)
				return EHOLQueueVerdict::NotProduced;
			if (bSiegeBlocked)
				return EHOLQueueVerdict::SiegeBlocked;
			if (bTierTooLow)
				return EHOLQueueVerdict::TierTooLow;
			if (!bUnitFactionOpen)
				return EHOLQueueVerdict::UnitFaction;
		}
		FHOLProductionQueueState Scratch;
		for (int i = 0; i < QueueLength; i++)
		{
			Scratch.Types.Add(n"Item");
			Scratch.StartSteps.Add(-1);
		}
		FHOLResources Paying = Stock;
		int Refusal = HOLProductionRead::TryEnqueue(Scratch, n"Item", Cost, 1, 0, MaxQueue, Paying);
		if (Refusal == 2)
			return EHOLQueueVerdict::QueueFull;
		if (Refusal == 3)
			return EHOLQueueVerdict::NotEnoughResources;
		return EHOLQueueVerdict::Enqueue;
	}

	/** The stock once the cost of the cancelled item is given back (D282: in full): the C++ rule HOLProduction::Cancel, on a one-item scratch queue. */
	FHOLResources StockAfterCancel(FHOLResources Stock, FHOLResources Cost)
	{
		FHOLProductionQueueState Queue;
		FHOLResources Scratch = Cost;
		HOLProductionRead::TryEnqueue(Queue, n"Item", Cost, 1, 0, 1, Scratch);
		FHOLResources Result = Stock;
		HOLProductionRead::Cancel(Queue, 0, Cost, 1, 0, Result);
		return Result;
	}
}

/** The script side of UHOLProductionBoard: only asks the namespace. */
class UHOLProductionBoardScript : UHOLProductionBoard
{
	UFUNCTION(BlueprintOverride)
	int OrderGateAnswer(bool bHasState, bool bOwner, bool bHasType, bool bFinished, bool bEarlyRally, bool bForumDown, bool bRally)
	{
		return int(HOLProductionBoard::OrderGateVerdict(bHasState, bOwner, bHasType, bFinished, bEarlyRally, bForumDown, bRally));
	}

	UFUNCTION(BlueprintOverride)
	int CancelAnswer(bool bValidIndex)
	{
		return int(HOLProductionBoard::CancelVerdict(bValidIndex));
	}

	UFUNCTION(BlueprintOverride)
	int QueueAnswer(bool bProduces, bool bTechnology, bool bTechFactionOpen, bool bTechBlocked, bool bSiegeBlocked, bool bTierTooLow, bool bUnitFactionOpen, int QueueLength, int MaxQueue, FHOLResources Stock, FHOLResources Cost)
	{
		return int(HOLProductionBoard::QueueVerdict(bProduces, bTechnology, bTechFactionOpen, bTechBlocked, bSiegeBlocked, bTierTooLow, bUnitFactionOpen, QueueLength, MaxQueue, Stock, Cost));
	}

	UFUNCTION(BlueprintOverride)
	int RallyAnswer(bool bOnBody, bool bShift, bool bHasRally)
	{
		return int(HOLProductionBoard::RallyOutcome(bOnBody, bShift, bHasRally));
	}

	UFUNCTION(BlueprintOverride)
	int RallySiteAnswer(bool bOtherBuilding, bool bOwnedByPlayer, bool bNeedsBuilders, bool bIsCamp)
	{
		return HOLProductionBoard::RallySiteKept(bOtherBuilding, bOwnedByPlayer, bNeedsBuilders, bIsCamp) ? 1 : 0;
	}

	UFUNCTION(BlueprintOverride)
	FHOLResources StockAfterCancelAnswer(FHOLResources Stock, FHOLResources Cost)
	{
		return HOLProductionBoard::StockAfterCancel(Stock, Cost);
	}
}
