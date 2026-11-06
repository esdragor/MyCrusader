// Copyright Epic Games, Inc. All Rights Reserved.

#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLGatherRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// 2026-11-05 (Forest Medium, 142 935 sources): the first gatherer with no room at its tree built the massifs of all the
// sources in one step of 1.1 to 1.5 s (a frame of the same length, 12 passes out of 14 of the scenario zz_perf_forest;
// 33 million links, 130 MB, 600 ms; the nearby groves 680 ms). The massifs, their links and their nearby groves must stay
// what they were (checked against a brute-force walk over every pair), with a bound on the work and on the memory.
namespace UE::HeroesOfLiberty::Tests::SourceMassifs
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr double Link = HOLGather::NoRoomSourceDistance;
	constexpr double Nearby = HOLGather::NearbyMassifDistance;
	/** A real forest: ~1.1e-4 trees per cm2 (230 trees within a link of a tree), measured on Forest Medium. */
	constexpr double ForestDensity = 1.1e-4;

	FHOLGatherSource MakeSource(const double X, const double Y, const EHOLResource Resource)
	{
		FHOLGatherSource Source;
		Source.Site.Position = FVector2D(X, Y);
		Source.Site.Radius = 60.f;
		Source.Resource = Resource;
		Source.Remaining = 100;
		return Source;
	}

	/** A square blob of Count sources of a real forest's density, its corner at (X, Y), a tenth of them berries. */
	void AddBlob(TArray<FHOLGatherSource>& Sources, FRandomStream& Random, const double X, const double Y, const int32 Count, double* OutSide = nullptr)
	{
		const double Side = FMath::Sqrt(Count / ForestDensity);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Sources.Add(MakeSource(X + Random.FRand() * Side, Y + Random.FRand() * Side, Index % 10 == 7 ? EHOLResource::Food : EHOLResource::Wood));
		}
		if (OutSide)
		{
			*OutSide = Side;
		}
	}

	/**
	 * Forests and what lies between them: four blobs of wood (with berries among the trees), the second 1 500 cm from the
	 * first (nearby), the third 3 000 cm from the second (not), the fourth 700 cm from the third (one massif with it);
	 * single veins 1 990, 2 000 and 2 010 cm apart and berries 790, 800 and 800.5 cm apart (the thresholds, to the cm),
	 * and 60 m-apart stone.
	 */
	TArray<FHOLGatherSource> MakeForest(const int32 Total)
	{
		FRandomStream Random(20261105);
		TArray<FHOLGatherSource> Sources;
		double Side = 0.;
		const int32 Unit = Total / 8;
		AddBlob(Sources, Random, 0., 0., 3 * Unit, &Side);
		const double FirstSide = Side;
		AddBlob(Sources, Random, FirstSide + 1500., 0., 2 * Unit, &Side);
		const double SecondEnd = FirstSide + 1500. + Side;
		AddBlob(Sources, Random, SecondEnd + 3000., 0., 2 * Unit, &Side);
		const double ThirdEnd = SecondEnd + 3000. + Side;
		AddBlob(Sources, Random, ThirdEnd + 700., 0., Unit, &Side);
		const double Far = 1000000.;
		const EHOLResource Veins[] = { EHOLResource::Stone, EHOLResource::Gold };
		Sources.Add(MakeSource(Far, Far, Veins[0]));
		Sources.Add(MakeSource(Far + 1990., Far, Veins[0]));
		Sources.Add(MakeSource(Far, Far + 5000., Veins[0]));
		Sources.Add(MakeSource(Far + 2000., Far + 5000., Veins[0]));
		Sources.Add(MakeSource(Far, Far + 10000., Veins[0]));
		Sources.Add(MakeSource(Far + 2010., Far + 10000., Veins[0]));
		Sources.Add(MakeSource(-Far, Far, EHOLResource::Food));
		Sources.Add(MakeSource(-Far + 790., Far, EHOLResource::Food));
		Sources.Add(MakeSource(-Far, Far + 5000., EHOLResource::Food));
		Sources.Add(MakeSource(-Far + 800., Far + 5000., EHOLResource::Food));
		Sources.Add(MakeSource(-Far, Far + 10000., EHOLResource::Food));
		Sources.Add(MakeSource(-Far + 800.5, Far + 10000., EHOLResource::Food));
		Sources.Add(MakeSource(-Far, -Far, EHOLResource::Gold));
		Sources.Add(MakeSource(-Far + 6000., -Far, EHOLResource::Gold));
		Sources.Add(MakeSource(-Far + 1000., -Far + 1000., EHOLResource::Stone));
		return Sources;
	}

	/** The sorted links, massifs and nearby groves of the brute-force walk over every pair of sources. */
	struct FReference
	{
		TArray<TArray<int32>> Links;
		TArray<int32> MassifOf;
		TArray<TArray<int32>> NearbyOf;
		int32 NumMassifs = 0;
	};

	FReference MakeReference(const TArray<FHOLGatherSource>& Sources)
	{
		FReference Reference;
		const int32 Num = Sources.Num();
		Reference.Links.SetNum(Num);
		for (int32 A = 0; A < Num; ++A)
		{
			for (int32 B = 0; B < Num; ++B)
			{
				if (A != B && Sources[A].Resource == Sources[B].Resource && FVector2D::DistSquared(Sources[A].Site.Position, Sources[B].Site.Position) <= Link * Link)
				{
					Reference.Links[A].Add(B);
				}
			}
		}
		Reference.MassifOf.Init(INDEX_NONE, Num);
		for (int32 Start = 0; Start < Num; ++Start)
		{
			if (Reference.MassifOf[Start] != INDEX_NONE)
			{
				continue;
			}
			TArray<int32> Pending{ Start };
			Reference.MassifOf[Start] = Reference.NumMassifs;
			while (Pending.Num() > 0)
			{
				for (const int32 Next : Reference.Links[Pending.Pop()])
				{
					if (Reference.MassifOf[Next] == INDEX_NONE)
					{
						Reference.MassifOf[Next] = Reference.NumMassifs;
						Pending.Add(Next);
					}
				}
			}
			++Reference.NumMassifs;
		}
		Reference.NearbyOf.SetNum(Reference.NumMassifs);
		for (int32 A = 0; A < Num; ++A)
		{
			for (int32 B = 0; B < Num; ++B)
			{
				const int32 MassifA = Reference.MassifOf[A], MassifB = Reference.MassifOf[B];
				if (MassifA < MassifB && Sources[A].Resource == Sources[B].Resource && FVector2D::DistSquared(Sources[A].Site.Position, Sources[B].Site.Position) <= Nearby * Nearby)
				{
					Reference.NearbyOf[MassifA].AddUnique(MassifB);
					Reference.NearbyOf[MassifB].AddUnique(MassifA);
				}
			}
		}
		return Reference;
	}

	TArray<int32> Sorted(TConstArrayView<int32> Values)
	{
		TArray<int32> Result(Values);
		Result.Sort();
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSourceMassifsReferenceTest, "HeroesOfLiberty.Gather.MassifsReference", UE::HeroesOfLiberty::Tests::SourceMassifs::Flags)
bool FHOLSourceMassifsReferenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SourceMassifs;
	// 2 400 sources against every pair of them: the links, the massifs (numbered by their first source) and the nearby groves
	// are those of the old walk, and a search through them tries the sources the old one tried (a massif, then its groves).
	const TArray<FHOLGatherSource> Sources = MakeForest(2400);
	const FReference Reference = MakeReference(Sources);
	FHOLSourceMassifs Massifs;
	Massifs.Build(Sources, Link, Nearby);
	TestEqual(TEXT("built for every source"), Massifs.Num(), Sources.Num());
	TestTrue(TEXT("several massifs, groves and thresholds to compare"), Reference.NumMassifs > 8);
	int32 NumLinkMismatches = 0, NumMassifMismatches = 0;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		NumLinkMismatches += Sorted(Massifs.GetLinks(Index)) != Sorted(Reference.Links[Index]) ? 1 : 0;
		NumMassifMismatches += Massifs.GetMassif(Index) != Reference.MassifOf[Index] ? 1 : 0;
	}
	TestEqual(TEXT("sources whose links differ from the brute-force walk"), NumLinkMismatches, 0);
	TestEqual(TEXT("sources whose massif differs from the brute-force walk"), NumMassifMismatches, 0);
	int32 NumNearbyMismatches = 0, NumNearbyPairs = 0;
	for (int32 Massif = 0; Massif < Reference.NumMassifs; ++Massif)
	{
		NumNearbyMismatches += Sorted(Massifs.GetNearbyMassifs(Massif)) != Sorted(Reference.NearbyOf[Massif]) ? 1 : 0;
		NumNearbyPairs += Reference.NearbyOf[Massif].Num();
	}
	TestEqual(TEXT("massifs whose nearby groves differ from the brute-force walk"), NumNearbyMismatches, 0);
	TestTrue(TEXT("some nearby groves among them"), NumNearbyPairs >= 4);

	// The thresholds, to the cm: 800 cm is linked, 800.5 not; 2 000 cm is nearby, 2 010 not (the veins, the berries).
	const int32 NumBase = Sources.Num() - 15;
	TestEqual(TEXT("berries 790 cm apart: one massif"), Massifs.GetMassif(NumBase + 6), Massifs.GetMassif(NumBase + 7));
	TestEqual(TEXT("berries 800 cm apart: one massif"), Massifs.GetMassif(NumBase + 8), Massifs.GetMassif(NumBase + 9));
	TestNotEqual(TEXT("berries 800.5 cm apart: two massifs"), Massifs.GetMassif(NumBase + 10), Massifs.GetMassif(NumBase + 11));
	TestTrue(TEXT("veins 1 990 cm apart: nearby"), Massifs.GetNearbyMassifs(Massifs.GetMassif(NumBase)).Contains(Massifs.GetMassif(NumBase + 1)));
	TestTrue(TEXT("veins 2 000 cm apart: nearby"), Massifs.GetNearbyMassifs(Massifs.GetMassif(NumBase + 2)).Contains(Massifs.GetMassif(NumBase + 3)));
	TestFalse(TEXT("veins 2 010 cm apart: not nearby"), Massifs.GetNearbyMassifs(Massifs.GetMassif(NumBase + 4)).Contains(Massifs.GetMassif(NumBase + 5)));
	TestEqual(TEXT("gold 60 m apart: no nearby grove"), Massifs.GetNearbyMassifs(Massifs.GetMassif(NumBase + 12)).Num(), 0);

	// A search refused everywhere tries the massif, then every nearby grove, once each, start excepted.
	for (const int32 Start : { 0, 700, 1200, 1900, 2399, NumBase, NumBase + 4 })
	{
		TSet<int32> Expected;
		for (int32 Index = 0; Index < Sources.Num(); ++Index)
		{
			const int32 Massif = Reference.MassifOf[Index];
			if (Index != Start && (Massif == Reference.MassifOf[Start] || Reference.NearbyOf[Reference.MassifOf[Start]].Contains(Massif)))
			{
				Expected.Add(Index);
			}
		}
		TArray<int32> Tried;
		Massifs.Search(Start, [&Tried](const int32 Index) { Tried.Add(Index); return false; });
		const TSet<int32> TriedSet(Tried);
		TestEqual(*FString::Printf(TEXT("search from %d: no source tried twice"), Start), TriedSet.Num(), Tried.Num());
		TestTrue(*FString::Printf(TEXT("search from %d tries the sources of its massif and of its groves (%d, expected %d)"), Start, TriedSet.Num(), Expected.Num()),
			TriedSet.Num() == Expected.Num() && TriedSet.Includes(Expected));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSourceMassifsBoundTest, "HeroesOfLiberty.Gather.MassifsBound", UE::HeroesOfLiberty::Tests::SourceMassifs::Flags)
bool FHOLSourceMassifsBoundTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SourceMassifs;
	// 20 000 sources of a real forest's density (a Forest Medium has 142 935): the work of the build and what it keeps in
	// memory are bounded by the number of sources, not by the pairs within reach (the old walk looked at 600 candidates
	// per source for the links and 4 000 for the nearby groves, and kept 230 links per source).
	const TArray<FHOLGatherSource> Sources = MakeForest(20000);
	const int32 Num = Sources.Num();
	FHOLSourceMassifs Massifs;
	Massifs.Build(Sources, Link, Nearby);
	TestEqual(TEXT("built for every source"), Massifs.Num(), Num);
	AddInfo(FString::Printf(TEXT("%d sources: %lld candidates looked at, %lld links held after the build"), Num, Massifs.GetNumBuildVisits(), Massifs.GetNumHeldLinks()));
	TestTrue(*FString::Printf(TEXT("the build looks at no more than 25 candidates per source (%lld for %d sources)"), Massifs.GetNumBuildVisits(), Num),
		Massifs.GetNumBuildVisits() <= 25ll * Num);
	TestTrue(*FString::Printf(TEXT("the build keeps no more than one link per source (%lld held for %d sources)"), Massifs.GetNumHeldLinks(), Num),
		Massifs.GetNumHeldLinks() <= Num);
	// A gatherer finding room at the 40th source it tries: the links it needed, no more.
	int32 Tried = 0;
	const int32 Found = Massifs.Search(1000, [&Tried](const int32) { return ++Tried == 40; });
	TestTrue(TEXT("the search found a source"), Found != INDEX_NONE);
	AddInfo(FString::Printf(TEXT("after a search of 40 sources: %lld links held"), Massifs.GetNumHeldLinks()));
	// A search refused everywhere in the biggest massif (a wood of 100 000 trees refusing a gatherer), its links not yet found.
	FHOLSourceMassifs Refused;
	Refused.Build(Sources, Link, Nearby);
	int32 NumTried = 0;
	const double Start = FPlatformTime::Seconds();
	Refused.Search(1000, [&NumTried](const int32) { ++NumTried; return false; });
	AddInfo(FString::Printf(TEXT("a search refused by all of its %d sources: %.1f ms, %lld links found"), NumTried, (FPlatformTime::Seconds() - Start) * 1000., Refused.GetNumHeldLinks()));
	TestTrue(*FString::Printf(TEXT("a search stopping at its 40th source holds no more than one link per source (%lld held)"), Massifs.GetNumHeldLinks()),
		Massifs.GetNumHeldLinks() <= Num);
	return true;
}

#endif
