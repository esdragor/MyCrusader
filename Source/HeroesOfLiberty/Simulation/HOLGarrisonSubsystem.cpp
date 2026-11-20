// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGarrisonSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypeLibrary.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLGarrisonRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "MassCommonFragments.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassEntitySubsystem.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLGarrisonBoard.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

bool UHOLGarrisonSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLGarrisonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UHOLCommandSubsystem>();
	Collection.InitializeDependency<UHOLUnitSubsystem>();
	if (UHOLSimulationSubsystem* Simulation = Collection.InitializeDependency<UHOLSimulationSubsystem>())
	{
		StepStartedHandle = Simulation->GetOnStepStarted().AddUObject(this, &UHOLGarrisonSubsystem::OnStepStarted);
	}
}

void UHOLGarrisonSubsystem::Deinitialize()
{
	UWorld* World = GetWorld();
	if (UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr)
	{
		Simulation->GetOnStepStarted().Remove(StepStartedHandle);
	}
	StepStartedHandle.Reset();
	Super::Deinitialize();
}

void UHOLGarrisonSubsystem::OnStepStarted(const int64 StepIndex)
{
	ApplyOrders();
}

// D850 (proof of the Garrison / UngarrisonAll order RPCs, no game data changes): BuildingTypes.csv has GarrisonSlots 0 for every building, so no order could ever
// be proved with an effect. This cvar gives every building that HAS a row at least that many places; 0 (default) = the table alone. Set on the server AND on the
// client (the client's hol.Units.Garrison reads the same places) by Tools/Scenarios/rpc_orders_p3p4_run.ps1 only.
static TAutoConsoleVariable<int32> CVarHOLGarrisonProofSlots(TEXT("hol.Garrison.ProofSlots"), 0,
	TEXT("Proof scenarios only (D850): every building with a type row has at least this many garrison places; 0 = the places of BuildingTypes.csv."));

int32 UHOLGarrisonSubsystem::GetSlots(const AHOLBuilding& Building)
{
	FHOLBuildingTypeRow Row;
	if (!UHOLBuildingTypeLibrary::FindRow(Building.GetTypeName(), Row))
	{
		return 0;
	}
	return FMath::Max(FMath::Max(0, Row.GarrisonSlots), FMath::Max(0, CVarHOLGarrisonProofSlots.GetValueOnAnyThread()));
}

bool UHOLGarrisonSubsystem::EjectUnit(AHOLBuilding& Building, const uint32 UnitId, const int32 ExitIndex)
{
	UWorld* World = Building.GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!Units || !EntitySubsystem || World->GetNetMode() == NM_Client || Units->GetGarrison(UnitId) != &Building || !Units->SetGarrison(UnitId, nullptr))
	{
		return false;
	}
	// On its own free point at the building's foot, where its births stand (on the side of its rally point, else below it on the screen), not at its
	// centre for the keep-out to push them all out the same way (lot 3b); the same when it falls (lot 3c).
	const UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	FMassEntityManager& Entities = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle Entity = Units->FindEntity(UnitId);
	if (FTransformFragment* Transform = Production && Entity.IsSet() ? Entities.GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr)
	{
		FVector Exit = Production->GetBirthLocation(Building, ExitIndex);
		Exit.Z = Transform->GetTransform().GetLocation().Z;
		Transform->GetMutableTransform().SetLocation(Exit);
	}
	return true;
}

int32 UHOLGarrisonSubsystem::EjectAll(AHOLBuilding& Building)
{
	UWorld* World = Building.GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!Units || World->GetNetMode() == NM_Client)
	{
		return 0;
	}
	TArray<uint32> Inside;
	Units->GetGarrisonUnitIds(&Building, Inside);
	int32 Moved = 0;
	for (const uint32 UnitId : Inside)
	{
		if (EjectUnit(Building, UnitId, Moved))
		{
			++Moved;
		}
	}
	return Moved;
}

