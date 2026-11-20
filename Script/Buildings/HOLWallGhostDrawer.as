// The wall drag ghost, drawing (F4 client, HALDIR-1120pc, D839): while the player drags a wall line (AHOLPlayerController::BeginWallDrag ..
// EndWallDrag), one flat box per cell of the line on the ground, green when the release lays the line, red when the server would refuse it (the
// refused cell darker apart is not drawn: the whole line is red, as the refusal is all or nothing). The rule is HOLWallGhost::Build
// (HOLWallGhostRules.as, one source with the server's Validate); this file only reads the controller and draws. Script only: the one C++ line is
// AHOLPlayerController::GetWallDragView (a UFUNCTION). Instanced boxes of the engine cube with M_MatteColor, as the biome decor does.

const FLinearColor WallGhostGreen = FLinearColor(0.15f, 0.85f, 0.3f, 1.0f);
const FLinearColor WallGhostRed = FLinearColor(0.95f, 0.12f, 0.08f, 1.0f);

class AHOLWallGhostActor : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;
}

class UHOLWallGhostDrawer : UScriptWorldSubsystem
{
	AHOLWallGhostActor Layer;
	UInstancedStaticMeshComponent GoodBoxes;
	UInstancedStaticMeshComponent BadBoxes;

	// What the map and the buildings held when the drag started (read once per drag, as the server reads them at the release).
	bool bWasDragging = false;
	FIntPoint MapMin;
	FIntPoint MapMax;
	TArray<FIntPoint> Occupied;

	// The cursor cell last drawn (the boxes are rebuilt only when it moves).
	FIntPoint LastEnd = FIntPoint(-2147483647, -2147483647);

	UInstancedStaticMeshComponent MakeBoxes(FName Name, FLinearColor Color)
	{
		UInstancedStaticMeshComponent Part = UInstancedStaticMeshComponent::Create(Layer, Name);
		Part.SetStaticMesh(Cast<UStaticMesh>(LoadObject(nullptr, "/Engine/BasicShapes/Cube.Cube")));
		Part.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part.SetCollisionProfileName(n"NoCollision");
		Part.SetGenerateOverlapEvents(false);
		Part.SetCastShadow(false);
		UMaterialInterface Matte = Cast<UMaterialInterface>(LoadObject(nullptr, "/Game/Rendering/M_MatteColor.M_MatteColor"));
		UMaterialInstanceDynamic Paint = Part.CreateDynamicMaterialInstance(0, Matte);
		if (Paint != nullptr)
		{
			Paint.SetVectorParameterValue(n"Color", Color);
		}
		return Part;
	}

	void Clear()
	{
		if (GoodBoxes != nullptr)
		{
			GoodBoxes.ClearInstances();
			BadBoxes.ClearInstances();
		}
		LastEnd = FIntPoint(-2147483647, -2147483647);
	}

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(Gameplay::GetPlayerController(0));
		FVector Start;
		FVector End;
		double Tile = 0.0;
		bool bDragging = Controller != nullptr && Controller.GetWallDragView(Start, End, Tile);
		if (!bDragging)
		{
			if (bWasDragging)
			{
				bWasDragging = false;
				Clear();
			}
			return;
		}

		if (!bWasDragging)
		{
			bWasDragging = true;
			Occupied.Empty();
			HOLConstructionRead::GetWallLineWorld(MapMin, MapMax, Occupied);
		}
		if (Layer == nullptr)
		{
			Layer = Cast<AHOLWallGhostActor>(SpawnActor(AHOLWallGhostActor::StaticClass(), FVector::ZeroVector));
			GoodBoxes = MakeBoxes(n"WallGhostGood", WallGhostGreen);
			BadBoxes = MakeBoxes(n"WallGhostBad", WallGhostRed);
		}

		FIntPoint EndCell = FIntPoint(HOLWallLine::CellOfCoordinate(End.X, Tile), HOLWallLine::CellOfCoordinate(End.Y, Tile));
		if (EndCell.X == LastEnd.X && EndCell.Y == LastEnd.Y)
		{
			return;
		}
		LastEnd = EndCell;

		HOLWallGhost::FHOLWallGhost Ghost = HOLWallGhost::BuildFromPoints(Start, End, Tile, MapMin, MapMax, Occupied, HOLWallLine::MaxLineCells());
		UInstancedStaticMeshComponent Shown = Ghost.bValid ? GoodBoxes : BadBoxes;
		GoodBoxes.ClearInstances();
		BadBoxes.ClearInstances();
		const float Side = float(Tile) / 100.0f * 0.92f;
		for (int Index = 0; Index < Ghost.Cells.Num(); Index++)
		{
			FVector At = HOLWallLine::CellToPoint(Ghost.Cells[Index], Tile, Start.Z + 8.0);
			Shown.AddInstance(FTransform(FRotator(0.0f, 0.0f, 0.0f), At, FVector(Side, Side, 0.08f)), true);
		}
		Log(n"LogHOLMatch", f"Wall ghost: cells={Ghost.Cells.Num()} valid={Ghost.bValid}");
	}
}
