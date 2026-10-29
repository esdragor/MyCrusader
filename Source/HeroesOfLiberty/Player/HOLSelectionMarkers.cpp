// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLSelectionMarkers.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLStartRules.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/HOLPlayerController.h"
#include "Resources/HOLSourceSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Simulation/HOLFormationRules.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLHeroLibrary.h"
#include "Simulation/HOLOrderChainRules.h"
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
	/**
	 * J8b.4 (J8b-Q4): a hero's level up, a golden flash ~1 s on him, seen by all: a ring at his feet, bright then gone at
	 * half the time, and a wider one going out from him and thinning.
	 */
	constexpr float HeroFlashSeconds = 1.0f;
	/**
	 * J8c.4 (J8c-Q4): the player's aura circles, light gold and thin (a frame, not a fill); the banner's reach while aimed,
	 * a thinner white; its aura under the cursor, gold within reach, red beyond. Pieces of a circle, height above the ground.
	 */
	const FLinearColor AuraRingColor(1.0f, 0.82f, 0.35f);
	constexpr double AuraRingWidth = 10.0;
	const FLinearColor ReachRingColor(0.85f, 0.85f, 0.85f);
	constexpr double ReachRingWidth = 6.0;
	const FLinearColor AimRefusedColor(0.9f, 0.25f, 0.2f);
	constexpr int32 AuraRingPieces = 72;
	constexpr double AuraRingOffset = 6.0;
	/** The ground's height is looked for between these heights (cm). */
	constexpr double AuraTraceHalfHeight = 100000.0;
	const FLinearColor HeroFlashColor(1.0f, 0.75f, 0.15f);
	constexpr double HeroFlashInnerFactor = 1.8;
	constexpr double HeroFlashInnerStroke = 32.0;
	constexpr double HeroFlashStartFactor = 2.0;
	constexpr double HeroFlashEndFactor = 7.0;
	constexpr double HeroFlashStartStroke = 40.0;
	constexpr double HeroFlashEndStroke = 8.0;
	/** An attack order's ping (J7b.3): on its target, or an attack-move's point. */
	const FLinearColor AttackPingColor(0.9f, 0.08f, 0.05f);
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

	/**
	 * The health bars (J7b.5), facing the camera above the head: size (cm) at the reference distance, scaled with the
	 * distance within bounds (smaller when close, larger when far, so they stay readable at every zoom, D254).
	 */
	constexpr double BarWidth = 90.0;
	constexpr double BarHeight = 12.0;
	/** The dark back's margin around the fill (cm, before scaling). */
	constexpr double BarMargin = 2.0;
	/** Above the unit's feet (cm, times its mesh scale): over the head of a ~180 cm figure. */
	constexpr double BarHeadHeight = 230.0;
	/** The fill in front of the back (cm, towards the camera), so that the two do not fight. */
	constexpr double BarFillLift = 1.0;
	/** The camera's distance at which the bar has its size (a default camera, 35 m high, D254), and the bounds of its scale. */
	constexpr double BarReferenceDistance = 4500.0;
	constexpr double BarMinScale = 0.8;
	constexpr double BarMaxScale = 1.8;
	/** Bars out of the camera's view are not drawn: the view's cone, its half-angle widened a little (degrees). */
	constexpr double BarCullMarginDegrees = 8.0;
	/** A building's bar (D345.2a): this many times as wide as a unit's, this far above its top (cm). */
	constexpr double BuildingBarWidthFactor = 3.0;
	constexpr double BuildingBarLift = 150.0;
	/** A building hit within this many seconds shows its bar, selected or not (D345.2b, the user's request at trial 5). */
	constexpr double BuildingBarAfterHitSeconds = 5.0;
	/** Colours (the user's nuance on J7-Q5 left the colour open): green for the player's own units, red for any other's. */
	const FLinearColor BarOwnColor(0.12f, 0.8f, 0.12f);
	const FLinearColor BarOtherColor(0.9f, 0.1f, 0.05f);
	const FLinearColor BarBackColor(0.02f, 0.02f, 0.02f);
	/** A stroke past the whole plane: the marker material fills it. */
	constexpr float BarFullStroke = 2.f;

	/** The lines between queued orders (D328, J7b.5b-2): their width (cm), read each frame (halved twice at the user's trials). */
	TAutoConsoleVariable<float> CVarOrderLineWidth(
		TEXT("hol.Orders.LineWidth"),
		6.25f,
		TEXT("Width (cm) of the lines drawn on the ground between the queued orders (Shift) of the selected units (D328)."));
	/**
	 * Below the selection's rings and the flag's foot (GroundOffset): the lines, and the step discs just above them,
	 * so that neither hides a ring.
	 */
	constexpr double OrderLineOffset = 1.5;
	constexpr double OrderPointOffset = 2.2;
	/** A step's disc (cm): its radius, filled. */
	constexpr double OrderPointRadius = 30.0;
	/** The colour of a segment, from the kind of the step it reaches (D328). */
	const FLinearColor OrderMoveColor(0.95f, 0.8f, 0.05f);
	const FLinearColor OrderAttackColor(0.9f, 0.08f, 0.05f);
	const FLinearColor OrderBuildColor(0.9f, 0.9f, 0.9f);
	const FLinearColor OrderGatherColor(0.1f, 0.4f, 0.95f);

	const FLinearColor& GetOrderColor(const uint8 Kind)
	{
		switch (static_cast<HOLOrderChain::EStepKind>(Kind))
		{
		case HOLOrderChain::EStepKind::Attack:
		case HOLOrderChain::EStepKind::AttackMove:
		case HOLOrderChain::EStepKind::AttackBuilding:
			return OrderAttackColor;
		case HOLOrderChain::EStepKind::Build:
		case HOLOrderChain::EStepKind::Rally:
			return OrderBuildColor;
		case HOLOrderChain::EStepKind::Gather:
		case HOLOrderChain::EStepKind::Deposit: // D485: an unload, blue as the gathering
			return OrderGatherColor;
		default:
			return OrderMoveColor;
		}
	}

	/**
	 * V3.3 (V3-Q5): the hold's ring around a forum down: the arc's width, the rest's (a thin dark outline of the zone), the
	 * pieces a whole ring is cut in, and its height above the forum's foot (cm; below the selection's rings, GroundOffset).
	 * V3.4b: both thickened (60 and 16 cm were 3-4 and 2 px at the default camera).
	 */
	constexpr double ForumRingFillWidth = 120.0;
	constexpr double ForumRingRestWidth = 28.0;
	constexpr double ForumRingPieces = 128.0;
	constexpr double ForumRingOffset = 2.0;
	const FLinearColor ForumRingRestColor(0.12f, 0.12f, 0.12f);

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
	HealthBars = MakeMarkerComponent(*this, TEXT("HealthBars"));
	GhostFrames = MakeMarkerComponent(*this, TEXT("GhostFrames"));
	OrderLines = MakeMarkerComponent(*this, TEXT("OrderLines"));
	OrderPoints = MakeMarkerComponent(*this, TEXT("OrderPoints"));
	ForumRings = MakeMarkerComponent(*this, TEXT("ForumRings"));
	AuraRings = MakeMarkerComponent(*this, TEXT("AuraRings"));
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
		AttackOrderHandle = InController->OnAttackOrderGiven.AddUObject(this, &AHOLSelectionMarkers::HandleAttackOrderGiven);
	}

	UStaticMesh* Plane = LoadMesh(PlanePath);
	UStaticMesh* Cube = LoadMesh(CubePath);
	Rings->SetStaticMesh(Plane);
	Rings->SetMaterial(0, LoadMaterial(RingMaterialPath));
	Frames->SetStaticMesh(Plane);
	Frames->SetMaterial(0, LoadMaterial(FrameMaterialPath));
	HealthBars->SetStaticMesh(Plane);
	HealthBars->SetMaterial(0, Frames->GetMaterial(0));
	GhostFrames->SetStaticMesh(Plane);
	GhostFrames->SetMaterial(0, Frames->GetMaterial(0));
	OrderLines->SetStaticMesh(Plane);
	OrderLines->SetMaterial(0, Frames->GetMaterial(0));
	OrderPoints->SetStaticMesh(Plane);
	OrderPoints->SetMaterial(0, Rings->GetMaterial(0));
	ForumRings->SetStaticMesh(Plane);
	ForumRings->SetMaterial(0, Frames->GetMaterial(0));
	AuraRings->SetStaticMesh(Plane);
	AuraRings->SetMaterial(0, Frames->GetMaterial(0));

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
		PlayerController->OnAttackOrderGiven.Remove(AttackOrderHandle);
	}
	OrderHandle.Reset();
	AttackOrderHandle.Reset();
	Super::EndPlay(EndPlayReason);
}