int32 UHOLGarrisonSubsystem::ApplyOrders()
{
	UWorld* World = GetWorld();
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!Commands || !Units || !EntitySubsystem || World->GetNetMode() == NM_Client)
	{
		return 0;
	}
	TArray<FHOLGarrisonOrder> Orders;
	Commands->ConsumeGarrisonOrders(Orders);
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	int32 Moved = 0;
	for (const FHOLGarrisonOrder& Order : Orders)
	{
		// One unit out: its own garrison at the step, the building's owner only (D374).
		if (Order.ExitUnitId != 0)
		{
			AHOLBuilding* Holding = Units->GetGarrison(Order.ExitUnitId);
			if (Holding && static_cast<int32>(Holding->GetOwnerPlayer()) == Order.PlayerIndex && EjectUnit(*Holding, Order.ExitUnitId, 0))
			{
				++Moved;
				UE_LOG(LogHOLSimulation, Display, TEXT("Garrison: slot %d takes unit %u out of %s"), Order.PlayerIndex, Order.ExitUnitId, *Holding->GetTypeName().ToString());
			}
			continue;
		}
		AHOLBuilding* Building = Order.Building.Get();
		// Only the building's owner orders its garrison (D374: never an ally's, nor an enemy's).
		if (!Building || static_cast<int32>(Building->GetOwnerPlayer()) != Order.PlayerIndex)
		{
			continue;
		}
		TArray<uint32> Inside;
		Units->GetGarrisonUnitIds(Building, Inside);
		if (Order.bExitAll)
		{
			Moved += EjectAll(*Building);
			UE_LOG(LogHOLSimulation, Display, TEXT("Garrison: slot %d takes %d units out of %s"), Order.PlayerIndex, Inside.Num(), *Building->GetTypeName().ToString());
			continue;
		}
		if (!Building->IsFinished())
		{
			continue;
		}
		TArray<uint32> Candidates;
		TArray<int32> UnitSlots;
		TArray<HOLGarrison::EKind> UnitKinds;
		for (const uint32 UnitId : Order.UnitIds)
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			const FHOLUnitStateFragment* State = Entity.IsSet() ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
			const FHOLUnitTypeConstants* Constants = Entity.IsSet() ? EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Entity) : nullptr;
			if (!State || !Constants || State->CurrentHealth <= 0 || Units->GetGarrison(UnitId) || Candidates.Contains(UnitId))
			{
				continue;
			}
			Candidates.Add(UnitId);
			UnitSlots.Add(State->OwnerPlayer);
			UnitKinds.Add(HOLGarrison::KindOf(Constants->HasArmorCategory(EHOLArmorCategory::Siege), Constants->HasArmorCategory(EHOLArmorCategory::Hero),
				Constants->HasArmorCategory(EHOLArmorCategory::Cavalry), HOLCombat::IsRangedAttack(Constants->AttackRange)));
		}
		// The script board decides (D836 point 4); without it nobody enters (R7).
		const TArray<int32> In = UHOLGarrisonBoard::AdmittedForOrder(World, UnitSlots, UnitKinds, Building->GetOwnerPlayer(), GetSlots(*Building), Inside.Num());
		TArray<uint32> Entered;
		for (const int32 Index : In)
		{
			if (Units->SetGarrison(Candidates[Index], Building))
			{
				Entered.Add(Candidates[Index]);
			}
		}
		// They stand inside: their walks and fights forgotten (the server's own order).
		if (!Entered.IsEmpty())
		{
			Commands->IssueCombatOrder(Entered, EHOLOrderKind::Stop, 0);
		}
		Moved += Entered.Num();
		UE_LOG(LogHOLSimulation, Display, TEXT("Garrison: slot %d puts %d of %d units in %s (%d places, %d inside before)"), Order.PlayerIndex, Entered.Num(),
			Order.UnitIds.Num(), *Building->GetTypeName().ToString(), GetSlots(*Building), Inside.Num());
	}
	return Moved;
}
