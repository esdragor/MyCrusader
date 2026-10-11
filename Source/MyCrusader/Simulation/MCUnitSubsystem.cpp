// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCUnitSubsystem.h"

#include "Avoidance/MassNavigationObstacleTrait.h"
#include "Engine/DataTable.h"
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
#include "MyCrusaderLog.h"
#include "NavigationSystem.h"
#include "Simulation/MCLightUnitAvoidanceTrait.h"
#include "Simulation/MCSimulationFragments.h"
#include "Simulation/MCSimulationSettings.h"
#include "Simulation/MCUnitMovementTrait.h"
#include "Simulation/MCUnitTrait.h"
#include "SmoothOrientation/MassSmoothOrientationTrait.h"
#include "Steering/MassSteeringTrait.h"

namespace UE::MyCrusader::Units
{
	/** Stress test knob (D224): overrides the settings' budget when not 0, negative meaning "no budget at all". */
	int32 SpawnBudgetOverride = 0;
	FAutoConsoleVariableRef CVarSpawnBudget(TEXT("mc.Units.SpawnBudget"), SpawnBudgetOverride,
		TEXT("Units created per step at most, 0 = UMCSimulationSettings value, -1 = the whole queue in one step"));
}

bool UMCUnitSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMCUnitSubsystem::Deinitialize()
{
	Units.Reset();
	EntityToId.Reset();
	BirthQueue.Reset();
	PendingDestruction.Reset();
	TemplateCache.Reset();
	Traits.Reset();

	Super::Deinitialize();
}

const UDataTable* UMCUnitSubsystem::GetTypeTable() const
{
	if (!bTypeTableLookedUp)
	{
		bTypeTableLookedUp = true;
		const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
		CachedTypeTable = Settings ? Settings->UnitTypesTable.LoadSynchronous() : nullptr;
		if (CachedTypeTable == nullptr)
		{
			UE_LOG(LogMCSimulation, Error, TEXT("No unit type table at '%s': no unit can be created (D221, D227)"),
				Settings ? *Settings->UnitTypesTable.ToString() : TEXT("unset"));
		}
	}
	return CachedTypeTable;
}

const FMCUnitTypeRow* UMCUnitSubsystem::FindTypeRow(const FName TypeName) const
{
	const UDataTable* Table = GetTypeTable();
	return Table ? Table->FindRow<FMCUnitTypeRow>(TypeName, TEXT("UnitSubsystem"), /*bWarnIfRowMissing*/ false) : nullptr;
}

int32 UMCUnitSubsystem::GetBirthBudget() const
{
	using namespace UE::MyCrusader::Units;
	if (SpawnBudgetOverride < 0)
	{
		return MAX_int32;
	}
	if (SpawnBudgetOverride > 0)
	{
		return SpawnBudgetOverride;
	}
	const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
	// 0 = no budget: a stress test measures a representative peak, one step at worst (D224).
	return Settings && Settings->MaxUnitBirthsPerStep > 0 ? Settings->MaxUnitBirthsPerStep : MAX_int32;
}

