// AR5-a (GLORFINDEL-1125ar3) : cabine de capture des icones. Console : hol.Art.Icon <Type|All|FirstSource>.
// Dans une partie Plain : pose chaque type SEUL sur sa place degagee (hol.Decor.ClearNear ... Trees), camera et lumiere FIXES
// (HOLArtIconRules::Shot : meme angle et meme champ pour toute une famille, distance selon la hauteur des bornes), capture
// 256x256 (SceneCaptureComponent2D, couleur finale LDR) exportee en PNG dans Saved\ArtIcons\T_Unit_<Type>.png (ou
// T_Building_ / T_Source_). Import en Texture2D (/Game/UI/Icons/Units/T_Unit_<Type>) : plus tard, dans l'editeur.
// AR5-b : fond neutre = mur uni (cube du moteur) pose debout derriere l'objet, face a la camera, plus un degagement des arbres
// a la mesure du champ (3000 cm, 6000 pour un batiment).
// Bornes : batiment = bornes de l'acteur pose ; unite = vrai maillage du DA_Art x MeshScale (HOLArtIconRules::UnitBounds) ; source = Radius /
// Height de SourceTypes ; AR5-c : vraie source du registre (HOLSourceAdd::AddNeutralSource), plus de forme d'attente.
// Journal : "Art icon: wrote <fichier> type=<T> family=<F> height=<h>" par type, "Art icon: refused <T>" si hors tables,
// puis "Art icon: done count=<n>".