void AHOLSelectionMarkers::HandleOrderGiven(const FVector& Point)
{
	PingPoint = Point;
	PingAge = 0.f;
	bAttackPing = false;
}

void AHOLSelectionMarkers::HandleAttackOrderGiven(const FVector& Point)
{
	HandleOrderGiven(Point);
	bAttackPing = true;
}

void AHOLSelectionMarkers::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (PingAge < TNumericLimits<float>::Max())
	{
		PingAge += DeltaSeconds;
	}
	// Where the units are drawn this frame (D271), once for the rings and the bars.
	if (const UWorld* World = GetWorld())
	{
		HOLUnitView::Gather(*World, Views);
	}
	UpdateHeroFlashes(DeltaSeconds);
	UpdateRings();
	UpdateHealthBars();
	UpdateOrderLines();
	UpdateForumRings();
	UpdateAuraRings();
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
		// The rings follow the units as drawn (Views, gathered in Tick).
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
				const int32 Priority = HOLFormation::GetRowPriority(View.TypeName, View.CarryCapacity, View.bRanged, View.bSiege);
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
			// The rows as the kinds fill them (the shooters behind the melee, the siege in rows of its own), as the server.
			TArray<int32> KindOfPlace;
			HOLFormation::PlaceKinds(Widths, CountOfKind, KindOfPlace);
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
	// J8b.4: the heroes' golden flashes, on their bodies as drawn.
	HeroFlashRings = 0;
	for (const TPair<int32, float>& Flash : HeroFlashes)
	{
		for (const FHOLUnitView& View : Views)
		{
			if (View.OwnerPlayer != Flash.Key || View.TypeName != FHOLStartRules::HeroTypeName)
			{
				continue;
			}
			const double Share = FMath::Clamp(static_cast<double>(Flash.Value / HeroFlashSeconds), 0.0, 1.0);
			if (Share < 0.5)
			{
				const double Inner = View.Radius * HeroFlashInnerFactor;
				Transforms.Add(MarkerTransform(View.Location, Inner * 2.0, Inner * 2.0));
				AddCustomData(CustomData, HeroFlashColor, HeroFlashInnerStroke / Inner);
				++HeroFlashRings;
			}
			const double Out = 1.0 - FMath::Square(1.0 - Share);
			const double Radius = View.Radius * FMath::Lerp(HeroFlashStartFactor, HeroFlashEndFactor, Out);
			Transforms.Add(MarkerTransform(View.Location, Radius * 2.0, Radius * 2.0));
			AddCustomData(CustomData, HeroFlashColor, FMath::Lerp(HeroFlashStartStroke, HeroFlashEndStroke, Share) / Radius);
			++HeroFlashRings;
		}
	}
	if (PingAge < PingSeconds)
	{
		const double Radius = FMath::Lerp(PingStartRadius, PingEndRadius, static_cast<double>(PingAge / PingSeconds));
		Transforms.Add(MarkerTransform(PingPoint, Radius * 2.0, Radius * 2.0));
		AddCustomData(CustomData, bAttackPing ? AttackPingColor : PingColor, PingStroke / Radius);
	}
	SetInstances(*Rings, Transforms, CustomData);
}

