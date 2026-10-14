// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitSubsystem.h"

#include "Avoidance/MassNavigationObstacleTrait.h"
#include "Animation/AnimSequenceTransformProviderData.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "MassEntityTemplate.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "MassLODTrait.h"
#include "MassMovableVisualizationTrait.h"
#include "MassNavMeshNavigationTrait.h"
#include "MassNavigationFragments.h"
#include "Movement/MassMovementTrait.h"
#include "HeroesOfLibertyLog.h"
#include "NavigationSystem.h"
#include "Net/HOLNetUnitDisplay.h"
#include "Simulation/HOLLightUnitAvoidanceTrait.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitMovementTrait.h"
#include "Simulation/HOLUnitTrait.h"
#include "SmoothOrientation/MassSmoothOrientationTrait.h"
#include "Steering/MassSteeringTrait.h"
#include "StructUtils/StructUtilsTypes.h"

namespace UE::HeroesOfLiberty::Units
{
	/** Stress test knob (D224): overrides the settings' budget when not 0, negative meaning "no budget at all". */
	int32 SpawnBudgetOverride = 0;
	FAutoConsoleVariableRef CVarSpawnBudget(TEXT("hol.Units.SpawnBudget"), SpawnBudgetOverride,
		TEXT("Units created per step at most, 0 = UHOLSimulationSettings value, -1 = the whole queue in one step"));

	/** Draws of a birth position before giving up on finding the navmesh under it. */
	constexpr int32 MaxSpawnProjectionAttempts = 4;
}

bool UHOLUnitSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLUnitSubsystem::Deinitialize()
{
	Units.Reset();
	EntityToId.Reset();
	BirthQueue.Reset();
	PendingDestruction.Reset();
	TemplateCache.Reset();
	DisplayTemplateCache.Reset();
	Traits.Reset();

	Super::Deinitialize();
}

const UDataTable* UHOLUnitSubsystem::GetTypeTable() const
{
	if (!bTypeTableLookedUp)
	{
		bTypeTableLookedUp = true;
		const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
		CachedTypeTable = Settings ? Settings->UnitTypesTable.LoadSynchronous() : nullptr;
		if (CachedTypeTable == nullptr)
		{
			UE_LOG(LogHOLSimulation, Error, TEXT("No unit type table at '%s': no unit can be created (D221, D227)"),
				Settings ? *Settings->UnitTypesTable.ToString() : TEXT("unset"));
		}
	}
	return CachedTypeTable;
}

const FHOLUnitTypeRow* UHOLUnitSubsystem::FindTypeRow(const FName TypeName) const
{
	const UDataTable* Table = GetTypeTable();
	return Table ? Table->FindRow<FHOLUnitTypeRow>(TypeName, TEXT("UnitSubsystem"), /*bWarnIfRowMissing*/ false) : nullptr;
}

void UHOLUnitSubsystem::GetTypeNames(TArray<FName>& OutTypeNames) const
{
	OutTypeNames.Reset();
	if (const UDataTable* Table = GetTypeTable())
	{
		OutTypeNames = Table->GetRowNames();
		OutTypeNames.Sort(FNameLexicalLess());
	}
}

int32 UHOLUnitSubsystem::GetBirthBudget() const
{
	using namespace UE::HeroesOfLiberty::Units;
	if (SpawnBudgetOverride < 0)
	{
		return MAX_int32;
	}
	if (SpawnBudgetOverride > 0)
	{
		return SpawnBudgetOverride;
	}
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	// 0 = no budget: a stress test measures a representative peak, one step at worst (D224).
	return Settings && Settings->MaxUnitBirthsPerStep > 0 ? Settings->MaxUnitBirthsPerStep : MAX_int32;
}

