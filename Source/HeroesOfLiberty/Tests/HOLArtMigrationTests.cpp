// Copyright Epic Games, Inc. All Rights Reserved.

// D925 (AR0-d1): the one-shot migration of the art out of the CSV rows into /Game/Art/<Units|Buildings|Sources>/<Type>/
// DA_Art_<Type> keeps every type as it is: for each row of the unit table, the constants built from the CSV equal (CRC, as
// HOLUnitSubsystem::GetTemplateKey compares them) the constants built from the row with its visual fields emptied and then
// dressed by the type's asset loaded from /Game/Art (HOLArtVisual::Apply); the same for the arrows' projectile and smoke,
// and for the mesh of every building and every source. Mass simulation test (D838): the Mass type registration reads them.

#include "Art/HOLArtVisual.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "StructUtils/StructView.h"
#include "StructUtils/StructUtilsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::ArtMigration
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	template <typename T>
	FSoftObjectPath PathOf(const TSoftObjectPtr<T>& Pointer)
	{
		return Pointer.ToSoftObjectPath();
	}

	inline FSoftObjectPath PathOf(const FSoftObjectPath& Path)
	{
		return Path;
	}

	/** The type's common art asset, loaded from /Game/Art/<Folder>/<Type>/DA_Art_<Type> and made a test entry; null if absent. */
	const UHOLArtVisual* LoadArt(FAutomationTestBase& Test, const TCHAR* Folder, const FName TypeName)
	{
		const FString Name = FString::Printf(TEXT("DA_Art_%s"), *TypeName.ToString());
		const FString Package = FString::Printf(TEXT("/Game/Art/%s/%s/%s"), Folder, *TypeName.ToString(), *Name);
		UHOLArtVisual* Asset = FPackageName::DoesPackageExist(Package)
			? LoadObject<UHOLArtVisual>(nullptr, *FString::Printf(TEXT("%s.%s"), *Package, *Name), nullptr, LOAD_NoWarn | LOAD_Quiet)
			: nullptr;
		if (!Asset)
		{
			Test.AddError(FString::Printf(TEXT("%s: no art asset %s"), *TypeName.ToString(), *Package));
			return nullptr;
		}
		Test.TestEqual(FString::Printf(TEXT("%s: the asset dresses its own type"), *Package), Asset->TypeName, TypeName);
		Test.TestTrue(FString::Printf(TEXT("%s: the asset is for every faction"), *Package), Asset->Faction.IsNone());
		HOLArtVisual::AddTestEntry(Asset);
		return Asset;
	}

	uint32 Crc(const FHOLUnitTypeConstants& Constants)
	{
		return UE::StructUtils::GetStructInstanceCrc32(FConstStructView::Make(Constants));
	}

	/** The unit row with every field the art asset carries put back to the row's defaults (the CSV column emptied). */
	FHOLUnitTypeRow WithoutVisual(const FHOLUnitTypeRow& Row)
	{
		const FHOLUnitTypeRow Empty;
		FHOLUnitTypeRow Out = Row;
		Out.Mesh = Empty.Mesh;
		Out.MeshYawOffset = Empty.MeshYawOffset;
		Out.MeshScale = Empty.MeshScale;
		Out.SkinnedMesh = Empty.SkinnedMesh;
		Out.AnimationProvider = Empty.AnimationProvider;
		Out.IdleSequence = Empty.IdleSequence;
		Out.WalkSequence = Empty.WalkSequence;
		Out.WalkPlayRate = Empty.WalkPlayRate;
		Out.GatherPlayRate = Empty.GatherPlayRate;
		Out.CarryWalkPlayRate = Empty.CarryWalkPlayRate;
		Out.BuildSequence = Empty.BuildSequence;
		Out.BuildPlayRate = Empty.BuildPlayRate;
		Out.AttackSequence = Empty.AttackSequence;
		Out.FallSequence = Empty.FallSequence;
		// AR0-d2: the sequences by resource and the props move to the asset too.
		Out.GatherSequences = Empty.GatherSequences;
		Out.CarryWalkSequences = Empty.CarryWalkSequences;
		Out.CarryIdleSequences = Empty.CarryIdleSequences;
		Out.GatherProps = Empty.GatherProps;
		Out.CarryProps = Empty.CarryProps;
		Out.BuildProp = Empty.BuildProp;
		Out.AttackProp = Empty.AttackProp;
		Out.Projectile = Empty.Projectile;
		Out.MuzzleSmoke = Empty.MuzzleSmoke;
		return Out;
	}

	/** The test entries cleared on both ends (the loaded assets stay out of the other tests). */
	struct FEntries
	{
		FEntries() { HOLArtVisual::ClearTestEntries(); }
		~FEntries() { HOLArtVisual::ClearTestEntries(); }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtMigrationKeepsEveryTypeTest, "HeroesOfLiberty.Art.Visual.MigrationKeepsEveryType", UE::HeroesOfLiberty::Tests::ArtMigration::Flags)
