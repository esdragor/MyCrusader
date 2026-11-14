// hol.Rocks.Review2 <letter A-H> (PlaceholdersInProgress.md, lot "ressources et decor", tour 2 des rochers et du decor des cartes,
// GDD D800: the user likes B (faceted rocks, thicket of cubes) AND D (clusters of round stones, stump with roots, thicket of balls)
// of tour 1, not yet a single choice). Shows on the loaded map, around the same spot as tour 1, a SCATTER like a real map corner:
// eight rocks of several sizes (a 2 m ridge rock to a 0,7 m one), two groups of small stones, two stumps and two thickets. Each
// letter gives the share of FACETED rocks (B) among the eight, the others are ROUND clusters (D):
//   A = B alone (8/8, B's stump and cube thicket), B = D alone (0/8, D's stump and ball thicket),
//   C = mostly B (6/8), D = half and half (4/8), E = mostly D (2/8): rocks mixed, small stones round, thickets of balls (D),
//       one stump of each;
//   F, G, H = half and half, B alone, D alone again, for the Dark forest ground (the scenario sets the biome, not this command).
// Tour 2 corrections of the defects seen in tour 1 (SAURON-1114b): DARKER colours (more contrast with the ground) and a stump twice
// as big (StumpScale), readable at the RTS camera. Same pieces as HOLRocksReview.as (engine basic shapes, matte colours); a REVIEW
// tool only: no gameplay, no collision, no shadow, nothing saved, the previous set is destroyed. Yellow label "ROCHERS <letter>".
// "Rocks review 2: letter=C shown=1 faceted=6/8 pieces=NN" in the log; shown=0 if the letter is unknown.

// Same spot as tour 1 (RocksReviewSpot of HOLRocksReview.as: a global const of another file is not visible here).
const FVector RocksReview2Spot = FVector(0.0, -3000.0, 0.0);

namespace HOLRocksReview2
{
	// Stump size against tour 1 (55-60 cm wide, 50 cm high there): x2 = about 1,2 m wide, 1 m high.
	const double StumpScale = 2.0;

	// Darker than tour 1 (B was 0,22 grey-blue, D 0,28 warm grey, both read light grey on screen; a first try at 0,07-0,09 still
	// read mid grey: the exposure of the game camera lifts these matte colours about 2,5 times).
	FLinearColor FacetBody() { return FLinearColor(0.026f, 0.028f, 0.035f, 1.0f); }
	FLinearColor FacetAccent() { return FLinearColor(0.042f, 0.045f, 0.054f, 1.0f); }
	FLinearColor RoundBody() { return FLinearColor(0.036f, 0.031f, 0.024f, 1.0f); }
	FLinearColor RoundAccent() { return FLinearColor(0.022f, 0.020f, 0.016f, 1.0f); }

	// The biome's ground under the scatter: the Plain's own ground instance (MI_Ground_Plain, parent M_GroundVaried) with the five
	// parameters HOLBiomeApplier::ApplyToWorld sets on a map's MI_Ground_* meshes, from HOLBiomeApply::LookOf(Biome). Needed because
	// the Plain loaded here draws the grey M_MatteGround tiles that the applier does not take over (tour 1 "damier gris").
	// A 120 m square, 2 cm above the map's ground: it covers the whole view of the scenario's camera.
	void BiomeGround(AHOLRocksReviewActor Owner, int Biome)
	{
		UStaticMeshComponent Patch = UStaticMeshComponent::Create(Owner, n"RocksBiomeGround");
		Patch.SetStaticMesh(Cast<UStaticMesh>(LoadObject(nullptr, "/Engine/BasicShapes/Plane.Plane")));
		Patch.AttachToComponent(Owner.Root);
		Patch.SetRelativeLocation(FVector(0.0, 0.0, 2.0));
		Patch.SetRelativeScale3D(FVector(120.0, 120.0, 1.0));
		Patch.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Patch.SetCastShadow(false);
		UMaterialInterface Ground = Cast<UMaterialInterface>(LoadObject(nullptr, "/Game/Rendering/MI_Ground_Plain.MI_Ground_Plain"));
		UMaterialInstanceDynamic Dye = Patch.CreateDynamicMaterialInstance(0, Ground);
		if (Dye == nullptr)
		{
			Log(n"LogHOLMatch", "Rocks review 2: no biome ground (MI_Ground_Plain not loaded)");
			return;
		}
		FHOLBiomeLook Look = HOLBiomeApply::LookOf(Biome);
		Dye.SetVectorParameterValue(n"ColorA", Look.GroundA);
		Dye.SetVectorParameterValue(n"ColorB", Look.GroundB);
		Dye.SetVectorParameterValue(n"ColorC", Look.GroundC);
		Dye.SetScalarParameterValue(n"TextureScale", Look.DetailScaleCm);
		Dye.SetScalarParameterValue(n"DetailStrength", Look.DetailStrength);
		Log(n"LogHOLMatch", f"Rocks review 2: biome ground key={Look.Key}");
	}

