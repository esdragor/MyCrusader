// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceNavComponent.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// The numbers of UHOLSourceNavComponent, written again here on purpose: a source's forbidden box is the cylinder
	// (from 100 cm under the ground, 400 cm high, of the source's radius) widened by 100 cm.
	constexpr double Bottom = -100.;
	constexpr double Height = 400.;
	constexpr double Margin = 100.;
	constexpr double Radius = 40.;
	constexpr int32 GridSide = 142;
	constexpr double Spacing = 150.;
	constexpr int32 NumSources = GridSide * GridSide;	// 20 164, a Forest Medium is 142 935

	FVector LocationOf(const int32 Index)
	{
		return FVector((Index % GridSide) * Spacing - 10000., (Index / GridSide) * Spacing - 10000., 0.);
	}

	FBox BoxOf(const int32 Index)
	{
		const FVector Base = LocationOf(Index) + FVector(0., 0., Bottom);
		return FBox(Base - FVector(Radius + Margin, Radius + Margin, Margin), Base + FVector(Radius + Margin, Radius + Margin, Height + Margin));
	}

	/** The union of the boxes of the sources standing, found the long way. */
	FBox ExpectedBounds(const TSet<int32>& Removed)
	{
		FBox Result(ForceInit);
		for (int32 Index = 0; Index < NumSources; ++Index)
		{
			if (!Removed.Contains(Index))
			{
				Result += BoxOf(Index);
			}
		}
		return Result;
	}

	bool SameBox(const FBox& A, const FBox& B)
	{
		return (A.IsValid != 0) == (B.IsValid != 0) && (!A.IsValid || (A.Min.Equals(B.Min, 0.01) && A.Max.Equals(B.Max, 0.01)));
	}
}

// 2026-11-05 (Forest Medium, 142 935 sources, load of 100 to 148 s): adding a source must not scan all the others.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSourceNavBoundsTest, "HeroesOfLiberty.Resources.SourceNavBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLSourceNavBoundsTest::RunTest(const FString& Parameters)
{
	UHOLSourceNavComponent* Nav = NewObject<UHOLSourceNavComponent>(GetTransientPackage());
	if (!TestNotNull(TEXT("the component is made"), Nav))
	{
		return false;
	}
	TestFalse(TEXT("no source: no bounds"), Nav->GetCylindersBounds().IsValid != 0);

	// 1. Adding: the bounds are exact and no add scans the cylinders already there.
	for (int32 Index = 0; Index < NumSources; ++Index)
	{
		Nav->AddSource(Index, LocationOf(Index), Radius);
	}
	TestEqual(TEXT("every source has its cylinder"), Nav->GetNumCylinders(), NumSources);
	TestTrue(TEXT("the bounds after the adds are the union of the boxes"), SameBox(Nav->GetCylindersBounds(), ExpectedBounds({})));
	TestTrue(*FString::Printf(TEXT("adding %d sources scans at most %d cylinders in all (%lld scanned)"), NumSources, NumSources, Nav->GetNumBoundsVisits()),
		Nav->GetNumBoundsVisits() <= NumSources);

	// 2. Removing sources inside the bounds changes nothing; the exact bounds cost one scan when asked for, none per removal.
	const int64 VisitsBefore = Nav->GetNumBoundsVisits();
	TSet<int32> Removed;
	for (int32 Index = 0; Index < NumSources; ++Index)
	{
		const int32 Column = Index % GridSide, Row = Index / GridSide;
		if (Column > 0 && Column < GridSide - 1 && Row > 0 && Row < GridSide - 1 && (Column + Row) % 3 == 0)
		{
			Nav->RemoveSource(Index);
			Removed.Add(Index);
		}
	}
	TestTrue(TEXT("some sources were removed"), Removed.Num() > 1000);
	TestEqual(TEXT("the removals themselves scan no cylinder"), Nav->GetNumBoundsVisits() - VisitsBefore, int64(0));
	TestEqual(TEXT("the cylinders left"), Nav->GetNumCylinders(), NumSources - Removed.Num());
	TestTrue(TEXT("the bounds after inner removals are still the union of the boxes left"), SameBox(Nav->GetCylindersBounds(), ExpectedBounds(Removed)));
	TestTrue(*FString::Printf(TEXT("%d inner removals and the bounds asked for scan at most %d cylinders in all (%lld scanned)"), Removed.Num(), NumSources, Nav->GetNumBoundsVisits() - VisitsBefore),
		Nav->GetNumBoundsVisits() - VisitsBefore <= NumSources);

	// 3. Removing the sources that make the bounds: they shrink to the union of what is left.
	for (const int32 Corner : { 0, GridSide - 1, NumSources - GridSide, NumSources - 1 })
	{
		Nav->RemoveSource(Corner);
		Removed.Add(Corner);
	}
	TestTrue(TEXT("the bounds after the corners are gone shrink to the union of the boxes left"), SameBox(Nav->GetCylindersBounds(), ExpectedBounds(Removed)));
	Nav->RemoveSource(NumSources + 5);	// unknown index: nothing
	TestTrue(TEXT("removing an unknown source changes nothing"), SameBox(Nav->GetCylindersBounds(), ExpectedBounds(Removed)));

	// 4. Adding a source again (same index, elsewhere) moves its cylinder: the bounds follow.
	const FVector Far(30000., 0., 50.);
	Nav->AddSource(GridSide + 1, Far, Radius);
	FBox Rebuilt(ForceInit);
	for (int32 Index = 0; Index < NumSources; ++Index)
	{
		if (!Removed.Contains(Index) && Index != GridSide + 1)
		{
			Rebuilt += BoxOf(Index);
		}
	}
	const FVector FarBase = Far + FVector(0., 0., Bottom);
	Rebuilt += FBox(FarBase - FVector(Radius + Margin, Radius + Margin, Margin), FarBase + FVector(Radius + Margin, Radius + Margin, Height + Margin));
	TestTrue(TEXT("a source added again at another place: the bounds are the union with its new box"), SameBox(Nav->GetCylindersBounds(), Rebuilt));
	TestEqual(TEXT("it still counts once"), Nav->GetNumCylinders(), NumSources - Removed.Num());

	// 5. The areas dirtied at registration: every cylinder's box is inside one of them and they are far fewer than the sources.
	TArray<FBox> Areas;
	Nav->GetRegistrationAreas(Areas);
	TestTrue(*FString::Printf(TEXT("fewer dirty areas than sources (%d for %d)"), Areas.Num(), Nav->GetNumCylinders()), Areas.Num() > 0 && Areas.Num() <= 400);
	int32 Uncovered = 0;
	for (int32 Index = 0; Index < NumSources; ++Index)
	{
		if (Removed.Contains(Index) || Index == GridSide + 1)
		{
			continue;
		}
		const FBox Box = BoxOf(Index);
		Uncovered += Areas.ContainsByPredicate([&Box](const FBox& Area) { return Area.IsInsideOrOn(Box.Min) && Area.IsInsideOrOn(Box.Max); }) ? 0 : 1;
	}
	TestEqual(TEXT("every standing cylinder's box lies inside a registration area"), Uncovered, 0);
	return true;
}

#endif
