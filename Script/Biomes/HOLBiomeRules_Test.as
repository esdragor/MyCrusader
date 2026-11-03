// The biomes' table and pure rules, alone (BiomeInProgress.md B1, GDD D651 to D656). Every expectation comes from the table's own
// functions (the five biomes by their indexes, the keys they say, the tints they carry), never from retyped numbers; the limits
// asked of the lights are the GDD's (D258, D265: readability) and are held to every line of the table.

FHOLDecorPiece RockPiece(int Shape, int X, int Y)
{
	FHOLDecorPiece Piece;
	Piece.Shape = Shape;
	Piece.X = X;
	Piece.Y = Y;
	Piece.YawDegrees = 37;
	Piece.ScalePercent = 140;
	// A grey-brown rock, the Cols' kind.
	Piece.R = 110;
	Piece.G = 100;
	Piece.B = 90;
	return Piece;
}

// D653/B-Q3: five biomes, each with its own key, the keys read back to the same index, an unknown key is -1.
void Test_BiomeTableFiveWithKeys(FUnitTest& T)
{
	T.AssertEquals(5, HOLBiome::Count, "five biomes to begin with");
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		FString Key = HOLBiome::KeyOf(Index);
		T.AssertTrue(Key.Len() > 0, f"biome {Index} has a key");
		T.AssertEquals(Index, HOLBiome::IndexOfKey(Key), f"biome {Index}: its key {Key} reads back to it");
		for (int Other = Index + 1; Other < HOLBiome::Count; Other++)
		{
			T.AssertTrue(Key != HOLBiome::KeyOf(Other), f"biomes {Index} and {Other} have different keys");
		}
	}
	T.AssertEquals(-1, HOLBiome::IndexOfKey("lava_world"), "an unknown key is not a biome");
	T.AssertEquals(-1, HOLBiome::IndexOfKey(""), "an empty key is not a biome");
}

// D652/B-Q4: the host's pick wins on any map; no pick (or a corrupt one) gives the map's own biome; neither valid gives the first biome.
void Test_BiomeResolve(FUnitTest& T)
{
	T.AssertEquals(HOLBiome::Snow, HOLBiome::Resolve(HOLBiome::Snow, HOLBiome::DarkForest), "the host's snow on a forest map");
	T.AssertEquals(HOLBiome::FreshGrass, HOLBiome::Resolve(HOLBiome::FreshGrass, HOLBiome::Snow), "fresh grass on a snowy map");
	T.AssertEquals(HOLBiome::DarkForest, HOLBiome::Resolve(HOLBiome::MapDefault, HOLBiome::DarkForest), "no pick: the map's own");
	T.AssertEquals(HOLBiome::Autumn, HOLBiome::Resolve(HOLBiome::Count, HOLBiome::Autumn), "a pick past the table: the map's own");
	T.AssertEquals(HOLBiome::Autumn, HOLBiome::Resolve(-7, HOLBiome::Autumn), "a negative pick: the map's own");
	T.AssertEquals(HOLBiome::FreshGrass, HOLBiome::Resolve(HOLBiome::MapDefault, HOLBiome::MapDefault), "neither is a biome: the first");
	T.AssertEquals(HOLBiome::FreshGrass, HOLBiome::Resolve(99, 99), "nothing valid at all: the first");
	// Every pick of the table is kept on every map's default: any biome on any map.
	for (int Pick = 0; Pick < HOLBiome::Count; Pick++)
	{
		for (int Default = 0; Default < HOLBiome::Count; Default++)
		{
			T.AssertEquals(Pick, HOLBiome::Resolve(Pick, Default), f"pick {Pick} on a map whose own is {Default}");
		}
	}
}

// An index out of the table reads the first biome and never outside the table.
void Test_BiomeAtOutOfRange(FUnitTest& T)
{
	T.AssertEquals(HOLBiome::KeyOf(HOLBiome::FreshGrass), HOLBiome::KeyOf(-1), "-1 reads the first biome");
	T.AssertEquals(HOLBiome::KeyOf(HOLBiome::FreshGrass), HOLBiome::KeyOf(HOLBiome::Count), "past the table reads the first biome");
	T.AssertFalse(HOLBiome::IsValid(-1), "-1 is not a biome");
	T.AssertFalse(HOLBiome::IsValid(HOLBiome::Count), "the count is not a biome");
	T.AssertTrue(HOLBiome::IsValid(HOLBiome::Count - 1), "the last one is");
}

