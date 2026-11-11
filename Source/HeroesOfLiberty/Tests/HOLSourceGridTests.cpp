// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceGrid.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLGatherRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// 2026-11-05 (Forest Medium, 142 935 sources): the AI's summary and the gatherers' stand points walked every source at each
// call (Think 33 ms against 1.3 on a Plain Medium; one step of 1.7 s). Both now go through the grid of the sources; the
// result must be the walk over all of them, with a bound on the entries visited.
namespace UE::HeroesOfLiberty::Tests::SourceGrid
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr int32 GridSide = 142;
	constexpr int32 NumSources = GridSide * GridSide;	// 20 164, a Forest Medium is 142 935
	constexpr double TestSpacing = 150.;

	/**
	 * Twenty thousand sources on a grid 21 km wide, a corner of it in the negative: the four resources interleaved, ties of
	 * distance everywhere (a regular grid), some used up, some of an index not used, some another slot's farm field, some
	 * slot 0's own, the odd rows shifted so that not everything is on a line.
	 */
	TArray<FHOLSource> MakeSources()
	{
		TArray<FHOLSource> Sources;
		Sources.Reserve(NumSources);
		for (int32 Index = 0; Index < NumSources; ++Index)
		{
			const int32 Column = Index % GridSide, Row = Index / GridSide;
			FHOLSource& Source = Sources.AddDefaulted_GetRef();
			Source.TypeIndex = Index % 13 == 5 ? FHOLSource::NoType : 0;
			Source.Resource = static_cast<EHOLResource>((Column + 2 * Row) % 4);
			Source.Location = FVector(Column * TestSpacing - 10000. + (Row % 2 == 1 ? (Index * 37) % 50 : 0), Row * TestSpacing - 10000., 0.);
			Source.Radius = 40.f + (Index % 5) * 10.f;
			Source.Remaining = Index % 11 == 3 ? 0 : 100;
			Source.OwnerSlot = Index % 17 == 4 ? 1 : (Index % 19 == 7 ? 0 : FHOLSource::NoOwner);
		}
		return Sources;
	}

	void FillGrid(const TArray<FHOLSource>& Sources, FHOLSourceGrid& Grid)
	{
		for (int32 Index = 0; Index < Sources.Num(); ++Index)
		{
			Grid.Add(Index, Sources[Index].Resource, FVector2D(Sources[Index].Location), Sources[Index].Radius);
		}
	}

	FHOLAIDeposit MakeDeposit(const double X, const double Y, const double Half, const float Yaw, const uint8 Owner, TArray<EHOLResource> Accepts)
	{
		FHOLAIDeposit Deposit;
		Deposit.Footprint.Centre = FVector2D(X, Y);
		Deposit.Footprint.HalfExtent = FVector2D(Half, Half * 0.6);
		Deposit.Footprint.Yaw = Yaw;
		Deposit.OwnerPlayer = Owner;
		Deposit.Accepts = MoveTemp(Accepts);
		return Deposit;
	}

	FString Describe(const FHOLAISummary& Summary)
	{
		FString Out;
		for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
		{
			Out += TEXT("|");
			for (const FHOLAISourceNear& Near : Summary.Get(Resource).Sources)
			{
				Out += FString::Printf(TEXT(" %d:%.3f:%d:%.0f,%.0f"), Near.SourceIndex, Near.DepositDistance, Near.Remaining, Near.Location.X, Near.Location.Y);
			}
		}
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSourceGridAISummaryTest, "HeroesOfLiberty.Resources.SourceGridAISummary", UE::HeroesOfLiberty::Tests::SourceGrid::Flags)
bool FHOLSourceGridAISummaryTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SourceGrid;
	const TArray<FHOLSource> Sources = MakeSources();
	FHOLSourceGrid Grid;
	FillGrid(Sources, Grid);
	TestEqual(TEXT("every source is in the grid"), Grid.Num(), NumSources);

	const TArray<EHOLResource> All = { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold };
	const TArray<FHOLAIDeposit> Deposits = {
		MakeDeposit(0., 0., 400., 0.f, 0, All),                       // slot 0's centre
		MakeDeposit(3000., -2000., 150., 30.f, 0, { EHOLResource::Wood }),  // its lumber camp
		MakeDeposit(-2500., 1500., 200., 70.f, 0, { EHOLResource::Food }),  // its mill
		MakeDeposit(30000., 30000., 300., 10.f, 0, { EHOLResource::Stone }), // a quarry far off the sources
		MakeDeposit(6000., 6000., 400., 0.f, 1, All),                  // slot 1's centre
		MakeDeposit(-7000., -7000., 150., 45.f, 1, { EHOLResource::Gold }),
	};
	const auto Known = [](const FHOLSource&) { return true; };
	const auto Odd = [](const FHOLSource& Source) { return FMath::RoundToInt(Source.Location.X) % 2 == 0; };	// a stand-in for the fog's known cells: half

	for (const int32 Player : { 0, 1 })
	{
		for (const int32 Max : { HOLAISummary::DefaultMaxSourcesPerResource, 3 })
		{
			for (const bool bFog : { false, true })
			{
				const FString Label = FString::Printf(TEXT("slot %d, %d per resource, %s"), Player, Max, bFog ? TEXT("half the sources unknown") : TEXT("all known"));
				// The reference: the walk over every source (the fog is the same sources used up, as Read did).
				TArray<FHOLSource> Seen = Sources;
				if (bFog)
				{
					for (FHOLSource& Source : Seen)
					{
						if (!Odd(Source))
						{
							Source.Remaining = 0;
						}
					}
				}
				const FHOLAISummary Walked = HOLAISummary::Make(Player, {}, Seen, Deposits, {}, Max);
				TArray<int32> Candidates;
				const int32 Visits = bFog ? HOLAISummary::FindCandidateSources(Grid, Sources, Player, Deposits, Max, Odd, Candidates)
					: HOLAISummary::FindCandidateSources(Grid, Sources, Player, Deposits, Max, Known, Candidates);
				const FHOLAISummary Found = HOLAISummary::Make(Player, {}, Sources, Deposits, {}, Max, {}, {}, {}, {}, 0, {}, false, FHOLCampTable(), &Candidates);
				const FString Expected = Describe(Walked);
				TestTrue(*FString::Printf(TEXT("%s: something is kept (%s)"), *Label, *Expected.Left(80)), Walked.Wood.Sources.Num() > 0);
				TestEqual(*FString::Printf(TEXT("%s: the same sources, distances and order as the walk over all %d"), *Label, NumSources), Describe(Found), Expected);
				TestTrue(*FString::Printf(TEXT("%s: %d entries visited of %d (at most %d)"), *Label, Visits, NumSources, NumSources / 10), Visits <= NumSources / 10);
			}
		}
	}

	// Each search alone: the nearest of one resource to one deposit, against the sort of all of them.
	for (const FHOLAIDeposit& Deposit : Deposits)
	{
		for (const EHOLResource Resource : All)
		{
			TArray<TPair<double, int32>> Order;
			for (int32 Index = 0; Index < NumSources; ++Index)
			{
				if (Sources[Index].Resource == Resource && Sources[Index].IsStanding())
				{
					Order.Emplace(HOLGather::GetDepositDistance(FVector2D(Sources[Index].Location), Deposit.Footprint), Index);
				}
			}
			Order.Sort([](const TPair<double, int32>& A, const TPair<double, int32>& B) { return A.Key != B.Key ? A.Key < B.Key : A.Value < B.Value; });
			TArray<int32> Nearest;
			const int32 Visits = Grid.FindNearest(Deposit.Footprint, Resource, 8, [&Sources](const int32 Index) { return Sources[Index].IsStanding(); }, Nearest);
			TArray<int32> ExpectedNearest;
			for (int32 Index = 0; Index < 8; ++Index)
			{
				ExpectedNearest.Add(Order[Index].Value);
			}
			const FString Label = FString::Printf(TEXT("deposit at %.0f,%.0f, resource %d"), Deposit.Footprint.Centre.X, Deposit.Footprint.Centre.Y, static_cast<int32>(Resource));
			TestTrue(*FString::Printf(TEXT("%s: the 8 nearest in order"), *Label), Nearest == ExpectedNearest);
			TestTrue(*FString::Printf(TEXT("%s: %d entries visited (at most %d)"), *Label, Visits, NumSources / 40), Visits <= NumSources / 40);
		}
	}

	// Fewer than asked for, and none: the whole resource is met and no more.
	TArray<int32> Few;
	FHOLSourceGrid Small;
	Small.Add(0, EHOLResource::Gold, FVector2D(-1500., -2500.), 40.f);
	Small.Add(1, EHOLResource::Gold, FVector2D(200000., 0.), 40.f);
	Small.FindNearest(Deposits[0].Footprint, EHOLResource::Gold, 8, [](int32) { return true; }, Few);
	TestTrue(TEXT("two gold sources, the far one too"), Few == TArray<int32>({ 0, 1 }));
	Small.FindNearest(Deposits[0].Footprint, EHOLResource::Wood, 8, [](int32) { return true; }, Few);
	TestEqual(TEXT("no wood: nothing"), Few.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSourceGridStandSitesTest, "HeroesOfLiberty.Resources.SourceGridStandSites", UE::HeroesOfLiberty::Tests::SourceGrid::Flags)
bool FHOLSourceGridStandSitesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SourceGrid;
	const TArray<FHOLSource> Sources = MakeSources();
	FHOLSourceGrid Grid;
	FillGrid(Sources, Grid);

	// The numbers of UHOLGatherProcessor::FindStandPoint, as it asked at each row: a body of 40 cm, spots 20 cm off, 80 cm apart.
	constexpr double SpotOffset = 60., Standing = 100.;
	const double Reaches[] = { 700., 1000., 1300. };
	const FVector Centres[] = { FVector(0., 0., 0.), FVector(-9950., -10000., 0.), FVector(1234., -4321., 0.), FVector(11000., 11000., 0.), FVector(90000., 0., 0.) };
	int32 Cases = 0, Met = 0;
	for (const FVector& Centre : Centres)
	{
		double SitesReach = -1.;
		for (const double Reach : Reaches)
		{
			// The walk over every source, as FindStandPoint did.
			TArray<FHOLGatherSite> Walked;
			for (const FHOLSource& Other : Sources)
			{
				const double Within = Reach + Other.Radius + SpotOffset + Standing;
				const double Before = SitesReach + Other.Radius + SpotOffset + Standing;
				const double DistanceSquared = FVector::DistSquared2D(Other.Location, Centre);
				if (Other.IsStanding() && DistanceSquared <= FMath::Square(Within) && (SitesReach < 0. || DistanceSquared > FMath::Square(Before)))
				{
					Walked.Add(FHOLGatherSite{ FVector2D(Other.Location), Other.Radius });
				}
			}
			TArray<FHOLGatherSite> Found;
			const int32 Visits = HOLGather::FindStandSites(Grid, Sources, Centre, Reach, SitesReach, SpotOffset, Standing, [&Found](const FHOLGatherSite& Site) { Found.Add(Site); });
			const FString Label = FString::Printf(TEXT("round %.0f,%.0f, reach %.0f after %.0f"), Centre.X, Centre.Y, Reach, SitesReach);
			bool bSame = Walked.Num() == Found.Num();
			for (int32 Index = 0; bSame && Index < Walked.Num(); ++Index)
			{
				bSame = Walked[Index].Position == Found[Index].Position && Walked[Index].Radius == Found[Index].Radius;
			}
			TestTrue(*FString::Printf(TEXT("%s: the same %d sites in the same order as the walk (%d found)"), *Label, Walked.Num(), Found.Num()), bSame);
			TestTrue(*FString::Printf(TEXT("%s: %d entries visited of %d (at most %d)"), *Label, Visits, NumSources, NumSources / 10), Visits <= NumSources / 10);
			++Cases;
			Met += Walked.Num();
			SitesReach = Reach;
		}
	}
	TestTrue(*FString::Printf(TEXT("%d cases met %d sites in all: the comparison is not empty"), Cases, Met), Met > 100);
	return true;
}

#endif
