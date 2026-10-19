// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Commands/HOLCommandSubsystem.h"
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
#include "MassRepresentationFragments.h"
#include "MassNavMeshNavigationTrait.h"
#include "MassNavigationFragments.h"
#include "Movement/MassMovementTrait.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLStartSlot.h"
#include "NavigationSystem.h"
#include "Net/HOLNetUnitDisplay.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Simulation/HOLBuildProcessor.h"
#include "Simulation/HOLLightUnitAvoidanceTrait.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitView.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitMovementTrait.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLUnitTrait.h"
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
	/** Seconds a unit takes from standing to its full speed (Mass' 250 cm/s² at the 300 cm/s of the first types). */
	constexpr float SecondsToFullSpeed = 1.2f;

	/** Where the units are and how they are drawn: for checking a match's units (J3) without looking at the screen. */
	FAutoConsoleCommandWithWorldAndArgs CommandReport(TEXT("hol.Units.Report"),
		TEXT("hol.Units.Report [MaxUnits=10]: logs the number of units and, for the first ones, their type, owner, location, LOD and drawing"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			// A client simulates nothing: it reports the units it displays (D271).
			const UHOLUnitReceptionSubsystem* Reception = World ? World->GetSubsystem<UHOLUnitReceptionSubsystem>() : nullptr;
			if (Reception && World->GetNetMode() == NM_Client)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Units report: client, %s"), *Reception->DescribeDisplayed());
				UE_LOG(LogHOLSimulation, Display, TEXT("Units looks: client, %s"), *Reception->DescribeLooks());
			}
			else if (const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Units report: %s"), *Units->DescribeUnits(Args.IsEmpty() ? 10 : FCString::Atoi(*Args[0])));
			}
		}));

	/** Groups to try the orders on (J4, D289), without producing them: outside the rules (no cost, over the cap). */
	FAutoConsoleCommandWithWorldAndArgs CommandSpawn(TEXT("hol.Units.Spawn"),
		TEXT("hol.Units.Spawn Count X Y [Slot=0] [Type=Peasant] [Radius=1000]: makes units of a slot around a ground point (cm), server or solo; a test command, free and over the population cap"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLUnitSubsystem* Units = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
			if (!Units || Args.Num() < 3)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Units.Spawn Count X Y [Slot=0] [Type=Peasant] [Radius=1000], on the server or in solo"));
				return;
			}
			FHOLUnitSpawnRequest Request;
			Request.Count = FCString::Atoi(*Args[0]);
			Request.Center = FVector(FCString::Atod(*Args[1]), FCString::Atod(*Args[2]), 0.);
			Request.OwnerPlayer = static_cast<uint8>(Args.Num() >= 4 ? FCString::Atoi(*Args[3]) : 0);
			Request.TypeName = FName(Args.Num() >= 5 ? *Args[4] : TEXT("Peasant"));
			Request.Radius = Args.Num() >= 6 ? FCString::Atof(*Args[5]) : 1000.f;
			Units->RequestUnits(Request);
			UE_LOG(LogHOLSimulation, Display, TEXT("Spawn: %d %s of slot %d around (%.0f, %.0f)"), Request.Count, *Request.TypeName.ToString(),
				Request.OwnerPlayer, Request.Center.X, Request.Center.Y);
		}));
}

bool UHOLUnitSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLUnitSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// A match map only (its start slots): the front end's map makes no unit, a stress test map builds its one type at
	// its first births, before its measure.
	if (AHOLStartSlot::WorldHasSlots(&InWorld))
	{
		PreloadTypes(InWorld);
	}
}

void UHOLUnitSubsystem::PreloadTypes(UWorld& InWorld)
{
	// Synchronous, while the map loads: the map itself is loaded synchronously around it, so this blocks no frame of the
	// match, and the templates (the archetypes) are built here too, which an asynchronous load would still leave to the
	// first birth. A client draws display entities only (D271); a dedicated server draws nothing, so FindTemplate leaves
	// the meshes there.
	const double StartSeconds = FPlatformTime::Seconds();
	const EHOLUnitTemplateKind Kind = InWorld.GetNetMode() == NM_Client ? EHOLUnitTemplateKind::Display : EHOLUnitTemplateKind::Simulation;
	TArray<FName> TypeNames;
	GetTypeNames(TypeNames);
	int32 NumBuilt = 0;
	for (const FName TypeName : TypeNames)
	{
		NumBuilt += FindTemplate(TypeName, Kind) != nullptr ? 1 : 0;
	}
	bTypesPreloaded = true;
	UE_LOG(LogHOLSimulation, Display, TEXT("Unit types preloaded: %d of %d (%s) as %s, meshes %s, in %.0f ms"), NumBuilt, TypeNames.Num(),
		*FString::JoinBy(TypeNames, TEXT(", "), [](const FName Name) { return Name.ToString(); }),
		Kind == EHOLUnitTemplateKind::Display ? TEXT("display entities") : TEXT("simulated units"),
		InWorld.GetNetMode() == NM_DedicatedServer ? TEXT("left out (dedicated server)") : TEXT("loaded"),
		(FPlatformTime::Seconds() - StartSeconds) * 1000.);
}

