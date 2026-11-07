// The generic decor PLAN of a biome, alone (BiomeInProgress.md B3, GDD D651 to D656): which pieces of generic scenery (tufts, flowers,
// bushes...) a match lays for a biome, where, how big and what colour, from the layout seed, the bounds, the points to keep clear and the
// densities the map asks for. Every expectation comes from the chain that exists (HOLBiome::GenericKinds / GenericCount, HOLDecor::Requested /
// Transforms, which HOLDecorRules_Test.as holds to the Python's own numbers), never from retyped places. The look of a kind (shape, size,
// colour) is a provisional table judged on captures (B4), so the tests hold it to the rules (nothing a player could take for a resource),
// not to its numbers.

const double PlanHalf = 20000.0;
const int64 PlanSeed = 1;

TArray<FVector2D> PlanAvoid()
{
	TArray<FVector2D> R;
	R.Add(FVector2D(0.0, 0.0));
	R.Add(FVector2D(6000.0, 0.0));
	R.Add(FVector2D(-6000.0, 4000.0));
	R.Add(FVector2D(12000.0, -12000.0));
	R.Add(FVector2D(-15000.0, -15000.0));
	R.Add(FVector2D(1500.0, 1200.0));
	R.Add(FVector2D(-9000.0, 9000.0));
	R.Add(FVector2D(16000.0, 15000.0));
	return R;
}

// The densities a map asks for, slot by slot (per hectare): the biome's first kind, second kind, third kind.
TArray<double> PlanReference()
{
	TArray<double> R;
	R.Add(40.0);
	R.Add(25.0);
	R.Add(10.0);
	return R;
}

int PlanCountOfKind(const TArray<FHOLGenericDecor>& Plan, int Kind)
{
	int N = 0;
	for (int I = 0; I < Plan.Num(); I++)
	{
		if (Plan[I].Kind == Kind)
		{
			N++;
		}
	}
	return N;
}

// B-Q6: the biome's generic decor REPLACES the map's: every piece is of a kind the biome lays, and a kind the biome lays is laid.
void Test_PlanOnlyTheBiomesKinds(FUnitTest& T)
{
	for (int Biome = 0; Biome < HOLBiome::Count; Biome++)
	{
		TArray<int> Kinds = HOLBiome::GenericKinds(Biome);
		TArray<FHOLGenericDecor> Plan = HOLBiomeDecor::Plan(Biome, PlanSeed, PlanHalf, PlanAvoid(), PlanReference());
		T.AssertTrue(Plan.Num() > 0, f"biome {Biome} lays some decor");
		int Counted = 0;
		for (int Slot = 0; Slot < Kinds.Num(); Slot++)
		{
			int Laid = PlanCountOfKind(Plan, Kinds[Slot]);
			T.AssertTrue(Laid > 0, f"biome {Biome}: its kind {Kinds[Slot]} is laid");
			Counted += Laid;
		}
		T.AssertEquals(Plan.Num(), Counted, f"biome {Biome}: nothing but its own kinds is laid");
	}
	// The snow lays no flower, no green tuft, no leafy bush (it replaces the fresh grass's decor).
	TArray<FHOLGenericDecor> Snow = HOLBiomeDecor::Plan(HOLBiome::Snow, PlanSeed, PlanHalf, PlanAvoid(), PlanReference());
	T.AssertEquals(0, PlanCountOfKind(Snow, HOLBiome::KindFlower), "snow: no flower");
	T.AssertEquals(0, PlanCountOfKind(Snow, HOLBiome::KindTuft), "snow: no green tuft");
	T.AssertEquals(0, PlanCountOfKind(Snow, HOLBiome::KindBush), "snow: no leafy bush");
}