bool FHOLArtMigrationKeepsEveryTypeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtMigration;
	FEntries Entries;

	const UDataTable* Units = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const UDataTable* Buildings = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	const UDataTable* Sources = GetDefault<UHOLSourceSettings>()->SourceTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("the unit type table"), Units) || !TestNotNull(TEXT("the building type table"), Buildings)
		|| !TestNotNull(TEXT("the source type table"), Sources))
	{
		return false;
	}

	int32 NumUnits = 0;
	for (const TPair<FName, uint8*>& Pair : Units->GetRowMap())
	{
		const FName TypeName = Pair.Key;
		const FHOLUnitTypeRow& Row = *reinterpret_cast<const FHOLUnitTypeRow*>(Pair.Value);
		if (!LoadArt(*this, TEXT("Units"), TypeName))
		{
			continue;
		}
		const FHOLUnitTypeConstants Csv = Row.ToConstants(TypeName);
		FHOLUnitTypeConstants Migrated = WithoutVisual(Row).ToConstants(TypeName);
		HOLArtVisual::Apply(nullptr, Migrated, TypeName, NAME_None);
		const FString Type = TypeName.ToString();
		TestEqual(FString::Printf(TEXT("%s: mesh"), *Type), Migrated.Mesh.ToString(), Csv.Mesh.ToString());
		TestEqual(FString::Printf(TEXT("%s: skinned mesh"), *Type), Migrated.SkinnedMesh.ToString(), Csv.SkinnedMesh.ToString());
		TestEqual(FString::Printf(TEXT("%s: animation provider"), *Type), Migrated.AnimationProvider.ToString(), Csv.AnimationProvider.ToString());
		if (!TestEqual(FString::Printf(TEXT("%s: the migrated constants (CRC) are the CSV's"), *Type), Crc(Migrated), Crc(Csv)))
		{
			continue;
		}
		TestEqual(FString::Printf(TEXT("%s: projectile"), *Type),
			HOLArtVisual::ResolveProjectile(nullptr, TypeName, NAME_None, FSoftObjectPath()).ToString(), PathOf(Row.Projectile).ToString());
		TestEqual(FString::Printf(TEXT("%s: muzzle smoke"), *Type),
			HOLArtVisual::ResolveMuzzleSmoke(nullptr, TypeName, NAME_None, FSoftObjectPath()).ToString(), PathOf(Row.MuzzleSmoke).ToString());
		++NumUnits;
	}

	const auto CheckMeshes = [this](const UDataTable& Table, const TCHAR* Folder, const auto* RowType)
	{
		using FRow = std::remove_const_t<std::remove_pointer_t<decltype(RowType)>>;
		for (const TPair<FName, uint8*>& Pair : Table.GetRowMap())
		{
			const FRow& Row = *reinterpret_cast<const FRow*>(Pair.Value);
			if (LoadArt(*this, Folder, Pair.Key))
			{
				TestEqual(FString::Printf(TEXT("%s: mesh"), *Pair.Key.ToString()),
					HOLArtVisual::ResolveMesh(nullptr, Pair.Key, NAME_None, FSoftObjectPath()).ToString(), Row.Mesh.ToSoftObjectPath().ToString());
			}
		}
	};
	CheckMeshes(*Buildings, TEXT("Buildings"), static_cast<const FHOLBuildingTypeRow*>(nullptr));
	CheckMeshes(*Sources, TEXT("Sources"), static_cast<const FHOLSourceTypeRow*>(nullptr));

	TestEqual(TEXT("every unit type keeps its constants"), NumUnits, Units->GetRowMap().Num());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
