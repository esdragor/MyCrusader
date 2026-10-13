// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// J2 checks (D264): the building type table (D260) and the provisional look of a building (D261).
namespace UE::HeroesOfLiberty::Tests::Buildings
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Pieces of a building painted with the given player colour. */
	int32 CountPiecesOfColor(const AHOLBuilding& Building, const int32 ColorIndex)
	{
		const FLinearColor Expected(HOLPlayerColors::GetLinearColor(ColorIndex));
		TArray<UStaticMeshComponent*> Pieces;
		Building.GetComponents(Pieces);
		int32 Count = 0;
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			FLinearColor Color;
			const UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Piece->GetMaterial(0));
			if (Material && Material->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Color")), Color)
				&& Color.Equals(Expected, 1.e-4f))
			{
				++Count;
			}
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingTypesTest, "HeroesOfLiberty.Buildings.Types", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingTypesTest::RunTest(const FString& Parameters)
{
	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("main centre in the building type table"), MainCentre))
	{
		TestFalse(TEXT("named"), MainCentre->DisplayName.IsEmpty());
		TestEqual(TEXT("~15 x 15 m (D261)"), MainCentre->Footprint, FVector2D(1500.0, 1500.0));
		TestEqual(TEXT("~10 m high (D261)"), MainCentre->Height, 1000.f);
		TestTrue(TEXT("health points"), MainCentre->MaxHealth > 0);
		TestTrue(TEXT("provisional shape: no final mesh yet"), MainCentre->Mesh.IsNull());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingAppearanceTest, "HeroesOfLiberty.Buildings.Appearance", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingAppearanceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Buildings;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	// As in a match: the actors spawned from now on are initialised (a building builds its look once initialised).
	World->InitializeActorsForPlay(FURL());

	AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>();
	if (TestNotNull(TEXT("building spawned"), Building))
	{
		TestTrue(TEXT("replicated"), Building->GetIsReplicated());
		Building->Setup(HOLBuildingTypeNames::MainCentre, 2, 3);
		TestEqual(TEXT("owner"), Building->GetOwnerPlayer(), static_cast<uint8>(2));

		TArray<UStaticMeshComponent*> Pieces;
		Building->GetComponents(Pieces);
		TestEqual(TEXT("a block, its top, a tower and its roof"), Pieces.Num(), 4);

		// The shape fills the type's footprint and height, standing on the ground, centred on the actor.
		const FBox Box = Building->GetComponentsBoundingBox(/*bNonColliding*/ true);
		TestTrue(TEXT("footprint 15 x 15 m"), Box.GetSize().Equals(FVector(1500.0, 1500.0, 1000.0), 1.0));
		TestTrue(TEXT("on the ground, centred"), Box.Min.Equals(FVector(-750.0, -750.0, 0.0), 1.0));

		TestEqual(TEXT("two pieces at the owner's colour"), CountPiecesOfColor(*Building, 3), 2);
		Building->Setup(HOLBuildingTypeNames::MainCentre, 2, 5);
		TestEqual(TEXT("a new colour repaints them"), CountPiecesOfColor(*Building, 5), 2);
		Building->GetComponents(Pieces);
		TestEqual(TEXT("without rebuilding the pieces"), Pieces.Num(), 4);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif
