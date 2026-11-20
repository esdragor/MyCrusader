// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HUD/HOLSelectionReadLibrary.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLControllerCommandLibrary.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// D813 garrison frame (ELROND-1120pb lot C++, GLORFINDEL-1120pc): what the script's frame reads of the selected building, through the REAL chain
// (the garrison order queued, applied at the step, the building's list of occupants, the unit as this machine knows it):
// - after two Spearmen are garrisoned, the view's GarrisonOccupants are their ids, in order, and GarrisonOccupantTypes names each one's type
//   ("Spearman", computed by HOLUnitView through GetUnitTypeKey: a garrisoned unit stays known);
// - a unit id nobody knows has an empty type; the two lists keep the same length;
// - the new commands refuse without a controller (the effect of the order itself is the existing HeroesOfLiberty.Garrison.UngarrisonUnit test).
namespace UE::HeroesOfLiberty::Tests::GarrisonSelection
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName TowerType(TEXT("GarrisonSelectionTestTower"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGarrisonSelectionViewTest, "HeroesOfLiberty.Garrison.SelectionView", UE::HeroesOfLiberty::Tests::GarrisonSelection::Flags)

bool FHOLGarrisonSelectionViewTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GarrisonSelection;
	UDataTable* Table = const_cast<UDataTable*>(GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous());
	const FHOLBuildingTypeRow* HouseRow = Table ? Table->FindRow<FHOLBuildingTypeRow>(HOLBuildingTypeNames::House, TEXT("garrison selection test")) : nullptr;
	if (!TestNotNull(TEXT("the building type table and its House row"), HouseRow))
	{
		return false;
	}
	FHOLBuildingTypeRow TowerRow = *HouseRow;
	TowerRow.GarrisonSlots = 2;
	Table->AddRow(TowerType, TowerRow);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLBuilding* Tower = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("tower"), Tower))
	{
		Tower->Setup(TowerType, 0, 0);
		const auto Place = [Units](const FVector& Location)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TEXT("Spearman"));
			Request.Count = 1;
			Request.OwnerPlayer = 0;
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Born = 0;
			for (const uint32 Id : After)
			{
				Born = Before.Contains(Id) ? Born : Id;
			}
			return Born;
		};
		const uint32 First = Place(FVector(2000.0, -600.0, 0.0));
		const uint32 Second = Place(FVector(2000.0, 600.0, 0.0));
		if (TestTrue(TEXT("the two Spearmen born"), First && Second))
		{
			const uint32 Both[] = { First, Second };
			TestTrue(TEXT("the garrison order enters the queue"), HOLPlayerOrders::Garrison(*World, 0, Both, Tower));
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			Simulation->GetOnStepEnded().Broadcast(Step);
			TestTrue(TEXT("both are inside"), Units->GetGarrison(First) == Tower && Units->GetGarrison(Second) == Tower);

			FHOLBuildingSelView View;
			UHOLSelectionReadLibrary::ReadGarrisonOccupants(World, Tower, View);
			TestTrue(TEXT("the view lists the two occupants' ids, in order"), View.GarrisonOccupants == TArray<int32>({ static_cast<int32>(First), static_cast<int32>(Second) }));
			TestEqual(TEXT("one type per occupant, same length"), View.GarrisonOccupantTypes.Num(), View.GarrisonOccupants.Num());
			TestTrue(TEXT("each occupant's type is its unit's (a garrisoned unit stays known)"),
				View.GarrisonOccupantTypes == TArray<FString>({ TEXT("Spearman"), TEXT("Spearman") }));
			TestEqual(TEXT("GetUnitTypeKey names the unit's type"), UHOLSelectionReadLibrary::GetUnitTypeKey(World, static_cast<int32>(First)), FString(TEXT("Spearman")));
			TestTrue(TEXT("an unknown unit has an empty type"), UHOLSelectionReadLibrary::GetUnitTypeKey(World, 0x7FFFFFF0).IsEmpty());
			TestTrue(TEXT("unit id 0 has an empty type"), UHOLSelectionReadLibrary::GetUnitTypeKey(World, 0).IsEmpty());
		}
		FHOLBuildingSelView Empty;
		UHOLSelectionReadLibrary::ReadGarrisonOccupants(World, nullptr, Empty);
		TestTrue(TEXT("no building: both lists stay empty"), Empty.GarrisonOccupants.IsEmpty() && Empty.GarrisonOccupantTypes.IsEmpty());
		TestFalse(TEXT("one unit out refused without a controller"), UHOLControllerCommandLibrary::RequestUngarrisonUnitOf(nullptr, 7));
		TestFalse(TEXT("everyone out refused without a controller"), UHOLControllerCommandLibrary::RequestUngarrisonAllSelected(nullptr));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	Table->RemoveRow(TowerType);
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
