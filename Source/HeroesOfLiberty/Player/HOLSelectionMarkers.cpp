// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLSelectionMarkers.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/HOLPlayerController.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLFormationRules.h"
#include "Simulation/HOLUnitView.h"

namespace UE::HeroesOfLiberty::SelectionMarkers
{
	// The engine's 1 m plane (facing up, UVs 0 to 1) and cube, and the materials of make_marker_materials.py.
	const TCHAR* PlanePath = TEXT("/Engine/BasicShapes/Plane.Plane");
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* RingMaterialPath = TEXT("/Game/Rendering/M_GroundRing.M_GroundRing");
	const TCHAR* FrameMaterialPath = TEXT("/Game/Rendering/M_GroundFrame.M_GroundFrame");
	const TCHAR* FlatMaterialPath = TEXT("/Game/Rendering/M_FlatColor.M_FlatColor");
	/** Size (cm) of the engine's basic shapes. */
	constexpr double ShapeSize = 100.0;

	/** Custom data of a marker instance: colour (3), stroke as a share of the half-size (1). */
	constexpr int32 NumCustomData = 4;
	/** Above the ground (cm), so that the plane does not fight with it. */
	constexpr double GroundOffset = 3.0;

	/** The selection's rings and frame: a neutral white, whoever the owner (the user's trial of J4). */
	const FLinearColor SelectionColor(0.85f, 0.85f, 0.85f);
	/**
	 * The frame of a group's building out of its active subgroup (Tab, the user's 2nd trial of J6a.5): the same white,
	 * its stroke a third as thick (a darker grey melts into the ground, as the first try showed).
	 */
	constexpr double DimmedStrokeScale = 1.0 / 3.0;
	/** A unit's ring: its radius over the body's, and its stroke (cm). */
	constexpr double RingRadiusFactor = 1.4;
	constexpr double RingStroke = 12.0;
	/** A resource source's ring (J5b): its margin around the source's body and its stroke (cm). */
	constexpr double SourceRingMargin = 60.0;
	constexpr double SourceRingStroke = 16.0;
	/** The building's frame: its margin around the footprint and its stroke (cm). */
	constexpr double FrameMargin = 100.0;
	constexpr double FrameStroke = 30.0;
	/** The order's ring (D288; halved in size and time at the user's trial): it shrinks from the first radius to the second (cm). */
	constexpr float PingSeconds = 0.25f;
	constexpr double PingStartRadius = 90.0;
	constexpr double PingEndRadius = 25.0;
	constexpr double PingStroke = 8.0;
	const FLinearColor PingColor(0.9f, 0.9f, 0.9f);
	/** The places of a right-button drag's formation (J7a.4): thin rings, smaller than a unit's, in the ping's white. */
	constexpr double PreviewRadiusFactor = 0.8;
	constexpr double PreviewStroke = 8.0;
	/** The rally flag (cm): a pole, and a cloth at its top at the owner's colour. */
	constexpr double PoleHeight = 500.0;
	constexpr double PoleWidth = 10.0;
	const FVector ClothSize(6.0, 160.0, 100.0);
	const FLinearColor PoleColor(0.12f, 0.08f, 0.04f);
	/** The placement ghost (J6a.3): green where it may go, red where not; its outline's and its tiles' strokes (cm). */
	const FLinearColor GhostValidColor(0.15f, 0.85f, 0.2f);
	const FLinearColor GhostRefusedColor(0.95f, 0.12f, 0.08f);
	constexpr double GhostOutlineStroke = 40.0;
	constexpr double GhostTileStroke = 8.0;
	/** Width (cm) of the posts and beams outlining its volume. */
	constexpr double GhostEdgeWidth = 12.0;
	constexpr int32 NumGhostEdges = 8;

