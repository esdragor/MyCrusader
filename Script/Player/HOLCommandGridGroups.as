// MIG-C-CommandGrid, lot G2b (HALDIR-1119y, D830, D836, D839): the CYCLE, the SUBGROUPS, the GROUPS and the MERGE of the command grid in script. NOT COMPILED when written.
// Port of Source/HeroesOfLiberty/Player/HOLCommandGrid.cpp lines 402-557 (GetCellFace, GetCycleRank, IsSelectedByCategory, IsRepeatPress, NextInCycle,
// IsAloneInGroup, SplitSubgroups, GetCommandPriority, MergeCommands, PickEvenly), same decisions. The C++ file is NOT touched: until the bridge and the green
// agreement test (Script/Tests/HOLCommandGridGroups_Test.as) two copies exist.
// Input and output are the USTRUCT views of UHOLCommandGridReadLibrary (FHOLCycleBuildingView, FHOLQueueChoiceView, FHOLGroupMemberView, FHOLSubgroupView,
// FHOLGridCommandView, FHOLGridCellFaceView): nothing else is called. This file does NOT depend on HOLCommandGridRules:: (the number of cells is a local constant).
// Differences of form (the decisions are the same):
//   - the buildings' keys are int32 in the views (the uint32 key's bit pattern): the sort compares them AS uint32, like the C++ (SortsBefore below);
//   - MergeCommands takes the buildings' cells end to end (building i, cell c at i * NumCells + c), as the library's does: a nested TArray is not allowed in
//     script; a building always has NumCells cells there (the C++ also accepts a shorter array, which an end-to-end array cannot say);
//   - GetCycleRank / IsSelectedByCategory read a FHOLBuildingTypeRow in C++, a struct not exposed to script: here the values read are the parameters.
// NOT ported: nothing of the range 402-557 (see the report's TYPES_A_EXPOSER: only the row-based call sites are left to the bridge).

namespace HOLCommandGridGroups
{
	// HOLCommandGrid.h:278 (a constant of a file is not seen by another file, hence the accessor).
	const int NumCells = 12;
	const int None = -1;

	int GetNumCells() { return NumCells; }

	// GetCellFace (cpp 402-408): the icon when there is one, else the name.
	FHOLGridCellFaceView GetCellFace(bool bHasIcon)
	{
		FHOLGridCellFaceView Face;
		Face.bIcon = bHasIcon;
		Face.bName = !bHasIcon;
		return Face;
	}

	// GetCycleRank (cpp 410-413): 0 for what cannot be built, else the type's slot in the build grid + 1 (the row's IsBuildable() and BuildGridSlot as parameters).
	int GetCycleRank(bool bBuildable, int BuildGridSlot)
	{
		return bBuildable ? BuildGridSlot + 1 : 0;
	}

	// IsSelectedByCategory (cpp 415-418): F1 / F2 / F3 take the buildings of their category (the row's Category as parameter).
	bool IsCategorySelectedByCategory(EHOLBuildingCategory TypeCategory, EHOLBuildingCategory Category)
	{
		return TypeCategory == Category;
	}

	bool ContainsKey(const TArray<int>& Keys, int Key)
	{
		for (int I = 0; I < Keys.Num(); I++)
		{
			if (Keys[I] == Key)
			{
				return true;
			}
		}
		return false;
	}

	// IsRepeatPress (cpp 420-434): the same key again on the very same selection (same size, every building of the one in the other).
	bool IsRepeatPress(const TArray<int>& Selected, const TArray<int>& LastSelected, bool bSameKey)
	{
		if (!bSameKey || Selected.Num() == 0 || Selected.Num() != LastSelected.Num())
		{
			return false;
		}
		for (int I = 0; I < Selected.Num(); I++)
		{
			if (!ContainsKey(LastSelected, Selected[I]))
			{
				return false;
			}
		}
		return true;
	}

	// The key is the uint32's bit pattern held in an int32: compared as the uint32 the C++ compares.
	int64 AsUnsigned(int Key)
	{
		return int64(Key) & int64(0xFFFFFFFF);
	}

	// The cycle's order (cpp 445-448): the rank, then the key.
	bool SortsBefore(const TArray<FHOLCycleBuildingView>& Buildings, int A, int B)
	{
		if (Buildings[A].Rank != Buildings[B].Rank)
		{
			return Buildings[A].Rank < Buildings[B].Rank;
		}
		return AsUnsigned(Buildings[A].Key) < AsUnsigned(Buildings[B].Key);
	}

