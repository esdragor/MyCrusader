// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"

#include "AI/NavigationSystemBase.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "HOLCollision.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NavAreas/NavArea_Null.h"
#include "Net/UnrealNetwork.h"
#include "Simulation/HOLSimulationSettings.h"
#include "UObject/ConstructorHelpers.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"
#endif

namespace UE::HeroesOfLiberty::Buildings
{
	// The engine's basic cube (1 m, centred), painted by the matte material's "Color" parameter (D265: no
	// specular highlight; Tools/Editor/make_matte_materials.py).
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* ShapeMaterialPath = TEXT("/Game/Rendering/M_MatteColor.M_MatteColor");
	// The owner's colour is unlit: the same colour on every face, with no darker side (D261).
	const TCHAR* OwnerMaterialPath = TEXT("/Game/Rendering/M_FlatColor.M_FlatColor");
	const FName ColorParameter(TEXT("Color"));

	const FLinearColor StoneColor(0.42f, 0.40f, 0.37f);
	const FLinearColor DarkStoneColor(0.30f, 0.29f, 0.27f);
	/** A foundation's slab (packed earth) and its corner stakes (wood). */
	const FLinearColor FoundationSlabColor(0.40f, 0.33f, 0.24f);
	const FLinearColor StakeColor(0.20f, 0.13f, 0.07f);
}

AHOLBuilding::AHOLBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// The building's pieces: squashed to a site's progress, it rises from the ground (J6a.4).
	RiseRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Rise"));
	RiseRoot->SetupAttachment(RootComponent);

	// The footprint is a forbidden area of the navmesh (D286). A dynamic obstacle is a modifier, the only change a
	// DynamicModifiersOnly navmesh takes at run time; the navmesh widens it by the agent's radius, FitNavObstacle by the
	// bodies' clearance (D310).
	NavObstacle = CreateDefaultSubobject<UBoxComponent>(TEXT("NavObstacle"));
	NavObstacle->SetupAttachment(RootComponent);
	NavObstacle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Counted by the navmesh only once sized and only while the building blocks it (FitNavObstacle; D312: a foundation
	// blocks nothing).
	NavObstacle->SetCanEverAffectNavigation(false);
	NavObstacle->bDynamicObstacle = true;
	NavObstacle->SetAreaClassOverride(UNavArea_Null::StaticClass());

	// Made by the server, seen by everyone: no movement to send, and nothing changes after the setup (until M5).
	bReplicates = true;
	SetReplicatingMovement(false);
	bAlwaysRelevant = true;	// until the fog of war (M7)
	SetNetUpdateFrequency(1.f);

	// Hard references, so that the shapes are cooked with the game.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(UE::HeroesOfLiberty::Buildings::CubePath);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(UE::HeroesOfLiberty::Buildings::ShapeMaterialPath);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OwnerMaterial(UE::HeroesOfLiberty::Buildings::OwnerMaterialPath);
	CubeMesh = Cube.Object;
	PieceMaterial = ShapeMaterial.Object;
	OwnerPieceMaterial = OwnerMaterial.Object;
}

void AHOLBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHOLBuilding, TypeName);
	DOREPLIFETIME(AHOLBuilding, OwnerPlayer);
	DOREPLIFETIME(AHOLBuilding, ColorIndex);
	DOREPLIFETIME(AHOLBuilding, ConstructionState);
	DOREPLIFETIME(AHOLBuilding, ConstructionWork);
	DOREPLIFETIME_CONDITION(AHOLBuilding, ProductionQueue, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLBuilding, RallyPoint, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLBuilding, bHasRallyPoint, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLBuilding, RallySource, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLBuilding, BuildersAtWork, COND_OwnerOnly);
}

const FHOLBuildingTypeRow* AHOLBuilding::GetType() const
{
	return TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
}

bool AHOLBuilding::IsFinished() const
{
	return HOLConstruction::IsServing(ConstructionState);
}

