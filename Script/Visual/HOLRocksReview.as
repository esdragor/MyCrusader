// hol.Rocks.Review <letter A-J> (PlaceholdersInProgress.md, lot "ressources et decor", PQ-1 = A, methode "dix echantillons", tour 1
// des rochers et du decor des cartes): shows ONE style set of the map scenery side by side on the loaded map, from left to right a
// ridge rock (rocher de crete), a boulder (gros rocher, 2,6 m like the map's Boulder), a group of small stones, a tree stump and a
// thicket (fourre, 3,6 m like the map's Thicket), all composed of engine basic shapes (cube, cylinder, cone, sphere) in matte colours,
// at the real size of the map's DECOR_KINDS (Tools/Editor/maps/common.py). Letter A = what the maps draw today (flattened spheres,
// a cylinder), for reference; B to J = nine other STYLES (silhouette and colour), NOT a ranking: the user says liked / disliked per
// letter. No colour may be taken for a resource (no yellow lump, no red berry). Purely a REVIEW tool for the scenario rocks_compare:
// no gameplay, no collision, no shadow, nothing saved, the previous set is destroyed. A yellow label "ROCHERS <letter>" is put on the
// ground in front of the row, turned towards the camera at the moment of the command.
// "Rocks review: letter=C shown=1 pieces=NN" in the log; shown=0 if the letter is unknown.

const FVector RocksReviewSpot = FVector(0.0, -3000.0, 0.0);

class AHOLRocksReviewActor : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;

	UPROPERTY(DefaultComponent)
	UTextRenderComponent Label;

	int Pieces = 0;
}

namespace HOLRocksReview
{
	const int Cube = 0;
	const int Cylinder = 1;
	const int Cone = 2;
	const int Sphere = 3;

	UStaticMesh ShapeOf(int Kind)
	{
		FString Path = "/Engine/BasicShapes/Cube.Cube";
		if (Kind == Cylinder)
		{
			Path = "/Engine/BasicShapes/Cylinder.Cylinder";
		}
		else if (Kind == Cone)
		{
			Path = "/Engine/BasicShapes/Cone.Cone";
		}
		else if (Kind == Sphere)
		{
			Path = "/Engine/BasicShapes/Sphere.Sphere";
		}
		return Cast<UStaticMesh>(LoadObject(nullptr, Path));
	}

	// One piece: Size in cm (the basic shapes are 100 cm), At = its CENTRE relative to the actor, Turn = pitch, yaw, roll.
	void Piece(AHOLRocksReviewActor Owner, int Kind, FVector At, FVector Size, FRotator Turn, FLinearColor Paint)
	{
		UStaticMeshComponent Part = UStaticMeshComponent::Create(Owner, FName(f"RocksPiece_{Owner.Pieces}"));
		Owner.Pieces += 1;
		Part.SetStaticMesh(ShapeOf(Kind));
		Part.AttachToComponent(Owner.Root);
		Part.SetRelativeLocation(At);
		Part.SetRelativeRotation(Turn);
		Part.SetRelativeScale3D(Size * 0.01);
		Part.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part.SetGenerateOverlapEvents(false);
		Part.SetCastShadow(false);
		UMaterialInterface Matte = Cast<UMaterialInterface>(LoadObject(nullptr, "/Game/Rendering/M_MatteColor.M_MatteColor"));
		UMaterialInstanceDynamic Dye = Part.CreateDynamicMaterialInstance(0, Matte);
		if (Dye != nullptr)
		{
			Dye.SetVectorParameterValue(n"Color", Paint);
		}
	}

	void Plain(AHOLRocksReviewActor Owner, int Kind, FVector At, FVector Size, FLinearColor Paint)
	{
		Piece(Owner, Kind, At, Size, FRotator(0.0f, 0.0f, 0.0f), Paint);
	}

	// A piece of a rock of scale S: offsets and sizes are those of a 2,6 m boulder (S = 1), multiplied by S.
	void Bit(AHOLRocksReviewActor Owner, int Kind, FVector Base, double S, FVector At, FVector Size, FRotator Turn, FLinearColor Paint)
	{
		Piece(Owner, Kind, Base + At * S, Size * S, Turn, Paint);
	}

	FLinearColor Body(int Style)
	{
		if (Style == 1) return FLinearColor(0.22f, 0.23f, 0.26f, 1.0f);
		if (Style == 2) return FLinearColor(0.33f, 0.23f, 0.11f, 1.0f);
		if (Style == 3) return FLinearColor(0.28f, 0.26f, 0.23f, 1.0f);
		if (Style == 4) return FLinearColor(0.12f, 0.13f, 0.15f, 1.0f);
		if (Style == 5) return FLinearColor(0.36f, 0.14f, 0.07f, 1.0f);
		if (Style == 6) return FLinearColor(0.55f, 0.53f, 0.48f, 1.0f);
		if (Style == 7) return FLinearColor(0.06f, 0.06f, 0.07f, 1.0f);
		if (Style == 9) return FLinearColor(0.22f, 0.22f, 0.24f, 1.0f);
		return FLinearColor(0.24f, 0.23f, 0.21f, 1.0f); // A and I: the map's Boulder grey
	}

	FLinearColor Accent(int Style)
	{
		if (Style == 0) return FLinearColor(0.27f, 0.255f, 0.23f, 1.0f);
		if (Style == 1) return FLinearColor(0.30f, 0.31f, 0.34f, 1.0f);
		if (Style == 2) return FLinearColor(0.26f, 0.17f, 0.08f, 1.0f);
		if (Style == 3) return FLinearColor(0.20f, 0.19f, 0.17f, 1.0f);
		if (Style == 4) return FLinearColor(0.20f, 0.21f, 0.24f, 1.0f);
		if (Style == 5) return FLinearColor(0.45f, 0.22f, 0.10f, 1.0f);
		if (Style == 6) return FLinearColor(0.35f, 0.34f, 0.31f, 1.0f);
		if (Style == 7) return FLinearColor(0.12f, 0.12f, 0.13f, 1.0f);
		if (Style == 8) return FLinearColor(0.10f, 0.14f, 0.07f, 1.0f); // moss
		return FLinearColor(0.80f, 0.85f, 0.90f, 1.0f); // snow
	}

	// One rock of style Style and scale S (S = 1: about 2,6 x 2 x 1,5 m, the map's Boulder).
	void Rock(AHOLRocksReviewActor Owner, int Style, FVector Base, double S)
	{
		FLinearColor B = Body(Style);
		FLinearColor A = Accent(Style);
		FRotator Flat(0.0f, 0.0f, 0.0f);
		if (Style == 0) // today: one flattened sphere, half sunk
		{
			Bit(Owner, Sphere, Base, S, FVector(0, 0, 45), FVector(260, 200, 150), Flat, B);
		}
		else if (Style == 1) // faceted: two tilted cubes
		{
			Bit(Owner, Cube, Base, S, FVector(0, 0, 55), FVector(200, 170, 140), FRotator(12.0f, 25.0f, 8.0f), B);
			Bit(Owner, Cube, Base, S, FVector(90, -80, 40), FVector(130, 120, 110), FRotator(-18.0f, 70.0f, 14.0f), A);
		}
		else if (Style == 2) // layered sandstone slabs
		{
			Bit(Owner, Cube, Base, S, FVector(0, 0, 25), FVector(260, 200, 50), FRotator(0.0f, 0.0f, 0.0f), B);
			Bit(Owner, Cube, Base, S, FVector(10, 0, 75), FVector(220, 170, 50), FRotator(0.0f, 12.0f, 0.0f), A);
			Bit(Owner, Cube, Base, S, FVector(-10, 10, 125), FVector(160, 130, 50), FRotator(0.0f, -8.0f, 0.0f), B);
		}
		else if (Style == 3) // cluster of round stones
		{
			Bit(Owner, Sphere, Base, S, FVector(0, 0, 60), FVector(180, 160, 140), Flat, B);
			Bit(Owner, Sphere, Base, S, FVector(90, 60, 40), FVector(140, 130, 110), Flat, A);
			Bit(Owner, Sphere, Base, S, FVector(-80, 70, 35), FVector(120, 110, 90), Flat, B);
		}
		else if (Style == 4) // needles: dark slate cones
		{
			Bit(Owner, Cone, Base, S, FVector(0, 0, 150), FVector(110, 110, 300), Flat, B);
			Bit(Owner, Cone, Base, S, FVector(70, 40, 100), FVector(80, 80, 200), Flat, A);
			Bit(Owner, Cone, Base, S, FVector(-60, 50, 80), FVector(70, 70, 160), Flat, B);
		}
		else if (Style == 5) // mesa: rust red, flat top
		{
			Bit(Owner, Cylinder, Base, S, FVector(0, 0, 60), FVector(260, 220, 120), Flat, B);
			Bit(Owner, Cylinder, Base, S, FVector(10, -10, 160), FVector(170, 150, 80), Flat, A);
		}
		else if (Style == 6) // pale limestone block with a dark crack
		{
			Bit(Owner, Cube, Base, S, FVector(0, 0, 70), FVector(240, 190, 140), FRotator(0.0f, 10.0f, 0.0f), B);
			Bit(Owner, Cube, Base, S, FVector(0, 0, 90), FVector(250, 200, 12), FRotator(0.0f, 10.0f, 0.0f), A);
			Bit(Owner, Cube, Base, S, FVector(40, 20, 165), FVector(100, 90, 50), FRotator(0.0f, 25.0f, 0.0f), B);
		}
		else if (Style == 7) // basalt columns
		{
			for (int K = 0; K < 5; K++)
			{
				double Angle = double(K) * 72.0 * 0.0174533;
				double Tall = 120.0 + double((K * 3) % 5) * 35.0;
				double Around = K == 0 ? 0.0 : 75.0;
				FVector At = FVector(Math::Cos(Angle) * Around, Math::Sin(Angle) * Around, Tall * 0.5);
				Bit(Owner, Cylinder, Base, S, At, FVector(75, 75, Tall), Flat, K % 2 == 0 ? B : A);
			}
		}
		else // I = moss cap, J = snow cap, on a grey boulder
		{
			Bit(Owner, Sphere, Base, S, FVector(0, 0, 45), FVector(260, 200, 150), Flat, B);
			Bit(Owner, Sphere, Base, S, FVector(0, 0, 100), FVector(200, 150, 60), Flat, A);
		}
	}

	void Ridge(AHOLRocksReviewActor Owner, int Style, FVector Base)
	{
		Rock(Owner, Style, Base, 2.2);
		Rock(Owner, Style, Base + FVector(260.0, 180.0, 0.0), 1.4);
	}

	void Stones(AHOLRocksReviewActor Owner, int Style, FVector Base)
	{
		for (int K = 0; K < 5; K++)
		{
			double Angle = double(K) * 137.5 * 0.0174533;
			double Around = K == 0 ? 0.0 : 140.0;
			Rock(Owner, Style, Base + FVector(Math::Cos(Angle) * Around, Math::Sin(Angle) * Around, 0.0), 0.28);
		}
	}