const FMassEntityTemplate* UHOLUnitSubsystem::FindTemplate(const FName TypeName, const EHOLUnitTemplateKind Kind)
{
	const bool bDisplay = Kind == EHOLUnitTemplateKind::Display;
	TMap<FName, TSharedPtr<FMassEntityConfig>>& Cache = bDisplay ? DisplayTemplateCache : TemplateCache;
	if (const TSharedPtr<FMassEntityConfig>* Found = Cache.Find(TypeName))
	{
		return &(*Found)->GetOrCreateEntityTemplate(*GetWorld());
	}

	const FHOLUnitTypeRow* Row = FindTypeRow(TypeName);
	if (Row == nullptr)
	{
		UE_LOG(LogHOLSimulation, Error, TEXT("Unknown unit type '%s': no row of that name in the type table (D221)"), *TypeName.ToString());
		return nullptr;
	}

	FHOLUnitTypeConstants Constants = Row->ToConstants(TypeName);

	// Instanced skeletal mesh (D232), drawn when the type has one and its animation provider loads; the static mesh
	// stays the fallback (AnimToTexture, D232).
	USkeletalMesh* SkinnedMesh = Cast<USkeletalMesh>(Constants.SkinnedMesh.TryLoad());
	UAnimSequenceTransformProviderData* AnimationProvider = Cast<UAnimSequenceTransformProviderData>(Constants.AnimationProvider.TryLoad());
	const bool bSkinned = SkinnedMesh && AnimationProvider;
	if (bSkinned)
	{
		const int32 NumSequences = AnimationProvider->GetSequences().Num();
		if (!ensureMsgf(Constants.IdleSequence < NumSequences && Constants.WalkSequence < NumSequences,
			TEXT("Unit type '%s': idle/walk sequence %d/%d out of the %d sequences of %s"),
			*TypeName.ToString(), Constants.IdleSequence, Constants.WalkSequence, NumSequences, *Constants.AnimationProvider.ToString()))
		{
			Constants.IdleSequence = Constants.WalkSequence = 0;
		}
		Constants.IdleLength = AnimationProvider->GetSequencePlayLength(Constants.IdleSequence);
		Constants.WalkLength = AnimationProvider->GetSequencePlayLength(Constants.WalkSequence);
	}
	else if (!Constants.SkinnedMesh.IsNull())
	{
		UE_LOG(LogHOLSimulation, Warning, TEXT("Unit type '%s': skinned mesh %s or animation provider %s missing, drawn as a static mesh"),
			*TypeName.ToString(), *Constants.SkinnedMesh.ToString(), *Constants.AnimationProvider.ToString());
	}

	// Mass identifies a shared fragment by the CRC of its reflected (UPROPERTY) fields, then compares the values
	// through that same reflection: a field left out of the reflection is invisible to both, and the second type
	// silently gets the first one's constants â€” an archer walking at the infantry's speed. Two types never share
	// their constants (the type name is one of them), so a fingerprint already taken is a mistake, not a hit.
	const uint32 ConstantsCrc = UE::StructUtils::GetStructInstanceCrc32(FConstStructView::Make(Constants));
	const FName* ExistingType = ConstantsCrcToType.Find(ConstantsCrc);
	if (ExistingType && *ExistingType != TypeName)
	{
		UE_LOG(LogHOLSimulation, Error, TEXT("Unit types '%s' and '%s' have the same constants fingerprint: check that every field of FHOLUnitTypeConstants is a UPROPERTY (D221, D222)"),
			*ExistingType->ToString(), *TypeName.ToString());
	}
	else if (ExistingType == nullptr)
	{
		ConstantsCrcToType.Add(ConstantsCrc, TypeName);
	}

	TSharedPtr<FMassEntityConfig> Config = MakeShared<FMassEntityConfig>();
	// The config's identity is its ConfigGuid: the constructor taking an owner derives it from the owner's path
	// name, so one owner for every type would give every type the same template id and the registry would answer
	// the first template built for the other types too (an archer walking with the infantry's constants). A
	// fresh guid per type, as the table wants one configuration (so one template) per type (D225).
	Config->SetOwner(*this);

	// Fragments common to every type, posed here and not by each type (D225).
	UHOLUnitTrait* UnitTrait = NewObject<UHOLUnitTrait>(this);
	UnitTrait->Setup(TypeName, Constants);

	// The type's speed (D221/D222).
	UHOLUnitMovementTrait* MovementTrait = NewObject<UHOLUnitMovementTrait>(this);
	FMassMovementParameters Movement;
	Movement.MaxSpeed = Constants.Speed;
	Movement.DefaultDesiredSpeed = Constants.Speed;
	Movement.DefaultDesiredSpeedVariance = 0.f;
	MovementTrait->SetMovement(Movement);

	// The type's obstacle detection distance: D220 kept it in the trait, it is data now.
	UHOLLightUnitAvoidanceTrait* AvoidanceTrait = NewObject<UHOLLightUnitAvoidanceTrait>(this);
	AvoidanceTrait->SetObstacleDetectionDistance(Constants.AvoidanceDistance);

	UMassMovableVisualizationTrait* VisualizationTrait = NewObject<UMassMovableVisualizationTrait>(this);
	const FTransform MeshOffset(FRotator(0.f, Constants.MeshYawOffset, 0.f));
	if (bSkinned)
	{
		FMassSkinnedMeshInstanceVisualizationMeshDesc& MeshDesc = VisualizationTrait->SkinnedMeshInstanceDesc.Meshes.AddDefaulted_GetRef();
		MeshDesc.Asset = SkinnedMesh;
		MeshDesc.TransformProvider = AnimationProvider;
		MeshDesc.bCastShadows = true;
		VisualizationTrait->SkinnedMeshInstanceDesc.bUseTransformOffset = true;
		VisualizationTrait->SkinnedMeshInstanceDesc.TransformOffset = MeshOffset;
	}
	else if (UStaticMesh* Mesh = Cast<UStaticMesh>(Constants.Mesh.TryLoad()))
	{
		FMassStaticMeshInstanceVisualizationMeshDesc& MeshDesc = VisualizationTrait->StaticMeshInstanceDesc.Meshes.AddDefaulted_GetRef();
		MeshDesc.Mesh = Mesh;
		MeshDesc.bCastShadows = true;
		VisualizationTrait->StaticMeshInstanceDesc.bUseTransformOffset = true;
		VisualizationTrait->StaticMeshInstanceDesc.TransformOffset = MeshOffset;
	}
	else
	{
		UE_LOG(LogHOLSimulation, Warning, TEXT("Unit type '%s' has no mesh (%s): it stays invisible"), *TypeName.ToString(), *Constants.Mesh.ToString());
	}
	// Instances at every distance: D218 measures 2 000 units on screen.
	const EMassRepresentationType InstanceRepresentation = bSkinned ? EMassRepresentationType::SkinnedMeshInstance : EMassRepresentationType::StaticMeshInstance;
	for (int32 LODIndex = 0; LODIndex < EMassLOD::Max; ++LODIndex)
	{
		VisualizationTrait->Params.LODRepresentation[LODIndex] = LODIndex == EMassLOD::Off ? EMassRepresentationType::None : InstanceRepresentation;
		VisualizationTrait->LODParams.LODMaxCount[LODIndex] = 5000; // above the 2 000 floor (D190)
	}
	VisualizationTrait->LODParams.BaseLODDistance[EMassLOD::Off] = 1000000.f;
	VisualizationTrait->LODParams.VisibleLODDistance[EMassLOD::Off] = 1000000.f;

	// A display entity is drawn like the unit, without moving by itself: it is placed from the replicated states (D271).
	const TArray<UMassEntityTraitBase*> TypeTraits = bDisplay
		? TArray<UMassEntityTraitBase*>{ UnitTrait,
			NewObject<UHOLNetUnitDisplayTrait>(this),
			NewObject<UMassLODCollectorTrait>(this),
			VisualizationTrait }
		: TArray<UMassEntityTraitBase*>{ UnitTrait, MovementTrait,
			NewObject<UMassSteeringTrait>(this),
			AvoidanceTrait,
			NewObject<UMassNavigationObstacleTrait>(this),
			NewObject<UMassSmoothOrientationTrait>(this),
			NewObject<UMassNavMeshNavigationTrait>(this),
			NewObject<UMassLODCollectorTrait>(this),
			VisualizationTrait };
	for (UMassEntityTraitBase* Trait : TypeTraits)
	{
		Config->AddTrait(*Trait);
		Traits.Add(Trait);
	}

	Cache.Add(TypeName, Config);
	UE_LOG(LogHOLSimulation, Log, TEXT("Unit type '%s' (%s%s): radius %.0f cm, speed %.0f cm/s, %d HP, avoidance %.0f cm, drawn as %s"),
		*TypeName.ToString(), *Row->DisplayName, bDisplay ? TEXT(", display entity") : TEXT(""), Constants.Radius, Constants.Speed, Constants.MaxHealth, Constants.AvoidanceDistance,
		bSkinned ? *FString::Printf(TEXT("skinned instances of %s"), *SkinnedMesh->GetName()) : TEXT("static instances"));

	return &Config->GetOrCreateEntityTemplate(*GetWorld());
}

