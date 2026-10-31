// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLVisionRules.h"

FHOLVisionGrid FHOLVisionGrid::MakeCentered(const int32 InSide, const double InCellSize)
{
	FHOLVisionGrid Grid;
	Grid.Side = FMath::Max(0, InSide);
	Grid.CellSize = FMath::Max(1., InCellSize);
	return Grid;
}

FIntPoint FHOLVisionGrid::ToCell(const FVector2D& Point) const
{
	const double Origin = -0.5 * Side * CellSize;
	return FIntPoint(FMath::FloorToInt32((Point.X - Origin) / CellSize), FMath::FloorToInt32((Point.Y - Origin) / CellSize));
}

FVector2D FHOLVisionGrid::GetCenter(const FIntPoint& Cell) const
{
	const double Origin = -0.5 * Side * CellSize;
	return FVector2D(Origin + (Cell.X + 0.5) * CellSize, Origin + (Cell.Y + 0.5) * CellSize);
}

double HOLVisionRules::GetSightRadius(const double BaseRadius, const double Height, const FHOLVisionTuning& Tuning)
{
	const double Bonus = FMath::Clamp(Height * Tuning.HeightBonusPerCm, 0., Tuning.MaxHeightBonus);
	return FMath::Max(0., BaseRadius + Bonus);
}

double HOLVisionRules::GetBuildingSight(const double SightRadius, const EHOLBuildingSight State)
{
	switch (State)
	{
	case EHOLBuildingSight::Standing: return FMath::Max(0., SightRadius);
	case EHOLBuildingSight::Site: return 0.5 * FMath::Max(0., SightRadius);
	default: return 0.;
	}
}

uint32 HOLVisionRules::GetReceivedCamps(const int32 Camp, const bool bDefeated, const bool bTeamCamp)
{
	if (Camp < 0 || Camp >= 32 || (bDefeated && !bTeamCamp))
	{
		return AllCamps;
	}
	return 1u << Camp;
}

bool HOLVisionRules::UpdateGhosts(TMap<uint64, FHOLGhost>& Ghosts, TConstArrayView<FHOLGhost> Present, TFunctionRef<bool(const FVector2D&)> IsVisible)
{
	bool bChanged = false;
	TSet<uint64> Standing;
	Standing.Reserve(Present.Num());
	for (const FHOLGhost& Thing : Present)
	{
		Standing.Add(Thing.Key);
		if (IsVisible(Thing.Position))
		{
			FHOLGhost* Ghost = Ghosts.Find(Thing.Key);
			if (!Ghost || !(*Ghost == Thing))
			{
				Ghosts.Add(Thing.Key, Thing);
				bChanged = true;
			}
		}
	}
	// Gone: forgotten only where its place is in sight again (D382).
	for (auto It = Ghosts.CreateIterator(); It; ++It)
	{
		if (!Standing.Contains(It.Key()) && IsVisible(It.Value().Position))
		{
			It.RemoveCurrent();
			bChanged = true;
		}
	}
	return bChanged;
}

bool HOLVisionRules::BuildingDrawsFromMemory(const bool bOwnCamp, const bool bInSight)
{
	return !bOwnCamp && !bInSight;
}

EHOLBuildingDraw HOLVisionRules::GetBuildingDraw(const bool bOwnCamp, const bool bInSight, const bool bRemembered)
{
	if (!BuildingDrawsFromMemory(bOwnCamp, bInSight))
	{
		return EHOLBuildingDraw::Live;
	}
	return bRemembered ? EHOLBuildingDraw::Memory : EHOLBuildingDraw::Hidden;
}

TArray<FIntPoint> HOLVisionRules::GetDisc(const FHOLVisionGrid& Grid, const FIntPoint& Cell, const double Radius)
{
	TArray<FIntPoint> Disc;
	const double Cells = FMath::Max(0., Radius) / Grid.CellSize;
	const double Squared = Cells * Cells;
	const int32 Reach = FMath::FloorToInt32(Cells);
	for (int32 DY = -Reach; DY <= Reach; ++DY)
	{
		for (int32 DX = -Reach; DX <= Reach; ++DX)
		{
			const FIntPoint Seen(Cell.X + DX, Cell.Y + DY);
			if (DX * DX + DY * DY <= Squared && Grid.IsInside(Seen))
			{
				Disc.Add(Seen);
			}
		}
	}
	return Disc;
}

