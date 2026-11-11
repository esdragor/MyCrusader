// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitSubsystem.h"

#include "Match/HOLFactionRules.h"
#include "Match/HOLMatchStatsSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLUnitAbilityRules.h"
#include "Siege/HOLSiegeRules.h"
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
#include "Simulation/HOLDeployProcessor.h"
#include "Simulation/HOLLightUnitAvoidanceTrait.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitView.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitMovementTrait.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLUnitTrait.h"
#include "Steering/MassSteeringTrait.h"
#include "Match/HOLGameState.h"
#include "Presentation/HOLCorpse.h"
#include "Presentation/HOLCorpseTrait.h"
#include "StructUtils/StructUtilsTypes.h"

namespace UE::HeroesOfLiberty::Units
{
	/** Stress test knob (D224): overrides the settings' budget when not 0, negative meaning "no budget at all". */
	int32 SpawnBudgetOverride = 0;
	FAutoConsoleVariableRef CVarSpawnBudget(TEXT("hol.Units.SpawnBudget"), SpawnBudgetOverride,
		TEXT("Units created per step at most, 0 = UHOLSimulationSettings value, -1 = the whole queue in one step"));

	/** Draws of a birth position before giving up on finding the navmesh under it. */
	constexpr int32 MaxSpawnProjectionAttempts = 4;
	/** After the draws, the nearest navmesh within this horizontal half-size (cm): a unit born in an obstacle, a boulder
	 * of the map's ridges (no navmesh on top of them, MV.4: up to 8 x 5 m), is put on the ground beside it. */
	constexpr float SpawnFallbackHalfExtent = 800.f;
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

	/** A test scenario's births laid out independently of the units born before it (one stream feeds every birth). */
	FAutoConsoleCommandWithWorldAndArgs CommandSpawnSeed(TEXT("hol.Units.SpawnSeed"),
		TEXT("hol.Units.SpawnSeed Seed: restarts the draw of birth positions from Seed, server or solo; a test command"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLUnitSubsystem* Units = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
			if (!Units || Args.Num() < 1)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Units.SpawnSeed Seed, on the server or in solo"));
				return;
			}
			const int32 Seed = FCString::Atoi(*Args[0]);
			Units->ResetSpawnSeed(Seed);
			UE_LOG(LogHOLSimulation, Display, TEXT("Spawn seed: %d"), Seed);
		}));

	/** The other way (the user's trial of J7b): a scenario's units taken away at its end, so they weigh on no later part. */
	FAutoConsoleCommandWithWorldAndArgs CommandRemove(TEXT("hol.Units.Remove"),
		TEXT("hol.Units.Remove X Y Radius [Slot=0] [Type]: destroys the units of a slot (of one type: all if none) within Radius (cm) of a ground point at the end of the step, server or solo; a test command"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLUnitSubsystem* Units = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
			if (!Units || Args.Num() < 3)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Units.Remove X Y Radius [Slot=0], on the server or in solo"));
				return;
			}
			const FVector2D Center(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]));
			const double Radius = FCString::Atod(*Args[2]);
			const uint8 Slot = static_cast<uint8>(Args.Num() >= 4 ? FCString::Atoi(*Args[3]) : 0);
			// One type only (the part "rocks" of quick_check: its soldiers, never the centre's Peasants the parts after count).
			const FName Type = Args.Num() >= 5 ? FName(*Args[4]) : NAME_None;
			TArray<FHOLUnitView> Views;
			Units->GetUnitViews(Views);
			TArray<uint32> Removed;
			for (const FHOLUnitView& View : Views)
			{
				if (View.OwnerPlayer == Slot && (Type.IsNone() || View.TypeName == Type) && FVector2D::DistSquared(FVector2D(View.Location), Center) <= Radius * Radius)
				{
					Removed.Add(View.UnitId);
				}
			}
			Units->RequestDestroyUnits(Removed);
			UE_LOG(LogHOLSimulation, Display, TEXT("Remove: %d units of slot %d within %.0f of (%.0f, %.0f)%s"), Removed.Num(), Slot, Radius, Center.X, Center.Y,
				Type.IsNone() ? TEXT("") : *FString::Printf(TEXT(", %s only"), *Type.ToString()));
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
	// Every machine that draws (client, solo, listen server) lays bodies too (J7c, bodies kept 30 s); a dedicated server none.
	const bool bCorpses = !InWorld.IsNetMode(NM_DedicatedServer);
	int32 NumBuilt = 0;
	for (const FName TypeName : TypeNames)
	{
		NumBuilt += FindTemplate(TypeName, Kind) != nullptr ? 1 : 0;
		if (bCorpses)
		{
			FindTemplate(TypeName, EHOLUnitTemplateKind::Corpse);
		}
	}
	// L2 (client model): the factions' own models too, while the map loads.
	PreloadFactionTemplates(Kind);
	bTypesPreloaded = true;
	UE_LOG(LogHOLSimulation, Display, TEXT("Unit types preloaded: %d of %d (%s) as %s%s, meshes %s, in %.0f ms"), NumBuilt, TypeNames.Num(),
		*FString::JoinBy(TypeNames, TEXT(", "), [](const FName Name) { return Name.ToString(); }),
		Kind == EHOLUnitTemplateKind::Display ? TEXT("display entities") : TEXT("simulated units"),
		bCorpses ? TEXT(" and bodies") : TEXT(""),
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
	CorpseTemplateCache.Reset();
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
		else
		{
			// J7-Q5 revised: a wounded unit sends its exact health in FHOLNetUnitState::HealthBits; a larger maximum is sent as a
			// share of it (HOLUnitReplication::GetWireHealthShare, HOLUnitReplication::UsesHealthShare): the hero (J8-Q5, J8a.3,
			// whose exact health comes by the hero sheet) ; the siege engines (the Ram: 400) fit the 9 bits. Nothing to check.
		}
	}
	return CachedTypeTable;
}