// The number of pieces of a slot is the biome's density of the number the map asks for (the table's own rule), not less when the map is
// open enough to hold them: the EFFECT the biome has on the count.
void Test_PlanCountFollowsTheBiomeDensity(FUnitTest& T)
{
	TArray<double> Reference = PlanReference();
	for (int Biome = 0; Biome < HOLBiome::Count; Biome++)
	{
		TArray<int> Kinds = HOLBiome::GenericKinds(Biome);
		TArray<FHOLGenericDecor> Plan = HOLBiomeDecor::Plan(Biome, PlanSeed, PlanHalf, PlanAvoid(), Reference);
		for (int Slot = 0; Slot < Kinds.Num(); Slot++)
		{
			int Expected = HOLBiome::GenericCount(Biome, HOLDecor::Requested(PlanHalf, Reference[Slot]));
			T.AssertEquals(Expected, PlanCountOfKind(Plan, Kinds[Slot]), f"biome {Biome}, slot {Slot}: the biome's density of what the map asks");
		}
	}
	// The snow is sparser than the fresh grass for the same map (the table says 60 against 100 percent).
	T.AssertTrue(HOLBiome::At(HOLBiome::Snow).GenericDensityPercent < HOLBiome::At(HOLBiome::FreshGrass).GenericDensityPercent, "the table: the snow is sparser");
	TArray<FHOLGenericDecor> Snow = HOLBiomeDecor::Plan(HOLBiome::Snow, PlanSeed, PlanHalf, PlanAvoid(), Reference);
	TArray<FHOLGenericDecor> Grass = HOLBiomeDecor::Plan(HOLBiome::FreshGrass, PlanSeed, PlanHalf, PlanAvoid(), Reference);
	T.AssertTrue(Snow.Num() < Grass.Num(), f"the snow lays fewer pieces ({Snow.Num()}) than the fresh grass ({Grass.Num()})");
}

// A biome is sparser, never bare: a map that asks for a little still gets one piece of each kind (HOLBiome::GenericCount's promise), and a
// map that asks for none of a slot gets none of it.
void Test_PlanNeverBareNeverMoreThanAsked(FUnitTest& T)
{
	TArray<double> Little;
	Little.Add(10.0);
	Little.Add(10.0);
	Little.Add(10.0);
	TArray<FVector2D> NoAvoid;
	for (int Biome = 0; Biome < HOLBiome::Count; Biome++)
	{
		TArray<int> Kinds = HOLBiome::GenericKinds(Biome);
		TArray<FHOLGenericDecor> Plan = HOLBiomeDecor::Plan(Biome, PlanSeed, 2000.0, NoAvoid, Little);
		for (int Slot = 0; Slot < Kinds.Num(); Slot++)
		{
			T.AssertTrue(PlanCountOfKind(Plan, Kinds[Slot]) >= 1, f"biome {Biome}, slot {Slot}: at least one piece on a small ask");
		}
	}
	TArray<double> OnlyFirst;
	OnlyFirst.Add(40.0);
	TArray<int> Kinds = HOLBiome::GenericKinds(HOLBiome::FreshGrass);
	TArray<FHOLGenericDecor> Plan = HOLBiomeDecor::Plan(HOLBiome::FreshGrass, PlanSeed, PlanHalf, PlanAvoid(), OnlyFirst);
	T.AssertEquals(Plan.Num(), PlanCountOfKind(Plan, Kinds[0]), "a map that asks for the first slot only gets that kind");
	TArray<double> None;
	T.AssertEquals(0, HOLBiomeDecor::Plan(HOLBiome::Snow, PlanSeed, PlanHalf, PlanAvoid(), None).Num(), "a map that asks for nothing gets nothing");
}

// The server and every client lay the same decor (D468): the same call twice is the same plan, piece for piece, and another seed is another
// plan.
void Test_PlanDeterministicPerSeed(FUnitTest& T)
{
	for (int Biome = 0; Biome < HOLBiome::Count; Biome++)
	{
		TArray<FHOLGenericDecor> A = HOLBiomeDecor::Plan(Biome, PlanSeed, PlanHalf, PlanAvoid(), PlanReference());
		TArray<FHOLGenericDecor> B = HOLBiomeDecor::Plan(Biome, PlanSeed, PlanHalf, PlanAvoid(), PlanReference());
		T.AssertEquals(A.Num(), B.Num(), f"biome {Biome}: same count twice");
		bool bSame = A.Num() == B.Num();
		for (int I = 0; bSame && I < A.Num(); I++)
		{
			bSame = A[I].Kind == B[I].Kind && A[I].X == B[I].X && A[I].Y == B[I].Y && A[I].YawDegrees == B[I].YawDegrees
				&& A[I].ScaleXPercent == B[I].ScaleXPercent && A[I].ScaleZPercent == B[I].ScaleZPercent;
		}
		T.AssertTrue(bSame, f"biome {Biome}: the same plan piece for piece");
	}
	TArray<FHOLGenericDecor> One = HOLBiomeDecor::Plan(HOLBiome::FreshGrass, 1, PlanHalf, PlanAvoid(), PlanReference());
	TArray<FHOLGenericDecor> Two = HOLBiomeDecor::Plan(HOLBiome::FreshGrass, 2, PlanHalf, PlanAvoid(), PlanReference());
	bool bDiffers = One.Num() != Two.Num();
	for (int I = 0; !bDiffers && I < One.Num(); I++)
	{
		bDiffers = One[I].X != Two[I].X || One[I].Y != Two[I].Y;
	}
	T.AssertTrue(bDiffers, "another layout seed lays another decor");
}