class AHOLArtIconBooth : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;

	UPROPERTY(DefaultComponent)
	USceneCaptureComponent2D Capture;

	// Fixed key light, carried with the camera (same light for every type).
	UPROPERTY(DefaultComponent)
	UPointLightComponent Key;

	UPROPERTY(DefaultComponent)
	UStaticMeshComponent StandIn;

	// AR5-b: plain wall behind the object, facing the camera (neutral background, no tree or decor in the frame).
	UPROPERTY(DefaultComponent)
	UStaticMeshComponent Backdrop;

	// AR5-d: plain floor under a building (neutral ground, not the owner's coloured ground nor the grass).
	UPROPERTY(DefaultComponent)
	UStaticMeshComponent Floor;

	TArray<FName> Queue;
	FName Current = NAME_None;
	FString Family;
	FVector Spot;
	int Wait = 0;
	int Written = 0;
	// AR5-d: one spot counter per family (AR5-c shared one: the sources, last of 51, went 6 rows down).
	TMap<FString, int> IndexOf;
	int Total = 0;

	void Start(const TArray<FName>& Types)
	{
		Queue = Types;
		Total = Types.Num();
		Capture.bCaptureEveryFrame = false;
		Capture.bCaptureOnMovement = false;
		Capture.CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		Capture.TextureTarget = Rendering::CreateRenderTarget2D(256, 256, ETextureRenderTargetFormat::RTF_RGBA8);
		Key.SetIntensity(20000.0);
		Key.SetAttenuationRadius(20000.0);
		Key.SetCastShadows(false);
		StandIn.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StandIn.SetVisibility(false);
		Backdrop.SetStaticMesh(Cast<UStaticMesh>(LoadObject(nullptr, "/Engine/BasicShapes/Cube.Cube")));
		Backdrop.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Backdrop.SetCastShadow(false);
		Backdrop.SetVisibility(false);
		Floor.SetStaticMesh(Backdrop.StaticMesh);
		Floor.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Floor.SetCastShadow(false);
		Floor.SetVisibility(false);
	}

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaSeconds)
	{
		if (Wait > 0)
		{
			Wait -= 1;
			if (Wait == 0)
			{
				Shoot();
			}
			return;
		}
		if (Queue.Num() == 0)
		{
			Log(n"LogHOLMatch", f"Art icon: done count={Written} types={Total}");
			DestroyActor();
			return;
		}
		Current = Queue[0];
		Queue.RemoveAt(0);
		PlaceCurrent();
	}

	void PlaceCurrent()
	{
		Family = HOLArtIconRules::FamilyOf(Current);
		if (Family.IsEmpty())
		{
			Log(n"LogHOLMatch", f"Art icon: refused {Current.ToString()} (in no type table)");
			return;
		}
		// One clear spot per type, south-west of the map centre, away from the Forum and the gallery; a second call of the same
		// family reuses its spots. AR5-d: own counter per family; units rows from Y -6000 (21 types: 4 rows of 6, X step 2500),
		// sources half-way between the unit rows (Y -8250, X shifted 1250), buildings below the units (Y -23000, 5 a row, X step 3000).
		int Index = 0;
		IndexOf.Find(Family, Index);
		IndexOf.Add(Family, Index + 1);
		if (Family == "Buildings")
			Spot = FVector(-6000.0 + 3000.0 * double(Index % 5), -23000.0 - 4500.0 * double(Math::IntegerDivisionTrunc(Index, 5)), 0.0);
		else if (Family == "Sources")
			Spot = FVector(-4750.0 + 2500.0 * double(Index % 6), -8250.0 - 4500.0 * double(Math::IntegerDivisionTrunc(Index, 6)), 0.0);
		else
			Spot = FVector(-6000.0 + 2500.0 * double(Index % 6), -6000.0 - 4500.0 * double(Math::IntegerDivisionTrunc(Index, 6)), 0.0);
		const int Clear = Family == "Buildings" ? 6000 : 3000;
		System::ExecuteConsoleCommand(f"hol.Decor.ClearNear {Math::RoundToInt(Spot.X)} {Math::RoundToInt(Spot.Y)} {Clear} Trees");
		StandIn.SetVisibility(false);
		Floor.SetVisibility(false);
		bool bMade = false;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(Gameplay::GetPlayerController(0));
		if (Family == "Units")
		{
			bMade = Controller != nullptr && HOLControllerRead::SpawnUnitsAround(Controller, 1, Current, 0, 0.0, Spot);
		}
		else if (Family == "Buildings")
		{
			AActor Rpc = Controller != nullptr ? HOLRpcBridge::FindPlayerActor(Controller) : nullptr;
			bMade = Rpc != nullptr && HOLRpcBridge::CheatSpawnBuilding(Rpc, Current.ToString(), 0, Spot.X, Spot.Y, Spot.Z);
		}
		else
		{
			// AR5-c: a real source of the registry (HOLSourceAdd::AddNeutralSource, in main since integ-88-art), everyone's, its table amount.
			// AR5-d: the call itself (8517f28 removed the stand-in's bMade = true without writing it: every source "not placed").
			bMade = HOLSourceAdd::AddNeutralSource(Current, Spot, -1) >= 0;
		}
		if (!bMade)
		{
			Log(n"LogHOLMatch", f"Art icon: refused {Current.ToString()} (not placed)");
			return;
		}
		// Let the unit / building appear (Mass visuals, building actor) before the capture.
		Wait = 45;
	}

	void SourceBounds(double& OutHeight, double& OutRadius)
	{
		OutHeight = 150.0;
		OutRadius = 60.0;
		UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, "/Game/Resources/SourceTypes.SourceTypes"));
		if (Table == nullptr)
			return;
		TArray<FName> Names;
		Table.GetRowNames(Names);
		TArray<FString> Radii = Table.GetColumnAsString(n"Radius");
		TArray<FString> Heights = Table.GetColumnAsString(n"Height");
		const int Row = Names.FindIndex(Current);
		if (Row >= 0 && Row < Radii.Num())
			OutRadius = Math::Max(String::Conv_StringToDouble(Radii[Row]), 30.0);
		if (Row >= 0 && Row < Heights.Num())
			OutHeight = Math::Max(String::Conv_StringToDouble(Heights[Row]), 50.0);
	}

	void Shoot()
	{
		double Height = 180.0;
		double Radius = 60.0;
		double Ground = Spot.Z;
		bool bFloor = false;
		if (Family == "Buildings")
		{
			TArray<AHOLBuilding> Buildings;
			GetAllActorsOfClass(Buildings);
			double Best = 1500.0;
			for (AHOLBuilding Building : Buildings)
			{
				FVector Gap = Building.GetActorLocation() - Spot;
				Gap.Z = 0.0;
				if (Gap.Size() < Best)
				{
					Best = Gap.Size();
					FVector Origin;
					FVector Extent;
					Building.GetActorBounds(false, Origin, Extent);
					Height = Math::Max(Extent.Z * 2.0, 100.0);
					Radius = Math::Max(Math::Max(Extent.X, Extent.Y), 100.0);
					Ground = Building.GetActorLocation().Z;
					Spot = FVector(Building.GetActorLocation().X, Building.GetActorLocation().Y, Spot.Z);
				}
			}
			// AR5-d: framed on its BuildingTypes footprint (half diagonal) and Height; the actor's bounds cut the Barracks.
			double TableHeight = 0.0;
			double TableRadius = 0.0;
			if (HOLArtIconRules::BuildingBounds(Current, TableHeight, TableRadius))
			{
				Height = TableHeight;
				Radius = TableRadius;
			}
			bFloor = true;
		}
		else if (Family == "Units")
		{
			HOLArtIconRules::UnitBounds(Current, Height, Radius);
		}
		else if (Family == "Sources")
		{
			SourceBounds(Height, Radius);
		}
		const FHOLArtIconShot Shot = HOLArtIconRules::Shot(Family, Height, Radius);
		const FVector Eye = Spot + Shot.CameraOffset;
		Capture.SetWorldLocationAndRotation(Eye, Shot.Rotation);
		Capture.FOVAngle = Shot.FieldOfView;
		// The wall stands behind the object (past its bounds), upright, as wide and tall as twice the camera's reach.
		const double Yaw = Math::DegreesToRadians(Shot.Rotation.Yaw);
		const FVector Flat = FVector(Math::Cos(Yaw), Math::Sin(Yaw), 0.0);
		const double Gap = Radius * 1.5 + 200.0;
		const double Side = (Shot.Distance + Gap) * 2.0;
		Backdrop.SetWorldLocationAndRotation(Spot + Flat * Gap + FVector(0.0, 0.0, Side * 0.5 - 50.0), FRotator(0.0, Shot.Rotation.Yaw, 0.0));
		Backdrop.SetWorldScale3D(FVector(0.1, Side * 0.01, Side * 0.01));
		Backdrop.SetVisibility(true);
		// A thin plain slab whose top is 5 cm above the building's ground point (hides the coloured ground under it).
		Floor.SetWorldLocationAndRotation(FVector(Spot.X, Spot.Y, Ground + 4.0), FRotator(0.0, Shot.Rotation.Yaw, 0.0));
		Floor.SetWorldScale3D(FVector(Side * 0.01, Side * 0.01, 0.02));
		Floor.SetVisibility(bFloor);
		Key.SetWorldLocation(Eye + FVector(0.0, 0.0, Shot.Distance * 0.5));
		Capture.CaptureScene();
		const FString File = HOLArtIconRules::FileName(Family, Current);
		const FString Folder = FPaths::ProjectDir() + "Saved/ArtIcons";
		Rendering::ExportRenderTarget(Capture.TextureTarget, Folder, File);
		Written += 1;
		Log(n"LogHOLMatch", f"Art icon: wrote {Folder}/{File} type={Current.ToString()} family={Family} height={Height}");
	}
}