	void Stump(AHOLRocksReviewActor Owner, int Style, FVector Base)
	{
		FLinearColor Bark(0.11f, 0.075f, 0.045f, 1.0f);
		FLinearColor Cut(0.35f, 0.25f, 0.15f, 1.0f);
		if (Style == 0) // today: one cylinder
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 22), FVector(50, 50, 45), Bark);
		}
		else if (Style == 1) // light cut face
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 25), FVector(60, 60, 50), Bark);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 51), FVector(54, 54, 4), Cut);
		}
		else if (Style == 2) // flared cone
		{
			Plain(Owner, Cone, Base + FVector(0, 0, 30), FVector(80, 80, 60), Bark);
		}
		else if (Style == 3) // three roots
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 25), FVector(55, 55, 50), Bark);
			for (int K = 0; K < 3; K++)
			{
				double Angle = double(K) * 120.0 * 0.0174533;
				Piece(Owner, Cube, Base + FVector(Math::Cos(Angle) * 35.0, Math::Sin(Angle) * 35.0, 9.0), FVector(60, 18, 18), FRotator(0.0f, float(K) * 120.0f, 0.0f), Bark);
			}
		}
		else if (Style == 4) // tall broken trunk
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 55), FVector(45, 45, 110), Bark);
			Plain(Owner, Cone, Base + FVector(0, 0, 130), FVector(45, 45, 40), Bark);
		}
		else if (Style == 5) // moss cap
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 20), FVector(60, 60, 40), Bark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 42), FVector(70, 70, 25), FLinearColor(0.10f, 0.14f, 0.07f, 1.0f));
		}
		else if (Style == 6) // pale fresh cut
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 22), FVector(50, 50, 45), Bark);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 46), FVector(46, 46, 4), FLinearColor(0.55f, 0.42f, 0.25f, 1.0f));
		}
		else if (Style == 7) // charred
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 30), FVector(55, 55, 60), FLinearColor(0.03f, 0.03f, 0.03f, 1.0f));
		}
		else if (Style == 8) // two stumps
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 22), FVector(50, 50, 45), Bark);
			Plain(Owner, Cylinder, Base + FVector(40, 50, 15), FVector(35, 35, 30), Bark);
		}
		else // snow disc
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 22), FVector(50, 50, 45), Bark);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 47), FVector(56, 56, 8), FLinearColor(0.80f, 0.85f, 0.90f, 1.0f));
		}
	}

	void Thicket(AHOLRocksReviewActor Owner, int Style, FVector Base)
	{
		FLinearColor Dark(0.045f, 0.080f, 0.028f, 1.0f);
		FLinearColor Mid(0.07f, 0.13f, 0.04f, 1.0f);
		if (Style == 0) // today: one flattened sphere
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 56), FVector(360, 320, 160), Dark);
		}
		else if (Style == 1) // three tilted cubes
		{
			Piece(Owner, Cube, Base + FVector(0, 0, 60), FVector(200, 180, 120), FRotator(0.0f, 20.0f, 0.0f), Dark);
			Piece(Owner, Cube, Base + FVector(90, 80, 45), FVector(150, 140, 90), FRotator(0.0f, 55.0f, 0.0f), Mid);
			Piece(Owner, Cube, Base + FVector(-80, 90, 40), FVector(130, 120, 80), FRotator(0.0f, -10.0f, 0.0f), Dark);
		}
		else if (Style == 2) // five cones
		{
			for (int K = 0; K < 5; K++)
			{
				double Angle = double(K) * 72.0 * 0.0174533;
				Plain(Owner, Cone, Base + FVector(Math::Cos(Angle) * 110.0, Math::Sin(Angle) * 110.0, 90.0), FVector(130, 130, 180), K % 2 == 0 ? Dark : Mid);
			}
		}
		else if (Style == 3) // three round blobs
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 65), FVector(220, 200, 150), Dark);
			Plain(Owner, Sphere, Base + FVector(110, 70, 55), FVector(180, 160, 130), Mid);
			Plain(Owner, Sphere, Base + FVector(-100, 80, 45), FVector(160, 150, 110), Dark);
		}
		else if (Style == 4) // thin spikes
		{
			for (int K = 0; K < 7; K++)
			{
				double Angle = double(K) * 360.0 / 7.0 * 0.0174533;
				double Around = K == 0 ? 0.0 : 120.0;
				Plain(Owner, Cone, Base + FVector(Math::Cos(Angle) * Around, Math::Sin(Angle) * Around, 110.0), FVector(70, 70, 220), Dark);
			}
		}
		else if (Style == 5) // dry brown
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 50), FVector(320, 280, 130), FLinearColor(0.150f, 0.100f, 0.040f, 1.0f));
		}
		else if (Style == 6) // bright green pair
		{
			Plain(Owner, Sphere, Base + FVector(0, -60, 60), FVector(240, 220, 150), FLinearColor(0.10f, 0.25f, 0.05f, 1.0f));
			Plain(Owner, Sphere, Base + FVector(40, 90, 50), FVector(200, 180, 120), FLinearColor(0.07f, 0.19f, 0.04f, 1.0f));
		}
		else if (Style == 7) // very dark, low
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 40), FVector(380, 340, 110), FLinearColor(0.02f, 0.05f, 0.02f, 1.0f));
		}
		else if (Style == 8) // two-tone: lighter top
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 56), FVector(360, 320, 160), Dark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 110), FVector(230, 200, 90), Mid);
		}
		else // snow cap
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 56), FVector(360, 320, 160), Dark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 115), FVector(220, 190, 60), FLinearColor(0.80f, 0.85f, 0.90f, 1.0f));
		}
	}
}

void HOLRocksReviewCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		Log(n"LogHOLMatch", "Rocks review: usage hol.Rocks.Review <letter A-J>");
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
	Letters.Add("I");
	Letters.Add("J");
	FString Letter = Args[0].ToUpper();
	int Style = -1;
	for (int K = 0; K < Letters.Num(); K++)
	{
		if (Letters[K] == Letter)
		{
			Style = K;
		}
	}
	TArray<AHOLRocksReviewActor> Existing;
	GetAllActorsOfClass(Existing);
	for (AHOLRocksReviewActor Old : Existing)
	{
		Old.DestroyActor();
	}
	if (Style < 0)
	{
		Log(n"LogHOLMatch", f"Rocks review: letter={Letter} shown=0 pieces=0");
		return;
	}
	AHOLRocksReviewActor Shown = Cast<AHOLRocksReviewActor>(SpawnActor(AHOLRocksReviewActor::StaticClass(), RocksReviewSpot));
	HOLRocksReview::Ridge(Shown, Style, FVector(-100.0, -1450.0, 0.0));
	HOLRocksReview::Rock(Shown, Style, FVector(0.0, -500.0, 0.0), 1.0);
	HOLRocksReview::Stones(Shown, Style, FVector(0.0, 150.0, 0.0));
	HOLRocksReview::Stump(Shown, Style, FVector(0.0, 650.0, 0.0));
	HOLRocksReview::Thicket(Shown, Style, FVector(0.0, 1350.0, 0.0));
	FVector CameraPlace = Gameplay::GetPlayerCameraManager(0).GetCameraLocation();
	FVector ToCamera = CameraPlace - RocksReviewSpot;
	ToCamera.Z = 0.0;
	ToCamera = ToCamera.GetSafeNormal();
	FVector LabelPlace = RocksReviewSpot + ToCamera * 700.0 + FVector(0.0, 0.0, 150.0);
	Shown.Label.SetText(FText::FromString(f"ROCHERS {Letter}"));
	Shown.Label.SetWorldSize(170.0);
	Shown.Label.SetTextRenderColor(FColor::Yellow);
	Shown.Label.SetCastShadow(false);
	Shown.Label.SetWorldLocation(LabelPlace);
	Shown.Label.SetWorldRotation((CameraPlace - LabelPlace).Rotation());
	Log(n"LogHOLMatch", f"Rocks review: letter={Letter} shown=1 pieces={Shown.Pieces}");
}

const FConsoleCommand HOLRocksReviewConsole("hol.Rocks.Review", n"HOLRocksReviewCommand");