void AHOLSelectionMarkers::UpdateHeroFlashes(const float DeltaSeconds)
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;
	for (auto It = HeroFlashes.CreateIterator(); It; ++It)
	{
		It.Value() += DeltaSeconds;
		if (It.Value() >= HeroFlashSeconds)
		{
			It.RemoveCurrent();
		}
	}
	const UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	for (int32 Slot = 0; GameState && Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		const int32 Level = GameState->GetHeroSheet(Slot).Level;
		const int32 OldLevel = HeroLevelWatch.Update(Slot, Level);
		if (OldLevel > 0)
		{
			HeroFlashes.Add(Slot, 0.f);
			UE_LOG(LogHOLMatch, Display, TEXT("Hero flash: slot %d level %d -> %d"), Slot, OldLevel, Level);
		}
	}
}

void AHOLSelectionMarkers::UpdateAuraRings()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	TArray<FString> Parts;
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLPlayerState* PlayerState = PlayerController ? PlayerController->GetHOLPlayerState() : nullptr;
	UWorld* World = GetWorld();
	if (World && PlayerState && !PlayerController->IsSpectator())
	{
		const int32 Slot = PlayerState->GetPlayerIndex();
		// The lowest static surface under a point (a tree's crown is no ground), as the units' (UHOLUnitReceptionSubsystem).
		const auto GroundAt = [World](const FVector2D& Point, const double Fallback)
		{
			TArray<FHitResult> Hits;
			const FCollisionQueryParams Params(SCENE_QUERY_STAT(HOLAuraRing), false);
			World->LineTraceMultiByObjectType(Hits, FVector(Point.X, Point.Y, AuraTraceHalfHeight), FVector(Point.X, Point.Y, -AuraTraceHalfHeight),
				FCollisionObjectQueryParams(ECC_WorldStatic), Params);
			double Height = TNumericLimits<double>::Max();
			for (const FHitResult& Hit : Hits)
			{
				Height = FMath::Min(Height, Hit.ImpactPoint.Z);
			}
			return Hits.Num() > 0 ? Height : Fallback;
		};
		// A circle of strips, each end at the ground's height.
		const auto AddCircle = [&](const FVector2D& Centre, const double Radius, const double Width, const FLinearColor& Color, const double FallbackHeight)
		{
			TArray<FVector, TInlineAllocator<AuraRingPieces + 1>> Points;
			for (int32 Piece = 0; Piece <= AuraRingPieces; ++Piece)
			{
				const double Angle = UE_DOUBLE_TWO_PI * Piece / AuraRingPieces;
				const FVector2D Point = Centre + Radius * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
				Points.Add(FVector(Point.X, Point.Y, GroundAt(Point, FallbackHeight) + AuraRingOffset));
			}
			for (int32 Piece = 0; Piece < AuraRingPieces; ++Piece)
			{
				const FVector Span = Points[Piece + 1] - Points[Piece];
				const double Length = Span.Size();
				Transforms.Add(FTransform(FRotationMatrix::MakeFromX(Span / Length).ToQuat(), (Points[Piece] + Points[Piece + 1]) * 0.5,
					FVector(Length / ShapeSize, Width / ShapeSize, 1.0)));
				CustomData.Append({ Color.R, Color.G, Color.B, BarFullStroke });
			}
		};
		// His living hero, as drawn (one a player, the oldest if a test made more).
		const FHOLUnitView* Hero = nullptr;
		if (UHOLHeroLibrary::GetHeroState(this, Slot) == EHOLHeroState::Alive)
		{
			for (const FHOLUnitView& View : Views)
			{
				if (View.OwnerPlayer == Slot && View.TypeName == FHOLStartRules::HeroTypeName && (!Hero || View.UnitId < Hero->UnitId))
				{
					Hero = &View;
				}
			}
		}
		const FVector2D HeroPoint = Hero ? FVector2D(Hero->Location) : FVector2D::ZeroVector;
		if (Hero)
		{
			AddCircle(HeroPoint, HOLHeroAbility::AuraRadius, AuraRingWidth, AuraRingColor, Hero->Location.Z);
			Parts.Add(FString::Printf(TEXT("hero (%.0f, %.0f) %.0f"), HeroPoint.X, HeroPoint.Y, HOLHeroAbility::AuraRadius));
		}
		// His banner while it stands (public sheet: its place and the step it falls at).
		const int32 Step = UHOLHeroLibrary::GetServerStep(this);
		if (Step >= 0 && UHOLHeroLibrary::GetHeroBannerEndStep(this, Slot) > Step)
		{
			const FIntPoint Banner = UHOLHeroLibrary::GetHeroBannerLocation(this, Slot);
			const FVector2D BannerPoint(Banner.X, Banner.Y);
			AddCircle(BannerPoint, HOLHeroAbility::AuraRadius, AuraRingWidth, AuraRingColor, GroundAt(BannerPoint, 0.0));
			Parts.Add(FString::Printf(TEXT("banner (%d, %d) %.0f"), Banner.X, Banner.Y, HOLHeroAbility::AuraRadius));
		}
		// The banner aimed: its reach around the hero, its aura under the cursor (red out of reach: the server would refuse it).
		FVector Aim;
		if (Hero && PlayerController->IsBannerArmed())
		{
			AddCircle(HeroPoint, HOLHeroAbility::BannerPlantRange, ReachRingWidth, ReachRingColor, Hero->Location.Z);
			Parts.Add(FString::Printf(TEXT("reach %.0f"), HOLHeroAbility::BannerPlantRange));
			if (PlayerController->GetCursorGroundPoint(Aim))
			{
				const bool bInReach = HOLHeroAbility::CanPlantBanner(FVector2D::Distance(FVector2D(Aim), HeroPoint));
				AddCircle(FVector2D(Aim), HOLHeroAbility::AuraRadius, AuraRingWidth, bInReach ? AuraRingColor : AimRefusedColor, Aim.Z);
				Parts.Add(FString::Printf(TEXT("aim (%.0f, %.0f) %s"), Aim.X, Aim.Y, bInReach ? TEXT("ok") : TEXT("out of reach")));
			}
		}
	}
	AuraRingsDescription = Parts.IsEmpty() ? FString(TEXT("none")) : FString::Join(Parts, TEXT(", "));
	SetInstances(*AuraRings, Transforms, CustomData);
}