bool HOLVisionRules::IsInLineOfSight(const FHOLVisionGrid& Grid, TConstArrayView<bool> Blocked, const FIntPoint& From, const FIntPoint& To)
{
	const auto IsForest = [&Grid, &Blocked](const FIntPoint& Cell)
	{
		const int32 Index = Grid.IsInside(Cell) ? Grid.ToIndex(Cell) : INDEX_NONE;
		return Blocked.IsValidIndex(Index) && Blocked[Index];
	};
	if (From == To)
	{
		return true;
	}
	// Bresenham from the observer: integers only, the same cells on every machine.
	const int32 DX = FMath::Abs(To.X - From.X);
	const int32 DY = -FMath::Abs(To.Y - From.Y);
	const int32 StepX = From.X < To.X ? 1 : -1;
	const int32 StepY = From.Y < To.Y ? 1 : -1;
	int32 Error = DX + DY;
	FIntPoint Cell = From;
	bool bInOwnForest = IsForest(From);
	while (true)
	{
		const int32 Twice = 2 * Error;
		if (Twice >= DY)
		{
			Error += DY;
			Cell.X += StepX;
		}
		if (Twice <= DX)
		{
			Error += DX;
			Cell.Y += StepY;
		}
		if (Cell == To)
		{
			return true;
		}
		if (!IsForest(Cell))
		{
			bInOwnForest = false;
		}
		else if (!bInOwnForest)
		{
			return false;
		}
	}
}

TArray<FIntPoint> HOLVisionRules::GetSeenCells(const FHOLVisionGrid& Grid, TConstArrayView<bool> Blocked, const FIntPoint& Cell, const double Radius)
{
	TArray<FIntPoint> Seen = GetDisc(Grid, Cell, Radius);
	if (Blocked.Contains(true))
	{
		Seen.RemoveAll([&Grid, &Blocked, &Cell](const FIntPoint& Other) { return Other != Cell && !IsInLineOfSight(Grid, Blocked, Cell, Other); });
	}
	return Seen;
}

void FHOLVisionMap::Init(const FHOLVisionGrid& InGrid, const int32 InNumCamps, const FHOLVisionTuning& InTuning)
{
	Grid = InGrid;
	Tuning = InTuning;
	NumCamps = FMath::Max(0, InNumCamps);
	Blocked.Init(false, Grid.Num());
	Trees.Init(0, Grid.Num());
	Counts.Init(0, NumCamps * Grid.Num());
	Explored.Init(false, NumCamps * Grid.Num());
	Observers.Empty();
}

bool FHOLVisionMap::SetBlocked(const FIntPoint& Cell, const bool bBlocked)
{
	if (!Grid.IsInside(Cell) || Blocked[Grid.ToIndex(Cell)] == bBlocked)
	{
		return false;
	}
	Blocked[Grid.ToIndex(Cell)] = bBlocked;
	for (FObserver& Observer : Observers)
	{
		const int32 Reach = FMath::FloorToInt32(Observer.Radius / Grid.CellSize);
		if (FMath::Abs(Cell.X - Observer.Cell.X) <= Reach && FMath::Abs(Cell.Y - Observer.Cell.Y) <= Reach)
		{
			TakeOff(Observer);
			Lay(Observer);
		}
	}
	return true;
}

bool FHOLVisionMap::IsBlocked(const FIntPoint& Cell) const
{
	return Grid.IsInside(Cell) && Blocked[Grid.ToIndex(Cell)];
}

void FHOLVisionMap::AddBlocker(const FVector2D& Position)
{
	const FIntPoint Cell = Grid.ToCell(Position);
	if (Grid.IsInside(Cell))
	{
		uint16& Count = Trees[Grid.ToIndex(Cell)];
		ensure(Count < MAX_uint16);
		++Count;
		SetBlocked(Cell, Count >= Tuning.ForestTrees);
	}
}

void FHOLVisionMap::RemoveBlocker(const FVector2D& Position)
{
	const FIntPoint Cell = Grid.ToCell(Position);
	if (Grid.IsInside(Cell) && ensure(Trees[Grid.ToIndex(Cell)] > 0))
	{
		uint16& Count = Trees[Grid.ToIndex(Cell)];
		--Count;
		SetBlocked(Cell, Count >= Tuning.ForestTrees);
	}
}

