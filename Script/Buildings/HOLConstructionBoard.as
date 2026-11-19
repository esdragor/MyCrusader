// The order of the gates of a building's placement and cancel (MIG-CPP step 5, lot K1a; D839: the C++ keeps the world, the replicated actor and the
// stock; the verdict is the script's). Pure with parameters: nothing here reads the world, an actor or the player state. UHOLConstructionSubsystem::Place
// and ::Cancel (Buildings/HOLConstructionSubsystem.cpp) read what they need, ask UHOLConstructionBoardScript and act on the whole-number answer, and write
// the same journal lines and refusals as before. The formulas are NOT copied: IsAgeOpen, TryPay, CanCancel and GetCancelRefund are the exposed C++
// rules (UHOLConstructionReadLibrary, namespace HOLConstructionRead), called here; only their ORDER is new.
// Place gates, in the C++ order: buildable, faction, age, placement, payment. The stock is paid by the C++ only AFTER the foundation is born (a player
// who pays without a building is a bug): StockAfterPlace is the stock to write then, never an earlier write.
// NOT in a Dev / Developers / Examples / Editor folder: the dedicated server runs it.

/** The verdict of a placement, in the order of the gates (the bridge sends it as a whole number; the C++ fallback uses the same order). */
enum EHOLPlaceVerdict
{
	Ok,
	NotBuildable,
	NotOfFaction,
	AgeNotReached,
	PlacementRefused,
	NotEnoughResources
}

/** The verdict of a cancel: the order is carried out (stock refunded, foundation destroyed), or ignored (a client may name any building). */
enum EHOLCancelVerdict
{
	Allowed,
	Ignored
}

/** K1b: the refund of a production queue is made (RefundQueue) or there is nothing to refund. */
enum EHOLQueueRefundVerdict
{
	RefundQueue,
	NothingToRefund
}

/** K1b: the building is destroyed (DestroyIt) or left as it is (LeaveIt). */
enum EHOLDestroyVerdict
{
	DestroyIt,
	LeaveIt
}

/** K1b: rubble is left where the building stood, or not. */
enum EHOLRubbleVerdict
{
	NoRubble,
	LeaveRubble
}

namespace HOLConstructionBoard
{
	/**
	 * Refusal = EHOLPlacementRefusal as a whole number (0 None). bBuildable = the row exists and the player's seat may build it
	 * (UHOLBuildingSettings::IsBuildableBy); bFactionOpen = HOLFactionRules::IsBuildingOpenTo. Stock and Cost are not changed here.
	 */
	EHOLPlaceVerdict PlaceVerdict(bool bBuildable, bool bFactionOpen, int BuildingAge, int Tier, int Refusal, FHOLResources Stock, FHOLResources Cost)
	{
		if (!bBuildable)
			return EHOLPlaceVerdict::NotBuildable;
		if (!bFactionOpen)
			return EHOLPlaceVerdict::NotOfFaction;
		if (!HOLConstructionRead::IsAgeOpen(BuildingAge, Tier))
			return EHOLPlaceVerdict::AgeNotReached;
		if (Refusal != 0)
			return EHOLPlaceVerdict::PlacementRefused;
		FHOLResources Paid = Stock;
		if (!HOLConstructionRead::TryPay(Paid, Cost))
			return EHOLPlaceVerdict::NotEnoughResources;
		return EHOLPlaceVerdict::Ok;
	}

	/** The stock once Cost is taken; the stock unchanged when it does not cover the cost (TryPay leaves it alone). */
	FHOLResources StockAfterPlace(FHOLResources Stock, FHOLResources Cost)
	{
		FHOLResources Paid = Stock;
		HOLConstructionRead::TryPay(Paid, Cost);
		return Paid;
	}

	/** Only the owner cancels, and only what is not finished (D311). State = EHOLConstructionState as a whole number. */
	EHOLCancelVerdict CancelVerdict(bool bHasBuilding, int OwnerPlayer, int AskerPlayer, int State)
	{
		if (!bHasBuilding || OwnerPlayer != AskerPlayer || !HOLConstructionRead::CanCancel(State))
			return EHOLCancelVerdict::Ignored;
		return EHOLCancelVerdict::Allowed;
	}