bool AHOLBuilding::IsCamp() const
{
	const FHOLBuildingTypeRow* Type = GetType();
	return Type && HOLConstruction::SendsBuildersToGather(TypeName, Type->Accepts);
}

bool AHOLBuilding::BlocksNavigation() const
{
	return HOLConstruction::BlocksNavigation(ConstructionState);
}

bool AHOLBuilding::HasWorkStarted() const
{
	return HOLConstruction::HasWorkStarted(ConstructionState, ConstructionWork);
}

int64 AHOLBuilding::GetTotalWork() const
{
	const FHOLBuildingTypeRow* Type = GetType();
	return Type ? HOLConstruction::GetTotalWork(HOLConstruction::GetBuildSteps(Type->BuildTime, GetDefault<UHOLSimulationSettings>()->GetStepSeconds())) : 0;
}

float AHOLBuilding::GetProgress() const
{
	return IsFinished() ? 1.f : HOLConstruction::GetProgress(ConstructionWork, GetTotalWork());
}

bool AHOLBuilding::IsSeenHere() const
{
	if (!IsFoundation())
	{
		return true;
	}
	// A client receives only its own foundations (ApplyRelevancy); elsewhere, only the owner's own machine draws one.
	if (GetNetMode() == NM_Client)
	{
		return true;
	}
	const APlayerController* Controller = Cast<APlayerController>(GetOwner());
	return Controller && Controller->IsLocalController();
}

void AHOLBuilding::ApplyRelevancy()
{
	if (!HasAuthority())
	{
		return;
	}
	// A bare foundation reaches its owner alone (J6a.4); from its first work, everyone (until the fog of war, M7).
	const bool bOwnerOnly = IsFoundation();
	if (bAlwaysRelevant == !bOwnerOnly && bOnlyRelevantToOwner == bOwnerOnly)
	{
		return;
	}
	bAlwaysRelevant = !bOwnerOnly;
	bOnlyRelevantToOwner = bOwnerOnly;
#if UE_WITH_IRIS
	// Iris reads the flags once, when the actor starts replicating (the owner filter of a foundation spawned so): its
	// filter is cleared here. The class is always relevant, so no spatial filter comes in its place.
	if (!bOwnerOnly)
	{
		UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(this);
		const UEngineReplicationBridge* Bridge = UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(this);
		const UE::Net::FNetRefHandle Handle = Bridge ? Bridge->GetReplicatedRefHandle(this) : UE::Net::FNetRefHandle::GetInvalid();
		if (ReplicationSystem && Handle.IsValid())
		{
			ReplicationSystem->SetFilter(Handle, UE::Net::InvalidNetObjectFilterHandle);
		}
	}
#endif
	ForceNetUpdate();
}

void AHOLBuilding::SetConstructionState(const EHOLConstructionState InState)
{
	if (ConstructionState == InState)
	{
		return;
	}
	ConstructionState = InState;
	ApplyRelevancy();
	ForceNetUpdate();
	RefreshAppearance();
	OnConstructionChanged.Broadcast();
}

void AHOLBuilding::SetConstructionWork(const int64 InWork)
{
	if (ConstructionWork == InWork)
	{
		return;
	}
	ConstructionWork = InWork;
	ForceNetUpdate();
	UpdateRise();
	OnConstructionChanged.Broadcast();
}

void AHOLBuilding::OnRep_Construction()
{
	// The state rebuilds the look, the work raises the site (RefreshAppearance does both).
	RefreshAppearance();
	OnConstructionChanged.Broadcast();
}

void AHOLBuilding::SetBuildersAtWork(const int32 InBuilders)
{
	const uint8 Builders = static_cast<uint8>(FMath::Clamp(InBuilders, 0, 255));
	if (BuildersAtWork == Builders)
	{
		return;
	}
	BuildersAtWork = Builders;
	ForceNetUpdate();
	OnConstructionChanged.Broadcast();
}

void AHOLBuilding::OnRep_BuildersAtWork()
{
	OnConstructionChanged.Broadcast();
}