int32 FHOLVisionMap::GetBlockers(const FIntPoint& Cell) const
{
	return Grid.IsInside(Cell) ? Trees[Grid.ToIndex(Cell)] : 0;
}

int32 FHOLVisionMap::AddObserver(const int32 Camp, const FVector2D& Position, const double Radius)
{
	if (Camp < 0 || Camp >= NumCamps)
	{
		return INDEX_NONE;
	}
	FObserver Observer;
	Observer.Camp = Camp;
	Observer.Cell = Grid.ToCell(Position);
	Observer.Radius = Radius;
	const int32 Handle = Observers.Add(MoveTemp(Observer));
	Lay(Observers[Handle]);
	return Handle;
}

bool FHOLVisionMap::MoveObserver(const int32 Handle, const FVector2D& Position)
{
	if (!Observers.IsValidIndex(Handle))
	{
		return false;
	}
	FObserver& Observer = Observers[Handle];
	const FIntPoint Cell = Grid.ToCell(Position);
	if (Cell == Observer.Cell)
	{
		return false;
	}
	TakeOff(Observer);
	Observer.Cell = Cell;
	Lay(Observer);
	return true;
}

void FHOLVisionMap::SetObserverRadius(const int32 Handle, const double Radius)
{
	if (Observers.IsValidIndex(Handle))
	{
		FObserver& Observer = Observers[Handle];
		TakeOff(Observer);
		Observer.Radius = Radius;
		Lay(Observer);
	}
}

void FHOLVisionMap::RemoveObserver(const int32 Handle)
{
	if (Observers.IsValidIndex(Handle))
	{
		TakeOff(Observers[Handle]);
		Observers.RemoveAt(Handle);
	}
}

void FHOLVisionMap::ExploreAll(const int32 Camp)
{
	if (Camp >= 0 && Camp < NumCamps)
	{
		for (int32 Index = Camp * Grid.Num(); Index < (Camp + 1) * Grid.Num(); ++Index)
		{
			Explored[Index] = true;
		}
	}
}

TArray<uint32> FHOLVisionMap::GetExploredRuns(const int32 Camp) const
{
	TArray<uint32> Runs;
	if (Camp < 0 || Camp >= NumCamps)
	{
		return Runs;
	}
	// Alternating runs, the first of unexplored cells (empty when cell 0 is explored).
	bool bRunExplored = false;
	uint32 Length = 0;
	for (int32 Index = Camp * Grid.Num(); Index < (Camp + 1) * Grid.Num(); ++Index)
	{
		if (Explored[Index] != bRunExplored)
		{
			Runs.Add(Length);
			bRunExplored = !bRunExplored;
			Length = 0;
		}
		++Length;
	}
	Runs.Add(Length);
	return Runs;
}

bool FHOLVisionMap::AddExploredRuns(const int32 Camp, const TConstArrayView<uint32> Runs)
{
	if (Camp < 0 || Camp >= NumCamps)
	{
		return false;
	}
	int64 Total = 0;
	for (const uint32 Run : Runs)
	{
		Total += Run;
	}
	if (Total != Grid.Num())
	{
		return false;
	}
	int32 Index = Camp * Grid.Num();
	bool bRunExplored = false;
	for (const uint32 Run : Runs)
	{
		for (uint32 Cell = 0; Cell < Run; ++Cell, ++Index)
		{
			Explored[Index] |= bRunExplored;
		}
		bRunExplored = !bRunExplored;
	}
	return true;
}

int32 FHOLVisionMap::GetCount(const int32 Camp, const FIntPoint& Cell) const
{
	return Camp >= 0 && Camp < NumCamps && Grid.IsInside(Cell) ? Counts[Camp * Grid.Num() + Grid.ToIndex(Cell)] : 0;
}

EHOLCellVision FHOLVisionMap::GetVision(const int32 Camp, const FIntPoint& Cell) const
{
	if (GetCount(Camp, Cell) > 0)
	{
		return EHOLCellVision::Visible;
	}
	const bool bExplored = Camp >= 0 && Camp < NumCamps && Grid.IsInside(Cell) && Explored[Camp * Grid.Num() + Grid.ToIndex(Cell)];
	return bExplored ? EHOLCellVision::Explored : EHOLCellVision::Unexplored;
}