bool AHOLSelectionMarkers::ShowsHealthBar(const bool bSelected, const bool bFullHealth)
{
	return bSelected || !bFullHealth;
}

void AHOLSelectionMarkers::UpdateHealthBars()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	ShownBars.Reset();
	UnhurtUnselected = 0;
	const AHOLPlayerController* PlayerController = Controller.Get();
	const APlayerCameraManager* Camera = PlayerController ? PlayerController->PlayerCameraManager.Get() : nullptr;
	if (Camera)
	{
		const TArray<uint32>& Selected = PlayerController->GetSelectedUnits();
		const AHOLPlayerState* PlayerState = PlayerController->GetHOLPlayerState();
		const int32 OwnPlayer = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
		const FVector CameraLocation = Camera->GetCameraLocation();
		const FQuat CameraRotation = Camera->GetCameraRotation().Quaternion();
		const FVector Forward = CameraRotation.GetForwardVector();
		const FVector Right = CameraRotation.GetRightVector();
		// Facing the camera: the plane's X along the screen's right, its face (Z) towards the camera.
		const FQuat Facing = FRotationMatrix::MakeFromXZ(Right, -Forward).ToQuat();
		// The view's cone, from its horizontal angle and a 16:9 screen, to leave out the bars off screen.
		const double HalfTan = FMath::Tan(FMath::DegreesToRadians(static_cast<double>(Camera->GetFOVAngle()) * 0.5));
		const double CullCos = FMath::Cos(FMath::Atan(HalfTan * FMath::Sqrt(1.0 + FMath::Square(9.0 / 16.0))) + FMath::DegreesToRadians(BarCullMarginDegrees));
		for (const FHOLUnitView& View : Views)
		{
			const bool bSelected = Selected.Contains(View.UnitId);
			// Always for a selected unit, whatever its side (D.1), otherwise only for a wounded one (the user's nuance on
			// J7-Q5); Views has no dying unit.
			if (!ShowsHealthBar(bSelected, View.bFullHealth))
			{
				++UnhurtUnselected;
				continue;
			}
			const FVector Head = View.Location + FVector(0.0, 0.0, BarHeadHeight * View.MeshScale);
			const FVector ToHead = Head - CameraLocation;
			const double Distance = ToHead.Size();
			if (Distance <= UE_KINDA_SMALL_NUMBER || FVector::DotProduct(ToHead / Distance, Forward) < CullCos)
			{
				continue;
			}
			const float Share = View.GetHealthShare();
			ShownBars.Add({ View.UnitId, View.Health, View.MaxHealth, bSelected });
			const double Scale = FMath::Clamp(Distance / BarReferenceDistance, BarMinScale, BarMaxScale);
			const double Width = BarWidth * Scale;
			const double Height = BarHeight * Scale;
			const double Margin = BarMargin * Scale;
			Transforms.Add(FTransform(Facing, Head, FVector((Width + 2.0 * Margin) / ShapeSize, (Height + 2.0 * Margin) / ShapeSize, 1.0)));
			CustomData.Append({ BarBackColor.R, BarBackColor.G, BarBackColor.B, BarFullStroke });
			// The fill from the left end, its share of the width.
			const double FillWidth = FMath::Max(Width * Share, 1.0);
			const FVector FillCentre = Head + Right * ((FillWidth - Width) * 0.5) - Forward * BarFillLift;
			const FLinearColor& Color = View.OwnerPlayer == OwnPlayer ? BarOwnColor : BarOtherColor;
			Transforms.Add(FTransform(Facing, FillCentre, FVector(FillWidth / ShapeSize, Height / ShapeSize, 1.0)));
			CustomData.Append({ Color.R, Color.G, Color.B, BarFullStroke });
		}
		// The buildings' bars (D345.2a): over the selected building (the user's choice) and, D345.2b, over one hit within
		// BuildingBarAfterHitSeconds (an attacked building); as the units' but wider; above its top (a site's, risen with its work).
		const AHOLBuilding* SelectedBuilding = PlayerController->GetSelection();
		const double Now = GetWorld()->GetTimeSeconds();
		for (TActorIterator<AHOLBuilding> It(GetWorld()); It; ++It)
		{
			const AHOLBuilding* Building = *It;
			const FHOLBuildingTypeRow* Type = Building->GetType();
			const bool bRecentlyHit = Building->GetLastHitTime() >= 0.0 && Now - Building->GetLastHitTime() <= BuildingBarAfterHitSeconds;
			if (!Type || !Building->IsSeenHere() || (Building != SelectedBuilding && !bRecentlyHit) || Building->GetMaxHealth() <= 0)
			{
				continue;
			}
			// As shown: a site's rise, a forum's rubble or rising again (V2.3a).
			const float Rise = Building->GetShownRise();
			const FVector Top = Building->GetActorLocation() + FVector(0.0, 0.0, Type->Height * Rise + BuildingBarLift);
			const FVector ToTop = Top - CameraLocation;
			const double Distance = ToTop.Size();
			if (Distance <= UE_KINDA_SMALL_NUMBER || FVector::DotProduct(ToTop / Distance, Forward) < CullCos)
			{
				continue;
			}
			const float Share = FMath::Clamp(static_cast<float>(Building->GetHealth()) / static_cast<float>(Building->GetMaxHealth()), 0.f, 1.f);
			const double Scale = FMath::Clamp(Distance / BarReferenceDistance, BarMinScale, BarMaxScale);
			const double Width = BarWidth * BuildingBarWidthFactor * Scale;
			const double Height = BarHeight * Scale;
			const double Margin = BarMargin * Scale;
			Transforms.Add(FTransform(Facing, Top, FVector((Width + 2.0 * Margin) / ShapeSize, (Height + 2.0 * Margin) / ShapeSize, 1.0)));
			CustomData.Append({ BarBackColor.R, BarBackColor.G, BarBackColor.B, BarFullStroke });
			const double FillWidth = FMath::Max(Width * Share, 1.0);
			const FVector FillCentre = Top + Right * ((FillWidth - Width) * 0.5) - Forward * BarFillLift;
			const FLinearColor& Color = Building->GetOwnerPlayer() == OwnPlayer ? BarOwnColor : BarOtherColor;
			Transforms.Add(FTransform(Facing, FillCentre, FVector(FillWidth / ShapeSize, Height / ShapeSize, 1.0)));
			CustomData.Append({ Color.R, Color.G, Color.B, BarFullStroke });
		}
	}
	SetInstances(*HealthBars, Transforms, CustomData);
}