// The places ARE the Python-checked chain's: the pieces of a slot are, in order, the places HOLDecor::Transforms draws for the slot's salt
// and the density that asks for that many (rounded to the centimetre), so the plan inherits the agreement with decor_transforms.
void Test_PlanPlacesAreTheTransforms(FUnitTest& T)
{
	TArray<double> Reference = PlanReference();
	for (int Biome = 0; Biome < HOLBiome::Count; Biome++)
	{
		TArray<int> Kinds = HOLBiome::GenericKinds(Biome);
		TArray<FHOLGenericDecor> Plan = HOLBiomeDecor::Plan(Biome, PlanSeed, PlanHalf, PlanAvoid(), Reference);
		for (int Slot = 0; Slot < Kinds.Num(); Slot++)
		{
			int Wanted = HOLBiome::GenericCount(Biome, HOLDecor::Requested(PlanHalf, Reference[Slot]));
			TArray<FHOLDecorPlace> Places = HOLDecor::Transforms(PlanSeed, PlanHalf, PlanAvoid(), HOLBiomeDecor::DensityForCount(PlanHalf, Wanted), HOLBiomeDecor::SaltOfSlot(Slot));
			int Seen = 0;
			for (int I = 0; I < Plan.Num(); I++)
			{
				if (Plan[I].Kind != Kinds[Slot])
				{
					continue;
				}
				if (Seen < Places.Num())
				{
					T.AssertTrue(Math::Abs(double(Plan[I].X) - Places[Seen].X) <= 1.0 && Math::Abs(double(Plan[I].Y) - Places[Seen].Y) <= 1.0,
						f"biome {Biome}, slot {Slot}, piece {Seen}: where the Transforms put it");
					T.AssertTrue(Plan[I].YawDegrees >= int(360.0 * Places[Seen].Turn) - 1 && Plan[I].YawDegrees <= int(360.0 * Places[Seen].Turn) + 1,
						f"biome {Biome}, slot {Slot}, piece {Seen}: turned as the Transforms turn it");
				}
				Seen++;
			}
			T.AssertEquals(Places.Num(), Seen, f"biome {Biome}, slot {Slot}: as many pieces as the Transforms lay");
		}
	}
}