bool FHOLVisionMap::IsVisible(const int32 Camp, const FVector2D& Position) const
{
	return GetCount(Camp, Grid.ToCell(Position)) > 0;
}

int32 FHOLVisionMap::CountVisible(const int32 Camp) const
{
	int32 Visible = 0;
	if (Camp >= 0 && Camp < NumCamps)
	{
		for (int32 Index = Camp * Grid.Num(); Index < (Camp + 1) * Grid.Num(); ++Index)
		{
			Visible += Counts[Index] > 0 ? 1 : 0;
		}
	}
	return Visible;
}

int32 FHOLVisionMap::CountExplored(const int32 Camp) const
{
	int32 Seen = 0;
	if (Camp >= 0 && Camp < NumCamps)
	{
		for (int32 Index = Camp * Grid.Num(); Index < (Camp + 1) * Grid.Num(); ++Index)
		{
			Seen += Explored[Index] ? 1 : 0;
		}
	}
	return Seen;
}

void FHOLVisionMap::Lay(FObserver& Observer)
{
	const int32 Offset = Observer.Camp * Grid.Num();
	for (const FIntPoint& Cell : HOLVisionRules::GetSeenCells(Grid, Blocked, Observer.Cell, Observer.Radius))
	{
		const int32 Index = Grid.ToIndex(Cell);
		uint16& Count = Counts[Offset + Index];
		ensure(Count < MAX_uint16);
		++Count;
		Explored[Offset + Index] = true;
		Observer.Seen.Add(Index);
	}
}

void FHOLVisionMap::TakeOff(FObserver& Observer)
{
	const int32 Offset = Observer.Camp * Grid.Num();
	for (const int32 Index : Observer.Seen)
	{
		--Counts[Offset + Index];
	}
	Observer.Seen.Reset();
}

uint8 HOLFogPicture::ToByte(const EHOLCellVision Vision)
{
	switch (Vision)
	{
	case EHOLCellVision::Visible:
		return VisibleByte;
	case EHOLCellVision::Explored:
		return ExploredByte;
	default:
		return UnexploredByte;
	}
}

EHOLCellVision HOLFogPicture::FromByte(const uint8 Byte)
{
	// Halfway between the bytes of two states: a blurred byte takes the nearer state.
	constexpr uint8 VisibleFrom = (ExploredByte + VisibleByte) / 2 + 1;
	constexpr uint8 ExploredFrom = ExploredByte / 2 + 1;
	return Byte >= VisibleFrom ? EHOLCellVision::Visible : (Byte >= ExploredFrom ? EHOLCellVision::Explored : EHOLCellVision::Unexplored);
}

void HOLFogPicture::Fill(const FHOLVisionMap& Map, const int32 Camp, TArray<uint8>& Bytes, int32& NumVisible, int32& NumExplored)
{
	const FHOLVisionGrid& Grid = Map.GetGrid();
	NumVisible = 0;
	NumExplored = 0;
	if (Camp < 0)
	{
		Bytes.Init(VisibleByte, Grid.Num());
		NumVisible = Grid.Num();
		NumExplored = Grid.Num();
		return;
	}
	Bytes.SetNumUninitialized(Grid.Num());
	for (int32 Index = 0; Index < Grid.Num(); ++Index)
	{
		const EHOLCellVision Vision = Map.GetVision(Camp, Grid.ToCell(Index));
		Bytes[Index] = ToByte(Vision);
		NumVisible += Vision == EHOLCellVision::Visible ? 1 : 0;
		NumExplored += Vision != EHOLCellVision::Unexplored ? 1 : 0;
	}
}

EHOLCellVision HOLFogPicture::Read(const FHOLVisionGrid& Grid, const TConstArrayView<uint8> Bytes, const FVector2D& Point)
{
	const FIntPoint Cell = Grid.ToCell(Point);
	return Grid.IsInside(Cell) && Bytes.IsValidIndex(Grid.ToIndex(Cell)) ? FromByte(Bytes[Grid.ToIndex(Cell)]) : EHOLCellVision::Unexplored;
}

FVector2D HOLFogPicture::ToUV(const FHOLVisionGrid& Grid, const FVector2D& Point)
{
	const double Side = Grid.Side * Grid.CellSize;
	return Side > 0. ? (Point + FVector2D(0.5 * Side, 0.5 * Side)) / Side : FVector2D::ZeroVector;
}