void AHOLSelectionMarkers::UpdateForumRings()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	const UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (GameState)
	{
		const FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();
		const FHOLMatchConfig& Config = GameState->GetLaunchConfig();
		// A strip on the ground between two points of the outline, at the forum's height (below the selection's rings).
		const auto AddStrip = [&Transforms, &CustomData](const FVector& Start, const FVector& End, const double Width, const FLinearColor& Color)
		{
			const FVector Span = End - Start;
			const double Length = Span.Size();
			if (Length > UE_KINDA_SMALL_NUMBER)
			{
				Transforms.Add(FTransform(FRotationMatrix::MakeFromX(Span / Length).ToQuat(), (Start + End) * 0.5,
					FVector(Length / ShapeSize, Width / ShapeSize, 1.0)));
				CustomData.Append({ Color.R, Color.G, Color.B, BarFullStroke });
			}
		};
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			const FHOLForumRing Ring = HOLForumRules::GetRing(GameState->GetForumState(Slot), Tuning);
			const AHOLBuilding* Forum = Ring.bShown && !GameState->IsSlotDefeated(Slot) ? AHOLGameState::FindPrimaryForum(*World, Slot) : nullptr;
			if (!Forum)
			{
				continue;
			}
			FLinearColor HolderColor = ForumRingRestColor;
			if (Config.Slots.IsValidIndex(Ring.HolderSlot))
			{
				const FVector3f& Linear = HOLPlayerColors::GetLinearColor(Config.Slots[Ring.HolderSlot].ColorIndex);
				HolderColor = FLinearColor(Linear.X, Linear.Y, Linear.Z);
			}
			const FLinearColor FillColor = HOLForumRules::GetRingColor(Ring.Tint, HolderColor);
			const FBox2D Body = Forum->GetBodyBounds();
			const double Height = Forum->GetActorLocation().Z + ForumRingOffset;
			const auto PointAt = [&Body, &Tuning, Height](const double Share)
			{
				const FVector2D Point = HOLForumRules::GetZoneOutlinePoint(Body, Tuning.HoldRadius, Share);
				return FVector(Point.X, Point.Y, Height);
			};
			// The hold's arc from the top, clockwise, then the rest of the outline, each cut in pieces of the same length.
			const auto AddArc = [&AddStrip, &PointAt](const double From, const double To, const double Width, const FLinearColor& Color)
			{
				const int32 Pieces = FMath::CeilToInt((To - From) * ForumRingPieces);
				for (int32 Piece = 0; Piece < Pieces; ++Piece)
				{
					AddStrip(PointAt(FMath::Lerp(From, To, static_cast<double>(Piece) / Pieces)),
						PointAt(FMath::Lerp(From, To, static_cast<double>(Piece + 1) / Pieces)), Width, Color);
				}
			};
			AddArc(0.0, Ring.Fill, ForumRingFillWidth, FillColor);
			AddArc(Ring.Fill, 1.0, ForumRingRestWidth, ForumRingRestColor);
		}
	}
	SetInstances(*ForumRings, Transforms, CustomData);
}

