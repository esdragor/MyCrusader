// The biomes' table and pure rules (BiomeInProgress.md B0/B1, GDD D651 to D656): five visual ambiances the host picks at the lobby for
// everybody. A biome is a LINE OF TABLE (ground colours and detail texture, sun, sky, haze, the generic decor it lays, the tint it gives
// the map's identity decor); nothing here touches the world, the navigation, the resources, the AI or the neutrals (D656: purely
// visual), and nothing here is a shadow, a reflection or a post process (D258, D265). Whole numbers only, so that the server and the
// clients agree to the last digit (D468): colours and light strengths in per mille of 1 (1000 = 1.0), the sun's strength in tenths of
// lux, the haze's density in 0.0001, angles in degrees, distances in centimetres. Tested alone in script (HOLBiomeRules_Test.as).
//
// The numbers are PROVISIONAL (BiomeInProgress.md "Questions ouvertes" 2): proposed from the ambiences of Tools/Editor/maps/ambience.py
// (the Hinterland's validated light, the Forest's, the Steppe's tried one, Cols' cold one) and judged by the user on captures (B1 view,
// B4 compare scenario). The snow has no CC0 ground texture yet (to choose): its DetailStrength is 0 until then.

// One biome: what the client sets on the ground instance, the lights and the haze, and what the decor does. The ground colours are the
// material parameters ColorA (the main ground), ColorB (the wide patches), ColorC (the rarer ones) in linear per mille.
struct FHOLBiome
{
	// The identifier the lobby, the match rules and Maps.csv speak of ("fresh_grass"): stable, never translated.
	FString Key;

	// The ground (M_GroundVaried parameters, per mille).
	int GroundAR = 0;
	int GroundAG = 0;
	int GroundAB = 0;
	int GroundBR = 0;
	int GroundBG = 0;
	int GroundBB = 0;
	int GroundCR = 0;
	int GroundCG = 0;
	int GroundCB = 0;
	// The CC0 detail texture (the source name of Tools/Editor/maps/ground.py, "" for none), the size of one repeat in cm, its strength in
	// percent (0 = none).
	FString DetailTexture;
	int DetailScaleCm = 900;
	int DetailStrengthPercent = 70;

	// The sun: colour (per mille), strength (tenths of lux, the engine's default is 100), pitch and yaw (degrees).
	int SunR = 1000;
	int SunG = 1000;
	int SunB = 1000;
	int SunIntensityTenths = 100;
	int SunPitch = -50;
	int SunYaw = 30;

	// The sky light: tint (per mille) and strength (per mille, 1000 = the default 1.0).
	int SkyR = 1000;
	int SkyG = 1000;
	int SkyB = 1000;
	int SkyIntensityPermille = 1000;

	// The height haze: colour (per mille, linear), density (0.0001: 200 = the default 0.02), start distance (cm).
	int FogR = 447;
	int FogG = 638;
	int FogB = 1000;
	int FogDensity = 200;
	int FogStartCm = 0;

	// The tint the biome gives the map's identity decor (an 8-bit colour) and how far the decor goes toward it (percent, 0 = untouched).
	int IdentityTintR = 255;
	int IdentityTintG = 255;
	int IdentityTintB = 255;
	int IdentityTintPercent = 0;

	// How much generic decor the biome lays, in percent of the reference density (100 = as much as the reference).
	int GenericDensityPercent = 100;
}

// One piece of decor as the maps' recipes place it (position, turn, size, shape and 8-bit colour): the identity decor of a map (D576: the
// Cols' rocks, El Dorado's reeds) and the generic decor. A game object (a resource tree, a gold or stone vein, a berry bush, a game rock:
// crests, walls, outcrops) is NOT decor: bGameObject says so and the biome never touches it (D655).
struct FHOLDecorPiece
{
	// The shape (a mesh of the recipe's list, by index): the biome never changes it for identity decor (D656).
	int Shape = 0;
	int X = 0;
	int Y = 0;
	int YawDegrees = 0;
	// The size in percent of the shape's own.
	int ScalePercent = 100;
	// The 8-bit colour.
	int R = 128;
	int G = 128;
	int B = 128;
	bool bGameObject = false;
}