	UStaticMesh* LoadMesh(const TCHAR* Path)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
		UE_CLOG(!Mesh, LogHOLMatch, Error, TEXT("Selection markers: mesh %s missing"), Path);
		return Mesh;
	}

	UMaterialInterface* LoadMaterial(const TCHAR* Path)
	{
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, Path);
		UE_CLOG(!Material, LogHOLMatch, Error, TEXT("Selection markers: material %s missing (Tools/Editor/make_marker_materials.py)"), Path);
		return Material;
	}

	UInstancedStaticMeshComponent* MakeMarkerComponent(AActor& Owner, const FName Name)
	{
		UInstancedStaticMeshComponent* Component = Owner.CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);
		Component->SetupAttachment(Owner.GetRootComponent());
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(false);
		Component->SetReceivesDecals(false);
		Component->NumCustomDataFloats = NumCustomData;
		return Component;
	}

	UStaticMeshComponent* MakeFlagComponent(AActor& Owner, const FName Name)
	{
		UStaticMeshComponent* Component = Owner.CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Component->SetupAttachment(Owner.GetRootComponent());
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(false);
		Component->SetHiddenInGame(true);
		return Component;
	}

	/** A marker plane lying on the ground, centred on Centre, of a size (cm) on each axis. */
	FTransform MarkerTransform(const FVector& Centre, const double SizeX, const double SizeY)
	{
		return FTransform(FQuat::Identity, Centre + FVector(0.0, 0.0, GroundOffset), FVector(SizeX / ShapeSize, SizeY / ShapeSize, 1.0));
	}

	void AddCustomData(TArray<float>& CustomData, const FLinearColor& Color, const double StrokeShare)
	{
		CustomData.Append({ Color.R, Color.G, Color.B, static_cast<float>(FMath::Clamp(StrokeShare, 0.0, 1.0)) });
	}

	/** Puts the instances of a marker component, rebuilding them only when their number changes. */
	void SetInstances(UInstancedStaticMeshComponent& Component, const TArray<FTransform>& Transforms, const TArray<float>& CustomData)
	{
		if (Component.GetInstanceCount() != Transforms.Num())
		{
			Component.ClearInstances();
			if (!Transforms.IsEmpty())
			{
				Component.AddInstances(Transforms, /*bShouldReturnIndices*/ false, /*bWorldSpace*/ true);
			}
		}
		else if (!Transforms.IsEmpty())
		{
			Component.BatchUpdateInstancesTransforms(0, Transforms, /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ false, /*bTeleport*/ true);
		}
		for (int32 Index = 0; Index < Transforms.Num(); ++Index)
		{
			Component.SetCustomData(Index, TArrayView<const float>(CustomData.GetData() + Index * NumCustomData, NumCustomData),
				/*bMarkRenderStateDirty*/ Index == Transforms.Num() - 1);
		}
	}
}

AHOLSelectionMarkers::AHOLSelectionMarkers()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	PrimaryActorTick.bCanEverTick = true;
	// After the controller has read the input and the units have been drawn this frame.
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	SetReplicates(false);

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	Rings = MakeMarkerComponent(*this, TEXT("Rings"));
	Frames = MakeMarkerComponent(*this, TEXT("Frames"));
	GhostFrames = MakeMarkerComponent(*this, TEXT("GhostFrames"));
	for (int32 Index = 0; Index < NumGhostEdges; ++Index)
	{
		GhostEdges.Add(MakeFlagComponent(*this, *FString::Printf(TEXT("GhostEdge%d"), Index)));
	}
}

void AHOLSelectionMarkers::Init(AHOLPlayerController* InController)
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	Controller = InController;
	if (InController)
	{
		OrderHandle = InController->OnOrderGiven.AddUObject(this, &AHOLSelectionMarkers::HandleOrderGiven);
	}

	UStaticMesh* Plane = LoadMesh(PlanePath);
	UStaticMesh* Cube = LoadMesh(CubePath);
	Rings->SetStaticMesh(Plane);
	Rings->SetMaterial(0, LoadMaterial(RingMaterialPath));
	Frames->SetStaticMesh(Plane);
	Frames->SetMaterial(0, LoadMaterial(FrameMaterialPath));
	GhostFrames->SetStaticMesh(Plane);
	GhostFrames->SetMaterial(0, Frames->GetMaterial(0));

	UMaterialInterface* Flat = LoadMaterial(FlatMaterialPath);
	CubeMesh = Cube;
	if (Flat)
	{
		PoleMaterial = UMaterialInstanceDynamic::Create(Flat, this);
		PoleMaterial->SetVectorParameterValue(TEXT("Color"), PoleColor);
		ClothMaterial = UMaterialInstanceDynamic::Create(Flat, this);
		GhostMaterial = UMaterialInstanceDynamic::Create(Flat, this);
	}
	for (UStaticMeshComponent* Edge : GhostEdges)
	{
		Edge->SetStaticMesh(Cube);
		Edge->SetMaterial(0, GhostMaterial);
	}
}

void AHOLSelectionMarkers::AddFlag()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	const int32 Index = FlagPoles.Num();
	UStaticMeshComponent* Parts[2] = {};
	for (int32 Part = 0; Part < 2; ++Part)
	{
		UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this, *FString::Printf(TEXT("Flag%s%d"), Part == 0 ? TEXT("Pole") : TEXT("Cloth"), Index));
		Component->SetupAttachment(GetRootComponent());
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(false);
		Component->SetStaticMesh(CubeMesh);
		Component->SetMaterial(0, Part == 0 ? PoleMaterial.Get() : ClothMaterial.Get());
		Component->SetRelativeScale3D(Part == 0 ? FVector(PoleWidth, PoleWidth, PoleHeight) / ShapeSize : ClothSize / ShapeSize);
		Component->RegisterComponent();
		Parts[Part] = Component;
	}
	FlagPoles.Add(Parts[0]);
	FlagCloths.Add(Parts[1]);
}