void UHOLUnitSubsystem::Deinitialize()
{
	bTypesPreloaded = false;
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

	if (bTypesPreloaded)
	{
		// PreloadTypes() built every type of this world's kind: this one loads its assets in the middle of the match.
		UE_LOG(LogHOLSimulation, Warning, TEXT("Unit type '%s' built after the preload (%s): its assets load during the match"),
			*TypeName.ToString(), bDisplay ? TEXT("display entity") : TEXT("simulated unit"));
	}

	FHOLUnitTypeConstants Constants = Row->ToConstants(TypeName);

	// Instanced skeletal mesh (D232), drawn when the type has one and its animation provider loads; the static mesh
	// stays the fallback (AnimToTexture, D232). A dedicated server draws nothing (Mass' visualization trait adds nothing
	// there): it loads no mesh, only the animation provider, whose sequence lengths time the gestures.
	const bool bDrawn = !GetWorld()->IsNetMode(NM_DedicatedServer);
	USkeletalMesh* SkinnedMesh = bDrawn ? Cast<USkeletalMesh>(Constants.SkinnedMesh.TryLoad()) : nullptr;
	UAnimSequenceTransformProviderData* AnimationProvider = Cast<UAnimSequenceTransformProviderData>(Constants.AnimationProvider.TryLoad());
	const bool bSkinned = AnimationProvider && (bDrawn ? SkinnedMesh != nullptr : !Constants.SkinnedMesh.IsNull());
	if (bSkinned)
	{
		const int32 NumSequences = AnimationProvider->GetSequences().Num();
		if (!ensureMsgf(Constants.IdleSequence < NumSequences && Constants.WalkSequence < NumSequences,
			TEXT("Unit type '%s': idle/walk sequence %d/%d out of the %d sequences of %s"),
			*TypeName.ToString(), Constants.IdleSequence, Constants.WalkSequence, NumSequences, *Constants.AnimationProvider.ToString()))
		{
			Constants.IdleSequence = Constants.WalkSequence = 0;
		}
		// Gathering, carrying walks (J5c.1) and loaded idles (J5c.2): one out of the provider is dropped (idle or walk
		// played instead).
		const TPair<FHOLResourceSequences*, const TCHAR*> SequenceSets[] = { { &Constants.GatherSequences, TEXT("gathering") },
			{ &Constants.CarryWalkSequences, TEXT("carrying walk") }, { &Constants.CarryIdleSequences, TEXT("loaded idle") } };
		for (const TPair<FHOLResourceSequences*, const TCHAR*>& Set : SequenceSets)
		{
			for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
			{
				int32& Sequence = (*Set.Key)[Resource];
				if (!ensureMsgf(Sequence >= INDEX_NONE && Sequence < NumSequences,
					TEXT("Unit type '%s': %s sequence %d of %s out of the %d sequences of %s"), *TypeName.ToString(), Set.Value, Sequence,
					*StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Resource)), NumSequences, *Constants.AnimationProvider.ToString()))
				{
					Sequence = INDEX_NONE;
				}
			}
		}
		// Props (J5c.2): the material knows ids 1 to 8 (the bits of a vertex's mask); one outside is dropped (nothing shown).
		for (FHOLResourceProps* Props : { &Constants.GatherProps, &Constants.CarryProps })
		{
			for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
			{
				int32& Prop = (*Props)[Resource];
				if (!ensureMsgf(Prop >= 0 && Prop <= FHOLResourceProps::MaxId, TEXT("Unit type '%s': %s prop %d of %s out of 0-%d"), *TypeName.ToString(),
					Props == &Constants.GatherProps ? TEXT("gathering") : TEXT("carrying"), Prop,
					*StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Resource)), FHOLResourceProps::MaxId))
				{
					Prop = 0;
				}
			}
		}
		// Building (J6a.4): the same, the gesture or the prop out of range dropped (idle played, nothing shown).
		if (!ensureMsgf(Constants.BuildSequence >= INDEX_NONE && Constants.BuildSequence < NumSequences,
			TEXT("Unit type '%s': building sequence %d out of the %d sequences of %s"), *TypeName.ToString(), Constants.BuildSequence, NumSequences,
			*Constants.AnimationProvider.ToString()))
		{
			Constants.BuildSequence = INDEX_NONE;
		}
		// Fighting (J7b.5): the same, a strike or a fall out of range dropped (idle played).
		for (int32* Sequence : { &Constants.AttackSequence, &Constants.FallSequence })
		{
			if (!ensureMsgf(*Sequence >= INDEX_NONE && *Sequence < NumSequences, TEXT("Unit type '%s': %s sequence %d out of the %d sequences of %s"),
				*TypeName.ToString(), Sequence == &Constants.AttackSequence ? TEXT("attack") : TEXT("fall"), *Sequence, NumSequences,
				*Constants.AnimationProvider.ToString()))
			{
				*Sequence = INDEX_NONE;
			}
		}
		if (!ensureMsgf(Constants.BuildProp >= 0 && Constants.BuildProp <= FHOLResourceProps::MaxId, TEXT("Unit type '%s': building prop %d out of 0-%d"),
			*TypeName.ToString(), Constants.BuildProp, FHOLResourceProps::MaxId))
		{
			Constants.BuildProp = 0;
		}
		Constants.SequenceLengths.SetNum(NumSequences);
		for (int32 Sequence = 0; Sequence < NumSequences; ++Sequence)
		{
			Constants.SequenceLengths[Sequence] = AnimationProvider->GetSequencePlayLength(Sequence);
		}
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
	// Full speed in the same time whatever the speed: Mass' fixed 250 cm/s² took a 600 cm/s Peasant 2.4 s (the user's
	// trial of J4); a 300 cm/s type (the stress test's) keeps its 250.
	Movement.MaxAcceleration = Constants.Speed / UE::HeroesOfLiberty::Units::SecondsToFullSpeed;
	Movement.DefaultDesiredSpeed = Constants.Speed;
	Movement.DefaultDesiredSpeedVariance = 0.f;
	MovementTrait->SetMovement(Movement);

	// The type's obstacle detection distance: D220 kept it in the trait, it is data now.
	UHOLLightUnitAvoidanceTrait* AvoidanceTrait = NewObject<UHOLLightUnitAvoidanceTrait>(this);
	AvoidanceTrait->SetObstacleDetectionDistance(Constants.AvoidanceDistance);

	UMassMovableVisualizationTrait* VisualizationTrait = NewObject<UMassMovableVisualizationTrait>(this);
	const FTransform MeshOffset(FRotator(0.f, Constants.MeshYawOffset, 0.f));
	if (!bDrawn)
	{
		// No mesh on a dedicated server (above).
	}
	else if (bSkinned)
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
	const EMassRepresentationType InstanceRepresentation = !bDrawn ? EMassRepresentationType::None
		: bSkinned ? EMassRepresentationType::SkinnedMeshInstance : EMassRepresentationType::StaticMeshInstance;
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
		!bDrawn ? TEXT("nothing (dedicated server)")
			: bSkinned ? *FString::Printf(TEXT("skinned instances of %s"), *SkinnedMesh->GetName()) : TEXT("static instances"));

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

		Units.Add(UnitId, FHOLUnitRecord{Entity, Request.TypeName, Request.OwnerPlayer});
		EntityToId.Add(Entity, UnitId);
	}

	// Born with somewhere to go (a rally point): one order for the units of the request, applied at this step; a rally
	// point on a source still standing sends them to gather it (J5, D298).
	if (Request.bHasMoveTarget)
	{
		if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
		{
			TArray<uint32> BornIds;
			for (const FMassEntityHandle Entity : Created)
			{
				BornIds.Add(EntityToId.FindChecked(Entity));
			}
			const UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			// A rally point on a site not finished: they build it (the user's trial of J6a.4); a camp finished by then, they
			// gather the nearest source it accepts, as its builders (D312, J6b.1); another site finished (or cancelled) by
			// then, they walk to its ground point as to any other. Soldiers (who carry nothing: no worker, J7a.2) only walk there.
			const bool bWorker = Constants.CarryCapacity > 0;
			AHOLBuilding* Site = bWorker ? Request.BuildSite.Get() : nullptr;
			double CampGap = 0.;
			const int32 CampSource = Site && Site->IsFinished() && Sources ? UHOLBuildProcessor::FindCampSource(*Sources, *Site, &CampGap) : INDEX_NONE;
			if (Site && !Site->IsFinished())
			{
				FHOLBuildOrder Order;
				Order.UnitIds = BornIds;
				Order.Building = Site;
				Commands->IssueBuildOrder(Order);
			}
			else if (CampSource != INDEX_NONE)
			{
				Commands->IssueGatherOrder(BornIds, CampSource, Request.OwnerPlayer);
				const FVector Location = Site->GetActorLocation();
				UE_LOG(LogHOLSimulation, Display, TEXT("Rally: %d units of slot %d born for the finished camp %s at (%.0f, %.0f) sent to gather %s (source %d, %.1f m)"),
					BornIds.Num(), Request.OwnerPlayer, *Site->GetTypeName().ToString(), Location.X, Location.Y,
					*StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Sources->GetSources()[CampSource].Resource)), CampSource, CampGap / 100.);
			}
			else if (bWorker && Sources && Sources->GetSources().IsValidIndex(Request.GatherSource) && Sources->GetSources()[Request.GatherSource].IsStanding())
			{
				Commands->IssueGatherOrder(BornIds, Request.GatherSource);
			}
			else
			{
				Commands->IssueMoveOrder(BornIds, Request.MoveTarget);
			}
		}
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