const FHOLUnitTypeRow* UHOLUnitSubsystem::FindTypeRow(const FName TypeName) const
{
	const UDataTable* Table = GetTypeTable();
	return Table ? Table->FindRow<FHOLUnitTypeRow>(TypeName, TEXT("UnitSubsystem"), /*bWarnIfRowMissing*/ false) : nullptr;
}

const FHOLUnitTypeRow* UHOLUnitSubsystem::FindTypeRow(const FName TypeName, const FName FactionKey) const
{
	const FHOLUnitTypeRow* Base = FindTypeRow(TypeName);
	if (!Base)
	{
		return nullptr;
	}
	const HOLFactionRules::FTable& Table = HOLFactionRules::GetTable();
	const FName Key = Table.ResolveKey(FactionKey);
	if (!Table.HasOverride(Key, TypeName))
	{
		return Base;
	}
	if (FactionRowsVersion != HOLFactionRules::GetTableVersion())
	{
		FactionRows.Reset();
		FactionRowsVersion = HOLFactionRules::GetTableVersion();
	}
	TSharedPtr<FHOLUnitTypeRow>& Cached = FactionRows.FindOrAdd(Key).FindOrAdd(TypeName);
	if (!Cached.IsValid())
	{
		Cached = MakeShared<FHOLUnitTypeRow>(*Base);
		TArray<FString> Problems;
		Table.ApplyOverrides(Key, TypeName, FHOLUnitTypeRow::StaticStruct(), Cached.Get(), &Problems);
		for (const FString& Problem : Problems)
		{
			UE_LOG(LogHOLSimulation, Warning, TEXT("Faction override ignored: %s"), *Problem);
		}
	}
	return Cached.Get();
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

const FHOLUnitTypeRow* UHOLUnitSubsystem::FindTypeRowFor(const FName TypeName, const int32 OwnerPlayer) const
{
	return FindTypeRow(TypeName, HOLFactionRules::GetSlotFactionKey(GetWorld(), OwnerPlayer));
}

FName UHOLUnitSubsystem::GetTemplateKey(const FName TypeName, const FName FactionKey) const
{
	if (FactionKey.IsNone())
	{
		return TypeName;
	}
	const FHOLUnitTypeRow* Base = FindTypeRow(TypeName);
	const FHOLUnitTypeRow* ForFaction = FindTypeRow(TypeName, FactionKey);
	if (!Base || ForFaction == Base)
	{
		return TypeName;
	}
	const auto Crc = [TypeName](const FHOLUnitTypeRow& Row)
	{
		return UE::StructUtils::GetStructInstanceCrc32(FConstStructView::Make(Row.ToConstants(TypeName)));
	};
	if (Crc(*Base) == Crc(*ForFaction))
	{
		return TypeName;
	}
	return FName(*FString::Printf(TEXT("%s@%s"), *TypeName.ToString(), *HOLFactionRules::GetTable().ResolveKey(FactionKey).ToString()));
}

bool UHOLUnitSubsystem::HasTemplate(const FName TypeName, const EHOLUnitTemplateKind Kind, const FName FactionKey) const
{
	const TMap<FName, TSharedPtr<FMassEntityConfig>>& Cache = Kind == EHOLUnitTemplateKind::Display ? DisplayTemplateCache
		: Kind == EHOLUnitTemplateKind::Corpse ? CorpseTemplateCache : TemplateCache;
	return Cache.Contains(GetTemplateKey(TypeName, FactionKey));
}

void UHOLUnitSubsystem::PreloadFactionTemplates(const EHOLUnitTemplateKind Kind)
{
	// Every (faction, type) the overrides name: FindTemplate builds the faction's own template when its row builds other constants
	// than the table's (GetTemplateKey), and answers the type's own, already built, when it does not (a name or a cost override).
	// L13: a machine that draws lays bodies (J7c), so each faction's body is built with its model; a dedicated server none.
	const double StartSeconds = FPlatformTime::Seconds();
	const bool bCorpses = !GetWorld()->IsNetMode(NM_DedicatedServer);
	int32 NumBuilt = 0;
	const HOLFactionRules::FTable& Table = HOLFactionRules::GetTable();
	for (const HOLFactionRules::FOverrideRow& Override : Table.Overrides)
	{
		if (!Override.Faction.IsNone() && !Override.Type.IsNone() && FindTypeRow(Override.Type))
		{
			NumBuilt += FindTemplate(Override.Type, Kind, Override.Faction) != nullptr ? 1 : 0;
			if (bCorpses)
			{
				FindTemplate(Override.Type, EHOLUnitTemplateKind::Corpse, Override.Faction);
			}
		}
	}
	// The measure of the faction preload (L13 reste: "valeur non mesuree"): rows visited, and the time it took.
	UE_LOG(LogHOLSimulation, Display, TEXT("Faction templates preloaded: %d override rows visited%s, %d faction models built in all, in %.0f ms"),
		NumBuilt, bCorpses ? TEXT(" (bodies too)") : TEXT(""), DisplayTemplateCache.Num() + TemplateCache.Num() + CorpseTemplateCache.Num(),
		(FPlatformTime::Seconds() - StartSeconds) * 1000.);
}

const FMassEntityTemplate* UHOLUnitSubsystem::FindTemplate(const FName TypeName, const EHOLUnitTemplateKind Kind, const FName FactionKey)
{
	const bool bDisplay = Kind == EHOLUnitTemplateKind::Display;
	const bool bCorpse = Kind == EHOLUnitTemplateKind::Corpse;
	TMap<FName, TSharedPtr<FMassEntityConfig>>& Cache = bDisplay ? DisplayTemplateCache : bCorpse ? CorpseTemplateCache : TemplateCache;
	const TCHAR* KindName = bDisplay ? TEXT("display entity") : bCorpse ? TEXT("body") : TEXT("simulated unit");
	// L2: a faction whose row builds other constants than the table's has a template of its own (health, attack, mesh at birth).
	const FName CacheName = GetTemplateKey(TypeName, FactionKey);
	if (const TSharedPtr<FMassEntityConfig>* Found = Cache.Find(CacheName))
	{
		return &(*Found)->GetOrCreateEntityTemplate(*GetWorld());
	}

	const FHOLUnitTypeRow* Row = FindTypeRow(TypeName, CacheName == TypeName ? NAME_None : FactionKey);
	if (Row == nullptr)
	{
		UE_LOG(LogHOLSimulation, Error, TEXT("Unknown unit type '%s': no row of that name in the type table (D221)"), *TypeName.ToString());
		return nullptr;
	}

	if (bTypesPreloaded)
	{
		// PreloadTypes() built every type of this world's kind: this one loads its assets in the middle of the match.
		UE_LOG(LogHOLSimulation, Warning, TEXT("Unit type '%s' built after the preload (%s): its assets load during the match"),
			*TypeName.ToString(), KindName);
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
		if (!ensureMsgf(Constants.AttackProp >= 0 && Constants.AttackProp <= FHOLResourceProps::MaxId, TEXT("Unit type '%s': attack prop %d out of 0-%d"),
			*TypeName.ToString(), Constants.AttackProp, FHOLResourceProps::MaxId))
		{
			Constants.AttackProp = 0;
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
	// A body (J7c, bodies kept 30 s) is drawn like the unit too, by the same visualization trait (same mesh, provider and
	// offset, so the same instanced mesh), with nothing of the simulation: no unit trait, so no FHOLLightUnitTag.
	const TArray<UMassEntityTraitBase*> TypeTraits = bCorpse
		? TArray<UMassEntityTraitBase*>{ NewObject<UHOLCorpseTrait>(this),
			NewObject<UMassLODCollectorTrait>(this),
			VisualizationTrait }
		: bDisplay
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

	Cache.Add(CacheName, Config);
	UE_LOG(LogHOLSimulation, Log, TEXT("Unit type '%s' (%s%s): radius %.0f cm, speed %.0f cm/s, %d HP, avoidance %.0f cm, drawn as %s"),
		*TypeName.ToString(), *Row->DisplayName, Kind == EHOLUnitTemplateKind::Simulation ? TEXT("") : *FString::Printf(TEXT(", %s"), KindName), Constants.Radius, Constants.Speed, Constants.MaxHealth, Constants.AvoidanceDistance,
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
	// D358: a defeated slot has nothing born any more.
	if (const AHOLGameState* GameState = World->GetGameState<AHOLGameState>(); GameState && GameState->IsSlotDefeated(Request.OwnerPlayer))
	{
		UE_LOG(LogHOLSimulation, Verbose, TEXT("Birth of %d '%s' of defeated slot %d refused (D358)"), Request.Count, *Request.TypeName.ToString(), Request.OwnerPlayer);
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
		// L2: born with the figures of its owner's faction (the table's own row for the Dawn and for a type the faction leaves alone).
		const FMassEntityTemplate* Template = FindTemplate(Request.TypeName, EHOLUnitTemplateKind::Simulation, HOLFactionRules::GetSlotFactionKey(GetWorld(), Request.OwnerPlayer));
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

/**
 * A rally point in several steps (EtapeDefini N.1, AoE4): the units just born get the queue a player's Shift orders would
 * give them, each step in order through the queue of moves (L.6, M.1): a ground step a move (the first one at once, the
 * others with Shift), a source a Shift gather, a site not finished a Shift build, a camp finished the nearest source it
 * accepts; soldiers walk to each step's point. Under the caller's FRallyOrderScope (the chain drawn, D328).
 */
static void HOLIssueSpawnRallySteps(UHOLCommandSubsystem& Commands, const UWorld& World, const FHOLUnitSpawnRequest& Request,
	const TConstArrayView<uint32> BornIds, const bool bWorker)
{
	const UHOLSourceSubsystem* Sources = World.GetSubsystem<UHOLSourceSubsystem>();
	TArray<FHOLUnitSpawnWaypoint> Steps = Request.Waypoints;
	FHOLUnitSpawnWaypoint& Last = Steps.AddDefaulted_GetRef();
	Last.Point = Request.MoveTarget;
	Last.GatherSource = Request.GatherSource;
	Last.BuildSite = Request.BuildSite;
	for (int32 StepIndex = 0; StepIndex < Steps.Num(); ++StepIndex)
	{
		const FHOLUnitSpawnWaypoint& Step = Steps[StepIndex];
		AHOLBuilding* Site = bWorker ? Step.BuildSite.Get() : nullptr;
		const int32 CampSource = Site && !Site->NeedsBuilders() && Sources ? UHOLBuildProcessor::FindCampSource(*Sources, *Site) : INDEX_NONE;
		const bool bSource = bWorker && Sources && Sources->GetSources().IsValidIndex(Step.GatherSource) && Sources->GetSources()[Step.GatherSource].IsStanding();
		if (Site && Site->NeedsBuilders())
		{
			Commands.IssueQueuedWorkOrder(BornIds, INDEX_NONE, Site, Site->GetActorLocation());
		}
		else if (CampSource != INDEX_NONE)
		{
			Commands.IssueQueuedWorkOrder(BornIds, CampSource, nullptr, Sources->GetSources()[CampSource].Location);
		}
		else if (bSource)
		{
			Commands.IssueQueuedWorkOrder(BornIds, Step.GatherSource, nullptr, Sources->GetSources()[Step.GatherSource].Location);
		}
		else
		{
			Commands.IssueMoveOrder(BornIds, Step.Point, INDEX_NONE, /*bFromTask*/ false, FVector2D::ZeroVector, /*bFrontWidth*/ false, /*bQueue*/ StepIndex > 0);
		}
	}
	UE_LOG(LogHOLSimulation, Display, TEXT("Rally: %d units of slot %d born with a rally point in %d steps"), BornIds.Num(), Request.OwnerPlayer, Steps.Num());
}

void UHOLUnitSubsystem::CreateUnitEntities(const FHOLUnitSpawnRequest& Request, const FMassEntityTemplate& Template, const int32 Count)
{
	UWorld* World = GetWorld();
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	const FHOLUnitTypeRow* Row = FindTypeRowFor(Request.TypeName, Request.OwnerPlayer);
	if (EntitySubsystem == nullptr || Row == nullptr)
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	const FHOLUnitTypeConstants Constants = Row->ToConstants(Request.TypeName);

	int32 NumProjectionFailures = 0;
	int32 NumProjectionFallbacks = 0;
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
			if (!bProjected)
			{
				// Every draw off the navmesh (inside a boulder): the nearest navigable ground point, rather than a unit
				// standing in the obstacle with no way out.
				FNavLocation Projected;
				const float HalfExtent = UE::HeroesOfLiberty::Units::SpawnFallbackHalfExtent;
				if (NavSystem->ProjectPointToNavigation(Request.Center, Projected, FVector(HalfExtent, HalfExtent, 500.f)))
				{
					Location = Projected.Location;
					bProjected = true;
					++NumProjectionFallbacks;
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
		// S5: a Ram born after Reinforced Ram has the health the technology gives (HOLCombat::GetMaxHealth with the slot's percent).
		const int32 BornHealthPercent = GetTechnologyHealthPercent(Constants, Request.OwnerPlayer);
		State.CurrentHealth = BornHealthPercent > 0 ? static_cast<float>(HOLCombat::GetMaxHealth(Constants, 1, BornHealthPercent)) : static_cast<float>(Constants.MaxHealth);
		if (BornHealthPercent > 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Unit #%u %s of slot %d born with %d health (+%d %%)"), UnitId, *Constants.TypeName.ToString(),
				static_cast<int32>(Request.OwnerPlayer), FMath::RoundToInt(State.CurrentHealth), BornHealthPercent);
		}
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
		if (HOLIsHero(Constants.ArmorCategoryMask))
		{
			HeroUnitBySlot.Add(Request.OwnerPlayer, UnitId);
		}
		// Palier3InProgress.md G6 b: the Mangonel stands in for the deployable engine behind hol.Siege.Deployable (Q-palier3-deploy-wiring.md Q1 = A):
		// born packed (phase 0) with the deploy fragment. Last in the body: adding a fragment moves the entity to another archetype, so the
		// references taken above would dangle.
		if (Request.TypeName == FName(TEXT("Mangonel")) && HOLDeployStandInEnabled())
		{
			EntityManager.AddFragmentToEntity(Entity, FHOLDeployFragment::StaticStruct());
		}
	}

	// Born with somewhere to go (a rally point): one order for the units of the request, applied at this step; a rally
	// point on a source still standing sends them to gather it (J5, D298).
	if (Request.bHasMoveTarget)
	{
		if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
		{
			// Their chain is drawn (D328, the user's trial): one white step to where they go, whatever the order.
			const UHOLCommandSubsystem::FRallyOrderScope RallyOrder(*Commands);
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
			if (!Request.Waypoints.IsEmpty())
			{
				HOLIssueSpawnRallySteps(*Commands, *World, Request, BornIds, bWorker);
			}
			else
			{
				AHOLBuilding* Site = bWorker ? Request.BuildSite.Get() : nullptr;
				double CampGap = 0.;
				// Its main forum down: they rebuild it, as a site (V2.3a).
				const int32 CampSource = Site && !Site->NeedsBuilders() && Sources ? UHOLBuildProcessor::FindCampSource(*Sources, *Site, &CampGap) : INDEX_NONE;
				if (Site && Site->NeedsBuilders())
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
	}

	if (NumProjectionFallbacks > 0)
	{
		UE_LOG(LogHOLSimulation, Log, TEXT("%d of %d '%s' units found no navmesh under their birth position after %d draws: born on the nearest navigable ground"),
			NumProjectionFallbacks, Created.Num(), *Request.TypeName.ToString(), UE::HeroesOfLiberty::Units::MaxSpawnProjectionAttempts);
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

void UHOLUnitSubsystem::MarkUnitDying(const uint32 UnitId, const int32 KillerPlayer)
{
	FHOLUnitRecord* Record = Units.Find(UnitId);
	if (Record && !Record->bDying)
	{
		Record->bDying = true;
		if (UHOLMatchStatsSubsystem* MatchStats = GetWorld() ? GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
		{
			MatchStats->RecordUnitDeath(Record->OwnerPlayer, IsWorkerType(Record->TypeName), KillerPlayer);
		}
	}
}

int32 UHOLUnitSubsystem::KillUnitsOfOwner(const uint8 Owner, const FName TypeName)
{
	if (TypeName.IsNone())
	{
		BirthQueue.RemoveAll([Owner](const FHOLUnitSpawnRequest& Request) { return Request.OwnerPlayer == Owner; });
	}
	UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem)
	{
		return 0;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	int32 Killed = 0;
	for (TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		FHOLUnitRecord& Record = Unit.Value;
		FHOLUnitStateFragment* State = Record.OwnerPlayer == Owner && !Record.bDying && (TypeName.IsNone() || Record.TypeName == TypeName)
			? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Record.Entity) : nullptr;
		if (State && State->CurrentHealth > 0)
		{
			// At 0 health "by another way" (UHOLCombatProcessor::FinishDying): the same dying path as a unit killed in combat.
			State->CurrentHealth = 0;
			State->Activity = EHOLUnitActivity::Dying;
			Record.bDying = true;
			++Killed;
			if (UHOLMatchStatsSubsystem* MatchStats = GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>())
			{
				MatchStats->RecordUnitDeath(Owner, IsWorkerType(Record.TypeName), INDEX_NONE);
			}
		}
	}
	return Killed;
}

int32 UHOLUnitSubsystem::KillUnitsWhere(const TFunctionRef<bool(uint32 UnitId, FName TypeName, uint8 Owner, const FVector& Location, float Radius)> Doomed)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem)
	{
		return 0;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	int32 Killed = 0;
	for (TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		FHOLUnitRecord& Record = Unit.Value;
		if (Record.bDying)
		{
			continue;
		}
		FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Record.Entity);
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Record.Entity);
		if (!State || !Transform || State->CurrentHealth <= 0 || State->Activity == EHOLUnitActivity::Dying)
		{
			continue;
		}
		const FHOLUnitTypeRow* Row = FindTypeRow(Record.TypeName);
		if (Doomed(Unit.Key, Record.TypeName, Record.OwnerPlayer, Transform->GetTransform().GetLocation(), Row ? Row->Radius : 0.f))
		{
			State->CurrentHealth = 0;
			State->Activity = EHOLUnitActivity::Dying;
			Record.bDying = true;
			++Killed;
			if (UHOLMatchStatsSubsystem* MatchStats = GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>())
			{
				MatchStats->RecordUnitDeath(Record.OwnerPlayer, IsWorkerType(Record.TypeName), INDEX_NONE);
			}
		}
	}
	return Killed;
}

int32 UHOLUnitSubsystem::ForEachLivingUnit(const TFunctionRef<void(uint8 Owner, const FVector& Location)> Visit) const
{
	return ForEachLivingUnitWithType([&Visit](uint32, FName, const uint8 Owner, const FVector& Location) { Visit(Owner, Location); });
}

int32 UHOLUnitSubsystem::ForEachLivingUnitWithType(const TFunctionRef<void(uint32 UnitId, FName TypeName, uint8 Owner, const FVector& Location)> Visit) const
{
	const UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem)
	{
		return 0;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		const FHOLUnitRecord& Record = Unit.Value;
		if (Record.bDying)
		{
			continue;
		}
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Record.Entity);
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Record.Entity);
		if (State && Transform && State->CurrentHealth > 0 && State->Activity != EHOLUnitActivity::Dying)
		{
			Visit(Unit.Key, Record.TypeName, Record.OwnerPlayer, Transform->GetTransform().GetLocation());
		}
	}
	return Units.Num();
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

	// The bodies a displaying machine keeps (J7c, bodies kept 30 s): solo and host read them off the dying units before they
	// go, and lay them once gone; never a dedicated server (nothing drawn). A unit removed alive (stress test, end of the
	// match) is neither Dying nor at 0 health: it leaves none.
	TArray<FHOLCorpseSeed> CorpseSeeds;
	const bool bCorpses = !World->IsNetMode(NM_DedicatedServer);
	if (bCorpses)
	{
		TArray<FMassEntityHandle> Dying;
		Dying.Reserve(PendingDestruction.Num());
		for (const uint32 UnitId : PendingDestruction)
		{
			if (const FHOLUnitRecord* Record = Units.Find(UnitId))
			{
				Dying.Add(Record->Entity);
			}
		}
		HOLCorpse::CollectSeeds(EntityManager, Dying, World->GetTimeSeconds(), CorpseSeeds);
	}

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
		if (const uint32* Hero = HeroUnitBySlot.Find(Record->OwnerPlayer); Hero && *Hero == UnitId)
		{
			HeroUnitBySlot.Remove(Record->OwnerPlayer);
		}
		Units.Remove(UnitId);
		CommandBuffer.DestroyEntity(Record->Entity);
		++NumDestroyed;
	}
	PendingDestruction.Reset();
	EntityManager.FlushCommands();
	if (CorpseSeeds.Num() > 0)
	{
		HOLCorpse::Lay(EntityManager, *this, CorpseSeeds);
	}

	return NumDestroyed;
}

bool UHOLUnitSubsystem::CastRage(const uint32 UnitId)
{
	const FHOLUnitRecord* Record = Units.Find(UnitId);
	UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!Record || Record->bDying || Record->TypeName != HOLUnitAbility::RageUnitType || !EntitySubsystem
		|| GetWorld()->GetNetMode() == NM_Client)
	{
		return false;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Record->Entity);
	FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Record->Entity);
	if (!State || !Combat || Combat->DeathStep >= 0
		|| !HOLUnitAbility::CanCastRage(State->CurrentHealth > 0.f, State->CurrentHealth, Combat->RageCooldownMs))
	{
		return false;
	}
	Combat->RageMs = HOLUnitAbility::RageDurationMs;
	Combat->RageCooldownMs = HOLUnitAbility::RageCooldownMs;
	UE_LOG(LogHOLSimulation, Display, TEXT("Unit ability: #%u %s casts the necrotic rage (%d health)"), UnitId, *Record->TypeName.ToString(), FMath::CeilToInt(State->CurrentHealth));
	return true;
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

uint32 UHOLUnitSubsystem::FindHeroUnit(const int32 Slot) const
{
	const uint32* UnitId = Slot >= 0 && Slot <= MAX_uint8 ? HeroUnitBySlot.Find(static_cast<uint8>(Slot)) : nullptr;
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
		const float Alpha, FHOLUnitView& OutView, const UHOLUnitSubsystem* Subsystem = nullptr)
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
			OutView.bRanged = HOLCombat::IsRangedAttack(Constants->AttackRange);
			OutView.bSiege = Constants->HasArmorCategory(EHOLArmorCategory::Siege);
			OutView.bKing = Constants->HasArmorCategory(EHOLArmorCategory::King);
			OutView.bHero = Constants->HasArmorCategory(EHOLArmorCategory::Hero);
			// J8b.2: the maximum at its level (the hero's grows).
			// S5: and the technologies' share of it (the Ram's).
			const int32 MaxHealth = HOLCombat::GetMaxHealth(*Constants, Combat ? Combat->Level : 1,
				Subsystem ? Subsystem->GetTechnologyHealthPercent(*Constants, State->OwnerPlayer) : 0);
			OutView.bFullHealth = State->CurrentHealth >= static_cast<float>(MaxHealth);
			OutView.Health = HOLCombat::GetShownHealth(State->CurrentHealth);
			OutView.MaxHealth = MaxHealth;
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
		if (UE::HeroesOfLiberty::Units::MakeUnitView(EntityManager, Unit.Key, Unit.Value.TypeName, Unit.Value.Entity, Alpha, View, this))
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
		Simulation ? Simulation->GetInterpolationAlpha() : 1.f, OutUnit, this);
}