void AHOLSelectionMarkers::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		PlayerController->OnOrderGiven.Remove(OrderHandle);
	}
	OrderHandle.Reset();
	Super::EndPlay(EndPlayReason);
}

void AHOLSelectionMarkers::HandleOrderGiven(const FVector& Point)
{
	PingPoint = Point;
	PingAge = 0.f;
}

void AHOLSelectionMarkers::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (PingAge < TNumericLimits<float>::Max())
	{
		PingAge += DeltaSeconds;
	}
	UpdateRings();
	UpdateFrame();
	UpdateFlag();
	UpdateGhost();
}

void AHOLSelectionMarkers::UpdateRings()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	const AHOLPlayerController* PlayerController = Controller.Get();
	const UWorld* World = GetWorld();
	if (PlayerController && World && !PlayerController->GetSelectedUnits().IsEmpty())
	{
		// Where the units are drawn this frame (D271): the rings follow them.
		TArray<FHOLUnitView> Views;
		HOLUnitView::Gather(*World, Views);
		const TArray<uint32>& Selected = PlayerController->GetSelectedUnits();
		FVector2D Centre = FVector2D::ZeroVector;
		double LargestRadius = 0.;
		int32 CountOfKind[HOLFormation::PeasantRowPriority + 1] = {};
		bool bPeasantsOnly = true;
		int32 Count = 0;
		double Height = 0.;
		for (const FHOLUnitView& View : Views)
		{
			if (Selected.Contains(View.UnitId))
			{
				const double Radius = View.Radius * RingRadiusFactor;
				Transforms.Add(MarkerTransform(View.Location, Radius * 2.0, Radius * 2.0));
				AddCustomData(CustomData, SelectionColor, RingStroke / Radius);
				Centre += FVector2D(View.Location);
				LargestRadius = FMath::Max(LargestRadius, static_cast<double>(View.Radius));
				const int32 Priority = HOLFormation::GetRowPriority(View.TypeName, View.CarryCapacity);
				++CountOfKind[Priority];
				bPeasantsOnly &= Priority == HOLFormation::PeasantRowPriority;
				Height += View.Location.Z;
				++Count;
			}
		}
		// The right-button drag (J7a.4): a small ring on each place of the formation the release will give, as the server
		// makes it from the line (its projection on the navmesh aside).
		FVector LineStart, LineEnd;
		if (Count >= 2 && PlayerController->GetOrderLine(LineStart, LineEnd))
		{
			Centre /= Count;
			const FVector2D Line(LineEnd - LineStart);
			// The arrow (the user's 2nd trial): the front row's middle at its base, facing along it.
			const FVector2D Middle(LineStart);
			const double ColumnSpacing = 2. * LargestRadius * HOLFormation::ColumnSpacingInDiameters;
			const double RowSpacing = 2. * LargestRadius * HOLFormation::RowSpacingInDiameters;
			// Ctrl held, read live (the user's 1st trial): the drag's width; otherwise the group's default rows, as the server.
			TArray<int32> Widths;
			if (PlayerController->IsOrderWidthHeld())
			{
				HOLFormation::MakeRowsOfWidth(Count, HOLFormation::GetFrontWidth(Line.Size(), ColumnSpacing, Count), Widths);
			}
			else
			{
				HOLFormation::MakeGroupRows(CountOfKind, bPeasantsOnly, Widths);
			}
			TArray<FVector2D> Places;
			HOLFormation::MakePlaces(Widths, Middle, HOLFormation::GetFrontFacing(Line), ColumnSpacing, RowSpacing, Places);
			const double Radius = LargestRadius * PreviewRadiusFactor;
			for (const FVector2D& Place : Places)
			{
				Transforms.Add(MarkerTransform(FVector(Place.X, Place.Y, Height / Count), Radius * 2.0, Radius * 2.0));
				AddCustomData(CustomData, PingColor, PreviewStroke / Radius);
			}
		}
	}
	// The selected resource source (J5b, D298): a ring around its body.
	const UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const int32 SourceIndex = PlayerController ? PlayerController->GetSelectedSource() : INDEX_NONE;
	if (Sources && Sources->GetSources().IsValidIndex(SourceIndex) && Sources->GetSources()[SourceIndex].IsStanding())
	{
		const FHOLSource& Source = Sources->GetSources()[SourceIndex];
		const double Radius = Source.Radius + SourceRingMargin;
		Transforms.Add(MarkerTransform(Source.Location, Radius * 2.0, Radius * 2.0));
		AddCustomData(CustomData, SelectionColor, SourceRingStroke / Radius);
	}
	if (PingAge < PingSeconds)
	{
		const double Radius = FMath::Lerp(PingStartRadius, PingEndRadius, static_cast<double>(PingAge / PingSeconds));
		Transforms.Add(MarkerTransform(PingPoint, Radius * 2.0, Radius * 2.0));
		AddCustomData(CustomData, PingColor, PingStroke / Radius);
	}
	SetInstances(*Rings, Transforms, CustomData);
}