void UHOLUnitSubsystem::MarkUnitDying(const uint32 UnitId)
{
	if (FHOLUnitRecord* Record = Units.Find(UnitId))
	{
		Record->bDying = true;
	}
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

namespace UE::HeroesOfLiberty::Units
{
	/** The view of one simulated unit; false when its entity has no transform or state. */
	bool MakeUnitView(const FMassEntityManager& EntityManager, const uint32 UnitId, const FName TypeName, const FMassEntityHandle Entity,
		const float Alpha, FHOLUnitView& OutView)
	{
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
		if (!Transform || !State)
		{
			return false;
		}
		OutView = FHOLUnitView();
		OutView.UnitId = UnitId;
		OutView.OwnerPlayer = State->OwnerPlayer;
		OutView.TypeName = TypeName;
		// Drawn between the last two fixed steps, as UHOLInterpolatedInstancesProcessor draws them.
		const FHOLPreviousTransformFragment* Previous = EntityManager.GetFragmentDataPtr<FHOLPreviousTransformFragment>(Entity);
		OutView.Location = Previous ? FMath::Lerp(Previous->Transform.GetLocation(), Transform->GetTransform().GetLocation(), Alpha)
			: Transform->GetTransform().GetLocation();
		OutView.Activity = State->Activity;
		OutView.GatherResource = State->GatherResource;
		// As the clients have it (J7b.4, UHOLUnitReplicationSubsystem): Dying at 0 health, Attacking while it strikes.
		const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
		if (State->CurrentHealth <= 0)
		{
			OutView.Activity = EHOLUnitActivity::Dying;
		}
		else if (Combat && Combat->bAttacking)
		{
			OutView.Activity = EHOLUnitActivity::Attacking;
			const FHOLUnitStateFragment* TargetState = EntityManager.IsEntityValid(Combat->Target)
				? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Combat->Target) : nullptr;
			OutView.TargetUnitId = TargetState ? TargetState->UnitId : 0;
		}
		if (const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity))
		{
			OutView.bCarrying = Gather->Load.Amount > 0;
			OutView.LoadResource = Gather->Load.Resource;
			OutView.LoadAmount = Gather->Load.Amount;
		}
		OutView.bLoadAmountKnown = true;
		if (const FHOLUnitTypeConstants* Constants = EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Entity))
		{
			OutView.Radius = Constants->Radius;
			OutView.MeshScale = Constants->MeshScale;
			OutView.CarryCapacity = Constants->CarryCapacity;
			OutView.bFullHealth = State->CurrentHealth >= Constants->MaxHealth;
			OutView.HealthLevel = OutView.bFullHealth ? FHOLNetUnitState::MaxHealthLevel
				: HOLUnitReplication::GetHealthLevel(State->CurrentHealth, Constants->MaxHealth);
			OutView.Health = FMath::Max(State->CurrentHealth, 0);
			OutView.MaxHealth = Constants->MaxHealth;
			OutView.bHealthExact = true;
		}
		return true;
	}
}

