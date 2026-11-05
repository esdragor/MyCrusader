// The generic decor's pure rule (BiomeInProgress.md B3, GDD D651 to D656): the places of one kind of scenery (tufts, flowers, bushes),
// drawn from a layout seed and the kind's salt, in patches (a smooth noise thins some stretches out and crowds others), leaving out the
// places within ClearCm of any point to avoid. It is the Angelscript port of decor_transforms and of the Rng, mix_seed and Noise it stands on
// (Tools/Editor/maps/common.py), the same algorithm on the same seed so that the server and every client lay the SAME decor when the biome
// is applied at load time (D468: the same numbers everywhere). Nothing here touches the world, the navigation, the resources, the AI or the
// neutrals (D656: purely visual); the laying itself is not here. Tested alone in script against the Python's own numbers
// (HOLDecorRules_Test.as, Tools/Tests/biome_decor_golden.py).
//
// Doubles throughout, as the Python's floats are: an unsigned 32 bit integer for the seeds and the xorshift state (it wraps at 2^32 as the
// Python masks do), a double for everything drawn from them.

// xorshift32, seeded from integers: the same numbers on every machine, no global state (common.py Rng).
struct FHOLDecorRng
{
	uint State = 1;
}

// A smooth field in about [-1, 1] over the plane: a few cosines of random direction, wavelength and phase (common.py Noise).
struct FHOLDecorNoise
{
	TArray<double> Angles;
	TArray<double> Frequencies;
	TArray<double> Phases;
}

// One place of decor: where (cm), its turn and its size as draws in [0, 1) (the recipe turns them into a yaw and a scale).
struct FHOLDecorPlace
{
	double X = 0.0;
	double Y = 0.0;
	double Turn = 0.0;
	double Size = 0.0;
}

// The points to avoid that fall in one cell of the avoid grid (a container cannot sit inside a container, hence the struct).
struct FHOLDecorCell
{
	TArray<int> Indexes;
}

namespace HOLDecor
{
	// A place is left out when it is nearer than this (cm) to a point to avoid (common.py DECOR_CLEAR).
	const double ClearCm = 800.0;

	// math.pi
	const double Pi = 3.141592653589793;

	// One 32 bit seed from some integers (common.py mix_seed): murmur-like finalizer, close inputs give unrelated seeds. A value is taken
	// by its 32 low bits, as the Python masks it (so -5 is 4294967291); never 0.
	uint MixSeed(const TArray<int64>& Values)
	{
		uint H = 0x9E3779B9;
		for (int64 Value : Values)
		{
			int64 Masked = Value & int64(4294967295);
			uint Word = uint(Masked);
			H = H ^ Word;
			H = H * 0x85EBCA6B;
			H = H ^ (H >> 13);
			H = H * 0xC2B2AE35;
			H = H ^ (H >> 16);
		}
		if (H == 0)
		{
			return 1;
		}
		return H;
	}

	// The next 32 bit draw (common.py Rng.next_u32).
	uint NextU32(FHOLDecorRng& Rng)
	{
		uint X = Rng.State;
		X = X ^ (X << 13);
		X = X ^ (X >> 17);
		X = X ^ (X << 5);
		Rng.State = X;
		return X;
	}

	// A generator seeded from integers, its first four draws thrown away (common.py Rng.__init__).
	FHOLDecorRng MakeRng(const TArray<int64>& Seed)
	{
		FHOLDecorRng Rng;
		Rng.State = MixSeed(Seed);
		for (int Index = 0; Index < 4; Index++)
		{
			NextU32(Rng);
		}
		return Rng;
	}

	// In [0, 1) (common.py Rng.uniform).
	double Uniform(FHOLDecorRng& Rng)
	{
		return double(NextU32(Rng)) / 4294967296.0;
	}

	// low + (high - low) * uniform (common.py Rng.range).
	double Range(FHOLDecorRng& Rng, double Low, double High)
	{
		return Low + (High - Low) * Uniform(Rng);
	}

	// The field of Noise(seed, waves, shortest, longest): its waves are drawn from Rng(seed, 5) (common.py Noise.__init__).
	FHOLDecorNoise MakeNoise(uint Seed, int Waves, double Shortest, double Longest)
	{
		TArray<int64> SeedList;
		SeedList.Add(int64(Seed));
		SeedList.Add(5);
		FHOLDecorRng Rng = MakeRng(SeedList);
		FHOLDecorNoise Noise;
		for (int Index = 0; Index < Waves; Index++)
		{
			double Angle = Range(Rng, 0.0, 2.0 * Pi);
			double Frequency = 2.0 * Pi / Range(Rng, Shortest, Longest);
			double Phase = Range(Rng, 0.0, 2.0 * Pi);
			Noise.Angles.Add(Angle);
			Noise.Frequencies.Add(Frequency);
			Noise.Phases.Add(Phase);
		}
		return Noise;
	}