const FMassEntityTemplate* UMCUnitSubsystem::FindTemplate(const FName TypeName)
{
	if (const TSharedPtr<FMassEntityConfig>* Found = TemplateCache.Find(TypeName))
	{
		return &(*Found)->GetOrCreateEntityTemplate(*GetWorld());
	}

	const FMCUnitTypeRow* Row = FindTypeRow(TypeName);
	if (Row == nullptr)
	{
		UE_LOG(LogMCSimulation, Error, TEXT("Unknown unit type '%s': no row of that name in the type table (D221)"), *TypeName.ToString());
		return nullptr;
	}

	const FMCUnitTypeConstants Constants = Row->ToConstants(TypeName);
	TSharedPtr<FMassEntityConfig> Config = MakeShared<FMassEntityConfig>(*this);

	// Fragments common to every type, posed here and not by each type (D225).
	UMCUnitTrait* UnitTrait = NewObject<UMCUnitTrait>(this);
	UnitTrait->Setup(TypeName, Constants);

	// The type's speed (D221/D222).
	UMCUnitMovementTrait* MovementTrait = NewObject<UMCUnitMovementTrait>(this);
	FMassMovementParameters Movement;
	Movement.MaxSpeed = Constants.Speed;
	Movement.DefaultDesiredSpeed = Constants.Speed;
	Movement.DefaultDesiredSpeedVariance = 0.f;
	MovementTrait->SetMovement(Movement);

	// The type's obstacle detection distance: D220 kept it in the trait, it is data now.
	UMCLightUnitAvoidanceTrait* AvoidanceTrait = NewObject<UMCLightUnitAvoidanceTrait>(this);
	AvoidanceTrait->SetObstacleDetectionDistance(Constants.AvoidanceDistance);

	UMassMovableVisualizationTrait* VisualizationTrait = NewObject<UMassMovableVisualizationTrait>(this);
	if (UStaticMesh* Mesh = Cast<UStaticMesh>(Constants.Mesh.TryLoad()))
	{
		FMassStaticMeshInstanceVisualizationMeshDesc& MeshDesc = VisualizationTrait->StaticMeshInstanceDesc.Meshes.AddDefaulted_GetRef();
		MeshDesc.Mesh = Mesh;
		MeshDesc.bCastShadows = true;
		VisualizationTrait->StaticMeshInstanceDesc.bUseTransformOffset = true;
		VisualizationTrait->StaticMeshInstanceDesc.TransformOffset = FTransform(FRotator(0.f, Constants.MeshYawOffset, 0.f));
	}
	else
	{
		UE_LOG(LogMCSimulation, Warning, TEXT("Unit type '%s' has no mesh (%s): it stays invisible"), *TypeName.ToString(), *Constants.Mesh.ToString());
	}
	// Instances at every distance: D218 measures 2 000 units on screen.
	for (int32 LODIndex = 0; LODIndex < EMassLOD::Max; ++LODIndex)
	{
		VisualizationTrait->Params.LODRepresentation[LODIndex] = LODIndex == EMassLOD::Off ? EMassRepresentationType::None : EMassRepresentationType::StaticMeshInstance;
		VisualizationTrait->LODParams.LODMaxCount[LODIndex] = 5000; // above the 2 000 floor (D190)
	}
	VisualizationTrait->LODParams.BaseLODDistance[EMassLOD::Off] = 1000000.f;
	VisualizationTrait->LODParams.VisibleLODDistance[EMassLOD::Off] = 1000000.f;

	const TArray<UMassEntityTraitBase*> TypeTraits = { UnitTrait, MovementTrait,
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

	TemplateCache.Add(TypeName, Config);
	UE_LOG(LogMCSimulation, Log, TEXT("Unit type '%s' (%s): radius %.0f cm, speed %.0f cm/s, %d HP, avoidance %.0f cm"),
		*TypeName.ToString(), *Row->DisplayName, Constants.Radius, Constants.Speed, Constants.MaxHealth, Constants.AvoidanceDistance);

	return &Config->GetOrCreateEntityTemplate(*GetWorld());
}

void UMCUnitSubsystem::RequestUnits(const FMCUnitSpawnRequest& Request)
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->GetNetMode() == NM_Client)
	{
		// The simulation is authoritative (D29): a client never creates a unit, M2.5 replicates them.
		UE_LOG(LogMCSimulation, Error, TEXT("Units are created on the server only (D29, D224)"));
		return;
	}
	if (Request.Count <= 0)
	{
		return;
	}
	if (FindTypeRow(Request.TypeName) == nullptr)
	{
		UE_LOG(LogMCSimulation, Error, TEXT("Birth request for unknown unit type '%s' ignored (D221)"), *Request.TypeName.ToString());
		return;
	}
	BirthQueue.Add(Request);
}