	/** D345: the share of the cost not built; nothing when the building has no type row. */
	FHOLResources CancelRefund(bool bHasType, FHOLResources Cost, int State, int64 Work, int64 TotalWork)
	{
		if (!bHasType)
			return FHOLResources();
		return HOLConstructionRead::GetCancelRefund(Cost, State, Work, TotalWork);
	}

	/** K1b: refund of a building's production queue. The server refunds a non-empty queue; a client, no world or an empty queue: nothing. */
	EHOLQueueRefundVerdict QueueRefundVerdict(bool bHasWorld, bool bAuthority, int QueueLength)
	{
		if (!bHasWorld || !bAuthority || QueueLength <= 0)
			return EHOLQueueRefundVerdict::NothingToRefund;
		return EHOLQueueRefundVerdict::RefundQueue;
	}

	/** Every item of the queue back in full, the head too: the sum of the costs the C++ read (one per item). */
	FHOLResources QueueRefund(TArray<FHOLResources> ItemCosts)
	{
		FHOLResources Total;
		for (int i = 0; i < ItemCosts.Num(); ++i)
		{
			Total.Food += ItemCosts[i].Food;
			Total.Wood += ItemCosts[i].Wood;
			Total.Stone += ItemCosts[i].Stone;
			Total.Gold += ItemCosts[i].Gold;
		}
		return Total;
	}

	/** K1b: a building is destroyed by the server only, and only a valid actor (the site's share is CancelRefund, D345). */
	EHOLDestroyVerdict DestroyVerdict(bool bHasWorld, bool bAuthority, bool bValid)
	{
		if (!bHasWorld || !bAuthority || !bValid)
			return EHOLDestroyVerdict::LeaveIt;
		return EHOLDestroyVerdict::DestroyIt;
	}

	/** D345.2a: rubble where it stood, none for a bare foundation (the others never saw it) nor for a building without a type row. */
	EHOLRubbleVerdict RubbleVerdict(bool bFoundation, bool bHasType)
	{
		if (bFoundation || !bHasType)
			return EHOLRubbleVerdict::NoRubble;
		return EHOLRubbleVerdict::LeaveRubble;
	}
}

/** The script side of UHOLConstructionBoard: only asks the namespace. */
class UHOLConstructionBoardScript : UHOLConstructionBoard
{
	UFUNCTION(BlueprintOverride)
	int PlaceAnswer(bool bBuildable, bool bFactionOpen, int BuildingAge, int Tier, int Refusal, FHOLResources Stock, FHOLResources Cost)
	{
		return int(HOLConstructionBoard::PlaceVerdict(bBuildable, bFactionOpen, BuildingAge, Tier, Refusal, Stock, Cost));
	}

	UFUNCTION(BlueprintOverride)
	FHOLResources StockAfterPlaceAnswer(FHOLResources Stock, FHOLResources Cost)
	{
		return HOLConstructionBoard::StockAfterPlace(Stock, Cost);
	}

	UFUNCTION(BlueprintOverride)
	int CancelAnswer(bool bHasBuilding, int OwnerPlayer, int AskerPlayer, int State)
	{
		return int(HOLConstructionBoard::CancelVerdict(bHasBuilding, OwnerPlayer, AskerPlayer, State));
	}

	UFUNCTION(BlueprintOverride)
	FHOLResources CancelRefundAnswer(bool bHasType, FHOLResources Cost, int State, int64 Work, int64 TotalWork)
	{
		return HOLConstructionBoard::CancelRefund(bHasType, Cost, State, Work, TotalWork);
	}

	UFUNCTION(BlueprintOverride)
	int QueueRefundAnswer(bool bHasWorld, bool bAuthority, int QueueLength)
	{
		return int(HOLConstructionBoard::QueueRefundVerdict(bHasWorld, bAuthority, QueueLength));
	}

	UFUNCTION(BlueprintOverride)
	FHOLResources QueueTotalAnswer(TArray<FHOLResources> ItemCosts)
	{
		return HOLConstructionBoard::QueueRefund(ItemCosts);
	}

	UFUNCTION(BlueprintOverride)
	int DestroyAnswer(bool bHasWorld, bool bAuthority, bool bValid)
	{
		return int(HOLConstructionBoard::DestroyVerdict(bHasWorld, bAuthority, bValid));
	}

	UFUNCTION(BlueprintOverride)
	int RubbleAnswer(bool bFoundation, bool bHasType)
	{
		return int(HOLConstructionBoard::RubbleVerdict(bFoundation, bHasType));
	}
}