	void Rock(AHOLRocksReviewActor Owner, bool Facet, FVector Base, double S)
	{
		if (Facet) // B: two tilted cubes
		{
			HOLRocksReview::Bit(Owner, HOLRocksReview::Cube, Base, S, FVector(0, 0, 55), FVector(200, 170, 140), FRotator(12.0f, 25.0f, 8.0f), FacetBody());
			HOLRocksReview::Bit(Owner, HOLRocksReview::Cube, Base, S, FVector(90, -80, 40), FVector(130, 120, 110), FRotator(-18.0f, 70.0f, 14.0f), FacetAccent());
		}
		else // D: cluster of three round stones
		{
			FRotator Flat(0.0f, 0.0f, 0.0f);
			HOLRocksReview::Bit(Owner, HOLRocksReview::Sphere, Base, S, FVector(0, 0, 60), FVector(180, 160, 140), Flat, RoundBody());
			HOLRocksReview::Bit(Owner, HOLRocksReview::Sphere, Base, S, FVector(90, 60, 40), FVector(140, 130, 110), Flat, RoundAccent());
			HOLRocksReview::Bit(Owner, HOLRocksReview::Sphere, Base, S, FVector(-80, 70, 35), FVector(120, 110, 90), Flat, RoundBody());
		}
	}

	void Stones(AHOLRocksReviewActor Owner, bool Facet, FVector Base)
	{
		for (int K = 0; K < 5; K++)
		{
			double Angle = double(K) * 137.5 * 0.0174533;
			double Around = K == 0 ? 0.0 : 140.0;
			Rock(Owner, Facet, Base + FVector(Math::Cos(Angle) * Around, Math::Sin(Angle) * Around, 0.0), 0.28);
		}
	}

	void Stump(AHOLRocksReviewActor Owner, bool Facet, FVector Base)
	{
		FLinearColor Bark(0.11f, 0.075f, 0.045f, 1.0f);
		FLinearColor Cut(0.35f, 0.25f, 0.15f, 1.0f);
		double K = StumpScale;
		if (Facet) // B: light cut face
		{
			HOLRocksReview::Plain(Owner, HOLRocksReview::Cylinder, Base + FVector(0, 0, 25) * K, FVector(60, 60, 50) * K, Bark);
			HOLRocksReview::Plain(Owner, HOLRocksReview::Cylinder, Base + FVector(0, 0, 51) * K, FVector(54, 54, 4) * K, Cut);
		}
		else // D: three roots
		{
			HOLRocksReview::Plain(Owner, HOLRocksReview::Cylinder, Base + FVector(0, 0, 25) * K, FVector(55, 55, 50) * K, Bark);
			for (int R = 0; R < 3; R++)
			{
				double Angle = double(R) * 120.0 * 0.0174533;
				HOLRocksReview::Piece(Owner, HOLRocksReview::Cube, Base + FVector(Math::Cos(Angle) * 35.0, Math::Sin(Angle) * 35.0, 9.0) * K, FVector(60, 18, 18) * K, FRotator(0.0f, float(R) * 120.0f, 0.0f), Bark);
			}
		}
	}
}