namespace HOLBiome
{
	// The five biomes (D653, B-Q3), by index as everywhere in script; the lobby shows them in this order.
	const int FreshGrass = 0;
	const int DrySteppe = 1;
	const int Autumn = 2;
	const int DarkForest = 3;
	const int Snow = 4;
	const int Count = 5;

	// The host's choice when he picks none: the map's own biome (the ambience each map carries today), the first choice of the lobby list.
	const int MapDefault = -1;

	// The generic decor kinds a biome can lay (B-Q6: they REPLACE the map's generic decor; the identity decor keeps its shapes).
	const int KindFlower = 0;
	const int KindTuft = 1;
	const int KindBush = 2;
	const int KindDryGrass = 3;
	const int KindFallenLeaves = 4;
	const int KindFern = 5;
	const int KindFrozenTuft = 6;
	const int KindBareBush = 7;
	const int KindCount = 8;

	bool IsValid(int Index)
	{
		return Index >= 0 && Index < Count;
	}

	// The biome of an index, FreshGrass for an index out of the table (a corrupt packet never reads outside it).
	FHOLBiome At(int Index)
	{
		FHOLBiome B;
		switch (IsValid(Index) ? Index : FreshGrass)
		{
		case DrySteppe:
			B.Key = "dry_steppe";
			B.GroundAR = 115; B.GroundAG = 145; B.GroundAB = 55;
			B.GroundBR = 300; B.GroundBG = 200; B.GroundBB = 85;
			B.GroundCR = 60; B.GroundCG = 45; B.GroundCB = 30;
			B.DetailTexture = "sparse_grass";
			B.SunR = 1000; B.SunG = 950; B.SunB = 860; B.SunIntensityTenths = 105; B.SunPitch = -52; B.SunYaw = 30;
			B.SkyR = 1000; B.SkyG = 980; B.SkyB = 920; B.SkyIntensityPermille = 1000;
			B.FogR = 700; B.FogG = 640; B.FogB = 420; B.FogDensity = 80; B.FogStartCm = 2500;
			B.IdentityTintR = 176; B.IdentityTintG = 150; B.IdentityTintB = 96; B.IdentityTintPercent = 35;
			B.GenericDensityPercent = 70;
			break;
		case Autumn:
			B.Key = "autumn";
			B.GroundAR = 150; B.GroundAG = 95; B.GroundAB = 40;
			B.GroundBR = 230; B.GroundBG = 130; B.GroundBB = 50;
			B.GroundCR = 70; B.GroundCG = 50; B.GroundCB = 35;
			B.DetailTexture = "forest_ground_06";
			B.SunR = 1000; B.SunG = 920; B.SunB = 840; B.SunIntensityTenths = 95; B.SunPitch = -42; B.SunYaw = 45;
			B.SkyR = 1000; B.SkyG = 940; B.SkyB = 860; B.SkyIntensityPermille = 950;
			B.FogR = 720; B.FogG = 550; B.FogB = 380; B.FogDensity = 140; B.FogStartCm = 1800;
			B.IdentityTintR = 150; B.IdentityTintG = 96; B.IdentityTintB = 48; B.IdentityTintPercent = 35;
			B.GenericDensityPercent = 90;
			break;
		case DarkForest:
			B.Key = "dark_forest";
			B.GroundAR = 45; B.GroundAG = 75; B.GroundAB = 40;
			B.GroundBR = 90; B.GroundBG = 100; B.GroundBB = 45;
			B.GroundCR = 40; B.GroundCG = 38; B.GroundCB = 32;
			B.DetailTexture = "forest_ground_06";
			B.SunR = 880; B.SunG = 960; B.SunB = 860; B.SunIntensityTenths = 75; B.SunPitch = -42; B.SunYaw = 15;
			B.SkyR = 880; B.SkyG = 960; B.SkyB = 880; B.SkyIntensityPermille = 900;
			B.FogR = 300; B.FogG = 450; B.FogB = 340; B.FogDensity = 180; B.FogStartCm = 1500;
			B.IdentityTintR = 40; B.IdentityTintG = 52; B.IdentityTintB = 44; B.IdentityTintPercent = 40;
			B.GenericDensityPercent = 110;
			break;
		case Snow:
			B.Key = "snow";
			B.GroundAR = 550; B.GroundAG = 580; B.GroundAB = 620;
			B.GroundBR = 400; B.GroundBG = 430; B.GroundBB = 480;
			B.GroundCR = 250; B.GroundCG = 280; B.GroundCB = 320;
			// No CC0 snow texture chosen yet (BiomeInProgress.md): the colours alone, until then.
			B.DetailTexture = "";
			B.DetailStrengthPercent = 0;
			B.SunR = 920; B.SunG = 960; B.SunB = 1000; B.SunIntensityTenths = 105; B.SunPitch = -48; B.SunYaw = 60;
			B.SkyR = 900; B.SkyG = 960; B.SkyB = 1000; B.SkyIntensityPermille = 1100;
			B.FogR = 800; B.FogG = 860; B.FogB = 960; B.FogDensity = 100; B.FogStartCm = 2000;
			B.IdentityTintR = 235; B.IdentityTintG = 242; B.IdentityTintB = 250; B.IdentityTintPercent = 70;
			B.GenericDensityPercent = 60;
			break;
		default:
			B.Key = "fresh_grass";
			B.GroundAR = 90; B.GroundAG = 150; B.GroundAB = 50;
			B.GroundBR = 200; B.GroundBG = 220; B.GroundBB = 80;
			B.GroundCR = 60; B.GroundCG = 60; B.GroundCB = 40;
			B.DetailTexture = "grass_ground";
			B.SunR = 1000; B.SunG = 980; B.SunB = 930; B.SunIntensityTenths = 100; B.SunPitch = -50; B.SunYaw = 30;
			B.SkyR = 970; B.SkyG = 1000; B.SkyB = 1000; B.SkyIntensityPermille = 1000;
			B.FogR = 550; B.FogG = 720; B.FogB = 920; B.FogDensity = 120; B.FogStartCm = 2000;
			// The identity decor's own colours are the fresh grass's: untouched.
			B.IdentityTintPercent = 0;
			B.GenericDensityPercent = 100;
			break;
		}
		return B;
	}

