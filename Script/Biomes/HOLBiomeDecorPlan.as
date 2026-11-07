// The generic decor PLAN of a biome (BiomeInProgress.md B3, GDD D651 to D656): the pieces of generic scenery (tufts, flowers, bushes...) a
// match lays for the biome the host picked, from the layout seed, the bounds of the map, the points to keep clear (bases, sources, rocks)
// and the densities the map asks for. It joins what already exists and is tested alone: HOLBiome::GenericKinds / GenericCount say WHICH kinds
// and HOW MANY (B-Q6: the biome's decor replaces the map's generic decor; a biome is sparser, never bare), HOLDecor::Transforms says WHERE
// (the port of the Python's decor_transforms, held to its numbers by HOLDecorRules_Test.as), and the look table below says WHAT each piece
// is (a shape of the engine's, a size, a matte colour) as DECOR_KINDS of Tools/Editor/maps/common.py does for the baked decor.
//
// Whole numbers come out (centimetres, degrees, percent, per mille) so that the server and every client lay the SAME decor (D468) and the
// laying itself (an instanced mesh per kind, no collision, no navigation, no shadow, cut at DECOR_CULL: C++ or script, B3's rest) has
// nothing to decide. Nothing here touches the world, the navigation, the resources, the AI or the neutrals (D656: purely visual). The look
// numbers are PROVISIONAL, copied from DECOR_KINDS where a kind exists there and judged on captures (B4).

// What a kind of generic decor looks like: one of the engine's three basic shapes (a 1 m mesh), its size in percent of that mesh on each
// axis, how high its centre sits (the share of its half height above the ground, percent), and its matte colour (linear, per mille).
struct FHOLDecorLook
{
	int Shape = 0;
	int ScaleXPercent = 100;
	int ScaleYPercent = 100;
	int ScaleZPercent = 100;
	int LiftPercent = 100;
	int R = 128;
	int G = 128;
	int B = 128;
}

// One piece of generic decor of a plan, ready to lay: its kind (a HOLBiome::Kind...), the look it takes from the table, where (cm), how
// high above the ground (cm), its turn (degrees) and its size (percent of the shape, per axis, the kind's own times a drawn 0.75 to 1.25).
struct FHOLGenericDecor
{
	int Kind = 0;
	int Shape = 0;
	int X = 0;
	int Y = 0;
	int ZCm = 0;
	int YawDegrees = 0;
	int ScaleXPercent = 100;
	int ScaleYPercent = 100;
	int ScaleZPercent = 100;
	int R = 128;
	int G = 128;
	int B = 128;
	// Scenery is never a game object (D655): a plan never lays a resource, a rock of the game or a wall.
	bool bGameObject = false;
}

namespace HOLBiomeDecor
{
	// The engine's basic shapes the decor is made of (no new art).
	const int ShapeCone = 0;
	const int ShapeSphere = 1;
	const int ShapeCylinder = 2;
	const int ShapeCount = 3;

	// The look of a kind (HOLBiome::Kind...), the first kind's for a kind out of the table (a corrupt packet never reads outside it). Sizes
	// and colours are DECOR_KINDS' where the baked decor has the kind (GrassGreen, GrassStraw, Bush, FlowerWhite); the others are proposed
	// in the same family. Nothing a player could take for a resource: no yellow lump, no red bush (M-Q8).
	FHOLDecorLook Look(int Kind)
	{
		FHOLDecorLook L;
		if (Kind == HOLBiome::KindTuft)
		{
			L.Shape = ShapeCone; L.ScaleXPercent = 55; L.ScaleYPercent = 55; L.ScaleZPercent = 60; L.LiftPercent = 100;
			L.R = 75; L.G = 115; L.B = 35;
			return L;
		}
		if (Kind == HOLBiome::KindBush)
		{
			L.Shape = ShapeSphere; L.ScaleXPercent = 150; L.ScaleYPercent = 150; L.ScaleZPercent = 70; L.LiftPercent = 30;
			L.R = 110; L.G = 90; L.B = 40;
			return L;
		}
		if (Kind == HOLBiome::KindDryGrass)
		{
			L.Shape = ShapeCone; L.ScaleXPercent = 50; L.ScaleYPercent = 50; L.ScaleZPercent = 75; L.LiftPercent = 100;
			L.R = 200; L.G = 170; L.B = 75;
			return L;
		}
		if (Kind == HOLBiome::KindFallenLeaves)
		{
			L.Shape = ShapeSphere; L.ScaleXPercent = 90; L.ScaleYPercent = 90; L.ScaleZPercent = 12; L.LiftPercent = 50;
			L.R = 190; L.G = 100; L.B = 40;
			return L;
		}
		if (Kind == HOLBiome::KindFern)
		{
			L.Shape = ShapeCone; L.ScaleXPercent = 65; L.ScaleYPercent = 65; L.ScaleZPercent = 50; L.LiftPercent = 100;
			L.R = 40; L.G = 90; L.B = 30;
			return L;
		}
		if (Kind == HOLBiome::KindFrozenTuft)
		{
			L.Shape = ShapeCone; L.ScaleXPercent = 55; L.ScaleYPercent = 55; L.ScaleZPercent = 55; L.LiftPercent = 100;
			L.R = 550; L.G = 620; L.B = 700;
			return L;
		}
		if (Kind == HOLBiome::KindBareBush)
		{
			L.Shape = ShapeSphere; L.ScaleXPercent = 110; L.ScaleYPercent = 110; L.ScaleZPercent = 60; L.LiftPercent = 30;
			L.R = 70; L.G = 55; L.B = 40;
			return L;
		}
		{
			// KindFlower, and the fallback: a white flower, a small flat sphere.
			L.Shape = ShapeSphere; L.ScaleXPercent = 45; L.ScaleYPercent = 45; L.ScaleZPercent = 28; L.LiftPercent = 50;
			L.R = 450; L.G = 450; L.B = 400;
			return L;
		}
		return L;
	}

