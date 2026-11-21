// The C++ controller's call into the Follow rule (GLORFINDEL-1121sk, D839). UHOLFollowBridge (Source/HeroesOfLiberty/Player/HOLFollowBridge.h) finds this class
// from AHOLPlayerController (OrderAt: right click on an ally; Tick: the order step). Envelopes only: each override is a call of HOLFollowOrders.as, the single source.
// bPlayerOrderActive is passed false (no API reads "the unit holds a player order" yet). NOT COMPILED.
class UHOLFollowBridgeScript : UHOLFollowBridge
{
	UFUNCTION(BlueprintOverride)
	bool RightClickOnUnit(AHOLPlayerController Controller, TArray<int> Selected, int TargetId, bool bTargetIsAlly)
	{
		UHOLFollowBookHolder Holder = UHOLFollowBookHolder::Get();
		if (Holder == nullptr)
			return false;
		return HOLFollowOrders::ApplyRightClick(Holder.Book, Selected, true, TargetId, bTargetIsAlly) == EHOLRightClickOrder::Follow;
	}

	UFUNCTION(BlueprintOverride)
	void ManualOrder(TArray<int> UnitIds)
	{
		UHOLFollowBookHolder Holder = UHOLFollowBookHolder::Get();
		if (Holder == nullptr)
			return;
		for (int UnitId : UnitIds)
		{
			HOLFollowOrders::OnManualOrder(Holder.Book, UnitId);
		}
	}

	UFUNCTION(BlueprintOverride)
	void Step(AHOLPlayerController Controller, int PlayerIndex)
	{
		UHOLFollowBookHolder Holder = UHOLFollowBookHolder::Get();
		if (Holder == nullptr || Holder.Book.UnitIds.Num() == 0)
			return;
		UHOLCommandSubsystem Commands = HOLCommandScript::GetCommands();
		if (Commands == nullptr)
			return;
		// Backwards: a finished follow (unit or target gone) is removed from the book while iterating.
		for (int Index = Holder.Book.UnitIds.Num() - 1; Index >= 0; Index--)
		{
			const int UnitId = Holder.Book.UnitIds[Index];
			if (HOLFollowOrders::StepUnit(Controller, Commands, PlayerIndex, Holder.Book, UnitId, false))
				continue;
			Holder.Book.UnitIds.RemoveAt(Index);
			Holder.Book.TargetIds.RemoveAt(Index);
			Holder.Book.States.RemoveAt(Index);
			Holder.Book.LastAttackIds.RemoveAt(Index);
		}
	}
}