void UHOLUnitSubsystem::RequestUnits(const FHOLUnitSpawnRequest& Request)
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->GetNetMode() == NM_Client)
	{
		// The simulation is authoritative (D29): a client never creates a unit, M2.5 replicates them.
		UE_LOG(LogHOLSimulation, Error, TEXT("Units are created on the server only (D29, D224)"));
		return;
	}
	if (Request.Count <= 0)
	{
		return;
	}
	if (FindTypeRow(Request.TypeName) == nullptr)
	{
		UE_LOG(LogHOLSimulation, Error, TEXT("Birth request for unknown unit type '%s' ignored (D221)"), *Request.TypeName.ToString());
		return;
	}
	BirthQueue.Add(Request);
}

int32 UHOLUnitSubsystem::ServeBirthRequests()
{
	if (BirthQueue.Num() == 0)
	{
		return 0;
	}

	const int32 Budget = GetBirthBudget();
	int32 NumCreated = 0;
	while (BirthQueue.Num() > 0 && NumCreated < Budget)
	{
		FHOLUnitSpawnRequest& Request = BirthQueue[0];
		const FMassEntityTemplate* Template = FindTemplate(Request.TypeName);
		if (Template == nullptr)
		{
			BirthQueue.RemoveAt(0);
			continue;
		}

		const int32 Count = FMath::Min(Request.Count, Budget - NumCreated);
		CreateUnitEntities(Request, *Template, Count);
		NumCreated += Count;
		Request.Count -= Count;
		if (Request.Count == 0)
		{
			BirthQueue.RemoveAt(0);
		}
	}
	return NumCreated;
}

