// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"

#include "AI/NavigationSystemBase.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLRubble.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HOLCollision.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
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
	// The tick applies the look outside the simulation step (D325): on only while the look is to be redone, then off.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
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
	DOREPLIFETIME(AHOLBuilding, DamageTaken);
	DOREPLIFETIME(AHOLBuilding, bPrimaryForum);
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
	// The navmesh's forbidden area counts for the simulation, at once; the look waits for the frame (D325).
	RefreshNavObstacle();
	MarkAppearanceDirty();
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
	// The site rises at the frame, outside the simulation step (D325).
	MarkAppearanceDirty();
	OnConstructionChanged.Broadcast();
}

void AHOLBuilding::OnRep_Construction()
{
	// The state rebuilds the look, the work raises the site (RefreshVisuals does both, at the frame, D325).
	RefreshNavObstacle();
	MarkAppearanceDirty();
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

int32 AHOLBuilding::GetMaxHealth() const
{
	const FHOLBuildingTypeRow* Type = GetType();
	return Type ? Type->MaxHealth : 0;
}

int32 AHOLBuilding::GetHealth() const
{
	// Finished: the whole of it; a site: its work's share (D312); both less the damage taken.
	const bool bFinished = IsFinished();
	return HOLConstruction::GetHealthWithDamage(GetMaxHealth(), bFinished ? 1 : ConstructionWork, bFinished ? 1 : GetTotalWork(), DamageTaken);
}

int32 AHOLBuilding::TakeDamagePoints(const int32 Points)
{
	const int32 Health = GetHealth();
	const int32 Taken = FMath::Clamp(Points, 0, Health);
	if (Taken > 0)
	{
		DamageTaken = static_cast<uint16>(FMath::Min(static_cast<int32>(DamageTaken) + Taken, static_cast<int32>(MAX_uint16)));
		LastHitTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
		ForceNetUpdate();
		OnConstructionChanged.Broadcast();
	}
	return Health - Taken;
}

void AHOLBuilding::RestoreHealth()
{
	if (DamageTaken > 0)
	{
		DamageTaken = 0;
		ForceNetUpdate();
		OnConstructionChanged.Broadcast();
	}
}

bool AHOLBuilding::IsForumDown() const
{
	const UWorld* World = GetWorld();
	const AHOLGameState* GameState = bPrimaryForum && World ? World->GetGameState<AHOLGameState>() : nullptr;
	return GameState && GameState->IsForumDown(OwnerPlayer);
}

FHOLForumBlock AHOLBuilding::GetForumBlock(const bool bOwn) const
{
	const UWorld* World = GetWorld();
	const AHOLGameState* GameState = bPrimaryForum && World ? World->GetGameState<AHOLGameState>() : nullptr;
	return HOLForumRules::GetSelectionBlock(GameState ? &GameState->GetForumState(OwnerPlayer) : nullptr, FHOLForumTuning::FromSettings(), bOwn,
		BuildersAtWork);
}

bool AHOLBuilding::TakesDeliveries() const
{
	return IsFinished() && !IsForumDown();
}

bool AHOLBuilding::NeedsBuilders() const
{
	return !IsFinished() || IsForumDown();
}

void AHOLBuilding::OnRep_DamageTaken(const uint16 OldDamageTaken)
{
	if (DamageTaken > OldDamageTaken)
	{
		LastHitTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	}
	OnConstructionChanged.Broadcast();
}

void AHOLBuilding::ReleaseFootprint()
{
	if (!NavObstacle || !NavObstacle->CanEverAffectNavigation())
	{
		return;
	}
	const FBox Area = NavObstacle->Bounds.GetBox();
	NavObstacle->SetCanEverAffectNavigation(false);
	if (UHOLCommandSubsystem* Commands = GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr)
	{
		Commands->NotifyNavigationChanged(Area);
	}
}

FBox2D AHOLBuilding::GetFootprintBounds() const
{
	const FHOLBuildingTypeRow* Type = GetType();
	return HOLConstruction::GetFootprintBounds(FVector2D(GetActorLocation()), Type ? Type->Footprint : FVector2D::ZeroVector,
		static_cast<float>(GetActorRotation().Yaw));
}

FVector2D AHOLBuilding::GetBodySize() const
{
	const FHOLBuildingTypeRow* Type = GetType();
	return Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D::ZeroVector;
}

bool AHOLBuilding::FindNearestBodyPoint(const UWorld& World, const FVector2D& From, const double MaxDistance, const int32 OwnerToSkip, FVector2D& OutPoint)
{
	double Best = FMath::Square(MaxDistance);
	bool bFound = false;
	for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
	{
		const AHOLBuilding* Building = *It;
		if (!IsValid(Building) || Building->IsFoundation() || !Building->IsSeenHere() || static_cast<int32>(Building->GetOwnerPlayer()) == OwnerToSkip)
		{
			continue;
		}
		const FBox2D Box = Building->GetBodyBounds();
		const FVector2D Point(FMath::Clamp(From.X, Box.Min.X, Box.Max.X), FMath::Clamp(From.Y, Box.Min.Y, Box.Max.Y));
		const double Distance = FVector2D::DistSquared(Point, From);
		if (Distance <= Best)
		{
			Best = Distance;
			OutPoint = Point;
			bFound = true;
		}
	}
	return bFound;
}

FBox2D AHOLBuilding::GetBodyBounds() const
{
	return HOLConstruction::GetFootprintBounds(FVector2D(GetActorLocation()), GetBodySize(), static_cast<float>(GetActorRotation().Yaw));
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

bool AHOLBuilding::IsOnBody(const FVector& Point) const
{
	if (!GetType())
	{
		return false;
	}
	const FVector2D Half = GetBodySize() * 0.5;
	const FVector Local = GetActorTransform().InverseTransformPosition(Point);
	return FMath::Abs(Local.X) <= Half.X && FMath::Abs(Local.Y) <= Half.Y;
}

void AHOLBuilding::Setup(const FName InTypeName, const uint8 InOwnerPlayer, const uint8 InColorIndex, const EHOLConstructionState InState)
{
	TypeName = InTypeName;
	OwnerPlayer = InOwnerPlayer;
	ColorIndex = InColorIndex;
	ConstructionState = InState;
	// Before the spawn is finished: a foundation starts replicating to its owner alone (J6a.4).
	ApplyRelevancy();
	// A deferred spawn builds its look in BeginPlay; an actor already spawned fits its forbidden area now, its look at the
	// frame (D325).
	if (IsActorInitialized())
	{
		RefreshNavObstacle();
		MarkAppearanceDirty();
	}
}

void AHOLBuilding::BeginPlay()
{
	Super::BeginPlay();
	// At once: a building that appears is seen at the frame it appears.
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
	RefreshNavObstacle();
	MarkAppearanceDirty();
}

void AHOLBuilding::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ApplyPendingAppearance();
}

void AHOLBuilding::MarkAppearanceDirty()
{
	// Nobody looks at a dedicated server: no piece is made there (D325; selection is the client's, D262).
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	bAppearanceDirty = true;
	SetActorTickEnabled(true);
}

void AHOLBuilding::ApplyPendingAppearance()
{
	if (bAppearanceDirty)
	{
		RefreshVisuals();
	}
	SetActorTickEnabled(false);
}

EHOLForumLook AHOLBuilding::GetShownLook() const
{
	return BuiltLook == ELook::Rubble ? EHOLForumLook::Rubble : BuiltLook == ELook::Site ? EHOLForumLook::Site : EHOLForumLook::Standing;
}

void AHOLBuilding::NotifyForumChanged()
{
	if (bPrimaryForum)
	{
		MarkAppearanceDirty();
	}
}

void AHOLBuilding::OnRep_PrimaryForum()
{
	MarkAppearanceDirty();
}

FHOLForumLookChoice AHOLBuilding::GetForumLook() const
{
	const UWorld* World = GetWorld();
	const AHOLGameState* GameState = bPrimaryForum && World ? World->GetGameState<AHOLGameState>() : nullptr;
	return GameState && GameState->IsForumDown(OwnerPlayer)
		? HOLForumRules::GetLook(GameState->GetForumState(OwnerPlayer), FHOLForumTuning::FromSettings()) : FHOLForumLookChoice();
}

float AHOLBuilding::GetTargetRise() const
{
	const FHOLForumLookChoice Forum = GetForumLook();
	if (Forum.Look == EHOLForumLook::Rubble)
	{
		// Its heap's height (the health bar over it).
		const FHOLBuildingTypeRow* Type = GetType();
		const float Height = Type ? static_cast<float>(Type->Height) : 0.f;
		return Height > 0.f ? FMath::Clamp((AHOLRubble::HeapHeight + AHOLRubble::PieceMaxHeight) / Height, MinRiseRatio, 1.f) : MinRiseRatio;
	}
	// A forum rising again rises as any site, at the rebuilding's progress (V2.3a), not its own work.
	if (Forum.Look == EHOLForumLook::Site)
	{
		return FMath::Max(MinRiseRatio, Forum.Progress);
	}
	return IsFinished() ? 1.f : FMath::Max(MinRiseRatio, GetProgress());
}

void AHOLBuilding::BuildRubbleShape(const FHOLBuildingTypeRow& Type)
{
	TArray<AHOLRubble::FLookPiece> Heap;
	AHOLRubble::GetLookPieces(UHOLBuildingSettings::GetVisualSize(Type.Footprint), ColorIndex, Heap);
	for (const AHOLRubble::FLookPiece& Piece : Heap)
	{
		if (UStaticMeshComponent* Component = AddPiece(CubeMesh, Piece.Size, 0.f, Piece.Color, /*bOwnerColor*/ false))
		{
			Component->SetRelativeLocationAndRotation(Component->GetRelativeLocation() + FVector(Piece.Offset, 0.0), FRotator(0.f, Piece.Yaw, 0.f));
		}
	}
}

float AHOLBuilding::GetShownRise() const
{
	return RiseRoot ? static_cast<float>(RiseRoot->GetRelativeScale3D().Z) : 1.f;
}

void AHOLBuilding::RefreshAppearance()
{
	RefreshNavObstacle();
	RefreshVisuals();
}

void AHOLBuilding::RefreshNavObstacle()
{
	if (const FHOLBuildingTypeRow* Type = GetType())
	{
		FitNavObstacle(*Type);
	}
}

void AHOLBuilding::RefreshVisuals()
{
	bAppearanceDirty = false;
	if (TypeName.IsNone() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	// A bare foundation is a ghost its owner alone sees (J6a.4); a site, the foundation's slab under the building rising
	// with its work; then the building.
	// A main forum down (V2.3a, V2-Q1): rubble through the ruin, then any site's look while it rises again.
	const FHOLForumLookChoice Forum = GetForumLook();
	const ELook Look = Forum.Look == EHOLForumLook::Rubble ? ELook::Rubble : Forum.Look == EHOLForumLook::Site ? ELook::Site
		: IsFinished() ? ELook::Finished : !IsFoundation() ? ELook::Site : IsSeenHere() ? ELook::Ghost : ELook::Hidden;
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
		if (Look == ELook::Rubble)
		{
			BuildRubbleShape(*Type);
		}
		else if (Look == ELook::Ghost)
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
	UpdateRise();
	ApplyColors();
}

void AHOLBuilding::UpdateRise()
{
	// Squashed from the ground up: the building grows out of its slab (J6a.4); whole once finished.
	const float Rise = GetTargetRise();
	if (!RiseRoot)
	{
		return;
	}
	// Grown by a visible step only, or whole, or lower than shown (a site begun again, its pieces built whole).
	const float Shown = static_cast<float>(RiseRoot->GetRelativeScale3D().Z);
	if (!FMath::IsNearlyEqual(Shown, Rise) && (Rise >= 1.f || Rise < Shown || Rise - Shown >= RiseStepRatio))
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
	// is the same everywhere (a lit cone showed a dark side, an unlit one read as a flat disc). Its visible size, a little
	// inset in its tiles and wider than its body (D324 refined).
	const FVector2D Footprint = UHOLBuildingSettings::GetVisualSize(Type.Footprint);
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
	// The building's body (D324), inset in its tiles, widened by the bodies' clearance (D310): the navmesh widens a forbidden
	// area by the agent's radius (measured at J6a.1: its edge 63 to 77 cm from the sides, 82 to 95 cm from the corners), so a
	// walker's body brushed the wall.
	const FVector2D Body = UHOLBuildingSettings::GetBodySize(Type.Footprint);
	const FVector Extent(Body * 0.5 + FVector2D(UHOLBuildingSettings::GetBodyClearance()), Type.Height * 0.5);
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
	// The paths kept for the gathering's walks across its footprint are forgotten (D305), when the navmesh changed there;
	// a footprint that starts blocking it also gives a new path to the walkers whose way it cuts, once its tiles are
	// rebuilt (J6b.2, D314).
	UHOLCommandSubsystem* Commands = (bBlocks || bWasBlocking) && GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (Commands && bBlocks && !bWasBlocking)
	{
		FHOLBlockedFootprint Blocked;
		Blocked.TypeName = TypeName;
		Blocked.Centre = FVector2D(GetActorLocation());
		Blocked.Size = Body;
		Blocked.Yaw = static_cast<float>(GetActorRotation().Yaw);
		Blocked.Area = NavObstacle->Bounds.GetBox();
		Commands->NotifyFootprintBlocked(Blocked);
	}
	else if (Commands)
	{
		Commands->NotifyNavigationChanged(NavObstacle->Bounds.GetBox());
	}
}

void AHOLBuilding::BuildFoundationShape(const FHOLBuildingTypeRow& Type)
{
	using namespace UE::HeroesOfLiberty::Buildings;

	// Flat and clearly unfinished (J6a.3): packed earth over the footprint, its edges at the owner's colour (unlit, the
	// colour seen from the RTS camera), a wooden stake at each corner. Units walk over it until its first work (D312). Under
	// the building's visible shape (D324 refined).
	const FVector2D Footprint = UHOLBuildingSettings::GetVisualSize(Type.Footprint);
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
	// of the volume, light at the owner's colour; no slab before the first work. The building's visible shape (D324 refined).
	const FVector2D Footprint = UHOLBuildingSettings::GetVisualSize(Type.Footprint);
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
	// Traced (the selection), never simulated, and nobody listens to its overlaps: no overlap test when it is made or
	// rises (a site's rise moved its pieces' bodies at each visible step, J6b.3).
	Piece->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Piece->SetGenerateOverlapEvents(false);
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
	// As BuildFinalMesh: traced only, no overlap test (J6b.3).
	Piece->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Piece->SetGenerateOverlapEvents(false);
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
