// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIOrders.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"

namespace
{
	/** The world of WorldContextObject when it holds the authority (server, solo); null on a client. */
	UWorld* GetAuthorityWorld(const UObject* WorldContextObject)
	{
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		if (World && World->GetNetMode() == NM_Client)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("AI order refused on a client: the AI runs on the server"));
			return nullptr;
		}
		return World;
	}

	/**
	 * The building nearest Location, within 100 cm (a summary's FHOLAIBuilding::Location), of Owner's; of ANY owner when Owner is
	 * INDEX_NONE (an enemy's, to attack it). Null if none.
	 */
	AHOLBuilding* FindBuildingAt(const UWorld& World, const int32 Owner, const FVector& Location)
	{
		AHOLBuilding* Nearest = nullptr;
		double NearestDistance = 100.0;
		for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
		{
			const double Distance = FVector::Dist2D(It->GetActorLocation(), Location);
			if ((Owner == INDEX_NONE || It->GetOwnerPlayer() == Owner) && Distance <= NearestDistance)
			{
				Nearest = *It;
				NearestDistance = Distance;
			}
		}
		return Nearest;
	}

	/** PlayerIndex's building nearest Location, within 100 cm; null if none (and for no slot: INDEX_NONE would mean any owner's). */
	AHOLBuilding* FindOwnBuilding(const UWorld& World, const int32 PlayerIndex, const FVector& Location)
	{
		return PlayerIndex == INDEX_NONE ? nullptr : FindBuildingAt(World, PlayerIndex, Location);
	}

	/** A point of the footprint's middle on the ground. */
	FVector ToPoint(const UWorld& World, const FBox2D& Footprint, const double DefaultZ)
	{
		const FVector2D Middle = Footprint.GetCenter();
		return FVector(Middle, UHOLConstructionSubsystem::FindGroundZ(World, Middle, DefaultZ));
	}

	/** Script's int32 identifiers back to the simulation's uint32 (a negative one names no unit: 0). */
	TArray<uint32> ToUnitIds(const TArray<int32>& UnitIds)
	{
		TArray<uint32> Ids;
		Ids.Reserve(UnitIds.Num());
		for (const int32 Id : UnitIds)
		{
			Ids.Add(Id > 0 ? static_cast<uint32>(Id) : 0u);
		}
		return Ids;
	}
}

bool UHOLAIOrderLibrary::IssueAIGatherOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const int32 SourceIndex,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && HOLPlayerOrders::Gather(*World, PlayerIndex, ToUnitIds(UnitIds), SourceIndex, bQueue);
}

bool UHOLAIOrderLibrary::IssueAIMoveOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector Target,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && HOLPlayerOrders::Move(*World, PlayerIndex, ToUnitIds(UnitIds), Target, bQueue);
}


bool UHOLAIOrderLibrary::IssueAITalentOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const int32 Family)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (!Commands || PlayerIndex < 0 || PlayerIndex >= FHOLMatchConfig::MaxSlots)
	{
		return false;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Hero talent order: slot %d family %d queued (AI)"), PlayerIndex, Family);
	Commands->IssueHeroTalentOrder(PlayerIndex, Family);
	return true;
}

bool UHOLAIOrderLibrary::IssueAIDuelChallenge(const UObject* WorldContextObject, const int32 PlayerIndex, const int32 TargetSlot)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (!Commands || PlayerIndex < 0 || PlayerIndex >= FHOLMatchConfig::MaxSlots)
	{
		return false;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Duel order: slot %d challenge at slot %d queued (AI)"), PlayerIndex, TargetSlot);
	Commands->IssueDuelOrder(PlayerIndex, EHOLDuelOrderKind::Challenge, TargetSlot);
	return true;
}

bool UHOLAIOrderLibrary::IssueAIDuelAnswer(const UObject* WorldContextObject, const int32 PlayerIndex, const bool bAccept)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (!Commands || PlayerIndex < 0 || PlayerIndex >= FHOLMatchConfig::MaxSlots)
	{
		return false;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Duel order: slot %d %s queued (AI)"), PlayerIndex, bAccept ? TEXT("accept") : TEXT("refuse"));
	Commands->IssueDuelOrder(PlayerIndex, bAccept ? EHOLDuelOrderKind::Accept : EHOLDuelOrderKind::Refuse, INDEX_NONE);
	return true;
}

bool UHOLAIOrderLibrary::IssueAIAttackMoveOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector Target,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && HOLPlayerOrders::AttackMove(*World, PlayerIndex, ToUnitIds(UnitIds), Target, bQueue);
}

