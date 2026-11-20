// The C++ controller's read of the wall-line rule for the drag gesture (F4, HALDIR-1120j, D839). UHOLWallLineBridge
// (Source/HeroesOfLiberty/Player/HOLWallLineBridge.h) finds this class from AHOLPlayerController (placement drag). Envelopes only: each override is one
// call of HOLWallLineRules.as, the single source of the rule (the cap included: hol.Wall.MaxLineCells).
class UHOLWallLineBridgeScript : UHOLWallLineBridge
{
	UFUNCTION(BlueprintOverride)
	bool IsWallType(FName TypeName)
	{
		return HOLWallLine::IsWallType(TypeName);
	}

	UFUNCTION(BlueprintOverride)
	TArray<FIntPoint> DragCells(FVector Start, FVector End, double Tile)
	{
		return HOLWallLine::DragCellsFromPoints(Start, End, Tile, HOLWallLine::MaxLineCells());
	}
}