	// NextInCycle (cpp 436-453): the index (in Buildings) of the one after LastKey in the order rank then key, round; the first when LastKey is gone; -1 when none.
	int NextInCycle(const TArray<FHOLCycleBuildingView>& Buildings, int LastKey)
	{
		if (Buildings.Num() == 0)
		{
			return None;
		}
		TArray<int> Order;
		for (int Index = 0; Index < Buildings.Num(); Index++)
		{
			Order.Add(Index);
		}
		// Insertion sort (stable).
		for (int I = 1; I < Order.Num(); I++)
		{
			int Moving = Order[I];
			int J = I - 1;
			while (J >= 0 && SortsBefore(Buildings, Moving, Order[J]))
			{
				Order[J + 1] = Order[J];
				J--;
			}
			Order[J + 1] = Moving;
		}
		int Last = None;
		for (int I = 0; I < Order.Num() && Last == None; I++)
		{
			if (Buildings[Order[I]].Key == LastKey)
			{
				Last = I;
			}
		}
		return Order[Last == None ? 0 : (Last + 1) % Order.Num()];
	}

	// IsAloneInGroup (cpp 455-458): a main centre is never joined by another building in a subgroup (HOLBuildingTypeNames::MainCentre).
	bool IsAloneInGroup(FName TypeName)
	{
		return TypeName == n"MainCentre";
	}

	// SplitSubgroups (cpp 460-491): first "All" (every building), then one subgroup per type in the order of first appearance; a building that is alone
	// (a main centre) is a subgroup of its own, never joined, and joins no one.
	TArray<FHOLSubgroupView> SplitSubgroups(const TArray<FHOLGroupMemberView>& Members)
	{
		TArray<FHOLSubgroupView> Subgroups;
		if (Members.Num() == 0)
		{
			return Subgroups;
		}
		FHOLSubgroupView All;
		All.bAll = true;
		Subgroups.Add(All);
		// Whether each subgroup is a main centre's (never joined by another building).
		TArray<bool> Alone;
		Alone.Add(false);
		for (int Index = 0; Index < Members.Num(); Index++)
		{
			Subgroups[0].Members.Add(Index);
			int Same = None;
			for (int Other = 1; Other < Subgroups.Num() && !Members[Index].bAlone && Same == None; Other++)
			{
				Same = (!Alone[Other] && Subgroups[Other].TypeName == Members[Index].TypeName) ? Other : None;
			}
			if (Same != None)
			{
				Subgroups[Same].Members.Add(Index);
				continue;
			}
			FHOLSubgroupView Subgroup;
			Subgroup.TypeName = Members[Index].TypeName;
			Subgroup.Members.Add(Index);
			Subgroups.Add(Subgroup);
			Alone.Add(Members[Index].bAlone);
		}
		return Subgroups;
	}

	// GetCommandPriority (cpp 493-507): the indexes of the centres first, then the others, each in the group's order.
	TArray<int> GetCommandPriority(const TArray<FHOLGroupMemberView>& Members)
	{
		TArray<int> Order;
		for (int Pass = 0; Pass < 2; Pass++)
		{
			bool bCentres = Pass == 0;
			for (int Index = 0; Index < Members.Num(); Index++)
			{
				if (Members[Index].bAlone == bCentres)
				{
					Order.Add(Index);
				}
			}
		}
		return Order;
	}

	// MergeCommands (cpp 509-529): each cell takes the first non-empty command of the buildings in the priority's order. PerBuilding: end to end, NumCells each.
	TArray<FHOLGridCommandView> MergeCommands(const TArray<FHOLGridCommandView>& PerBuilding, const TArray<int>& Priority)
	{
		TArray<FHOLGridCommandView> Merged;
		for (int Cell = 0; Cell < NumCells; Cell++)
		{
			Merged.Add(FHOLGridCommandView());
		}
		int Buildings = PerBuilding.Num() / NumCells;
		for (int P = 0; P < Priority.Num(); P++)
		{
			int Index = Priority[P];
			if (Index < 0 || Index >= Buildings)
			{
				continue;
			}
			for (int Cell = 0; Cell < NumCells; Cell++)
			{
				if (Merged[Cell].Command == EHOLGridCommandKind::None && PerBuilding[Index * NumCells + Cell].Command != EHOLGridCommandKind::None)
				{
					Merged[Cell] = PerBuilding[Index * NumCells + Cell];
				}
			}
		}
		return Merged;
	}

	// PickEvenly (cpp 531-557): the index of the building (of the group) with the shortest queue that still has room; round robin between equals, from the one
	// after LastKey in the group's order; -1 when every queue is full.
	int PickEvenly(const TArray<FHOLQueueChoiceView>& Choices, int LastKey)
	{
		int Fewest = 2147483647;
		for (int I = 0; I < Choices.Num(); I++)
		{
			if (Choices[I].QueueLength < Choices[I].MaxQueue)
			{
				Fewest = Math::Min(Fewest, Choices[I].QueueLength);
			}
		}
		if (Fewest == 2147483647)
		{
			return None;
		}
		int Last = None;
		for (int I = 0; I < Choices.Num() && Last == None; I++)
		{
			if (Choices[I].Key == LastKey)
			{
				Last = I;
			}
		}
		for (int Step = 1; Step <= Choices.Num(); Step++)
		{
			int Index = Last == None ? Step - 1 : (Last + Step) % Choices.Num();
			if (Choices[Index].QueueLength == Fewest)
			{
				return Index;
			}
		}
		return None;
	}
}