void AHOLSelectionMarkers::UpdateFrame()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	const AHOLPlayerController* PlayerController = Controller.Get();
	// The selected building, or each of a group (an F key, D30), those out of its active subgroup thinner (Tab).
	TArray<AHOLBuilding*> Buildings;
	FHOLSubgroup Active;
	if (PlayerController)
	{
		PlayerController->GetSelectedBuildings(Buildings);
		PlayerController->GetActiveSubgroup(Active);
	}
	DimmedFrames = 0;
	for (int32 Index = 0; Index < Buildings.Num(); ++Index)
	{
		if (const FHOLBuildingTypeRow* Type = Buildings[Index]->GetType())
		{
			const bool bDimmed = !Active.bAll && !Active.Members.IsEmpty() && !Active.Members.Contains(Index);
			DimmedFrames += bDimmed ? 1 : 0;
			// Around the building's visible shape (D324 refined), not its tiles.
			const FVector2D Size = UHOLBuildingSettings::GetVisualSize(Type->Footprint) + FVector2D(FrameMargin * 2.0);
			Transforms.Add(MarkerTransform(Buildings[Index]->GetActorLocation(), Size.X, Size.Y));
			AddCustomData(CustomData, SelectionColor, FrameStroke * (bDimmed ? DimmedStrokeScale : 1.0) * 2.0 / Size.GetMax());
		}
	}
	SetInstances(*Frames, Transforms, CustomData);
}

void AHOLSelectionMarkers::UpdateFlag()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	// The rally point reaches its owner only (D282): shown while the player's own building is selected, or for each
	// building of a group's active subgroup (all of them for « Tous », the user's 3rd trial of J6a.5); followed each
	// frame, so Tab and a right click on the group move them at once.
	const AHOLPlayerController* PlayerController = Controller.Get();
	TArray<AHOLBuilding*> Buildings;
	if (PlayerController)
	{
		PlayerController->GetCommandBuildings(Buildings);
	}
	Buildings.RemoveAll([](const AHOLBuilding* Building) { return !Building || !Building->HasRallyPoint(); });
	while (FlagPoles.Num() < Buildings.Num())
	{
		AddFlag();
	}
	FlagFeet.Reset();
	for (int32 Index = 0; Index < FlagPoles.Num(); ++Index)
	{
		const bool bShown = Buildings.IsValidIndex(Index);
		if (FlagPoles[Index]->bHiddenInGame == bShown)
		{
			FlagPoles[Index]->SetHiddenInGame(!bShown);
			FlagCloths[Index]->SetHiddenInGame(!bShown);
		}
		if (!bShown)
		{
			continue;
		}
		const FVector Foot = Buildings[Index]->GetRallyPoint();
		FlagFeet.Add(Foot);
		FlagPoles[Index]->SetWorldLocation(Foot + FVector(0.0, 0.0, PoleHeight * 0.5));
		// The cloth hangs from the top of the pole, on its right as the camera sees it (the camera looks along +X, D254).
		FlagCloths[Index]->SetWorldLocation(Foot + FVector(0.0, ClothSize.Y * 0.5, PoleHeight - ClothSize.Z * 0.5));
	}
	// The player's own buildings: one colour for every cloth.
	if (ClothMaterial && !Buildings.IsEmpty())
	{
		ClothMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(HOLPlayerColors::GetColor(Buildings[0]->GetColorIndex())));
	}
}