void UHOLUnitSubsystem::GetUnitViews(TArray<FHOLUnitView>& OutUnits) const
{
	const UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	const UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>();
	const float Alpha = Simulation ? Simulation->GetInterpolationAlpha() : 1.f;
	OutUnits.Reserve(OutUnits.Num() + Units.Num());
	FHOLUnitView View;
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		if (UE::HeroesOfLiberty::Units::MakeUnitView(EntityManager, Unit.Key, Unit.Value.TypeName, Unit.Value.Entity, Alpha, View))
		{
			OutUnits.Add(View);
		}
	}
}

bool UHOLUnitSubsystem::GetUnitView(const uint32 UnitId, FHOLUnitView& OutUnit) const
{
	const UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	const FHOLUnitRecord* Unit = Units.Find(UnitId);
	if (!EntitySubsystem || !Unit)
	{
		return false;
	}
	const UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>();
	return UE::HeroesOfLiberty::Units::MakeUnitView(EntitySubsystem->GetEntityManager(), UnitId, Unit->TypeName, Unit->Entity,
		Simulation ? Simulation->GetInterpolationAlpha() : 1.f, OutUnit);
}

FString UHOLUnitSubsystem::DescribeUnits(const int32 MaxUnits) const
{
	const UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return TEXT("no Mass");
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TArray<uint32> UnitIds;
	GetAllUnitIds(UnitIds);
	TArray<FString> Lines;
	for (int32 Index = 0; Index < FMath::Min(MaxUnits, UnitIds.Num()); ++Index)
	{
		const FHOLUnitRecord& Record = Units[UnitIds[Index]];
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Record.Entity);
		const FMassRepresentationLODFragment* LOD = EntityManager.GetFragmentDataPtr<FMassRepresentationLODFragment>(Record.Entity);
		const FMassRepresentationFragment* Representation = EntityManager.GetFragmentDataPtr<FMassRepresentationFragment>(Record.Entity);
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Record.Entity);
		const FVector Location = Transform ? Transform->GetTransform().GetLocation() : FVector::ZeroVector;
		Lines.Add(FString::Printf(TEXT("#%u %s slot %d at (%.0f, %.0f, %.0f) LOD %d visibility %d, representation %d, %s"), UnitIds[Index],
			*Record.TypeName.ToString(), Record.OwnerPlayer, Location.X, Location.Y, Location.Z,
			LOD ? static_cast<int32>(LOD->LOD) : -1, LOD ? static_cast<int32>(LOD->Visibility) : -1,
			Representation ? static_cast<int32>(Representation->CurrentRepresentation) : -1,
			State ? *StaticEnum<EHOLUnitActivity>()->GetNameStringByValue(static_cast<int64>(State->Activity)) : TEXT("no state")));
	}
	return FString::Printf(TEXT("%d units: %s"), Units.Num(), *FString::Join(Lines, TEXT("; ")));
}

void UHOLUnitSubsystem::GetPopulationByOwner(TArray<int32>& OutPopulation) const
{
	OutPopulation.Reset();
	// One table lookup per type, not per unit.
	TMap<FName, int32> PlacesByType;
	const auto Add = [this, &OutPopulation, &PlacesByType](const FName TypeName, const uint8 OwnerPlayer, const int32 Count)
	{
		int32* Places = PlacesByType.Find(TypeName);
		if (!Places)
		{
			const FHOLUnitTypeRow* Row = FindTypeRow(TypeName);
			Places = &PlacesByType.Add(TypeName, Row ? Row->Population : 0);
		}
		if (OutPopulation.Num() <= OwnerPlayer)
		{
			OutPopulation.SetNumZeroed(OwnerPlayer + 1);
		}
		OutPopulation[OwnerPlayer] += *Places * Count;
	};
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		// A dying unit's places are free at its death (the user's trial of J7b), its destruction ~1 s later.
		Add(Unit.Value.TypeName, Unit.Value.OwnerPlayer, Unit.Value.bDying ? 0 : 1);
	}
	for (const FHOLUnitSpawnRequest& Request : BirthQueue)
	{
		Add(Request.TypeName, Request.OwnerPlayer, Request.Count);
	}
}


