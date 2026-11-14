// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionRules.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLSpecializationSubsystem.h"

FVector2D UHOLBuildingSettings::GetBodySize(const FVector2D& Footprint)
{
	return HOLConstruction::GetBodySize(Footprint, GetBodyInset());
}

FVector2D UHOLBuildingSettings::GetVisualSize(const FVector2D& Footprint)
{
	return HOLConstruction::GetBodySize(Footprint, GetVisualInset());
}

const FHOLBuildingTypeRow* UHOLBuildingSettings::FindType(const FName TypeName)
{
	const UDataTable* Table = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No building type table (HOLBuildingSettings.BuildingTypesTable in DefaultGame.ini)"));
		return nullptr;
	}
	const FHOLBuildingTypeRow* Row = Table->FindRow<FHOLBuildingTypeRow>(TypeName, TEXT("building type"), /*bWarnIfRowMissing*/ false);
	if (!Row)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No building type %s in %s"), *TypeName.ToString(), *Table->GetPathName());
	}
	return Row;
}

void UHOLBuildingSettings::GetBuildableTypes(TArray<FHOLBuildable>& OutTypes, const FName FactionKey)
{
	OutTypes.Reset();
	const UDataTable* Table = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	if (!Table)
	{
		return;
	}
	// L0: with a faction (not None), only the types it builds (the `Faction` column).
	const HOLFactionRules::FTable& Factions = HOLFactionRules::GetTable();
	Table->ForeachRow<FHOLBuildingTypeRow>(TEXT("buildable types"), [&OutTypes, &Factions, FactionKey](const FName& Name, const FHOLBuildingTypeRow& Row)
	{
		if (Row.IsBuildable() && (FactionKey.IsNone() || Factions.Allows(Row.Faction, FactionKey)))
		{
			OutTypes.Add({ Name, Row.BuildGridSlot, Row.Age });
		}
	});
}

namespace UE::HeroesOfLiberty::BuildingTypes
{
	/** The cell of a Nomad seat's first forum in the builders' age I page (0 to 11; 10 = C, free in every faction's age I page). */
	static TAutoConsoleVariable<int32> CVarNomadForumCell(TEXT("hol.Nomad.ForumCell"), 10,
		TEXT("Cell (0-11) of a Nomad seat's first forum in the builders' age I grid (BUG nomad_start point 1, default 10 = C)."));
}

void UHOLBuildingSettings::GetBuildableTypesForSlot(const UWorld* World, const int32 Slot, TArray<FHOLBuildable>& OutTypes)
{
	GetBuildableTypes(OutTypes, HOLFactionRules::GetSlotFactionKey(World, Slot));
	if (IsFirstForumOpen(World, Slot))
	{
		const int32 Cell = UE::HeroesOfLiberty::BuildingTypes::CVarNomadForumCell.GetValueOnAnyThread();
		OutTypes.RemoveAll([Cell](const FHOLBuildable& Each) { return Each.Age == 0 && Each.Cell == Cell; });
		OutTypes.Add({ HOLBuildingTypeNames::MainCentre, Cell, 0 });
	}
}

bool UHOLBuildingSettings::IsFirstForumOpen(const UWorld* World, const int32 Slot)
{
	if (!World || Slot < 0 || UHOLSpecializationSubsystem::IsForumComplete(World, Slot))
	{
		return false;
	}
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		if (!It->IsActorBeingDestroyed() && It->GetOwnerPlayer() == Slot && It->GetTypeName() == HOLBuildingTypeNames::MainCentre)
		{
			return false;
		}
	}
	return true;
}

bool UHOLBuildingSettings::IsBuildableBy(const UWorld* World, const FName TypeName, const int32 Slot)
{
	const FHOLBuildingTypeRow* Type = TypeName.IsNone() ? nullptr : FindType(TypeName);
	return Type && (Type->IsBuildable() || (TypeName == HOLBuildingTypeNames::MainCentre && IsFirstForumOpen(World, Slot)));
}

FHOLResources UHOLBuildingSettings::GetBuildCost(const UWorld* World, const FName TypeName, const int32 Slot)
{
	const FHOLBuildingTypeRow* Type = TypeName.IsNone() ? nullptr : FindType(TypeName);
	if (!Type)
	{
		return FHOLResources();
	}
	// A Nomad seat's forum before its first complete one (its foundation included, for the refund of a cancel).
	if (TypeName == HOLBuildingTypeNames::MainCentre && World && Slot >= 0 && !UHOLSpecializationSubsystem::IsForumComplete(World, Slot))
	{
		return UHOLSpecializationSubsystem::FirstForumCost(World);
	}
	return Type->Cost;
}