// The five must be told apart on the screen: no two share the same sun, haze AND ground colour (the Cols/El Dorado/Fords problem).
void Test_BiomeAmbiencesAreDistinct(FUnitTest& T)
{
	for (int A = 0; A < HOLBiome::Count; A++)
	{
		for (int B = A + 1; B < HOLBiome::Count; B++)
		{
			FHOLBiome First = HOLBiome::At(A);
			FHOLBiome Second = HOLBiome::At(B);
			bool bSameGround = First.GroundAR == Second.GroundAR && First.GroundAG == Second.GroundAG && First.GroundAB == Second.GroundAB;
			bool bSameFog = First.FogR == Second.FogR && First.FogG == Second.FogG && First.FogB == Second.FogB;
			bool bSameSun = First.SunR == Second.SunR && First.SunG == Second.SunG && First.SunB == Second.SunB
				&& First.SunIntensityTenths == Second.SunIntensityTenths;
			T.AssertFalse(bSameGround, f"biomes {A} and {B} have different ground colours");
			T.AssertFalse(bSameFog, f"biomes {A} and {B} have different hazes");
			T.AssertFalse(bSameSun, f"biomes {A} and {B} have different suns");
		}
	}
}

// D258, D265: every biome's lights stay inside the moderate limits (readable team colours, selection, life bars), none outside; and the
// check itself refuses a light outside, so that it cannot pass by being empty.
void Test_BiomeLightsInBounds(FUnitTest& T)
{
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		T.AssertTrue(HOLBiome::LightsInBounds(HOLBiome::At(Index)), f"biome {Index} ({HOLBiome::KeyOf(Index)}): lights within the limits");
	}
	FHOLBiome Dark = HOLBiome::At(HOLBiome::Snow);
	Dark.SunIntensityTenths = 30;
	T.AssertFalse(HOLBiome::LightsInBounds(Dark), "a sun far too weak is refused");
	FHOLBiome Violet = HOLBiome::At(HOLBiome::Snow);
	Violet.SunR = 1000;
	Violet.SunG = 400;
	Violet.SunB = 1000;
	T.AssertFalse(HOLBiome::LightsInBounds(Violet), "a coloured sunset is refused");
	FHOLBiome Steep = HOLBiome::At(HOLBiome::Snow);
	Steep.SunPitch = -85;
	T.AssertFalse(HOLBiome::LightsInBounds(Steep), "a sun straight overhead (flat faces) is refused");
}

// B-Q6: the generic decor of each biome is its own: each one has some, the snow's has none of the fresh grass's kinds, every kind is a known one.
void Test_BiomeGenericDecorIsTheBiomes(FUnitTest& T)
{
	TArray<int> Fresh = HOLBiome::GenericKinds(HOLBiome::FreshGrass);
	TArray<int> Snowy = HOLBiome::GenericKinds(HOLBiome::Snow);
	T.AssertTrue(Fresh.Num() > 0, "the fresh grass lays some generic decor");
	T.AssertTrue(Snowy.Num() > 0, "the snow lays some too (D566: never bare)");
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		TArray<int> Kinds = HOLBiome::GenericKinds(Index);
		T.AssertTrue(Kinds.Num() > 0, f"biome {Index} has generic decor");
		for (int K = 0; K < Kinds.Num(); K++)
		{
			T.AssertTrue(Kinds[K] >= 0 && Kinds[K] < HOLBiome::KindCount, f"biome {Index}: kind {Kinds[K]} is a known kind");
		}
	}
	for (int S = 0; S < Snowy.Num(); S++)
	{
		for (int F = 0; F < Fresh.Num(); F++)
		{
			T.AssertTrue(Snowy[S] != Fresh[F], f"the snow's kind {Snowy[S]} is not one of the fresh grass's");
		}
	}
	// The autumn, the dark forest and the dry steppe each have a kind none of the others have in the same slot: not the fresh grass's flower.
	for (int Index = 1; Index < HOLBiome::Count; Index++)
	{
		TArray<int> Kinds = HOLBiome::GenericKinds(Index);
		for (int K = 0; K < Kinds.Num(); K++)
		{
			T.AssertTrue(Kinds[K] != HOLBiome::KindFlower, f"biome {Index} lays no flowers (they are the fresh grass's)");
		}
	}
}

// D656: the identity decor keeps its shape, place, turn and size; only the colour moves, toward the biome's tint.
void Test_BiomeIdentityDecorKeepsFormsRecolours(FUnitTest& T)
{
	TArray<FHOLDecorPiece> Rocks;
	Rocks.Add(RockPiece(2, 1200, -3400));
	Rocks.Add(RockPiece(5, -800, 900));
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		TArray<FHOLDecorPiece> Result = HOLBiome::RecolorIdentity(Index, Rocks);
		T.AssertEquals(Rocks.Num(), Result.Num(), f"biome {Index}: same number of pieces");
		for (int P = 0; P < Rocks.Num(); P++)
		{
			T.AssertEquals(Rocks[P].Shape, Result[P].Shape, f"biome {Index} piece {P}: same shape");
			T.AssertEquals(Rocks[P].X, Result[P].X, f"biome {Index} piece {P}: same X");
			T.AssertEquals(Rocks[P].Y, Result[P].Y, f"biome {Index} piece {P}: same Y");
			T.AssertEquals(Rocks[P].YawDegrees, Result[P].YawDegrees, f"biome {Index} piece {P}: same turn");
			T.AssertEquals(Rocks[P].ScalePercent, Result[P].ScalePercent, f"biome {Index} piece {P}: same size");
		}
	}
	// The snow whitens: every channel of the rock goes up toward the snow's tint; a fresh grass leaves the colours as they are.
	FHOLBiome Snow = HOLBiome::At(HOLBiome::Snow);
	T.AssertTrue(Snow.IdentityTintPercent > 0, "the snow recolours the identity decor");
	TArray<FHOLDecorPiece> Whitened = HOLBiome::RecolorIdentity(HOLBiome::Snow, Rocks);
	T.AssertTrue(Whitened[0].R > Rocks[0].R && Whitened[0].G > Rocks[0].G && Whitened[0].B > Rocks[0].B, "the rock is whiter in snow");
	T.AssertTrue(Whitened[0].R <= 255 && Whitened[0].G <= 255 && Whitened[0].B <= 255, "no channel past 255");
	TArray<FHOLDecorPiece> Fresh = HOLBiome::RecolorIdentity(HOLBiome::FreshGrass, Rocks);
	T.AssertEquals(Rocks[0].R, Fresh[0].R, "fresh grass leaves the red");
	T.AssertEquals(Rocks[0].G, Fresh[0].G, "fresh grass leaves the green");
	T.AssertEquals(Rocks[0].B, Fresh[0].B, "fresh grass leaves the blue");
	// The dark forest darkens: the rock goes down toward the forest's near black.
	TArray<FHOLDecorPiece> Darkened = HOLBiome::RecolorIdentity(HOLBiome::DarkForest, Rocks);
	T.AssertTrue(Darkened[0].R < Rocks[0].R && Darkened[0].G < Rocks[0].G && Darkened[0].B < Rocks[0].B, "the rock is darker in the dark forest");
}

// D655: a game object (resource, game rock) is never recoloured, whatever the biome; the same piece as decor is.
void Test_BiomeNeverTouchesGameObjects(FUnitTest& T)
{
	FHOLDecorPiece Tree = RockPiece(1, 0, 0);
	Tree.bGameObject = true;
	FHOLDecorPiece Decor = RockPiece(1, 0, 0);
	for (int Index = 0; Index < HOLBiome::Count; Index++)
	{
		FHOLBiome Biome = HOLBiome::At(Index);
		FHOLDecorPiece Kept = HOLBiome::Recolor(Biome, Tree);
		T.AssertEquals(Tree.R, Kept.R, f"biome {Index}: the resource's red is kept");
		T.AssertEquals(Tree.G, Kept.G, f"biome {Index}: the resource's green is kept");
		T.AssertEquals(Tree.B, Kept.B, f"biome {Index}: the resource's blue is kept");
		T.AssertTrue(Kept.bGameObject, f"biome {Index}: it stays a game object");
	}
	FHOLDecorPiece Changed = HOLBiome::Recolor(HOLBiome::At(HOLBiome::Snow), Decor);
	T.AssertTrue(Changed.R != Decor.R || Changed.G != Decor.G || Changed.B != Decor.B, "while the same piece as decor is recoloured in snow");
}

// Colours mix whole and stay in 0..255; the generic count follows the density and never drops to nothing when the reference lays some.
void Test_BiomeMixAndDensity(FUnitTest& T)
{
	T.AssertEquals(100, HOLBiome::MixChannel(100, 255, 0), "0 percent: the base");
	T.AssertEquals(255, HOLBiome::MixChannel(100, 255, 100), "100 percent: the target");
	T.AssertEquals(0, HOLBiome::MixChannel(0, 0, 50), "black stays black toward black");
	T.AssertTrue(HOLBiome::MixChannel(300, 400, 50) <= 255, "clamped above");
	T.AssertTrue(HOLBiome::MixChannel(-5, -9, 50) >= 0, "clamped below");
	T.AssertEquals(0, HOLBiome::GenericCount(HOLBiome::Snow, 0), "no reference decor: none");
	T.AssertEquals(1, HOLBiome::GenericCount(HOLBiome::Snow, 1), "a thin reference is never emptied");
	T.AssertEquals(HOLBiome::At(HOLBiome::Snow).GenericDensityPercent * 200 / 100, HOLBiome::GenericCount(HOLBiome::Snow, 200), "the biome's density");
	T.AssertTrue(HOLBiome::GenericCount(HOLBiome::Snow, 200) < HOLBiome::GenericCount(HOLBiome::DarkForest, 200), "snow is sparser than the dark forest");
}