	FString KeyOf(int Index)
	{
		return At(Index).Key;
	}

	// The index of a key, -1 for a key no biome has (a lobby or a Maps.csv line that names an unknown biome).
	int IndexOfKey(const FString& Key)
	{
		for (int Index = 0; Index < Count; Index++)
		{
			if (At(Index).Key == Key)
			{
				return Index;
			}
		}
		return -1;
	}

	// The biome a match uses: the host's pick when it is one of the five, else the map's own (MapDefault, an unknown index, a corrupt
	// packet), and FreshGrass when the map's own is not a biome either. B-Q4: any biome on any map, no list per map.
	int Resolve(int Requested, int MapDefaultBiome)
	{
		if (IsValid(Requested))
		{
			return Requested;
		}
		return IsValid(MapDefaultBiome) ? MapDefaultBiome : FreshGrass;
	}

	// The generic decor the biome lays in place of the map's generic decor (B-Q6), in the order of its recipe.
	TArray<int> GenericKinds(int Index)
	{
		TArray<int> Kinds;
		switch (IsValid(Index) ? Index : FreshGrass)
		{
		case DrySteppe:
			Kinds.Add(KindDryGrass);
			Kinds.Add(KindTuft);
			Kinds.Add(KindBush);
			break;
		case Autumn:
			Kinds.Add(KindFallenLeaves);
			Kinds.Add(KindTuft);
			Kinds.Add(KindBush);
			break;
		case DarkForest:
			Kinds.Add(KindFern);
			Kinds.Add(KindTuft);
			Kinds.Add(KindBush);
			break;
		case Snow:
			Kinds.Add(KindFrozenTuft);
			Kinds.Add(KindBareBush);
			break;
		default:
			Kinds.Add(KindFlower);
			Kinds.Add(KindTuft);
			Kinds.Add(KindBush);
			break;
		}
		return Kinds;
	}