	// The field's value at a point, clamped to [-1, 1] (common.py Noise.value).
	double NoiseValue(const FHOLDecorNoise& Noise, double X, double Y)
	{
		double Total = 0.0;
		for (int Index = 0; Index < Noise.Angles.Num(); Index++)
		{
			Total += Math::Cos(Noise.Frequencies[Index] * (X * Math::Cos(Noise.Angles[Index]) + Y * Math::Sin(Noise.Angles[Index])) + Noise.Phases[Index]);
		}
		double Value = Total / Math::Sqrt(0.5 * double(Noise.Angles.Num()));
		return Math::Max(-1.0, Math::Min(1.0, Value));
	}

	// How many places a density asks of a map: per_hectare of the bounded square (side 2 * half, cm; 1 hectare = 1e8 cm2).
	int Requested(double BoundsHalf, double PerHectare)
	{
		double Side = 2.0 * BoundsHalf;
		return int(PerHectare * (Side * Side) / 1.0e8);
	}

	// The key of a cell of the avoid grid (the grid only speeds the search up: a point within ClearCm is always in a neighbouring cell).
	int CellKey(int CellX, int CellY)
	{
		return (CellX + 16384) * 32768 + (CellY + 16384);
	}

	// The places of one kind of scenery (common.py decor_transforms): Requested places drawn in patches from the layout seed and the
	// kind's salt, each one kept unless it is nearer than ClearCm to a point of Avoid. At most 30 tries per requested place, so a crowded
	// map lays fewer. Every place is (x, y, turn, size), turn and size in [0, 1).
	TArray<FHOLDecorPlace> Transforms(int64 LayoutSeed, double BoundsHalf, const TArray<FVector2D>& Avoid, double PerHectare, int64 Salt)
	{
		TArray<FHOLDecorPlace> Places;
		int Count = Requested(BoundsHalf, PerHectare);
		TMap<int, FHOLDecorCell> Grid;
		for (int Index = 0; Index < Avoid.Num(); Index++)
		{
			int Key = CellKey(Math::FloorToInt(Avoid[Index].X / ClearCm), Math::FloorToInt(Avoid[Index].Y / ClearCm));
			FHOLDecorCell& Cell = Grid.FindOrAdd(Key);
			Cell.Indexes.Add(Index);
		}
		TArray<int64> RngSeed;
		RngSeed.Add(LayoutSeed);
		RngSeed.Add(71);
		RngSeed.Add(Salt);
		FHOLDecorRng Rng = MakeRng(RngSeed);
		TArray<int64> PatchSeed;
		PatchSeed.Add(LayoutSeed);
		PatchSeed.Add(72);
		PatchSeed.Add(Salt);
		FHOLDecorNoise Patches = MakeNoise(MixSeed(PatchSeed), 3, 2500.0, 9000.0);
		for (int Try = 0; Try < 30 * Count; Try++)
		{
			if (Places.Num() >= Count)
			{
				break;
			}
			double X = Range(Rng, -BoundsHalf, BoundsHalf);
			double Y = Range(Rng, -BoundsHalf, BoundsHalf);
			// Thinner where the patch noise is low: the tufts come in patches, not a sprinkle.
			if (Uniform(Rng) > 0.2 + 0.8 * (0.5 + 0.5 * NoiseValue(Patches, X, Y)))
			{
				continue;
			}
			int CellX = Math::FloorToInt(X / ClearCm);
			int CellY = Math::FloorToInt(Y / ClearCm);
			bool TooClose = false;
			for (int DX = -1; DX <= 1 && !TooClose; DX++)
			{
				for (int DY = -1; DY <= 1 && !TooClose; DY++)
				{
					FHOLDecorCell Cell;
					if (!Grid.Find(CellKey(CellX + DX, CellY + DY), Cell))
					{
						continue;
					}
					for (int Index : Cell.Indexes)
					{
						double OffX = X - Avoid[Index].X;
						double OffY = Y - Avoid[Index].Y;
						if (Math::Sqrt(OffX * OffX + OffY * OffY) < ClearCm)
						{
							TooClose = true;
							break;
						}
					}
				}
			}
			if (TooClose)
			{
				continue;
			}
			FHOLDecorPlace Place;
			Place.X = X;
			Place.Y = Y;
			Place.Turn = Uniform(Rng);
			Place.Size = Uniform(Rng);
			Places.Add(Place);
		}
		return Places;
	}
}