void AHOLSelectionMarkers::UpdateOrderLines()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> LineTransforms;
	TArray<float> LineData;
	TArray<FTransform> PointTransforms;
	TArray<float> PointData;
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLPlayerState* PlayerState = PlayerController ? PlayerController->GetHOLPlayerState() : nullptr;
	const UWorld* World = GetWorld();
	const double Width = FMath::Max(1.0, static_cast<double>(CVarOrderLineWidth.GetValueOnGameThread()));
	// A filled strip from one point to the next, tilted with the ground between them (X along it, Y level).
	const auto AddLine = [&LineTransforms, &LineData, Width](const FVector& Start, const FVector& End, const FLinearColor& Color)
	{
		const FVector Span = End - Start;
		const double Length = Span.Size();
		if (Length > UE_KINDA_SMALL_NUMBER)
		{
			const FQuat Rotation = FRotationMatrix::MakeFromX(Span / Length).ToQuat();
			LineTransforms.Add(FTransform(Rotation, (Start + End) * 0.5 + FVector(0.0, 0.0, OrderLineOffset),
				FVector(Length / ShapeSize, Width / ShapeSize, 1.0)));
			LineData.Append({ Color.R, Color.G, Color.B, BarFullStroke });
		}
	};
	// The rally line (D328, the user's trial): from each building whose flag shows (UpdateFlag's set) to its flag, white as
	// a rally. The flag stands on the rally point for every kind (the ground, a source's middle, a site's centre), so the
	// line ends there; it leaves the edge of the building's visible shape on the flag's side. Nothing without a building.
	if (PlayerController && (PlayerController->GetSelection() || PlayerController->HasBuildingGroup()))
	{
		TArray<AHOLBuilding*> Buildings;
		PlayerController->GetCommandBuildings(Buildings);
		for (const AHOLBuilding* Building : Buildings)
		{
			const FHOLBuildingTypeRow* Type = Building && Building->HasRallyPoint() ? Building->GetType() : nullptr;
			if (!Type)
			{
				continue;
			}
			const FVector Centre = Building->GetActorLocation();
			// A rally point in several steps (EtapeDefini N.1): to its first step, then from step to step to the flag, a point
			// on each step before the last one (as an order chain's).
			const TArray<FHOLRallyWaypoint>& Waypoints = Building->GetRallyWaypoints();
			const FVector End = Waypoints.IsEmpty() ? Building->GetRallyPoint() : FVector(Waypoints[0].Point);
			for (int32 Step = 0; Step < Waypoints.Num(); ++Step)
			{
				const FVector From = Waypoints[Step].Point;
				const FVector To = Step + 1 < Waypoints.Num() ? FVector(Waypoints[Step + 1].Point) : Building->GetRallyPoint();
				AddLine(From, To, OrderBuildColor);
				PointTransforms.Add(FTransform(FQuat::Identity, From + FVector(0.0, 0.0, OrderPointOffset),
					FVector(OrderPointRadius * 2.0 / ShapeSize, OrderPointRadius * 2.0 / ShapeSize, 1.0)));
				AddCustomData(PointData, OrderBuildColor, 1.0);
			}
			const FVector2D Half = UHOLBuildingSettings::GetVisualSize(Type->Footprint) * 0.5;
			const FVector2D Flat(End.X - Centre.X, End.Y - Centre.Y);
			// The share of the way to the flag at which the ray leaves the shape (axis-aligned): past 1, the flag is on it.
			const double Exit = FMath::Min(FMath::Abs(Flat.X) > UE_KINDA_SMALL_NUMBER ? Half.X / FMath::Abs(Flat.X) : UE_BIG_NUMBER,
				FMath::Abs(Flat.Y) > UE_KINDA_SMALL_NUMBER ? Half.Y / FMath::Abs(Flat.Y) : UE_BIG_NUMBER);
			if (Exit < 1.0)
			{
				const FVector Start(Centre.X + Flat.X * Exit, Centre.Y + Flat.Y * Exit, FMath::Lerp(Centre.Z, End.Z, Exit));
				AddLine(Start, End, OrderBuildColor);
			}
		}
	}
	// Nothing walked without a queue or a selection (D328): the chains reach their owner only, empty without a queue.
	if (World && PlayerState && !PlayerState->GetOrderChains().IsEmpty() && !PlayerController->GetSelectedUnits().IsEmpty())
	{
		const TArray<uint32>& Selected = PlayerController->GetSelectedUnits();
		TMap<uint32, FVector> Displayed;
		for (const FHOLNetOrderChain& Chain : PlayerState->GetOrderChains())
		{
			if (Chain.Steps.IsEmpty() || !Chain.UnitIds.ContainsByPredicate([&Selected](const uint32 Id) { return Selected.Contains(Id); }))
			{
				continue;
			}
			if (Displayed.IsEmpty())
			{
				Displayed.Reserve(Views.Num());
				for (const FHOLUnitView& View : Views)
				{
					Displayed.Add(View.UnitId, View.Location);
				}
			}
			// From the centre of the chain's units as drawn this frame (the group walking), those still standing.
			FVector Start = FVector::ZeroVector;
			int32 Count = 0;
			for (const uint32 Id : Chain.UnitIds)
			{
				if (const FVector* Location = Displayed.Find(Id))
				{
					Start += *Location;
					++Count;
				}
			}
			if (Count == 0)
			{
				continue;
			}
			Start /= Count;
			for (const FHOLNetChainStep& Step : Chain.Steps)
			{
				// An attack at its target as shown here (it walks), else where it stood.
				FVector End;
				AHOLPlayerState::LocateChainStep(*World, Step, End);
				const FLinearColor& Color = GetOrderColor(Step.Kind);
				AddLine(Start, End, Color);
				PointTransforms.Add(FTransform(FQuat::Identity, End + FVector(0.0, 0.0, OrderPointOffset),
					FVector(OrderPointRadius * 2.0 / ShapeSize, OrderPointRadius * 2.0 / ShapeSize, 1.0)));
				AddCustomData(PointData, Color, 1.0);
				Start = End;
			}
		}
	}
	SetInstances(*OrderLines, LineTransforms, LineData);
	SetInstances(*OrderPoints, PointTransforms, PointData);
}