	// One 8-bit channel moved Percent of the way from Base to Target (whole numbers, rounded down: the same on the server and the clients).
	int MixChannel(int Base, int Target, int Percent)
	{
		int Mixed = (Base * (100 - Percent) + Target * Percent) / 100;
		return Mixed < 0 ? 0 : (Mixed > 255 ? 255 : Mixed);
	}

	// D656: the map's identity decor KEEPS its shape, place, turn and size; the biome only recolours it (Cols in snow: whitened rocks).
	// A game object comes back untouched (D655: resources and game rocks do not change with the biome).
	FHOLDecorPiece Recolor(const FHOLBiome& Biome, const FHOLDecorPiece& Piece)
	{
		FHOLDecorPiece Result = Piece;
		if (Piece.bGameObject)
		{
			return Result;
		}
		Result.R = MixChannel(Piece.R, Biome.IdentityTintR, Biome.IdentityTintPercent);
		Result.G = MixChannel(Piece.G, Biome.IdentityTintG, Biome.IdentityTintPercent);
		Result.B = MixChannel(Piece.B, Biome.IdentityTintB, Biome.IdentityTintPercent);
		return Result;
	}

	// The whole list of a map's identity decor, recoloured by a biome (the same count, the same order).
	TArray<FHOLDecorPiece> RecolorIdentity(int Index, const TArray<FHOLDecorPiece>& Pieces)
	{
		FHOLBiome Biome = At(Index);
		TArray<FHOLDecorPiece> Result;
		for (int I = 0; I < Pieces.Num(); I++)
		{
			Result.Add(Recolor(Biome, Pieces[I]));
		}
		return Result;
	}

	// How many generic pieces a recipe lays for a reference count: the biome's density, never fewer than one when the reference lays any
	// (a biome is sparser, never bare: D566 keeps the map from emptiness).
	int GenericCount(int Index, int ReferenceCount)
	{
		if (ReferenceCount <= 0)
		{
			return 0;
		}
		int Count = ReferenceCount * At(Index).GenericDensityPercent / 100;
		return Count < 1 ? 1 : Count;
	}

	// The moderate limits every biome's lights hold to (the same as BOUNDS of Tools/Editor/maps/ambience.py, in this file's units): the team
	// colours, the selection circles, the life bars and the resources stay readable (D258, D265).
	bool ChannelsInBounds(int R, int G, int B, int Low, int High, int WidestRatioPercent)
	{
		int Smallest = Math::Min(R, Math::Min(G, B));
		int Largest = Math::Max(R, Math::Max(G, B));
		return Smallest >= Low && Largest <= High && Smallest > 0 && Largest * 100 <= Smallest * WidestRatioPercent;
	}

	bool LightsInBounds(const FHOLBiome& B)
	{
		return ChannelsInBounds(B.SunR, B.SunG, B.SunB, 840, 1000, 120)
			&& ChannelsInBounds(B.SkyR, B.SkyG, B.SkyB, 840, 1000, 120)
			&& B.SunIntensityTenths >= 70 && B.SunIntensityTenths <= 120
			&& B.SunPitch >= -70 && B.SunPitch <= -35
			&& B.SunYaw >= -30 && B.SunYaw <= 80
			&& B.SkyIntensityPermille >= 850 && B.SkyIntensityPermille <= 1300
			&& ChannelsInBounds(B.FogR, B.FogG, B.FogB, 300, 1000, 200)
			&& B.FogDensity >= 40 && B.FogDensity <= 250
			&& B.FogStartCm >= 0 && B.FogStartCm <= 4000;
	}
}