void AHOLSelectionMarkers::UpdateGhost()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	const AHOLPlayerController* PlayerController = Controller.Get();
	FName TypeName;
	FBox2D Footprint;
	double GroundZ = 0.0;
	bool bValid = false;
	const FHOLBuildingTypeRow* Type = PlayerController && PlayerController->GetPlacementGhost(TypeName, Footprint, GroundZ, bValid)
		? UHOLBuildingSettings::FindType(TypeName) : nullptr;
	const EGhost Ghost = !Type ? EGhost::None : bValid ? EGhost::Valid : EGhost::Refused;
	if (Ghost != ShownGhost)
	{
		for (UStaticMeshComponent* Edge : GhostEdges)
		{
			Edge->SetHiddenInGame(Ghost == EGhost::None);
		}
		if (GhostMaterial && Ghost != EGhost::None)
		{
			GhostMaterial->SetVectorParameterValue(TEXT("Color"), Ghost == EGhost::Valid ? GhostValidColor : GhostRefusedColor);
		}
		ShownGhost = Ghost;
	}
	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	if (Type)
	{
		const FLinearColor Color = bValid ? GhostValidColor : GhostRefusedColor;
		const FVector2D Size = Footprint.GetSize();
		const FVector Centre(Footprint.GetCenter(), GroundZ);
		// The outline, then each tile (D311: placement tile by tile), a thin frame.
		Transforms.Add(MarkerTransform(Centre, Size.X, Size.Y));
		AddCustomData(CustomData, Color, GhostOutlineStroke * 2.0 / Size.GetMax());
		const double Tile = UHOLBuildingSettings::GetTileSize();
		const int32 TilesX = FMath::Max(1, FMath::RoundToInt(Size.X / Tile));
		const int32 TilesY = FMath::Max(1, FMath::RoundToInt(Size.Y / Tile));
		const FVector2D TileSize(Size.X / TilesX, Size.Y / TilesY);
		for (int32 X = 0; X < TilesX && TilesX * TilesY > 1; ++X)
		{
			for (int32 Y = 0; Y < TilesY; ++Y)
			{
				const FVector TileCentre(Footprint.Min + TileSize * FVector2D(X + 0.5, Y + 0.5), GroundZ);
				Transforms.Add(MarkerTransform(TileCentre, TileSize.X, TileSize.Y));
				AddCustomData(CustomData, Color, GhostTileStroke * 2.0 / TileSize.GetMax());
			}
		}
		// The edges of its volume: a post at each corner, a beam along each side at its height; over its visible shape, a
		// little inset in the tiles it takes (D324 refined).
		const double Height = Type->Height;
		const FVector2D Half = UHOLBuildingSettings::GetVisualSize(Size) * 0.5 - FVector2D(GhostEdgeWidth * 0.5);
		const FVector2D Corners[] = { FVector2D(Half.X, Half.Y), FVector2D(Half.X, -Half.Y), FVector2D(-Half.X, -Half.Y), FVector2D(-Half.X, Half.Y) };
		for (int32 Index = 0; Index < 4; ++Index)
		{
			GhostEdges[Index]->SetWorldLocationAndRotation(Centre + FVector(Corners[Index], Height * 0.5), FQuat::Identity);
			GhostEdges[Index]->SetWorldScale3D(FVector(GhostEdgeWidth, GhostEdgeWidth, Height) / ShapeSize);
			const FVector2D Next = Corners[(Index + 1) % 4];
			const FVector2D Middle = (Corners[Index] + Next) * 0.5;
			const FVector2D Span = (Next - Corners[Index]).GetAbs() + FVector2D(GhostEdgeWidth);
			GhostEdges[4 + Index]->SetWorldLocationAndRotation(Centre + FVector(Middle, Height - GhostEdgeWidth * 0.5), FQuat::Identity);
			GhostEdges[4 + Index]->SetWorldScale3D(FVector(Span, GhostEdgeWidth) / ShapeSize);
		}
	}
	SetInstances(*GhostFrames, Transforms, CustomData);
}

FString AHOLSelectionMarkers::Describe() const
{
	const int32 NumPings = PingAge < UE::HeroesOfLiberty::SelectionMarkers::PingSeconds ? 1 : 0;
	FString Flags = FString::FromInt(FlagFeet.Num());
	for (int32 Index = 0; Index < FlagFeet.Num(); ++Index)
	{
		Flags += FString::Printf(TEXT("%s(%.0f, %.0f)"), Index == 0 ? TEXT(" at ") : TEXT(" "), FlagFeet[Index].X, FlagFeet[Index].Y);
	}
	return FString::Printf(TEXT("rings %d, frame %d, flag %s, ghost %s, frames dimmed %d"), Rings->GetInstanceCount() - NumPings, Frames->GetInstanceCount(),
		*Flags, ShownGhost == EGhost::None ? TEXT("none") : ShownGhost == EGhost::Valid ? TEXT("valid") : TEXT("refused"), DimmedFrames);
}
