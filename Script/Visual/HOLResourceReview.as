// hol.Resource.Review <letter A-J> (PlaceholdersInProgress.md, lot "ressources et decor", PQ-1 = A, methode "dix echantillons"):
// shows ONE style set of the four resource placeholders side by side on the loaded map: a tree, a berry bush, a stone vein and a gold
// vein, all composed of engine basic shapes (cube, cylinder, cone, sphere) in matte colours, at the real footprint of the source
// (tree 750 cm tall, bush 220 cm across, veins 300-560 cm across). The ten letters are ten different STYLES (silhouette and colour),
// NOT a ranking: the user says liked / disliked per letter. Purely a REVIEW tool for the scenario resources_compare: no gameplay, no
// collision, no shadow, nothing saved, the previous set is destroyed. A yellow label "RESSOURCES <letter>" is put on the ground in
// front of the row, turned towards the camera at the moment of the command (call it again after moving the camera).
// "Resource review: letter=C shown=1 pieces=NN" in the log; shown=0 if the letter is unknown.

const FVector ResourceReviewSpot = FVector(0.0, -3000.0, 0.0);

class AHOLResourceReviewActor : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;

	UPROPERTY(DefaultComponent)
	UTextRenderComponent Label;

	int Pieces = 0;
}

namespace HOLResourceReview
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
	void Piece(AHOLResourceReviewActor Owner, int Kind, FVector At, FVector Size, FRotator Turn, FLinearColor Paint)
	{
		UStaticMeshComponent Part = UStaticMeshComponent::Create(Owner, FName(f"ReviewPiece_{Owner.Pieces}"));
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

	void Plain(AHOLResourceReviewActor Owner, int Kind, FVector At, FVector Size, FLinearColor Paint)
	{
		Piece(Owner, Kind, At, Size, FRotator(0.0f, 0.0f, 0.0f), Paint);
	}

	// Count small pieces on a ring around Base: radius Ring, heights from ZLow in steps, a given size.
	void Dots(AHOLResourceReviewActor Owner, FVector Base, int Count, double Ring, double ZLow, double ZStep, double Size, FLinearColor Paint)
	{
		for (int K = 0; K < Count; K++)
		{
			double Angle = double(K) * 360.0 / double(Count) * 0.0174533;
			double Around = Ring * (1.0 - 0.18 * double(K % 3));
			// Round 2 (2026-11-11): the berries were hardly readable from the game camera, so they are 1.7 times bigger.
			double Berry = Size * 1.7;
			Plain(Owner, Sphere, Base + FVector(Math::Cos(Angle) * Around, Math::Sin(Angle) * Around, ZLow + double(K % 3) * ZStep), FVector(Berry, Berry, Berry), Paint);
		}
	}

	void Tree(AHOLResourceReviewActor Owner, int Style, FVector Base)
	{
		FLinearColor Bark(0.07f, 0.04f, 0.02f, 1.0f);
		FLinearColor Dark(0.03f, 0.17f, 0.05f, 1.0f);
		FLinearColor Mid(0.06f, 0.30f, 0.06f, 1.0f);
		FLinearColor Light(0.16f, 0.42f, 0.07f, 1.0f);
		if (Style == 0) // pine: three stacked cones
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 125), FVector(70, 70, 250), Bark);
			Plain(Owner, Cone, Base + FVector(0, 0, 330), FVector(520, 520, 300), Dark);
			Plain(Owner, Cone, Base + FVector(0, 0, 470), FVector(400, 400, 280), Dark);
			Plain(Owner, Cone, Base + FVector(0, 0, 610), FVector(270, 270, 260), Dark);
		}
		else if (Style == 1) // round oak: one big sphere crown
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 150), FVector(90, 90, 300), Bark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 520), FVector(520, 520, 460), Mid);
		}
		else if (Style == 2) // poplar: thin, tall egg
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 100), FVector(50, 50, 200), Bark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 430), FVector(240, 240, 640), Light);
		}
		else if (Style == 3) // umbrella: flat wide crown on a tall trunk
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 210), FVector(70, 70, 420), Bark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 560), FVector(700, 700, 260), Light);
		}
		else if (Style == 4) // one tall cone
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 100), FVector(60, 60, 200), Bark);
			Plain(Owner, Cone, Base + FVector(0, 0, 480), FVector(420, 420, 560), FLinearColor(0.02f, 0.20f, 0.14f, 1.0f));
		}
		else if (Style == 5) // blob: three offset spheres
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 150), FVector(80, 80, 300), Bark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 430), FVector(360, 360, 360), Light);
			Plain(Owner, Sphere, Base + FVector(-130, -60, 540), FVector(300, 300, 300), Mid);
			Plain(Owner, Sphere, Base + FVector(40, 110, 640), FVector(280, 280, 280), Light);
		}
		else if (Style == 6) // voxel: stacked cubes
		{
			Plain(Owner, Cube, Base + FVector(0, 0, 125), FVector(70, 70, 250), Bark);
			Plain(Owner, Cube, Base + FVector(0, 0, 350), FVector(480, 480, 200), Dark);
			Plain(Owner, Cube, Base + FVector(0, 0, 550), FVector(340, 340, 200), Mid);
			Plain(Owner, Cube, Base + FVector(0, 0, 730), FVector(200, 200, 160), Light);
		}
		else if (Style == 7) // autumn: orange and red
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 150), FVector(90, 90, 300), Bark);
			Plain(Owner, Sphere, Base + FVector(0, 0, 520), FVector(500, 500, 420), FLinearColor(0.55f, 0.15f, 0.02f, 1.0f));
			Plain(Owner, Sphere, Base + FVector(80, 40, 720), FVector(300, 300, 260), FLinearColor(0.50f, 0.05f, 0.02f, 1.0f));
		}
		else if (Style == 8) // bare: a trunk and three branches
		{
			FLinearColor Dead(0.10f, 0.08f, 0.06f, 1.0f);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 200), FVector(70, 70, 400), Dead);
			Piece(Owner, Cylinder, Base + FVector(0, 90, 540), FVector(36, 36, 320), FRotator(0.0f, 0.0f, -30.0f), Dead);
			Piece(Owner, Cylinder, Base + FVector(0, -90, 600), FVector(36, 36, 360), FRotator(0.0f, 0.0f, 30.0f), Dead);
			Piece(Owner, Cylinder, Base + FVector(90, 0, 660), FVector(30, 30, 300), FRotator(-30.0f, 0.0f, 0.0f), Dead);
		}
		else // snow pine: green cones, white top
		{
			FLinearColor Snow(0.80f, 0.85f, 0.90f, 1.0f);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 125), FVector(70, 70, 250), Bark);
			Plain(Owner, Cone, Base + FVector(0, 0, 330), FVector(520, 520, 300), Dark);
			Plain(Owner, Cone, Base + FVector(0, 0, 470), FVector(400, 400, 280), Snow);
			Plain(Owner, Cone, Base + FVector(0, 0, 610), FVector(270, 270, 260), Snow);
		}
	}

	void Bush(AHOLResourceReviewActor Owner, int Style, FVector Base)
	{
		FLinearColor Leaf(0.05f, 0.28f, 0.05f, 1.0f);
		FLinearColor Deep(0.03f, 0.16f, 0.05f, 1.0f);
		FLinearColor Red(0.60f, 0.03f, 0.03f, 1.0f);
		if (Style == 0) // dome with red berries
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 70), FVector(220, 220, 160), Leaf);
			Dots(Owner, Base, 7, 92.0, 70.0, 14.0, 30.0, Red);
		}
		else if (Style == 1) // three blobs, blue berries
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 55), FVector(140, 140, 110), Leaf);
			Plain(Owner, Sphere, Base + FVector(70, 40, 50), FVector(130, 130, 100), Deep);
			Plain(Owner, Sphere, Base + FVector(-60, 50, 50), FVector(120, 120, 100), Leaf);
			Dots(Owner, Base, 8, 70.0, 80.0, 12.0, 26.0, FLinearColor(0.04f, 0.07f, 0.45f, 1.0f));
		}
		else if (Style == 2) // flat cone
		{
			Plain(Owner, Cone, Base + FVector(0, 0, 80), FVector(240, 240, 160), Deep);
			Dots(Owner, Base, 7, 70.0, 50.0, 22.0, 30.0, Red);
		}
		else if (Style == 3) // hedge block
		{
			Plain(Owner, Cube, Base + FVector(0, 0, 60), FVector(200, 200, 120), Leaf);
			Dots(Owner, Base, 8, 104.0, 60.0, 20.0, 28.0, Red);
		}
		else if (Style == 4) // ring of five small spheres
		{
			for (int K = 0; K < 5; K++)
			{
				double Angle = double(K) * 72.0 * 0.0174533;
				FVector At = Base + FVector(Math::Cos(Angle) * 70.0, Math::Sin(Angle) * 70.0, 45.0);
				Plain(Owner, Sphere, At, FVector(100, 100, 90), K % 2 == 0 ? Leaf : Deep);
				Plain(Owner, Sphere, At + FVector(0, 0, 50), FVector(30, 30, 30), Red);
			}
		}
		else if (Style == 5) // low dome, many tiny orange berries
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 50), FVector(260, 260, 120), Deep);
			Dots(Owner, Base, 12, 100.0, 55.0, 12.0, 20.0, FLinearColor(0.70f, 0.25f, 0.02f, 1.0f));
		}
		else if (Style == 6) // olive dome, pink berries
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 70), FVector(200, 200, 150), FLinearColor(0.16f, 0.22f, 0.04f, 1.0f));
			Dots(Owner, Base, 7, 84.0, 70.0, 14.0, 30.0, FLinearColor(0.70f, 0.10f, 0.25f, 1.0f));
		}
		else if (Style == 7) // drum with a cap
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 45), FVector(220, 220, 90), Deep);
			Plain(Owner, Sphere, Base + FVector(0, 0, 95), FVector(200, 200, 100), Leaf);
			Dots(Owner, Base, 6, 80.0, 90.0, 12.0, 28.0, Red);
		}
		else if (Style == 8) // two-tone, pale berries
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 70), FVector(220, 220, 150), Deep);
			Plain(Owner, Sphere, Base + FVector(0, 0, 130), FVector(140, 140, 100), Leaf);
			Dots(Owner, Base, 8, 88.0, 70.0, 14.0, 28.0, FLinearColor(0.80f, 0.60f, 0.60f, 1.0f));
		}
		else // spiky cones with red tips
		{
			for (int K = 0; K < 7; K++)
			{
				double Angle = double(K) * 360.0 / 7.0 * 0.0174533;
				double Around = K == 0 ? 0.0 : 70.0;
				FVector At = Base + FVector(Math::Cos(Angle) * Around, Math::Sin(Angle) * Around, 75.0);
				Plain(Owner, Cone, At, FVector(70, 70, 150), Leaf);
				Plain(Owner, Sphere, At + FVector(0, 0, 80), FVector(28, 28, 28), Red);
			}
		}
	}

	// A vein: Rock = the body colour, Ore = the colour of what is mined (grey-blue for stone, gold for gold).
	void Vein(AHOLResourceReviewActor Owner, int Style, FVector Base, FLinearColor Rock, FLinearColor Ore)
	{
		if (Style == 0) // three tilted cubes
		{
			Piece(Owner, Cube, Base + FVector(0, 0, 130), FVector(300, 300, 260), FRotator(0.0f, 20.0f, 0.0f), Rock);
			Piece(Owner, Cube, Base + FVector(150, -120, 100), FVector(220, 220, 200), FRotator(0.0f, 50.0f, 0.0f), Ore);
			Piece(Owner, Cube, Base + FVector(-140, 100, 90), FVector(180, 180, 180), FRotator(0.0f, -15.0f, 0.0f), Ore);
		}
		else if (Style == 1) // boulder with ore spots
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 130), FVector(560, 480, 300), Rock);
			Dots(Owner, Base, 6, 190.0, 200.0, 20.0, 80.0, Ore);
		}
		else if (Style == 2) // crystals on a flat base
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 40), FVector(520, 520, 100), Rock);
			Plain(Owner, Cone, Base + FVector(0, 0, 200), FVector(130, 130, 340), Ore);
			Piece(Owner, Cone, Base + FVector(120, 40, 160), FVector(110, 110, 260), FRotator(0.0f, 0.0f, 15.0f), Ore);
			Piece(Owner, Cone, Base + FVector(-110, 60, 150), FVector(110, 110, 240), FRotator(0.0f, 0.0f, -15.0f), Ore);
			Piece(Owner, Cone, Base + FVector(20, -120, 140), FVector(100, 100, 220), FRotator(12.0f, 0.0f, 0.0f), Ore);
		}
		else if (Style == 3) // layers
		{
			Plain(Owner, Cylinder, Base + FVector(0, 0, 45), FVector(560, 560, 90), Rock);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 135), FVector(440, 440, 90), Ore);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 225), FVector(320, 320, 90), Rock);
			Plain(Owner, Cylinder, Base + FVector(0, 0, 305), FVector(200, 200, 70), Ore);
		}
		else if (Style == 4) // pile of small spheres
		{
			for (int K = 0; K < 8; K++)
			{
				double Angle = double(K) * 137.5 * 0.0174533;
				double Around = K < 5 ? 150.0 : 50.0;
				double Up = K < 5 ? 80.0 : 210.0;
				Plain(Owner, Sphere, Base + FVector(Math::Cos(Angle) * Around, Math::Sin(Angle) * Around, Up), FVector(230, 230, 200), K % 2 == 0 ? Rock : Ore);
			}
		}
		else if (Style == 5) // big cube, small ore cubes on top
		{
			Plain(Owner, Cube, Base + FVector(0, 0, 150), FVector(400, 400, 300), Rock);
			Piece(Owner, Cube, Base + FVector(100, 100, 330), FVector(120, 120, 120), FRotator(20.0f, 30.0f, 10.0f), Ore);
			Piece(Owner, Cube, Base + FVector(-100, 90, 320), FVector(110, 110, 110), FRotator(10.0f, 60.0f, 20.0f), Ore);
			Piece(Owner, Cube, Base + FVector(0, -110, 330), FVector(130, 130, 130), FRotator(30.0f, 10.0f, 5.0f), Ore);
		}
		else if (Style == 6) // ring of spikes
		{
			for (int K = 0; K < 7; K++)
			{
				double Angle = double(K) * 360.0 / 7.0 * 0.0174533;
				Plain(Owner, Cone, Base + FVector(Math::Cos(Angle) * 150.0, Math::Sin(Angle) * 150.0, 120.0 + double(K % 3) * 30.0), FVector(110, 110, 240 + double(K % 3) * 60.0), K % 2 == 0 ? Rock : Ore);
			}
			Plain(Owner, Cone, Base + FVector(0, 0, 200), FVector(140, 140, 400), Ore);
		}
		else if (Style == 7) // half-buried dome studded with ore
		{
			Plain(Owner, Sphere, Base + FVector(0, 0, 100), FVector(560, 560, 360), Rock);
			Dots(Owner, Base, 9, 215.0, 130.0, 40.0, 70.0, Ore);
		}
		else if (Style == 8) // leaning slabs
		{
			Piece(Owner, Cube, Base + FVector(0, 0, 160), FVector(280, 70, 320), FRotator(0.0f, 0.0f, 12.0f), Rock);
			Piece(Owner, Cube, Base + FVector(-110, 90, 140), FVector(280, 70, 280), FRotator(0.0f, 60.0f, -14.0f), Ore);
			Piece(Owner, Cube, Base + FVector(110, 90, 150), FVector(280, 70, 300), FRotator(0.0f, 120.0f, 10.0f), Rock);
		}
		else // diamond on a plinth
		{
			Plain(Owner, Cube, Base + FVector(0, 0, 80), FVector(420, 420, 160), Rock);
			Piece(Owner, Cube, Base + FVector(0, 0, 250), FVector(220, 220, 220), FRotator(45.0f, 0.0f, 45.0f), Ore);
		}
	}
}

void HOLResourceReviewCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		Log(n"LogHOLMatch", "Resource review: usage hol.Resource.Review <letter A-J>");
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
	TArray<AHOLResourceReviewActor> Existing;
	GetAllActorsOfClass(Existing);
	for (AHOLResourceReviewActor Old : Existing)
	{
		Old.DestroyActor();
	}
	if (Style < 0)
	{
		Log(n"LogHOLMatch", f"Resource review: letter={Letter} shown=0 pieces=0");
		return;
	}
	AHOLResourceReviewActor Shown = Cast<AHOLResourceReviewActor>(SpawnActor(AHOLResourceReviewActor::StaticClass(), ResourceReviewSpot));
	HOLResourceReview::Tree(Shown, Style, FVector(0.0, -1400.0, 0.0));
	HOLResourceReview::Bush(Shown, Style, FVector(0.0, -450.0, 0.0));
	HOLResourceReview::Vein(Shown, Style, FVector(0.0, 450.0, 0.0), FLinearColor(0.20f, 0.20f, 0.22f, 1.0f), FLinearColor(0.50f, 0.54f, 0.62f, 1.0f));
	HOLResourceReview::Vein(Shown, Style, FVector(0.0, 1400.0, 0.0), FLinearColor(0.16f, 0.13f, 0.09f, 1.0f), FLinearColor(0.95f, 0.62f, 0.04f, 1.0f));
	FVector CameraPlace = Gameplay::GetPlayerCameraManager(0).GetCameraLocation();
	FVector ToCamera = CameraPlace - ResourceReviewSpot;
	ToCamera.Z = 0.0;
	ToCamera = ToCamera.GetSafeNormal();
	FVector LabelPlace = ResourceReviewSpot + ToCamera * 700.0 + FVector(0.0, 0.0, 150.0);
	Shown.Label.SetText(FText::FromString(f"RESSOURCES {Letter}"));
	Shown.Label.SetWorldSize(170.0);
	Shown.Label.SetTextRenderColor(FColor::Yellow);
	Shown.Label.SetCastShadow(false);
	Shown.Label.SetWorldLocation(LabelPlace);
	Shown.Label.SetWorldRotation((CameraPlace - LabelPlace).Rotation());
	Log(n"LogHOLMatch", f"Resource review: letter={Letter} shown=1 pieces={Shown.Pieces}");
}

const FConsoleCommand HOLResourceReviewConsole("hol.Resource.Review", n"HOLResourceReviewCommand");
