// hol.Art.Gallery [Units|Buildings|Sources|All] (ArtistesInProgress.md, lot AR1-a, niveau Atelier):
// in a Plain match, puts ONE of each type of the game tables (UnitTypes, BuildingTypes, SourceTypes rows, read from the tables,
// never a hand-written list) on a grid far from the Forum, combat off (hol.Combat.Enabled 0), each object labelled on the ground
// with its type name and its display name. A type added to a table shows up on its own.
// Families side by side along X, rows going south (-Y) from GalleryStartY (the review spot of hol.Forum.Review, away from the
// map centre ring of trees): Units centred on X = -8000, Buildings on X = 0, Sources on X = +8000.
// Units: SpawnUnitsAround (slot 0, the player). Buildings: HOLRpcBridge::CheatSpawnBuilding (slot 0). Sources: the source
// subsystem's AddSource is not callable from script (no UFUNCTION): each source type is shown by a STAND-IN cylinder of its table
// footprint (Radius, Height) until AddSource is exposed (lot C++ groupe) ; the label says so.
// Log: "Art gallery: family=<F> places=<N> units=<n> buildings=<n> sources=<n> refused=<n>", one line per refused type.
// Calling it again removes the labels and stand-ins (not the units and buildings already made).

struct FHOLArtGalleryPlace
{
	FName TypeName;
	FString DisplayName;
	FString Family;
	FVector Location;
	double Radius = 0.0;
	double Height = 0.0;
}

class AHOLArtGalleryLabel : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;

	UPROPERTY(DefaultComponent)
	UTextRenderComponent Label;

	UPROPERTY(DefaultComponent)
	UStaticMeshComponent StandIn;
}

namespace HOLArtGallery
{
	const double GalleryStartY = -3000.0;

	FString TablePath(const FString& Family)
	{
		if (Family == "Units")
			return "/Game/Units/UnitTypes.UnitTypes";
		if (Family == "Buildings")
			return "/Game/Buildings/BuildingTypes.BuildingTypes";
		return "/Game/Resources/SourceTypes.SourceTypes";
	}

	double Spacing(const FString& Family)
	{
		if (Family == "Units")
			return 800.0;
		if (Family == "Buildings")
			return 2200.0;
		return 1200.0;
	}

	int Columns(const FString& Family)
	{
		if (Family == "Buildings")
			return 5;
		return 6;
	}

	double CentreX(const FString& Family)
	{
		if (Family == "Units")
			return -8000.0;
		if (Family == "Buildings")
			return 0.0;
		return 8000.0;
	}

	// The display name as the table exports it: an FText export may be NSLOCTEXT("ns", "key", "text") or INVTEXT("text"); keep the text.
	FString CleanText(const FString& Exported)
	{
		FString Text = Exported;
		if (Text.EndsWith("\")"))
		{
			int Last = Text.Len() - 2;
			int Open = -1;
			for (int K = Last - 1; K >= 0; K--)
			{
				if (Text.Mid(K, 1) == "\"")
				{
					Open = K;
					break;
				}
			}
			if (Open >= 0)
			{
				Text = Text.Mid(Open + 1, Last - Open - 1);
			}
		}
		return Text.Replace("\\'", "'");
	}

	double ToNumber(const FString& Text)
	{
		return String::Conv_StringToDouble(Text);
	}