int32 UHOLUnitSubsystem::GetTechnologyHealthPercent(const FHOLUnitTypeConstants& Constants, const int32 Slot) const
{
	if (!TechnologyHealthPercent.IsValidIndex(Slot) || HOLSiege::GetHealthStat(Constants.TypeName).IsNone())
	{
		return 0;
	}
	return TechnologyHealthPercent[Slot];
}

int32 UHOLUnitSubsystem::ApplyTechnologyHealth(const int32 Slot, const int32 Before, const int32 After)
{
	if (Slot < 0 || Slot >= 64)
	{
		return 0;
	}
	if (TechnologyHealthPercent.Num() <= Slot)
	{
		TechnologyHealthPercent.SetNumZeroed(Slot + 1);
	}
	TechnologyHealthPercent[Slot] = After;
	UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem || After == Before)
	{
		return 0;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	int32 Raised = 0;
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		const FHOLUnitRecord& Record = Unit.Value;
		if (Record.bDying || !EntityManager.IsEntityValid(Record.Entity) || HOLSiege::GetHealthStat(Record.TypeName).IsNone())
		{
			continue;
		}
		FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Record.Entity);
		const FHOLUnitTypeConstants* Constants = EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Record.Entity);
		if (!State || !Constants || State->OwnerPlayer != Slot || State->CurrentHealth <= 0.f)
		{
			continue;
		}
		const int32 Delta = HOLCombat::GetMaxHealth(*Constants, 1, After) - HOLCombat::GetMaxHealth(*Constants, 1, Before);
		State->CurrentHealth += static_cast<float>(Delta);
		++Raised;
	}
	return Raised;
}