namespace HOLArtIconBooth
{
	// The types asked: one name, All (every row of the three tables), or FirstSource (the first SourceTypes row).
	void TypesFor(const FString& Asked, TArray<FName>& OutTypes)
	{
		OutTypes.Reset();
		TArray<FString> Paths;
		if (Asked.ToLower() == "all")
		{
			Paths.Add("/Game/Units/UnitTypes.UnitTypes");
			Paths.Add("/Game/Buildings/BuildingTypes.BuildingTypes");
			Paths.Add("/Game/Resources/SourceTypes.SourceTypes");
		}
		else if (Asked.ToLower() == "firstsource")
		{
			Paths.Add("/Game/Resources/SourceTypes.SourceTypes");
		}
		else
		{
			OutTypes.Add(FName(Asked));
			return;
		}
		for (int K = 0; K < Paths.Num(); K++)
		{
			UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, Paths[K]));
			if (Table == nullptr)
				continue;
			TArray<FName> Names;
			Table.GetRowNames(Names);
			if (Asked.ToLower() == "firstsource")
			{
				if (Names.Num() > 0)
					OutTypes.Add(Names[0]);
				return;
			}
			OutTypes.Append(Names);
		}
	}
}

void HOLArtIconCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		Log(n"LogHOLMatch", "Art icon: usage hol.Art.Icon <Type|All|FirstSource>");
		return;
	}
	TArray<FName> Types;
	HOLArtIconBooth::TypesFor(Args[0], Types);
	System::ExecuteConsoleCommand("hol.Combat.Enabled 0");
	AHOLArtIconBooth Booth = Cast<AHOLArtIconBooth>(SpawnActor(AHOLArtIconBooth::StaticClass(), FVector(-6000.0, -9000.0, 0.0)));
	if (Booth == nullptr)
	{
		Log(n"LogHOLMatch", "Art icon: done count=0 (no booth)");
		return;
	}
	Booth.Start(Types);
}

const FConsoleCommand HOLArtIconConsole("hol.Art.Icon", n"HOLArtIconCommand");