void UHOLUnitSubsystem::CreateUnitEntities(const FHOLUnitSpawnRequest& Request, const FMassEntityTemplate& Template, const int32 Count)
{
	UWorld* World = GetWorld();
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	const FHOLUnitTypeRow* Row = FindTypeRow(Request.TypeName);
	if (EntitySubsystem == nullptr || Row == nullptr)
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	const FHOLUnitTypeConstants Constants = Row->ToConstants(Request.TypeName);

	int32 NumProjectionFailures = 0;
	TArray<FMassEntityHandle> Created;
	{
		TSharedRef<FMassEntityManager::FEntityCreationContext> CreationContext =
			EntityManager.BatchCreateEntities(Template.GetArchetype(), Template.GetSharedFragmentValues(), Count, Created);

		// Values the traits pose at build time, as the Mass spawner does. The observers run when the creation
		// context dies, at the end of this scope, so everything they are meant to see is already in place.
		TArray<FMassArchetypeEntityCollection> Collections;
		UE::Mass::Utils::CreateEntityCollections(EntityManager, Created, FMassArchetypeEntityCollection::NoDuplicates, Collections);
		EntityManager.BatchSetEntityFragmentValues(Collections, Template.GetInitialFragmentValues());
	}

	for (const FMassEntityHandle Entity : Created)
	{
		FVector Location = Request.Center;
		if (Request.Radius > 0.f && NavSystem)
		{
			// Own draw then projection onto the navmesh: the engine's random navigable point uses a global
			// generator, which would make two runs of the same stress test incomparable (D230: the overlaps of
			// the crowd start at the spawn pattern).
			// Narrow horizontal box: the point is already where it should be, only its height has to come from the
			// navmesh. A wide box makes the query snap the position onto a coarse grid of the navmesh. A point
			// drawn off the navmesh (edge, hole, obstacle) is drawn again; after the last draw the unit keeps that
			// point rather than the centre, where every failure would pile up.
			bool bProjected = false;
			for (int32 Attempt = 0; Attempt < UE::HeroesOfLiberty::Units::MaxSpawnProjectionAttempts && !bProjected; ++Attempt)
			{
				const float Angle = SpawnRandom.FRandRange(0.f, 2.f * PI);
				const float Distance = Request.Radius * FMath::Sqrt(SpawnRandom.FRand());
				Location = Request.Center + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);
				FNavLocation Projected;
				if (NavSystem->ProjectPointToNavigation(Location, Projected, FVector(10.f, 10.f, 500.f)))
				{
					Location = Projected.Location;
					bProjected = true;
				}
			}
			NumProjectionFailures += bProjected ? 0 : 1;
		}

		FTransformFragment& Transform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
		Transform.SetTransform(FTransform(FRotator(0.f, SpawnRandom.FRandRange(-180.f, 180.f), 0.f), Location));
		EntityManager.GetFragmentDataChecked<FHOLPreviousTransformFragment>(Entity).Transform = Transform.GetTransform();
		EntityManager.GetFragmentDataChecked<FAgentRadiusFragment>(Entity).Radius = Constants.Radius;

		const uint32 UnitId = NextUnitId++;
		FHOLUnitStateFragment& State = EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity);
		State.UnitId = UnitId;
		State.CurrentHealth = Constants.MaxHealth;
		State.Activity = EHOLUnitActivity::Idle;
		State.OwnerPlayer = Request.OwnerPlayer;

		// An immobile unit stands still, it does not walk at a null speed: Mass' moving avoidance skips the Stand
		// action, so an idle unit measures what an idle unit really costs (D220). Its target is its own position,
		// so an idle unit has no reason to steer anywhere before its first order (D226).
		FMassMoveTargetFragment& MoveTarget = EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(Entity);
		MoveTarget.Center = Location;
		MoveTarget.Forward = Transform.GetTransform().GetRotation().GetForwardVector();
		MoveTarget.DistanceToGoal = 0.f;
		MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
		MoveTarget.DesiredSpeed.Set(0.f);
		MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);

		Units.Add(UnitId, FHOLUnitRecord{Entity, Request.TypeName});
		EntityToId.Add(Entity, UnitId);
	}

	if (NumProjectionFailures > 0)
	{
		UE_LOG(LogHOLSimulation, Warning, TEXT("%d of %d '%s' units found no navmesh under their birth position after %d draws: born off the navmesh, at their last draw"),
			NumProjectionFailures, Created.Num(), *Request.TypeName.ToString(), UE::HeroesOfLiberty::Units::MaxSpawnProjectionAttempts);
	}
}

