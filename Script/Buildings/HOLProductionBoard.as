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

	/** The cancel is carried out for an index inside the queue, ignored otherwise (HOLProduction::Cancel answers false for a wrong index, silently). */
	EHOLProductionCancelVerdict CancelVerdict(bool bValidIndex)
	{
		if (!bValidIndex)
			return EHOLProductionCancelVerdict::IgnoreOrder;
		return EHOLProductionCancelVerdict::CancelItem;
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
	FHOLResources StockAfterCancelAnswer(FHOLResources Stock, FHOLResources Cost)
	{
		return HOLProductionBoard::StockAfterCancel(Stock, Cost);
	}
}