FString AHOLSelectionMarkers::DescribeHealthBars() const
{
	TArray<FShownBar> Sorted = ShownBars;
	Sorted.Sort([](const FShownBar& A, const FShownBar& B) { return A.UnitId < B.UnitId; });
	TArray<FString> Items;
	for (const FShownBar& Bar : Sorted)
	{
		Items.Add(FString::Printf(TEXT("#%u %d/%d%s"), Bar.UnitId, Bar.Health, Bar.MaxHealth, Bar.bSelected ? TEXT(" selected") : TEXT("")));
	}
	return FString::Printf(TEXT("bars %d (%s), unhurt unselected %d, instances %d"), Sorted.Num(), *FString::Join(Items, TEXT(", ")),
		UnhurtUnselected, HealthBars->GetInstanceCount());
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
	// building of a group's active subgroup (all of them for Â« Tous Â», the user's 3rd trial of J6a.5); followed each
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
	// The order lines drawn (D328): their segments and step points (quick_check's move part counts them).
	return FString::Printf(TEXT("rings %d, frame %d, flag %s, ghost %s, order lines %d, order points %d, frames dimmed %d"), Rings->GetInstanceCount() - NumPings - HeroFlashRings,
		Frames->GetInstanceCount(), *Flags, ShownGhost == EGhost::None ? TEXT("none") : ShownGhost == EGhost::Valid ? TEXT("valid") : TEXT("refused"),
		OrderLines->GetInstanceCount(), OrderPoints->GetInstanceCount(), DimmedFrames);
}