void UHOLUnitSubsystem::RequestDestroyUnits(const TConstArrayView<uint32> UnitIds)
{
	PendingDestruction.Append(UnitIds);
}

void UHOLUnitSubsystem::RequestDestroyAllUnits()
{
	PendingDestruction.Reserve(PendingDestruction.Num() + Units.Num());
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		PendingDestruction.Add(Unit.Key);
	}
}

int32 UHOLUnitSubsystem::FlushDestructions()
{
	if (PendingDestruction.Num() == 0)
	{
		return 0;
	}

	UWorld* World = GetWorld();
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (EntitySubsystem == nullptr)
	{
		PendingDestruction.Reset();
		return 0;
	}

	// Deferred destruction (D224): the commands are applied by the flush, at the end of the step, once every
	// processor has walked its chunks. Mass refuses to flush while a processor's execution scope is open.
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FMassCommandBuffer& CommandBuffer = EntityManager.Defer();
	int32 NumDestroyed = 0;
	for (const uint32 UnitId : PendingDestruction)
	{
		const FHOLUnitRecord* Record = Units.Find(UnitId);
		if (Record == nullptr)
		{
			continue;
		}
		EntityToId.Remove(Record->Entity);
		Units.Remove(UnitId);
		CommandBuffer.DestroyEntity(Record->Entity);
		++NumDestroyed;
	}
	PendingDestruction.Reset();
	EntityManager.FlushCommands();

	return NumDestroyed;
}

FMassEntityHandle UHOLUnitSubsystem::FindEntity(const uint32 UnitId) const
{
	const FHOLUnitRecord* Record = Units.Find(UnitId);
	return Record ? Record->Entity : FMassEntityHandle();
}

uint32 UHOLUnitSubsystem::FindUnitId(const FMassEntityHandle Entity) const
{
	const uint32* UnitId = EntityToId.Find(Entity);
	return UnitId ? *UnitId : 0;
}

void UHOLUnitSubsystem::GetUnitIdsOfType(const FName TypeName, TArray<uint32>& OutUnitIds) const
{
	OutUnitIds.Reset();
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		if (Unit.Value.TypeName == TypeName)
		{
			OutUnitIds.Add(Unit.Key);
		}
	}
	// Identifiers are attributed in creation order and never recycled (D223): sorting makes the order stable.
	OutUnitIds.Sort();
}

void UHOLUnitSubsystem::GetAllUnitIds(TArray<uint32>& OutUnitIds) const
{
	OutUnitIds.Reset();
	OutUnitIds.Reserve(Units.Num());
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		OutUnitIds.Add(Unit.Key);
	}
	OutUnitIds.Sort();
}

void UHOLUnitSubsystem::GetUnitCountsByType(TMap<FName, int32>& OutCounts) const
{
	OutCounts.Reset();
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		++OutCounts.FindOrAdd(Unit.Value.TypeName);
	}
}