// The promises of a plan: inside the bounds, none within the clear distance of a point to keep clear (a base, a source, a rock), no game
// object (D655), turn and size inside their ranges, the look of its kind.
void Test_PlanKeepsItsPromises(FUnitTest& T)
{
	TArray<FVector2D> Avoid = PlanAvoid();
	for (int Biome = 0; Biome < HOLBiome::Count; Biome++)
	{
		TArray<FHOLGenericDecor> Plan = HOLBiomeDecor::Plan(Biome, PlanSeed, PlanHalf, Avoid, PlanReference());
		for (int I = 0; I < Plan.Num(); I++)
		{
			const FHOLGenericDecor& P = Plan[I];
			T.AssertTrue(P.X >= -int(PlanHalf) && P.X <= int(PlanHalf) && P.Y >= -int(PlanHalf) && P.Y <= int(PlanHalf), f"biome {Biome}, piece {I}: inside the bounds");
			for (int A = 0; A < Avoid.Num(); A++)
			{
				double OffX = double(P.X) - Avoid[A].X;
				double OffY = double(P.Y) - Avoid[A].Y;
				T.AssertTrue(Math::Sqrt(OffX * OffX + OffY * OffY) >= HOLDecor::ClearCm - 1.5, f"biome {Biome}, piece {I}: clear of the avoid point {A}");
			}
			T.AssertTrue(P.YawDegrees >= 0 && P.YawDegrees < 360, f"biome {Biome}, piece {I}: turn in [0, 360)");
			T.AssertFalse(P.bGameObject, f"biome {Biome}, piece {I}: scenery, never a game object (D655)");
			FHOLDecorLook Look = HOLBiomeDecor::Look(P.Kind);
			T.AssertEquals(Look.Shape, P.Shape, f"biome {Biome}, piece {I}: the shape of its kind");
			T.AssertTrue(P.R == Look.R && P.G == Look.G && P.B == Look.B, f"biome {Biome}, piece {I}: the colour of its kind");
			// The size is 0.75 to 1.25 of the kind's own (a drawn grow), on every axis the same grow.
			T.AssertTrue(P.ScaleXPercent * 100 >= Look.ScaleXPercent * 74 && P.ScaleXPercent * 100 <= Look.ScaleXPercent * 126, f"biome {Biome}, piece {I}: size within 0.75 to 1.25");
			T.AssertTrue(P.ScaleZPercent * 100 >= Look.ScaleZPercent * 74 && P.ScaleZPercent * 100 <= Look.ScaleZPercent * 126, f"biome {Biome}, piece {I}: height within 0.75 to 1.25");
			// The centre sits the kind's share of its half height above the ground (the Python's 50 * height * lift, the height in metres).
			int ExpectedLift = P.ScaleZPercent * Look.LiftPercent / 200;
			T.AssertTrue(Math::Abs(P.ZCm - ExpectedLift) <= 1, f"biome {Biome}, piece {I}: lifted by its kind's share of its half height");
		}
	}
}

// M-Q8's rule on the look table: nothing the players could take for gold, stone or berries (no yellow lump, no red bush), every kind has
// a shape of the engine's three and a size.
void Test_PlanLooksAreNeverResources(FUnitTest& T)
{
	for (int Kind = 0; Kind < HOLBiome::KindCount; Kind++)
	{
		FHOLDecorLook Look = HOLBiomeDecor::Look(Kind);
		T.AssertTrue(Look.Shape >= 0 && Look.Shape < HOLBiomeDecor::ShapeCount, f"kind {Kind}: a shape of the engine's");
		T.AssertTrue(Look.ScaleXPercent > 0 && Look.ScaleYPercent > 0 && Look.ScaleZPercent > 0, f"kind {Kind}: a size");
		T.AssertTrue(Look.LiftPercent >= 0 && Look.LiftPercent <= 100, f"kind {Kind}: lifted by a share of its half height");
		bool bGoldLike = Look.R >= 300 && Look.G >= 250 && Look.B * 4 <= Look.R;
		bool bBerryLike = Look.R >= 350 && Look.G * 4 <= Look.R && Look.B * 4 <= Look.R;
		T.AssertFalse(bGoldLike, f"kind {Kind}: not a yellow lump (gold)");
		T.AssertFalse(bBerryLike, f"kind {Kind}: not a red bush (berries)");
	}
	// A kind out of the table reads as the first kind, never outside it (a corrupt packet).
	T.AssertEquals(HOLBiomeDecor::Look(0).Shape, HOLBiomeDecor::Look(HOLBiome::KindCount + 3).Shape, "an unknown kind reads as the first");
}

// An index outside the table plans as the fresh grass, as HOLBiome::At reads it.
void Test_PlanUnknownBiomeIsFreshGrass(FUnitTest& T)
{
	TArray<FHOLGenericDecor> Wrong = HOLBiomeDecor::Plan(99, PlanSeed, PlanHalf, PlanAvoid(), PlanReference());
	TArray<FHOLGenericDecor> Grass = HOLBiomeDecor::Plan(HOLBiome::FreshGrass, PlanSeed, PlanHalf, PlanAvoid(), PlanReference());
	T.AssertEquals(Grass.Num(), Wrong.Num(), "an unknown biome lays the fresh grass's decor");
	bool bSame = Grass.Num() == Wrong.Num();
	for (int I = 0; bSame && I < Grass.Num(); I++)
	{
		bSame = Grass[I].Kind == Wrong[I].Kind && Grass[I].X == Wrong[I].X && Grass[I].Y == Wrong[I].Y;
	}
	T.AssertTrue(bSame, "piece for piece");
}