FBox2D AHOLBuilding::GetFootprintBounds() const
{
	const FHOLBuildingTypeRow* Type = GetType();
	return HOLConstruction::GetFootprintBounds(FVector2D(GetActorLocation()), Type ? Type->Footprint : FVector2D::ZeroVector,
		static_cast<float>(GetActorRotation().Yaw));
}

void AHOLBuilding::NotifyProductionChanged()
{
	// The queue changes a few times a minute: sent at the next update rather than within the second of NetUpdateFrequency.
	ForceNetUpdate();
	OnProductionChanged.Broadcast();
}

void AHOLBuilding::OnRep_ProductionQueue()
{
	OnProductionChanged.Broadcast();
}

void AHOLBuilding::SetRallyPoint(const FVector& Point, const int32 SourceIndex, AHOLBuilding* Site)
{
	RallyPoint = Point;
	RallySource = SourceIndex;
	RallySite = Site;
	bHasRallyPoint = true;
	ForceNetUpdate();
	OnRallyChanged.Broadcast();
}

void AHOLBuilding::ClearRallyPoint()
{
	bHasRallyPoint = false;
	RallySource = INDEX_NONE;
	RallySite.Reset();
	ForceNetUpdate();
	OnRallyChanged.Broadcast();
}

void AHOLBuilding::OnRep_Rally()
{
	OnRallyChanged.Broadcast();
}

bool AHOLBuilding::IsOnFootprint(const FVector& Point) const
{
	const FHOLBuildingTypeRow* Type = GetType();
	if (!Type)
	{
		return false;
	}
	const FVector Local = GetActorTransform().InverseTransformPosition(Point);
	return FMath::Abs(Local.X) <= Type->Footprint.X * 0.5 && FMath::Abs(Local.Y) <= Type->Footprint.Y * 0.5;
}

void AHOLBuilding::Setup(const FName InTypeName, const uint8 InOwnerPlayer, const uint8 InColorIndex, const EHOLConstructionState InState)
{
	TypeName = InTypeName;
	OwnerPlayer = InOwnerPlayer;
	ColorIndex = InColorIndex;
	ConstructionState = InState;
	// Before the spawn is finished: a foundation starts replicating to its owner alone (J6a.4).
	ApplyRelevancy();
	// A deferred spawn builds its look in BeginPlay; an actor already spawned builds it now.
	if (IsActorInitialized())
	{
		RefreshAppearance();
	}
}

void AHOLBuilding::BeginPlay()
{
	Super::BeginPlay();
	RefreshAppearance();
	// For the scripted checks (J6a.4): what a client receives, and when (a foundation reaches its owner alone).
	if (GetNetMode() == NM_Client && !TypeName.IsNone())
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Building arrived: %s of slot %d, %s"), *TypeName.ToString(), OwnerPlayer,
			IsFoundation() ? TEXT("foundation") : IsFinished() ? TEXT("finished") : TEXT("under construction"));
	}
}

void AHOLBuilding::OnRep_Setup()
{
	RefreshAppearance();
}

void AHOLBuilding::RefreshAppearance()
{
	if (TypeName.IsNone())
	{
		return;
	}
	// A bare foundation is a ghost its owner alone sees (J6a.4); a site, the foundation's slab under the building rising
	// with its work; then the building.
	const ELook Look = IsFinished() ? ELook::Finished : !IsFoundation() ? ELook::Site : IsSeenHere() ? ELook::Ghost : ELook::Hidden;
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	if (BuiltTypeName != TypeName || BuiltLook != Look)
	{
		for (UStaticMeshComponent* Piece : Pieces)
		{
			Piece->DestroyComponent();
		}
		Pieces.Reset();
		OwnerMaterials.Reset();
		OwnerTones.Reset();
		OwnerWhitens.Reset();
		BuiltTypeName = TypeName;
		BuiltLook = Look;

		if (!Type)
		{
			return;
		}
		if (Look == ELook::Ghost)
		{
			BuildGhostShape(*Type);
		}
		else if (Look == ELook::Site || Look == ELook::Finished)
		{
			if (Look == ELook::Site)
			{
				BuildFoundationShape(*Type);
			}
			PieceParent = RiseRoot;
			if (Type->Mesh.IsNull())
			{
				BuildProvisionalShape(*Type);
			}
			else
			{
				BuildFinalMesh(*Type);
			}
			PieceParent = nullptr;
		}
	}
	if (Type)
	{
		FitNavObstacle(*Type);
	}

	UpdateRise();
	ApplyColors();
}

