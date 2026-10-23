// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLCommandGrid.h"

#include "Buildings/HOLBuildingTypes.h"

namespace HOLCommandGrid
{
	FHOLGridCommand GetCommand(const FHOLGridContext& Context, const int32 Cell)
	{
		FHOLGridCommand Result;
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
					Result.Command = EHOLGridCommand::CancelConstruction;
				}
				return Result;
			}
			if (Context.Produces.IsValidIndex(Cell))
			{
				Result.Command = EHOLGridCommand::Produce;
				Result.TypeName = Context.Produces[Cell];
			}
			return Result;
		}
		return Result;
	}

	FHOLGridCommand GetOrder(const FHOLGridContext& Context, const int32 Cell)
	{
		FHOLGridCommand Result;
		if (Context.bOwnUnits && !Context.bOwnBuilding)
		{
			Result.Command = Cell == AttackMoveCell && Context.bSoldiers ? EHOLGridCommand::AttackMove
				: Cell == StopCell ? EHOLGridCommand::Stop : Cell == HoldCell ? EHOLGridCommand::Hold : EHOLGridCommand::None;
		}
		return Result;
	}

	EMouseCursor::Type GetMouseCursor(const bool bAttackMoveArmed, const bool bPlacing)
	{
		return bAttackMoveArmed && !bPlacing ? EMouseCursor::Crosshairs : EMouseCursor::Default;
	}

	bool IsBuildGrid(const FHOLGridContext& Context)
	{
		return !Context.bOwnBuilding && Context.bBuilders && !Context.bSoldiers;
	}

	int32 GetReachedAge()
	{
		return 0;
	}

	bool IsAgeReached(const FHOLGridContext& Context, const int32 Age)
	{
		return Age >= 0 && Age < NumAges && Age <= Context.ReachedAge;
	}

	int32 GetCategoryRow(const EHOLBuildingCategory Category)
	{
		return Category == EHOLBuildingCategory::Economic ? 0 : Category == EHOLBuildingCategory::Military ? 1 : 2;
	}

	FHOLGridCommand GetAgeBuilding(const FHOLGridContext& Context, const int32 Age, const int32 Cell)
	{
		FHOLGridCommand Result;
		if (!IsBuildGrid(Context) || Age < 0 || Age >= NumAges || Cell < 0 || Cell >= NumCells)
		{
			return Result;
		}
		for (const FHOLBuildable& Buildable : Context.Buildable)
		{
			if (Buildable.Cell == Cell && Buildable.Age == Age && !Buildable.TypeName.IsNone())
			{
				Result.Command = EHOLGridCommand::Build;
				Result.TypeName = Buildable.TypeName;
				Result.Age = Age;
				Result.bGreyed = !IsAgeReached(Context, Age);
				break;
			}
		}
		return Result;
	}

	FHOLGridKeyResult PressKey(const FHOLGridContext& Context, const int32 ChosenAge, const int32 Cell)
	{
		FHOLGridKeyResult Result;
		if (Cell < 0 || Cell >= NumCells)
		{
			Result.ChosenAge = ChosenAge;
			return Result;
		}
		if (Context.bOwnBuilding)
		{
			Result.Target = EHOLGridKeyTarget::Grid;
			return Result;
		}
		if (!IsBuildGrid(Context))
		{
			Result.Target = EHOLGridKeyTarget::Orders;
			return Result;
		}
		// No age chosen: A Z E R choose one (one not reached: nothing), the others are the orders'.
		if (ChosenAge == INDEX_NONE)
		{
			const bool bAgeKey = Cell < NumAges;
			const bool bReached = bAgeKey && IsAgeReached(Context, Cell);
			Result.Target = bReached ? EHOLGridKeyTarget::Age : bAgeKey ? EHOLGridKeyTarget::None : EHOLGridKeyTarget::Orders;
			Result.ChosenAge = bReached ? Cell : INDEX_NONE;
			return Result;
		}
		// An age chosen: every key is its buildings' (an empty cell: nothing, never an order nor another age), the age kept.
		Result.ChosenAge = ChosenAge;
		const FHOLGridCommand Building = GetAgeBuilding(Context, ChosenAge, Cell);
		if (!Building.IsNone() && !Building.bGreyed)
		{
			Result.Target = EHOLGridKeyTarget::Build;
			Result.TypeName = Building.TypeName;
		}
		return Result;
	}

	FHOLGridKeyResult ClickAge(const FHOLGridContext& Context, const int32 ChosenAge, const int32 Age, const int32 Cell)
	{
		if (!IsBuildGrid(Context) || !IsAgeReached(Context, Age))
		{
			FHOLGridKeyResult Result;
			Result.ChosenAge = ChosenAge;
			return Result;
		}
		if (Cell == INDEX_NONE)
		{
			FHOLGridKeyResult Result;
			Result.Target = EHOLGridKeyTarget::Age;
			Result.ChosenAge = Age;
			return Result;
		}
		return PressKey(Context, Age, Cell);
	}

	int32 GetChosenAgeAfter(const int32 ChosenAge, const EHOLAgeEvent Event)
	{
		return Event == EHOLAgeEvent::PlacedKept ? ChosenAge : INDEX_NONE;
	}

	bool FindMissingResource(const FHOLResources& Stock, const FHOLResources& Cost, EHOLResource& OutResource)
	{
		for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
		{
			if (Stock[Resource] < Cost[Resource])
			{
				OutResource = Resource;
				return true;
			}
		}
		return false;
	}

	FHOLCommandFeedback CheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, const int32 QueueLength, const int32 MaxQueue,
		const int32 PopulationUsed, const int32 PopulationCap, const int32 Places)
	{
		FHOLCommandFeedback Feedback;
		if (QueueLength >= MaxQueue)
		{
			Feedback.Kind = EHOLCommandFeedback::QueueFull;
		}
		else if (FindMissingResource(Stock, Cost, Feedback.Resource))
		{
			Feedback.Kind = EHOLCommandFeedback::NotEnoughResources;
		}
		else if (IsPopulationBlocked(PopulationUsed, PopulationCap, Places))
		{
			Feedback.Kind = EHOLCommandFeedback::PopulationFull;
		}
		return Feedback;
	}

	FHOLCommandFeedback CheckBuild(const FHOLResources& Stock, const FHOLResources& Cost)
	{
		FHOLCommandFeedback Feedback;
		if (FindMissingResource(Stock, Cost, Feedback.Resource))
		{
			Feedback.Kind = EHOLCommandFeedback::NotEnoughResources;
		}
		return Feedback;
	}

	bool Blocks(const FHOLCommandFeedback& Feedback)
	{
		return Feedback.Kind == EHOLCommandFeedback::NotEnoughResources || Feedback.Kind == EHOLCommandFeedback::QueueFull
			|| Feedback.Kind == EHOLCommandFeedback::PlacementRefused;
	}

	bool IsPopulationBlocked(const int32 PopulationUsed, const int32 PopulationCap, const int32 Places)
	{
		return PopulationUsed + FMath::Max(Places, 0) > PopulationCap;
	}

	FString Describe(const FHOLCommandFeedback& Feedback)
	{
		switch (Feedback.Kind)
		{
		case EHOLCommandFeedback::NotEnoughResources:
			return FString::Printf(TEXT("not enough %s"), *StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Feedback.Resource)));
		case EHOLCommandFeedback::QueueFull: return TEXT("queue full");
		case EHOLCommandFeedback::PopulationFull: return TEXT("population full");
		case EHOLCommandFeedback::PlacementRefused:
			// The words of UHOLConstructionSubsystem::DescribeRefusal (the server's log).
			switch (Feedback.Refusal)
			{
			case EHOLPlacementRefusal::OutOfMap: return TEXT("placement refused (out of the map)");
			case EHOLPlacementRefusal::Building: return TEXT("placement refused (on a building)");
			case EHOLPlacementRefusal::Source: return TEXT("placement refused (on a resource source)");
			case EHOLPlacementRefusal::EnemyCentre: return TEXT("placement refused (too near an enemy main centre)");
			case EHOLPlacementRefusal::NavMesh: return TEXT("placement refused (off the navmesh)");
			default: return TEXT("placement refused");
			}
		case EHOLCommandFeedback::NoBuilding:
			return FString::Printf(TEXT("no %s building"), *StaticEnum<EHOLBuildingCategory>()->GetNameStringByValue(static_cast<int64>(Feedback.Category)));
		default: return TEXT("none");
		}
	}

	FHOLGridCellFace GetCellFace(const bool bHasIcon)
	{
		FHOLGridCellFace Face;
		Face.bIcon = bHasIcon;
		Face.bName = !bHasIcon;
		return Face;
	}

	int32 GetCycleRank(const FHOLBuildingTypeRow& Type)
	{
		return Type.IsBuildable() ? Type.BuildGridSlot + 1 : 0;
	}

	bool IsSelectedByCategory(const FHOLBuildingTypeRow& Type, const EHOLBuildingCategory Category)
	{
		return Type.Category == Category;
	}

	bool IsRepeatPress(const TConstArrayView<uint32> Selected, const TConstArrayView<uint32> LastSelected, const bool bSameKey)
	{
		if (!bSameKey || Selected.IsEmpty() || Selected.Num() != LastSelected.Num())
		{
			return false;
		}
		for (const uint32 Key : Selected)
		{
			if (!LastSelected.Contains(Key))
			{
				return false;
			}
		}
		return true;
	}

	int32 NextInCycle(const TConstArrayView<FHOLCycleBuilding> Buildings, const uint32 LastKey)
	{
		if (Buildings.IsEmpty())
		{
			return INDEX_NONE;
		}
		TArray<int32> Order;
		for (int32 Index = 0; Index < Buildings.Num(); ++Index)
		{
			Order.Add(Index);
		}
		Order.Sort([Buildings](const int32 A, const int32 B)
		{
			return Buildings[A].Rank != Buildings[B].Rank ? Buildings[A].Rank < Buildings[B].Rank : Buildings[A].Key < Buildings[B].Key;
		});
		const int32 Last = Order.IndexOfByPredicate([Buildings, LastKey](const int32 Index) { return Buildings[Index].Key == LastKey; });
		return Order[Last == INDEX_NONE ? 0 : (Last + 1) % Order.Num()];
	}

	bool IsAloneInGroup(const FName TypeName)
	{
		return TypeName == HOLBuildingTypeNames::MainCentre;
	}

	TArray<FHOLSubgroup> SplitSubgroups(const TConstArrayView<FHOLGroupMember> Members)
	{
		TArray<FHOLSubgroup> Subgroups;
		if (Members.IsEmpty())
		{
			return Subgroups;
		}
		FHOLSubgroup& All = Subgroups.AddDefaulted_GetRef();
		All.bAll = true;
		// Whether each subgroup is a main centre's (never joined by another building).
		TArray<bool> Alone = { false };
		for (int32 Index = 0; Index < Members.Num(); ++Index)
		{
			Subgroups[0].Members.Add(Index);
			const FHOLGroupMember& Member = Members[Index];
			int32 Same = INDEX_NONE;
			for (int32 Other = 1; Other < Subgroups.Num() && !Member.bAlone && Same == INDEX_NONE; ++Other)
			{
				Same = !Alone[Other] && Subgroups[Other].TypeName == Member.TypeName ? Other : INDEX_NONE;
			}
			if (Same != INDEX_NONE)
			{
				Subgroups[Same].Members.Add(Index);
				continue;
			}
			FHOLSubgroup& Subgroup = Subgroups.AddDefaulted_GetRef();
			Subgroup.TypeName = Member.TypeName;
			Subgroup.Members.Add(Index);
			Alone.Add(Member.bAlone);
		}
		return Subgroups;
	}

	TArray<int32> GetCommandPriority(const TConstArrayView<FHOLGroupMember> Members)
	{
		TArray<int32> Order;
		for (const bool bCentres : { true, false })
		{
			for (int32 Index = 0; Index < Members.Num(); ++Index)
			{
				if (Members[Index].bAlone == bCentres)
				{
					Order.Add(Index);
				}
			}
		}
		return Order;
	}

	TArray<FHOLGridCommand> MergeCommands(const TConstArrayView<TArray<FHOLGridCommand>> PerBuilding, const TConstArrayView<int32> Priority)
	{
		TArray<FHOLGridCommand> Merged;
		Merged.SetNum(NumCells);
		for (const int32 Index : Priority)
		{
			if (!PerBuilding.IsValidIndex(Index))
			{
				continue;
			}
			const TArray<FHOLGridCommand>& Cells = PerBuilding[Index];
			for (int32 Cell = 0; Cell < NumCells && Cell < Cells.Num(); ++Cell)
			{
				if (Merged[Cell].IsNone() && !Cells[Cell].IsNone())
				{
					Merged[Cell] = Cells[Cell];
				}
			}
		}
		return Merged;
	}

	int32 PickEvenly(const TConstArrayView<FHOLQueueChoice> Choices, const uint32 LastKey)
	{
		int32 Fewest = MAX_int32;
		for (const FHOLQueueChoice& Choice : Choices)
		{
			if (Choice.QueueLength < Choice.MaxQueue)
			{
				Fewest = FMath::Min(Fewest, Choice.QueueLength);
			}
		}
		if (Fewest == MAX_int32)
		{
			return INDEX_NONE;
		}
		// Round robin between equals: from the one after the last one picked, in the group's order.
		const int32 Last = Choices.IndexOfByPredicate([LastKey](const FHOLQueueChoice& Choice) { return Choice.Key == LastKey; });
		for (int32 Step = 1; Step <= Choices.Num(); ++Step)
		{
			const int32 Index = Last == INDEX_NONE ? Step - 1 : (Last + Step) % Choices.Num();
			if (Choices[Index].QueueLength == Fewest)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}
}
