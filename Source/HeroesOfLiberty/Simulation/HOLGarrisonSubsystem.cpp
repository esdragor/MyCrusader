// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGarrisonSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypeLibrary.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLGarrisonRules.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassEntitySubsystem.h"
#include "Simulation/HOLCombatRules.h"
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

int32 UHOLGarrisonSubsystem::GetSlots(const AHOLBuilding& Building)
{
	FHOLBuildingTypeRow Row;
	return UHOLBuildingTypeLibrary::FindRow(Building.GetTypeName(), Row) ? FMath::Max(0, Row.GarrisonSlots) : 0;
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
			for (const uint32 UnitId : Inside)
			{
				Moved += Units->SetGarrison(UnitId, nullptr) ? 1 : 0;
			}
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
		const TArray<int32> In = HOLGarrison::AdmittedBySettings(UnitSlots, UnitKinds, Building->GetOwnerPlayer(), GetSlots(*Building), Inside.Num());
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