void AHOLBuilding::UpdateRise()
{
	// Squashed from the ground up: the building grows out of its slab (J6a.4); whole once finished.
	const float Rise = IsFinished() ? 1.f : FMath::Max(MinRiseRatio, GetProgress());
	if (RiseRoot && !FMath::IsNearlyEqual(static_cast<float>(RiseRoot->GetRelativeScale3D().Z), Rise))
	{
		RiseRoot->SetRelativeScale3D(FVector(1.0, 1.0, Rise));
	}
}

void AHOLBuilding::ApplyColors()
{
	const FLinearColor OwnerColor(HOLPlayerColors::GetLinearColor(ColorIndex));
	for (int32 Index = 0; Index < OwnerMaterials.Num(); ++Index)
	{
		FLinearColor Shown = FMath::Lerp(OwnerColor * OwnerTones[Index], FLinearColor::White, OwnerWhitens[Index]);
		Shown.A = 1.f;
		OwnerMaterials[Index]->SetVectorParameterValue(UE::HeroesOfLiberty::Buildings::ColorParameter, Shown);
	}
}

void AHOLBuilding::BuildProvisionalShape(const FHOLBuildingTypeRow& Type)
{
	using namespace UE::HeroesOfLiberty::Buildings;

	// D261: a block and a central tower; the top of the block (a darker tone) and the tower's flat roof, a little wider
	// than the tower, wear the owner's colour, as they are what the RTS camera sees from above; unlit, so that the colour
	// is the same everywhere (a lit cone showed a dark side, an unlit one read as a flat disc).
	const FVector2D Footprint = Type.Footprint;
	const float Height = Type.Height;
	const float BaseHeight = Height * BaseHeightRatio;
	const float CapThickness = Height * CapThicknessRatio;
	const float TowerHeight = Height - CapThickness;

	AddPiece(CubeMesh, FVector(Footprint, BaseHeight), 0.f, StoneColor, false);
	AddPiece(CubeMesh, FVector(Footprint, CapThickness), BaseHeight, FLinearColor::White, true, BaseTopTone);
	AddPiece(CubeMesh, FVector(Footprint * TowerSizeRatio, TowerHeight), 0.f, DarkStoneColor, false);
	AddPiece(CubeMesh, FVector(Footprint * RoofSizeRatio, CapThickness), TowerHeight, FLinearColor::White, true);
}

void AHOLBuilding::FitNavObstacle(const FHOLBuildingTypeRow& Type)
{
	// Widened by the bodies' clearance (D310): the navmesh widens a forbidden area by the agent's radius (measured at J6a.1:
	// its edge 63 to 77 cm from the sides, 82 to 95 cm from the corners), so a walker's body brushed the wall.
	const FVector Extent(Type.Footprint * 0.5 + FVector2D(UHOLBuildingSettings::GetBodyClearance()), Type.Height * 0.5);
	const bool bBlocks = BlocksNavigation();
	const bool bWasBlocking = NavObstacle->CanEverAffectNavigation();
	if (bBlocks == bWasBlocking && NavObstacle->GetUnscaledBoxExtent().Equals(Extent, 0.01))
	{
		return;
	}
	NavObstacle->SetBoxExtent(Extent);
	NavObstacle->SetRelativeLocation(FVector(0., 0., Type.Height * 0.5));
	// Counted by the navmesh from now on with its size, or no more; a bare foundation never is (D312). Registered with
	// the default size when the actor spawned: the navigation takes the new size.
	NavObstacle->SetCanEverAffectNavigation(bBlocks);
	if (bBlocks)
	{
		FNavigationSystem::UpdateComponentData(*NavObstacle);
	}
	// The paths kept for the gathering's walks across its footprint are forgotten (D305), when the navmesh changed there.
	UHOLCommandSubsystem* Commands = (bBlocks || bWasBlocking) && GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (Commands)
	{
		Commands->NotifyNavigationChanged(NavObstacle->Bounds.GetBox());
	}
}