void HOLRocksReview2Command(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		Log(n"LogHOLMatch", "Rocks review 2: usage hol.Rocks.Review2 <letter A-H>");
		return;
	}
	TArray<FString> Letters;
	Letters.Add("A");
	Letters.Add("B");
	Letters.Add("C");
	Letters.Add("D");
	Letters.Add("E");
	Letters.Add("F");
	Letters.Add("G");
	Letters.Add("H");
	// Faceted rocks out of eight, per letter.
	TArray<int> FacetedOf;
	FacetedOf.Add(8);
	FacetedOf.Add(0);
	FacetedOf.Add(6);
	FacetedOf.Add(4);
	FacetedOf.Add(2);
	FacetedOf.Add(4);
	FacetedOf.Add(8);
	FacetedOf.Add(0);
	FString Letter = Args[0].ToUpper();
	int Variant = -1;
	for (int K = 0; K < Letters.Num(); K++)
	{
		if (Letters[K] == Letter)
		{
			Variant = K;
		}
	}
	TArray<AHOLRocksReviewActor> Existing;
	GetAllActorsOfClass(Existing);
	for (AHOLRocksReviewActor Old : Existing)
	{
		Old.DestroyActor();
	}
	if (Variant < 0)
	{
		Log(n"LogHOLMatch", f"Rocks review 2: letter={Letter} shown=0 pieces=0");
		return;
	}
	int Faceted = FacetedOf[Variant];
	bool AllB = Faceted == 8;
	AHOLRocksReviewActor Shown = Cast<AHOLRocksReviewActor>(SpawnActor(AHOLRocksReviewActor::StaticClass(), RocksReview2Spot));
	// A to E on Fresh grass (biome 0), F to H on Dark forest (biome 3), as the scenario sets with hol.Biome.Index.
	HOLRocksReview2::BiomeGround(Shown, Variant < 5 ? 0 : 3);
	// Eight rocks: X, Y (cm from the spot) and scale (1 = the map's 2,6 m Boulder).
	TArray<FVector> Rocks;
	Rocks.Add(FVector(-250.0, -1300.0, 2.0));
	Rocks.Add(FVector(30.0, -1020.0, 1.3));
	Rocks.Add(FVector(330.0, -550.0, 1.0));
	Rocks.Add(FVector(-320.0, -150.0, 1.2));
	Rocks.Add(FVector(330.0, 300.0, 0.8));
	Rocks.Add(FVector(-300.0, 800.0, 1.1));
	Rocks.Add(FVector(280.0, 1150.0, 1.5));
	Rocks.Add(FVector(520.0, 720.0, 0.7));
	int FacetedShown = 0;
	for (int K = 0; K < Rocks.Num(); K++)
	{
		// (K * 5) % 8 runs through 0..7 once: the faceted rocks are spread over the scatter, not grouped on one side.
		bool Facet = ((K * 5) % 8) < Faceted;
		if (Facet)
		{
			FacetedShown += 1;
		}
		HOLRocksReview2::Rock(Shown, Facet, FVector(Rocks[K].X, Rocks[K].Y, 0.0), Rocks[K].Z);
	}
	HOLRocksReview2::Stones(Shown, AllB, FVector(-50.0, -620.0, 0.0));
	HOLRocksReview2::Stones(Shown, AllB, FVector(50.0, 450.0, 0.0));
	HOLRocksReview2::Stump(Shown, Faceted > 0, FVector(-500.0, -750.0, 0.0));
	HOLRocksReview2::Stump(Shown, AllB, FVector(-450.0, 350.0, 0.0));
	HOLRocksReview::Thicket(Shown, AllB ? 1 : 3, FVector(400.0, -1550.0, 0.0));
	HOLRocksReview::Thicket(Shown, AllB ? 1 : 3, FVector(-150.0, 1500.0, 0.0));
	FVector CameraPlace = Gameplay::GetPlayerCameraManager(0).GetCameraLocation();
	FVector ToCamera = CameraPlace - RocksReview2Spot;
	ToCamera.Z = 0.0;
	ToCamera = ToCamera.GetSafeNormal();
	FVector LabelPlace = RocksReview2Spot + ToCamera * 900.0 + FVector(0.0, 0.0, 150.0);
	Shown.Label.SetText(FText::FromString(f"ROCHERS {Letter}"));
	Shown.Label.SetWorldSize(170.0);
	Shown.Label.SetTextRenderColor(FColor::Yellow);
	Shown.Label.SetCastShadow(false);
	Shown.Label.SetWorldLocation(LabelPlace);
	Shown.Label.SetWorldRotation((CameraPlace - LabelPlace).Rotation());
	Log(n"LogHOLMatch", f"Rocks review 2: letter={Letter} shown=1 faceted={FacetedShown}/8 pieces={Shown.Pieces}");
}

const FConsoleCommand HOLRocksReview2Console("hol.Rocks.Review2", n"HOLRocksReview2Command");
