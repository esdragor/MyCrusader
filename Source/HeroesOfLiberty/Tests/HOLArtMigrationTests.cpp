// Copyright Epic Games, Inc. All Rights Reserved.

// D925 (AR0-d1): the one-shot migration of the art out of the CSV rows into /Game/Art/<Units|Buildings|Sources>/<Type>/
// DA_Art_<Type> keeps every type as it was: AR0-d2 (D925) removed the visual columns of the unit table, so the reference is
// FROZEN (HOLArtMigrationFrozen.inl, the CSV's values before the removal, by type and field) and compared to the constants
// the game registers for the type, and to the arrows' projectile and smoke; the mesh of every building and every source
// is still compared to its row. Mass simulation test (D838): the Mass type registration reads them.

#include "Art/HOLArtVisual.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "UObject/UnrealType.h"

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

	struct FFrozenVisual
	{
		const TCHAR* Type;
		const TCHAR* Field;
		const TCHAR* Value;
	};

#include "HOLArtMigrationFrozen.inl"

	/** The constants registered for the type: its row's (the visual fields neutral), dressed by its art asset. */
	FHOLUnitTypeConstants Registered(const FHOLUnitTypeRow& Row, const FName TypeName)
	{
		FHOLUnitTypeConstants Constants = Row.ToConstants(TypeName);
		HOLArtVisual::Apply(nullptr, Constants, TypeName, NAME_None);
		return Constants;
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

	TMap<FName, FHOLUnitTypeConstants> Constants;
	for (const TPair<FName, uint8*>& Pair : Units->GetRowMap())
	{
		if (LoadArt(*this, TEXT("Units"), Pair.Key))
		{
			Constants.Add(Pair.Key, Registered(*reinterpret_cast<const FHOLUnitTypeRow*>(Pair.Value), Pair.Key));
		}
	}

	const UScriptStruct* Struct = FHOLUnitTypeConstants::StaticStruct();
	TSet<FName> FrozenTypes;
	int32 NumFields = 0;
	for (const FFrozenVisual& Frozen : FrozenVisuals)
	{
		const FName TypeName(Frozen.Type);
		const FString Field(Frozen.Field);
		const FString Label = FString::Printf(TEXT("%s: %s"), Frozen.Type, Frozen.Field);
		FrozenTypes.Add(TypeName);
		if (Field == TEXT("Projectile") || Field == TEXT("MuzzleSmoke"))
		{
			FSoftObjectPath Expected;
			if (*Frozen.Value)
			{
				TBaseStructure<FSoftObjectPath>::Get()->ImportText(Frozen.Value, &Expected, nullptr, PPF_None, GWarn, TEXT("SoftObjectPath"));
			}
			const FSoftObjectPath Actual = Field == TEXT("Projectile")
				? HOLArtVisual::ResolveProjectile(nullptr, TypeName, NAME_None, FSoftObjectPath())
				: HOLArtVisual::ResolveMuzzleSmoke(nullptr, TypeName, NAME_None, FSoftObjectPath());
			TestEqual(Label, Actual.ToString(), Expected.ToString());
			++NumFields;
			continue;
		}
		const FHOLUnitTypeConstants* Actual = Constants.Find(TypeName);
		const FProperty* Property = Struct->FindPropertyByName(FName(Frozen.Field));
		if (!TestNotNull(*FString::Printf(TEXT("%s: registered"), *Label), Actual) || !TestNotNull(*FString::Printf(TEXT("%s: a constant"), *Label), Property))
		{
			continue;
		}
		FHOLUnitTypeConstants Expected;
		if (*Frozen.Value && !Property->ImportText_Direct(Frozen.Value, Property->ContainerPtrToValuePtr<void>(&Expected), nullptr, PPF_None))
		{
			AddError(FString::Printf(TEXT("%s: the frozen text '%s' does not import"), *Label, Frozen.Value));
			continue;
		}
		if (!Property->Identical_InContainer(&Expected, Actual))
		{
			FString ExpectedText;
			FString ActualText;
			Property->ExportTextItem_InContainer(ExpectedText, &Expected, nullptr, nullptr, PPF_None);
			Property->ExportTextItem_InContainer(ActualText, Actual, nullptr, nullptr, PPF_None);
			AddError(FString::Printf(TEXT("%s: %s, frozen %s"), *Label, *ActualText, *ExpectedText));
		}
		++NumFields;
	}
	for (const TPair<FName, uint8*>& Pair : Units->GetRowMap())
	{
		TestTrue(FString::Printf(TEXT("%s: has its frozen reference"), *Pair.Key.ToString()), FrozenTypes.Contains(Pair.Key));
	}
	TestEqual(TEXT("every frozen field is checked"), NumFields, static_cast<int32>(UE_ARRAY_COUNT(FrozenVisuals)));

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

	TestEqual(TEXT("every unit type keeps its constants"), Constants.Num(), Units->GetRowMap().Num());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