bool UHOLAIOrderLibrary::IssueAIAttackUnitOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const int32 TargetUnitId,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && TargetUnitId >= 0 && HOLPlayerOrders::Attack(*World, PlayerIndex, ToUnitIds(UnitIds), static_cast<uint32>(TargetUnitId), bQueue);
}

bool UHOLAIOrderLibrary::IssueAIAttackBuildingOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds,
	const FVector BuildingLocation, const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	// An enemy's building as well as any other: the order's processor refuses one of the units' own camp. INDEX_NONE (the server's
	// own orders every unit obeys) is refused by the queue's slot check, as for the other AI orders.
	AHOLBuilding* Building = World ? FindBuildingAt(*World, INDEX_NONE, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::AttackBuilding(*World, PlayerIndex, ToUnitIds(UnitIds), Building, bQueue);
}

int32 UHOLAIOrderLibrary::CheckAIPlacement(const UObject* WorldContextObject, const int32 PlayerIndex, const FName TypeName, const FVector Point)
{
	const UWorld* World = GetAuthorityWorld(WorldContextObject);
	const FHOLBuildingTypeRow* Type = TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
	if (!World || !Type || !Type->IsBuildable())
	{
		return -1;
	}
	return static_cast<int32>(UHOLConstructionSubsystem::CheckPlacement(*World, UHOLConstructionSubsystem::SnapFootprint(*Type, Point), PlayerIndex));
}

bool UHOLAIOrderLibrary::FindAIPlacement(const UObject* WorldContextObject, const int32 PlayerIndex, const FName TypeName, const FVector Centre,
	const int32 MaxRings, FVector& OutPoint)
{
	const UWorld* World = GetAuthorityWorld(WorldContextObject);
	const FHOLBuildingTypeRow* Type = TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
	FBox2D Footprint;
	if (!World || !Type || !Type->IsBuildable()
		|| !UHOLConstructionSubsystem::FindRingFootprint(*World, *Type, FVector2D(Centre), PlayerIndex, MaxRings, {}, Footprint))
	{
		return false;
	}
	OutPoint = ToPoint(*World, Footprint, Centre.Z);
	return true;
}

bool UHOLAIOrderLibrary::FindAIFarmBlock(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector Centre, const int32 MaxRings,
	FVector& OutMill, TArray<FVector>& OutFarms)
{
	const UWorld* World = GetAuthorityWorld(WorldContextObject);
	const FHOLBuildingTypeRow* MillType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Mill);
	const FHOLBuildingTypeRow* FarmType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
	FBox2D Mill;
	TArray<FBox2D> Farms;
	if (!World || !MillType || !FarmType
		|| !UHOLConstructionSubsystem::FindFarmBlock(*World, *MillType, *FarmType, FVector2D(Centre), PlayerIndex, MaxRings, Mill, Farms))
	{
		return false;
	}
	OutMill = ToPoint(*World, Mill, Centre.Z);
	OutFarms.Reset();
	for (const FBox2D& Farm : Farms)
	{
		OutFarms.Add(ToPoint(*World, Farm, Centre.Z));
	}
	return true;
}

bool UHOLAIOrderLibrary::IssueAIPlaceOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FName TypeName, const FVector Point,
	const TArray<int32>& UnitIds, const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	if (!World || !HOLPlayerOrders::IsAccepted(PlayerIndex, 0))
	{
		return false;
	}
	// What the human's ghost shows before it sends the order: the placement checked here, the server (the construction at its
	// step) checks it again and takes the cost.
	const int32 Refusal = CheckAIPlacement(WorldContextObject, PlayerIndex, TypeName, Point);
	if (Refusal != 0)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("AI order of slot %d: %s not placed at (%.0f, %.0f): %s"), PlayerIndex, *TypeName.ToString(), Point.X, Point.Y,
			Refusal < 0 ? TEXT("not a building to place") : UHOLConstructionSubsystem::DescribeRefusal(static_cast<EHOLPlacementRefusal>(Refusal)));
		return false;
	}
	return HOLPlayerOrders::Place(*World, PlayerIndex, TypeName, Point, ToUnitIds(UnitIds), bQueue);
}

bool UHOLAIOrderLibrary::IssueAIBuildOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector BuildingLocation,
	const TArray<int32>& UnitIds, const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	AHOLBuilding* Building = World ? FindOwnBuilding(*World, PlayerIndex, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::Build(*World, PlayerIndex, ToUnitIds(UnitIds), Building, bQueue);
}

bool UHOLAIOrderLibrary::IssueAIProduceOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector BuildingLocation, const FName TypeName)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	AHOLBuilding* Building = World ? FindOwnBuilding(*World, PlayerIndex, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::QueueProduction(*World, PlayerIndex, Building, TypeName);
}