int32 UMCUnitSubsystem::ServeBirthRequests()
{
	if (BirthQueue.Num() == 0)
	{
		return 0;
	}

	const int32 Budget = GetBirthBudget();
	int32 NumCreated = 0;
	while (BirthQueue.Num() > 0 && NumCreated < Budget)
	{
		FMCUnitSpawnRequest& Request = BirthQueue[0];
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

void UMCUnitSubsystem::CreateUnitEntities(const FMCUnitSpawnRequest& Request, const FMassEntityTemplate& Template, const int32 Count)
{
	UWorld* World = GetWorld();
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	const FMCUnitTypeRow* Row = FindTypeRow(Request.TypeName);
	if (EntitySubsystem == nullptr || Row == nullptr)
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	const FMCUnitTypeConstants Constants = Row->ToConstants(Request.TypeName);

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
			FNavLocation NavLocation;
			if (NavSystem->GetRandomPointInNavigableRadius(Request.Center, Request.Radius, NavLocation))
			{
				Location = NavLocation.Location;
			}
		}

		FTransformFragment& Transform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
		Transform.SetTransform(FTransform(FRotator(0.f, SpawnRandom.FRandRange(-180.f, 180.f), 0.f), Location));
		EntityManager.GetFragmentDataChecked<FMCPreviousTransformFragment>(Entity).Transform = Transform.GetTransform();
		EntityManager.GetFragmentDataChecked<FAgentRadiusFragment>(Entity).Radius = Constants.Radius;

		const uint32 UnitId = NextUnitId++;
		FMCUnitStateFragment& State = EntityManager.GetFragmentDataChecked<FMCUnitStateFragment>(Entity);
		State.UnitId = UnitId;
		State.CurrentHealth = Constants.MaxHealth;
		State.Activity = EMCUnitActivity::Idle;

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

		Units.Add(UnitId, FMCUnitRecord{Entity, Request.TypeName});
		EntityToId.Add(Entity, UnitId);
	}
}

void UMCUnitSubsystem::RequestDestroyUnits(const TConstArrayView<uint32> UnitIds)
{
	PendingDestruction.Append(UnitIds);
}

void UMCUnitSubsystem::RequestDestroyAllUnits()
{
	PendingDestruction.Reserve(PendingDestruction.Num() + Units.Num());
	for (const TPair<uint32, FMCUnitRecord>& Unit : Units)
	{
		PendingDestruction.Add(Unit.Key);
	}
}

int32 UMCUnitSubsystem::FlushDestructions()
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
		const FMCUnitRecord* Record = Units.Find(UnitId);
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

FMassEntityHandle UMCUnitSubsystem::FindEntity(const uint32 UnitId) const
{
	const FMCUnitRecord* Record = Units.Find(UnitId);
	return Record ? Record->Entity : FMassEntityHandle();
}

uint32 UMCUnitSubsystem::FindUnitId(const FMassEntityHandle Entity) const
{
	const uint32* UnitId = EntityToId.Find(Entity);
	return UnitId ? *UnitId : 0;
}

void UMCUnitSubsystem::GetUnitIdsOfType(const FName TypeName, TArray<uint32>& OutUnitIds) const
{
	OutUnitIds.Reset();
	for (const TPair<uint32, FMCUnitRecord>& Unit : Units)
	{
		if (Unit.Value.TypeName == TypeName)
		{
			OutUnitIds.Add(Unit.Key);
		}
	}
	// Identifiers are attributed in creation order and never recycled (D223): sorting makes the order stable.
	OutUnitIds.Sort();
}

void UMCUnitSubsystem::GetUnitCountsByType(TMap<FName, int32>& OutCounts) const
{
	OutCounts.Reset();
	for (const TPair<uint32, FMCUnitRecord>& Unit : Units)
	{
		++OutCounts.FindOrAdd(Unit.Value.TypeName);
	}
}
