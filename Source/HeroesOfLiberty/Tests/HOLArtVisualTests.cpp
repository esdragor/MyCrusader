// Copyright Epic Games, Inc. All Rights Reserved.

// D924 (AR0-a): the art visual table. Mass simulation tests (D838): the type registration (FindTemplate) and its faction CRC
// (GetTemplateKey) read the art asset through the single resolver, field by field, the CSV row as the last level.

#include "Art/HOLArtVisual.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "MassEntityTemplate.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "StructUtils/SharedStruct.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::ArtVisual
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Spearman(TEXT("Spearman"));
	const FName Legions(TEXT("Legions"));
	/** A mesh every engine has, other than the CSV's Spearman (checked by each test before it counts). */
	const FSoftObjectPath ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));

	/** A game world with its subsystems (no actors needed: the type table, the faction table and Mass are world-wide). */
	struct FWorld
	{
		UWorld* World = nullptr;
		UHOLUnitSubsystem* Units = nullptr;

		FWorld()
		{
			HOLArtVisual::ClearTestEntries();
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
		}

		~FWorld()
		{
			HOLArtVisual::ClearTestEntries();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
	};

	UHOLArtVisual* MakeAsset(const FName TypeName, const FName Faction, const TCHAR* Name)
	{
		UHOLArtVisual* Asset = NewObject<UHOLArtVisual>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UHOLArtVisual::StaticClass(), FName(Name)));
		Asset->TypeName = TypeName;
		Asset->Faction = Faction;
		return Asset;
	}

	/** The constants the registered Mass template carries (the real chain: FindTemplate builds it). */
	const FHOLUnitTypeConstants* TemplateConstants(UHOLUnitSubsystem& Units, const FName TypeName, const FName FactionKey)
	{
		const FMassEntityTemplate* Template = Units.FindTemplate(TypeName, EHOLUnitTemplateKind::Simulation, FactionKey);
		if (!Template)
		{
			return nullptr;
		}
		for (const FConstSharedStruct& Shared : Template->GetSharedFragmentValues().GetConstSharedFragments())
		{
			if (Shared.GetScriptStruct() == FHOLUnitTypeConstants::StaticStruct())
			{
				return &Shared.Get<const FHOLUnitTypeConstants>();
			}
		}
		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualEmptyAssetTest, "HeroesOfLiberty.Art.Visual.EmptyAssetKeepsNeutral", UE::HeroesOfLiberty::Tests::ArtVisual::Flags)
bool FHOLArtVisualEmptyAssetTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisual;
	FWorld Test;
	const FHOLUnitTypeRow* Row = Test.Units ? Test.Units->FindTypeRow(Spearman) : nullptr;
	if (!TestNotNull(TEXT("the real unit type table has a Spearman"), Row))
	{
		return false;
	}
	// AR0-d2 (3): the row carries no visual field any more; the art asset is the only source, an empty one leaves the
	// neutral value (no mesh, scale and play rate of the row's neutral constants), never the old CSV's.
	const FHOLUnitTypeConstants Neutral = Row->ToConstants(Spearman);
	FHOLUnitTypeConstants Real = Row->ToConstants(Spearman);
	HOLArtVisual::Apply(Test.World, Real, Spearman, NAME_None);
	if (!TestFalse(TEXT("before: the real DA_Art_Spearman gives a mesh (the test can tell an empty asset apart)"), Real.Mesh.IsNull()))
	{
		return false;
	}
	const FName LegionsKeyBefore = Test.Units->GetTemplateKey(Spearman, Legions);
	HOLArtVisual::AddTestEntry(MakeAsset(Spearman, NAME_None, TEXT("DA_Art_Spearman")));
	HOLArtVisual::AddTestEntry(MakeAsset(Spearman, Legions, TEXT("DA_Art_Spearman_Legions")));
	FHOLUnitTypeConstants Built = Row->ToConstants(Spearman);
	HOLArtVisual::Apply(Test.World, Built, Spearman, NAME_None);
	TestTrue(TEXT("an empty art asset gives no mesh (neutral, no fallback on the row)"), Built.Mesh.IsNull());
	TestTrue(TEXT("... no skinned mesh"), Built.SkinnedMesh.IsNull());
	TestTrue(TEXT("... no animation provider"), Built.AnimationProvider.IsNull());
	TestEqual(TEXT("... the neutral scale"), Built.MeshScale, Neutral.MeshScale);
	TestEqual(TEXT("... the neutral yaw"), Built.MeshYawOffset, Neutral.MeshYawOffset);
	TestEqual(TEXT("... the neutral walk play rate"), Built.WalkPlayRate, Neutral.WalkPlayRate);
	TestEqual(TEXT("an empty faction asset gives the faction no Mass type of its own (the CRC sees no change)"), Test.Units->GetTemplateKey(Spearman, Legions), LegionsKeyBefore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualMeshTest, "HeroesOfLiberty.Art.Visual.MeshReplacesCsvAtRegistration", UE::HeroesOfLiberty::Tests::ArtVisual::Flags)
bool FHOLArtVisualMeshTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisual;
	FWorld Test;
	const FHOLUnitTypeRow* Row = Test.Units ? Test.Units->FindTypeRow(Spearman) : nullptr;
	if (!TestNotNull(TEXT("the real unit type table has a Spearman"), Row))
	{
		return false;
	}
	const FHOLUnitTypeConstants Csv = Row->ToConstants(Spearman);
	if (!TestNotEqual(TEXT("the CSV's Spearman mesh is not the test's cone (the test can tell them apart)"), Csv.Mesh.ToString(), ConeMesh.ToString()))
	{
		return false;
	}
	UHOLArtVisual* Asset = MakeAsset(Spearman, NAME_None, TEXT("DA_Art_Spearman"));
	Asset->Mesh = TSoftObjectPtr<UStaticMesh>(ConeMesh);
	Asset->bOverrideMeshScale = true;
	Asset->MeshScale = Csv.MeshScale * 2.f + 0.5f;
	HOLArtVisual::AddTestEntry(Asset);

	const FHOLUnitTypeConstants* Built = TemplateConstants(*Test.Units, Spearman, NAME_None);
	if (!TestNotNull(TEXT("the Spearman's Mass template is built"), Built))
	{
		return false;
	}
	TestEqual(TEXT("the registered type's mesh is the art asset's"), Built->Mesh.ToString(), ConeMesh.ToString());
	TestEqual(TEXT("the registered type's scale is the art asset's"), Built->MeshScale, Csv.MeshScale * 2.f + 0.5f);
	TestEqual(TEXT("a field the asset leaves empty falls back to the CSV (skinned mesh)"), Built->SkinnedMesh.ToString(), Csv.SkinnedMesh.ToString());
	TestEqual(TEXT("... and its walk sequence"), Built->WalkSequence, Csv.WalkSequence);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualFactionTypeTest, "HeroesOfLiberty.Art.Visual.FactionAssetGetsOwnMassType", UE::HeroesOfLiberty::Tests::ArtVisual::Flags)
bool FHOLArtVisualFactionTypeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisual;
	FWorld Test;
	if (!TestNotNull(TEXT("the unit subsystem"), Test.Units) || !TestNotNull(TEXT("the real unit type table has a Spearman"), Test.Units->FindTypeRow(Spearman)))
	{
		return false;
	}
	// Before: the Legions' Spearman shares the table's Mass type (its rows differ by name and cost, not by constants).
	if (!TestEqual(TEXT("without art, the Legions' Spearman shares the Spearman's Mass type"), Test.Units->GetTemplateKey(Spearman, Legions), Spearman))
	{
		return false;
	}
	UHOLArtVisual* Asset = MakeAsset(Spearman, Legions, TEXT("DA_Art_Spearman_Legions"));
	Asset->Mesh = TSoftObjectPtr<UStaticMesh>(ConeMesh);
	HOLArtVisual::AddTestEntry(Asset);

	TestEqual(TEXT("a Legions art asset gives the Legions' Spearman its own Mass type (the CRC sees the art)"),
		Test.Units->GetTemplateKey(Spearman, Legions), FName(TEXT("Spearman@Legions")));
	TestEqual(TEXT("the Dawn's Spearman keeps the table's type"), Test.Units->GetTemplateKey(Spearman, FName(TEXT("Dawn"))), Spearman);
	const FHOLUnitTypeConstants* LegionsBuilt = TemplateConstants(*Test.Units, Spearman, Legions);
	const FHOLUnitTypeConstants* DawnBuilt = TemplateConstants(*Test.Units, Spearman, NAME_None);
	if (TestNotNull(TEXT("the Legions' Spearman template"), LegionsBuilt) && TestNotNull(TEXT("the Spearman template"), DawnBuilt))
	{
		TestEqual(TEXT("the Legions' Spearman is built with the faction asset's mesh"), LegionsBuilt->Mesh.ToString(), ConeMesh.ToString());
		TestNotEqual(TEXT("the common Spearman keeps the CSV's mesh"), DawnBuilt->Mesh.ToString(), ConeMesh.ToString());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualDuplicateTest, "HeroesOfLiberty.Art.Visual.DuplicateIsAnError", UE::HeroesOfLiberty::Tests::ArtVisual::Flags)
bool FHOLArtVisualDuplicateTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisual;
	const UHOLArtVisual* First = MakeAsset(Spearman, Legions, TEXT("DA_Art_Spearman_Legions"));
	const UHOLArtVisual* Second = MakeAsset(Spearman, Legions, TEXT("DA_Art_Spearman_Legions_Copy"));
	const UHOLArtVisual* Other = MakeAsset(Spearman, NAME_None, TEXT("DA_Art_Spearman"));
	TMap<UHOLArtVisualRegistry::FKey, const UHOLArtVisual*> Map;
	TArray<FString> Errors;
	UHOLArtVisualRegistry::BuildMap({ First, Second, Other }, Map, Errors);
	if (TestEqual(TEXT("two assets for (Spearman, Legions) = one error"), Errors.Num(), 1))
	{
		TestTrue(TEXT("the error names the first file"), Errors[0].Contains(First->GetPathName()));
		TestTrue(TEXT("the error names the second file"), Errors[0].Contains(Second->GetPathName()));
	}
	TestEqual(TEXT("both keys are in the map"), Map.Num(), 2);
	const UHOLArtVisual* const* Kept = Map.Find(UHOLArtVisualRegistry::FKey(Spearman, Legions));
	TestTrue(TEXT("the first asset is kept"), Kept && *Kept == First);
	return true;
}

#endif