int32 UHOLUnitSubsystem::WoundUnits(const int32 Slot, const int32 Percent)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem)
	{
		return 0;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	int32 Wounded = 0;
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		const FHOLUnitRecord& Record = Unit.Value;
		if (Record.bDying || !EntityManager.IsEntityValid(Record.Entity))
		{
			continue;
		}
		FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Record.Entity);
		const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Record.Entity);
		const FHOLUnitTypeConstants* Constants = EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Record.Entity);
		if (!State || !Combat || !Constants || State->OwnerPlayer != Slot || State->CurrentHealth <= 0.f || HOLIsKing(Constants->ArmorCategoryMask))
		{
			continue;
		}
		const int32 Maximum = HOLCombat::GetMaxHealth(*Constants, Combat->Level, GetTechnologyHealthPercent(*Constants, Slot));
		State->CurrentHealth = FMath::Max(1.f, static_cast<float>(Maximum * FMath::Clamp(Percent, 1, 100)) / 100.f);
		++Wounded;
	}
	return Wounded;
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

bool UHOLUnitSubsystem::IsWorkerType(const FName TypeName) const
{
	const FHOLUnitTypeRow* Row = FindTypeRow(TypeName);
	return Row && Row->CarryCapacity > 0;
}

void UHOLUnitSubsystem::GetLivingCountsByOwner(TArray<int32>& OutWorkers, TArray<int32>& OutMilitary) const
{
	OutWorkers.Reset();
	OutMilitary.Reset();
	// One table lookup per type, not per unit.
	TMap<FName, bool> WorkerByType;
	for (const TPair<uint32, FHOLUnitRecord>& Unit : Units)
	{
		const FHOLUnitRecord& Record = Unit.Value;
		if (Record.bDying)
		{
			continue;
		}
		const bool* bWorker = WorkerByType.Find(Record.TypeName);
		if (!bWorker)
		{
			bWorker = &WorkerByType.Add(Record.TypeName, IsWorkerType(Record.TypeName));
		}
		if (OutWorkers.Num() <= Record.OwnerPlayer)
		{
			OutWorkers.SetNumZeroed(Record.OwnerPlayer + 1);
			OutMilitary.SetNumZeroed(Record.OwnerPlayer + 1);
		}
		++(*bWorker ? OutWorkers : OutMilitary)[Record.OwnerPlayer];
	}
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