	// The salt of a slot of the biome's kind list (spawn_decor's 1000 * (number + 1)): another salt, another stream, so two kinds do not
	// land on the same spots.
	int64 SaltOfSlot(int Slot)
	{
		return 1000 * (int64(Slot) + 1);
	}

	// The density (per hectare) that makes HOLDecor::Requested ask for exactly Count places on a map of that bounds: the middle of the
	// count's interval, so that no rounding of the doubles tips it to one more or one less. 0 for no place.
	double DensityForCount(double BoundsHalf, int Count)
	{
		if (Count <= 0)
		{
			return 0.0;
		}
		double Side = 2.0 * BoundsHalf;
		return (double(Count) + 0.5) * 1.0e8 / (Side * Side);
	}

	// The plan of a biome: for each slot of its kind list that the map asks a density for (Reference[Slot], per hectare; a slot past the
	// end of Reference, or a density of 0, is not asked for), the biome's share of that ask (HOLBiome::GenericCount), drawn by
	// HOLDecor::Transforms on the slot's salt. An index out of the table plans as the fresh grass, as HOLBiome::At reads it. Fewer pieces
	// than the count only if the map is so crowded that Transforms' tries run out.
	TArray<FHOLGenericDecor> Plan(int Biome, int64 LayoutSeed, double BoundsHalf, const TArray<FVector2D>& Avoid, const TArray<double>& Reference)
	{
		TArray<FHOLGenericDecor> Result;
		int Index = HOLBiome::IsValid(Biome) ? Biome : HOLBiome::FreshGrass;
		TArray<int> Kinds = HOLBiome::GenericKinds(Index);
		for (int Slot = 0; Slot < Kinds.Num() && Slot < Reference.Num(); Slot++)
		{
			if (Reference[Slot] <= 0.0)
			{
				continue;
			}
			int Wanted = HOLBiome::GenericCount(Index, HOLDecor::Requested(BoundsHalf, Reference[Slot]));
			if (Wanted <= 0)
			{
				continue;
			}
			FHOLDecorLook L = Look(Kinds[Slot]);
			TArray<FHOLDecorPlace> Places = HOLDecor::Transforms(LayoutSeed, BoundsHalf, Avoid, DensityForCount(BoundsHalf, Wanted), SaltOfSlot(Slot));
			for (int I = 0; I < Places.Num(); I++)
			{
				// The size is a drawn 0.75 to 1.25 of the kind's own on every axis (the Python's grow), in whole percent.
				int Grow = 75 + int(50.0 * Places[I].Size);
				FHOLGenericDecor Piece;
				Piece.Kind = Kinds[Slot];
				Piece.Shape = L.Shape;
				Piece.X = Math::RoundToInt(Places[I].X);
				Piece.Y = Math::RoundToInt(Places[I].Y);
				Piece.YawDegrees = int(360.0 * Places[I].Turn);
				Piece.ScaleXPercent = (L.ScaleXPercent * Grow + 50) / 100;
				Piece.ScaleYPercent = (L.ScaleYPercent * Grow + 50) / 100;
				Piece.ScaleZPercent = (L.ScaleZPercent * Grow + 50) / 100;
				// The centre sits the kind's share of its half height above the ground: 50 cm per metre of height, times the share.
				Piece.ZCm = Piece.ScaleZPercent * L.LiftPercent / 200;
				Piece.R = L.R;
				Piece.G = L.G;
				Piece.B = L.B;
				Result.Add(Piece);
			}
		}
		return Result;
	}
}