void AHOLBuilding::BuildFoundationShape(const FHOLBuildingTypeRow& Type)
{
	using namespace UE::HeroesOfLiberty::Buildings;

	// Flat and clearly unfinished (J6a.3): packed earth over the footprint, its edges at the owner's colour (unlit, the
	// colour seen from the RTS camera), a wooden stake at each corner. Units walk over it until its first work (D312).
	const FVector2D Footprint = Type.Footprint;
	AddPiece(CubeMesh, FVector(Footprint, FoundationSlabHeight), 0.f, FoundationSlabColor, false);
	const double Border = FMath::Min(static_cast<double>(FoundationBorderWidth), Footprint.GetMin() * 0.25);
	const FVector AlongX(Footprint.X, Border, FoundationBorderHeight);
	const FVector AlongY(Border, Footprint.Y - 2.0 * Border, FoundationBorderHeight);
	const FVector2D Half = Footprint * 0.5;
	const auto Move = [](UStaticMeshComponent* Piece, const FVector2D& Offset)
	{
		if (Piece)
		{
			Piece->AddRelativeLocation(FVector(Offset, 0.0));
		}
	};
	Move(AddPiece(CubeMesh, AlongX, 0.f, FLinearColor::White, true), FVector2D(0.0, Half.Y - Border * 0.5));
	Move(AddPiece(CubeMesh, AlongX, 0.f, FLinearColor::White, true), FVector2D(0.0, -Half.Y + Border * 0.5));
	Move(AddPiece(CubeMesh, AlongY, 0.f, FLinearColor::White, true), FVector2D(Half.X - Border * 0.5, 0.0));
	Move(AddPiece(CubeMesh, AlongY, 0.f, FLinearColor::White, true), FVector2D(-Half.X + Border * 0.5, 0.0));
	const FVector Stake(FoundationStakeSize, FoundationStakeSize, FoundationStakeHeight);
	const FVector2D Corner = Half - FVector2D(FoundationStakeSize * 0.5);
	for (const FVector2D Sign : { FVector2D(1.0, 1.0), FVector2D(1.0, -1.0), FVector2D(-1.0, 1.0), FVector2D(-1.0, -1.0) })
	{
		Move(AddPiece(CubeMesh, Stake, 0.f, StakeColor, false), Corner * Sign);
	}
}