	void PlanFamily(const FString& Family, TArray<FHOLArtGalleryPlace>& OutPlaces)
	{
		UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, TablePath(Family)));
		if (Table == nullptr)
		{
			return;
		}
		TArray<FName> Names;
		Table.GetRowNames(Names);
		TArray<FString> Shown = Table.GetColumnAsString(n"DisplayName");
		TArray<FString> Radii;
		TArray<FString> Heights;
		if (Family == "Sources")
		{
			Radii = Table.GetColumnAsString(n"Radius");
			Heights = Table.GetColumnAsString(n"Height");
		}
		const double Step = Spacing(Family);
		const int Across = Columns(Family);
		const double Left = CentreX(Family) - Step * double(Across - 1) * 0.5;
		for (int K = 0; K < Names.Num(); K++)
		{
			FHOLArtGalleryPlace Place;
			Place.TypeName = Names[K];
			Place.Family = Family;
			Place.DisplayName = K < Shown.Num() ? CleanText(Shown[K]) : Names[K].ToString();
			if (Place.DisplayName.IsEmpty())
			{
				Place.DisplayName = Names[K].ToString();
			}
			Place.Location = FVector(Left + Step * double(K % Across), GalleryStartY - Step * double(Math::IntegerDivisionTrunc(K, Across)), 0.0);
			if (K < Radii.Num())
				Place.Radius = ToNumber(Radii[K]);
			if (K < Heights.Num())
				Place.Height = ToNumber(Heights[K]);
			OutPlaces.Add(Place);
		}
	}

	// The places of the gallery for one family (Units, Buildings, Sources, any case) or All, one per row of the type tables.
	void PlanGrid(const FString& Family, TArray<FHOLArtGalleryPlace>& OutPlaces)
	{
		OutPlaces.Reset();
		const FString Wanted = Family.ToLower();
		const bool bAll = Wanted == "all" || Wanted.IsEmpty();
		if (bAll || Wanted == "units")
			PlanFamily("Units", OutPlaces);
		if (bAll || Wanted == "buildings")
			PlanFamily("Buildings", OutPlaces);
		if (bAll || Wanted == "sources")
			PlanFamily("Sources", OutPlaces);
	}

	void PutLabel(const FHOLArtGalleryPlace& Place, double Ahead, const FString& Extra)
	{
		AHOLArtGalleryLabel Shown = Cast<AHOLArtGalleryLabel>(SpawnActor(AHOLArtGalleryLabel::StaticClass(), Place.Location));
		if (Shown == nullptr)
			return;
		Shown.Label.SetText(FText::FromString(f"{Place.TypeName.ToString()}\n{Place.DisplayName}{Extra}"));
		Shown.Label.SetWorldSize(Place.Family == "Buildings" ? 150.0 : 60.0);
		Shown.Label.SetTextRenderColor(FColor::Yellow);
		Shown.Label.SetCastShadow(false);
		// Flat on the ground in front of the object (towards the camera, south), read from above.
		Shown.Label.SetWorldLocation(Place.Location + FVector(0.0, -Ahead, 15.0));
		Shown.Label.SetWorldRotation(FRotator(90.0, 180.0, 0.0));
		Shown.StandIn.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Shown.StandIn.SetCastShadow(false);
		if (Place.Family == "Sources")
		{
			Shown.StandIn.SetStaticMesh(Cast<UStaticMesh>(LoadObject(nullptr, "/Engine/BasicShapes/Cylinder.Cylinder")));
			const double Wide = Math::Max(Place.Radius, 30.0) * 2.0;
			const double High = Math::Max(Place.Height, 50.0);
			Shown.StandIn.SetWorldLocation(Place.Location + FVector(0.0, 0.0, High * 0.5));
			Shown.StandIn.SetWorldScale3D(FVector(Wide, Wide, High) * 0.01);
		}
	}
}

void HOLArtGalleryCommand(const TArray<FString>& Args)
{
	const FString Family = Args.Num() >= 1 ? Args[0] : "All";
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanGrid(Family, Places);
	if (Places.Num() == 0)
	{
		Log(n"LogHOLMatch", f"Art gallery: family={Family} places=0 (usage hol.Art.Gallery [Units|Buildings|Sources|All])");
		return;
	}
	System::ExecuteConsoleCommand("hol.Combat.Enabled 0");
	TArray<AHOLArtGalleryLabel> Existing;
	GetAllActorsOfClass(Existing);
	for (AHOLArtGalleryLabel Old : Existing)
	{
		Old.DestroyActor();
	}
	AHOLPlayerController Controller = Cast<AHOLPlayerController>(Gameplay::GetPlayerController(0));
	AActor Rpc = Controller != nullptr ? HOLRpcBridge::FindPlayerActor(Controller) : nullptr;
	int Units = 0;
	int Buildings = 0;
	int Sources = 0;
	int Refused = 0;
	for (int K = 0; K < Places.Num(); K++)
	{
		const FHOLArtGalleryPlace Place = Places[K];
		bool bMade = false;
		if (Place.Family == "Units")
		{
			bMade = Controller != nullptr && HOLControllerRead::SpawnUnitsAround(Controller, 1, Place.TypeName, 0, 0.0, Place.Location);
			HOLArtGallery::PutLabel(Place, 150.0, bMade ? "" : "\n(non pose)");
			Units += bMade ? 1 : 0;
		}
		else if (Place.Family == "Buildings")
		{
			bMade = Rpc != nullptr && HOLRpcBridge::CheatSpawnBuilding(Rpc, Place.TypeName.ToString(), 0, Place.Location.X, Place.Location.Y, Place.Location.Z);
			HOLArtGallery::PutLabel(Place, 900.0, bMade ? "" : "\n(non pose)");
			Buildings += bMade ? 1 : 0;
		}
		else
		{
			bMade = true;
			HOLArtGallery::PutLabel(Place, Math::Max(Place.Radius, 30.0) + 120.0, "\n(forme d attente)");
			Sources += 1;
		}
		if (!bMade)
		{
			Refused += 1;
			Log(n"LogHOLMatch", f"Art gallery: refused {Place.Family} {Place.TypeName.ToString()}");
		}
	}
	Log(n"LogHOLMatch", f"Art gallery: family={Family} places={Places.Num()} units={Units} buildings={Buildings} sources={Sources} refused={Refused}");
}

const FConsoleCommand HOLArtGalleryConsole("hol.Art.Gallery", n"HOLArtGalleryCommand");