bool UHOLAIOrderLibrary::IssueAISetRallyOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector BuildingLocation, const FVector Point,
	const int32 SourceIndex)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	AHOLBuilding* Building = World ? FindOwnBuilding(*World, PlayerIndex, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::SetRally(*World, PlayerIndex, Building, Point, SourceIndex);
}

bool UHOLAIOrderLibrary::IssueAIMarketOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector BuildingLocation, const int32 Resource,
	const int32 Lots, const bool bBuy)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	AHOLBuilding* Building = World ? FindOwnBuilding(*World, PlayerIndex, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::MarketTrade(*World, PlayerIndex, Building, Resource, Lots, bBuy);
}

bool UHOLAIOrderLibrary::ReadAIMarketBook(const UObject* WorldContextObject, TArray<int32>& OutPrices)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AHOLGameState* State = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!State || !State->HasMarketBook())
	{
		return false;
	}
	const HOLMarket::FBook Book = State->GetMarketBook();
	OutPrices.Reset();
	for (int32 Resource = 0; Resource < HOLMarket::BookSize; ++Resource)
	{
		OutPrices.Add(Book.Prices[Resource]);
	}
	return true;
}

bool UHOLAIOrderLibrary::ReadAIUnitFigures(const UObject* WorldContextObject, const FName TypeName, FHOLAITypeFigures& OutFigures)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const FHOLUnitTypeRow* Row = (Units && !TypeName.IsNone()) ? Units->FindTypeRow(TypeName) : nullptr;
	if (!Row)
	{
		return false;
	}
	OutFigures = FHOLAITypeFigures();
	OutFigures.CostFood = Row->Cost.Food;
	OutFigures.CostWood = Row->Cost.Wood;
	OutFigures.CostStone = Row->Cost.Stone;
	OutFigures.CostGold = Row->Cost.Gold;
	OutFigures.BuildTime = Row->BuildTime;
	OutFigures.Population = Row->Population;
	OutFigures.Speed = Row->Speed;
	OutFigures.CarryCapacity = Row->CarryCapacity;
	OutFigures.GatherFood = Row->GatherRates.Food;
	OutFigures.GatherWood = Row->GatherRates.Wood;
	OutFigures.GatherStone = Row->GatherRates.Stone;
	OutFigures.GatherGold = Row->GatherRates.Gold;
	// Combat: the row's own values (the shared fragment of the combat code is copied from it, FHOLUnitTypeRow::ToConstants).
	OutFigures.Health = Row->MaxHealth;
	OutFigures.Damage = Row->Attack;
	OutFigures.AttackInterval = Row->AttackInterval;
	OutFigures.AttackRange = Row->AttackRange;
	OutFigures.MeleeArmor = Row->MeleeArmor;
	OutFigures.RangedArmor = Row->RangedArmor;
	OutFigures.bRanged = Row->AttackType == EHOLAttackType::Ranged;
	const UEnum* CategoryEnum = StaticEnum<EHOLArmorCategory>();
	const auto NameOf = [CategoryEnum](const EHOLArmorCategory Category) { return FName(*CategoryEnum->GetNameStringByValue(static_cast<int64>(Category))); };
	for (const EHOLArmorCategory Category : Row->ArmorCategories)
	{
		OutFigures.ArmorCategories.AddUnique(NameOf(Category));
	}
	// Each bonus adds its damage as is to a hit on a target of its category (HOLCombat::GetDamage): two bonuses of one category add up.
	for (const FHOLAttackBonus& Bonus : Row->Bonuses)
	{
		if (Bonus.Damage <= 0)
		{
			continue;
		}
		const FName Category = NameOf(Bonus.Category);
		const int32 Found = OutFigures.BonusCategories.IndexOfByKey(Category);
		if (Found == INDEX_NONE)
		{
			OutFigures.BonusCategories.Add(Category);
			OutFigures.BonusDamage.Add(static_cast<float>(Bonus.Damage));
		}
		else
		{
			OutFigures.BonusDamage[Found] += static_cast<float>(Bonus.Damage);
		}
	}
	return true;
}

bool UHOLAIOrderLibrary::ReadAIBuildingFigures(const FName TypeName, FHOLAITypeFigures& OutFigures)
{
	const FHOLBuildingTypeRow* Row = TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
	if (!Row)
	{
		return false;
	}
	OutFigures = FHOLAITypeFigures();
	OutFigures.CostFood = Row->Cost.Food;
	OutFigures.CostWood = Row->Cost.Wood;
	OutFigures.CostStone = Row->Cost.Stone;
	OutFigures.CostGold = Row->Cost.Gold;
	OutFigures.BuildTime = Row->BuildTime;
	OutFigures.Population = Row->PopulationProvided;
	return true;
}