void AHOLBuilding::BuildGhostShape(const FHOLBuildingTypeRow& Type)
{
	// Planned, not started (J6a.4, the user's choice): drawn as the placement ghost is, a frame on the ground and the edges
	// of the volume, light at the owner's colour; no slab before the first work.
	const FVector2D Footprint = Type.Footprint;
	const FVector2D Half = Footprint * 0.5;
	const float Height = Type.Height;
	const auto Move = [](UStaticMeshComponent* Piece, const FVector& Offset)
	{
		if (Piece)
		{
			Piece->AddRelativeLocation(Offset);
		}
	};
	const auto AddEdge = [this](const FVector& Size, const float BottomZ)
	{
		UStaticMeshComponent* Piece = AddPiece(CubeMesh, Size, BottomZ, FLinearColor::White, true);
		if (Piece)
		{
			OwnerWhitens.Last() = GhostWhiten;
		}
		return Piece;
	};
	const double Frame = FMath::Min(static_cast<double>(GhostFrameWidth), Footprint.GetMin() * 0.25);
	for (const double Sign : { 1.0, -1.0 })
	{
		// The frame on the ground, then the edges of the top, along each axis.
		Move(AddEdge(FVector(Footprint.X, Frame, GhostFrameHeight), 0.f), FVector(0.0, Sign * (Half.Y - Frame * 0.5), 0.0));
		Move(AddEdge(FVector(Frame, Footprint.Y - 2.0 * Frame, GhostFrameHeight), 0.f), FVector(Sign * (Half.X - Frame * 0.5), 0.0, 0.0));
		Move(AddEdge(FVector(Footprint.X, GhostEdgeWidth, GhostEdgeWidth), Height - GhostEdgeWidth), FVector(0.0, Sign * (Half.Y - GhostEdgeWidth * 0.5), 0.0));
		Move(AddEdge(FVector(GhostEdgeWidth, Footprint.Y, GhostEdgeWidth), Height - GhostEdgeWidth), FVector(Sign * (Half.X - GhostEdgeWidth * 0.5), 0.0, 0.0));
	}
	const FVector2D Corner = Half - FVector2D(GhostEdgeWidth * 0.5);
	for (const FVector2D Sign : { FVector2D(1.0, 1.0), FVector2D(1.0, -1.0), FVector2D(-1.0, 1.0), FVector2D(-1.0, -1.0) })
	{
		Move(AddEdge(FVector(GhostEdgeWidth, GhostEdgeWidth, Height), 0.f), FVector(Corner * Sign, 0.0));
	}
	// The whole volume answers a click (D262), unseen: the ghost is selected (and cancelled) as a foundation is.
	if (UStaticMeshComponent* Box = AddPiece(CubeMesh, FVector(Footprint, Height), 0.f, FLinearColor::White, false))
	{
		Box->SetVisibility(false);
	}
}

void AHOLBuilding::BuildFinalMesh(const FHOLBuildingTypeRow& Type)
{
	UStaticMesh* Mesh = Type.Mesh.LoadSynchronous();
	if (!Mesh)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("Building type %s: mesh %s not found"), *TypeName.ToString(), *Type.Mesh.ToString());
		return;
	}
	UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(this);
	Piece->SetStaticMesh(Mesh);
	Piece->SetCollisionResponseToChannel(HOLCollision::Selection, ECR_Block);	// D262
	Piece->SetCanEverAffectNavigation(false);	// the footprint is NavObstacle (D286)
	Piece->SetupAttachment(PieceParent ? PieceParent : RootComponent.Get());
	Piece->RegisterComponent();
	Pieces.Add(Piece);
}

UStaticMeshComponent* AHOLBuilding::AddPiece(UStaticMesh* Mesh, const FVector& Size, const float BottomZ, const FLinearColor& Color,
	const bool bOwnerColor, const float OwnerTone)
{
	UMaterialInterface* BaseMaterial = bOwnerColor ? OwnerPieceMaterial : PieceMaterial;
	if (!Mesh || !BaseMaterial)
	{
		return nullptr;
	}
	// Scaled to Size and laid with its bounding box's bottom at BottomZ, centred on the actor.
	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const FVector Scale = Size / (2.0 * Bounds.BoxExtent);
	const FVector Origin = Bounds.Origin * Scale;
	const double MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale.Z;

	UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(this);
	Piece->SetStaticMesh(Mesh);
	Piece->SetRelativeScale3D(Scale);
	Piece->SetRelativeLocation(FVector(-Origin.X, -Origin.Y, BottomZ - MeshBottom));
	Piece->SetCastShadow(false);	// D258
	Piece->SetCanEverAffectNavigation(false);	// the footprint is NavObstacle (D286)
	Piece->SetCollisionResponseToChannel(HOLCollision::Selection, ECR_Block);	// D262
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Piece);
	Material->SetVectorParameterValue(UE::HeroesOfLiberty::Buildings::ColorParameter, Color);
	Piece->SetMaterial(0, Material);
	Piece->SetupAttachment(PieceParent ? PieceParent : RootComponent.Get());
	Piece->RegisterComponent();

	Pieces.Add(Piece);
	if (bOwnerColor)
	{
		OwnerMaterials.Add(Material);
		OwnerTones.Add(OwnerTone);
		OwnerWhitens.Add(0.f);
	}
	return Piece;
}
