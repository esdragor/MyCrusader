// hol.Art.Gallery [Units|Buildings|Sources|All] (ArtistesInProgress.md, lot AR1-a, niveau Atelier):
// in a Plain match, puts ONE of each type of the game tables (UnitTypes, BuildingTypes, SourceTypes rows, read from the tables,
// never a hand-written list) on a grid far from the Forum, combat off (hol.Combat.Enabled 0), each object labelled on the ground
// with its type name and its display name. A type added to a table shows up on its own.
// Families side by side along X, rows going south (-Y) from GalleryStartY (the review spot of hol.Forum.Review, away from the
// map centre ring of trees): Units centred on X = -8000, Buildings on X = 0, Sources on X = +8000.
// Units: SpawnUnitsAround (slot 0, the player). Buildings: HOLRpcBridge::CheatSpawnBuilding (slot 0). Sources: real sources of the
// registry, everyone's (HOLSourceAdd::AddNeutralSource, AR1-b). Each family moves whole away from the local Forum (MinBaseDistance);
// hol.Art.GalleryFrame <Family> puts the camera on one family's frame (one capture per family).
// Log: "Art gallery: family=<F> places=<N> units=<n> buildings=<n> sources=<n> refused=<n>", one line per refused type.
// Calling it again removes the labels (not the units, buildings and sources already made).

struct FHOLArtGalleryPlace
{
	FName TypeName;
	FString DisplayName;
	FString Family;
	FVector Location;
	double Radius = 0.0;
	double Height = 0.0;
	// AR1-c: a building's ground footprint (cm, X by Y), read from the Footprint column of BuildingTypes; 0 for units and sources.
	double FootprintX = 0.0;
	double FootprintY = 0.0;
}

// AR1-c: the scale marks of the gallery (one actor, one instanced part per kind of mark).
class AHOLArtGalleryMarks : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;
}

class AHOLArtGalleryLabel : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;

	UPROPERTY(DefaultComponent)
	UTextRenderComponent Label;
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

	// AR1-c: the kinds of scale mark (one colour each): the footprint outline, a 4 m tile line, a sub-cell line, the Height pole.
	const int MarkOutline = 0;
	const int MarkTile = 1;
	const int MarkSub = 2;
	const int MarkHeight = 3;

	// AR1-c: the height of the reference figure standing beside each object (cm): a man of 1,80 m.
	double ReferenceHeightCm()
	{
		return 180.0;
	}

	// The number after Key in an exported struct ("(X=1600.0,Y=1600.0)"), up to the next comma or closing bracket; 0 without Key.
	double NumberAfter(const FString& Text, const FString& Key)
	{
		return 0.0;
	}

	// AR1-c: the footprint (cm) of an exported Footprint cell of BuildingTypes.
	FVector2D FootprintOf(const FString& Exported)
	{
		return FVector2D(NumberAfter(Exported, "X="), NumberAfter(Exported, "Y="));
	}

	// AR1-c: where the 1,80 m reference stands: beside the object, on its +Y side (the right of the screen, the camera looking towards +X),
	// clear of a building's footprint or of a unit's / source's radius.
	FVector ScaleBarBase(const FHOLArtGalleryPlace& Place)
	{
		return Place.Location;
	}

	// AR1-c: the ground lines of a building (none for a unit or a source): its footprint outline, the lines of the construction grid
	// inside it (4 m tiles of Tile cm, sub-cells of Tile / SubDivision), and the vertical Height pole at its near right corner.
	void MarkLines(const FHOLArtGalleryPlace& Place, double Tile, int SubDivision, TArray<FVector>& OutStarts, TArray<FVector>& OutEnds, TArray<int>& OutKinds)
	{
		OutStarts.Reset();
		OutEnds.Reset();
		OutKinds.Reset();
	}

	// AR1-c (hol.Art.Light): the index of a biome of the game's table (HOLBiome::At(I).Key for I < HOLBiome::Count, any case, or its index),
	// -1 for the map's own look ("default", "map"), -2 for a name the game does not know.
	int BiomeIndexFromName(const FString& Name)
	{
		return -2;
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
		TArray<FString> Footprints;
		if (Family == "Sources")
		{
			Radii = Table.GetColumnAsString(n"Radius");
			Heights = Table.GetColumnAsString(n"Height");
		}
		else if (Family == "Buildings")
		{
			Footprints = Table.GetColumnAsString(n"Footprint");
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
			if (K < Footprints.Num())
			{
				const FVector2D Size = FootprintOf(Footprints[K]);
				Place.FootprintX = Size.X;
				Place.FootprintY = Size.Y;
			}
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
		Shown.Label.SetWorldSize(Place.Family == "Buildings" ? 150.0 : (Place.Family == "Sources" ? 90.0 : 60.0));
		Shown.Label.SetTextRenderColor(FColor::Yellow);
		Shown.Label.SetCastShadow(false);
		// Flat on the ground in front of the object (towards the camera, south), read from above.
		Shown.Label.SetWorldLocation(Place.Location + FVector(0.0, -Ahead, 15.0));
		Shown.Label.SetWorldRotation(FRotator(90.0, 180.0, 0.0));
	}

	// AR1-b: the widest side one capture shows whole (cm, camera zoomed out), and the least distance from a player's Forum.
	double MaxFrameWidth()
	{
		return 12000.0;
	}

	double MinBaseDistance()
	{
		return 6000.0;
	}

	// AR1-b: the square that one capture of Family frames (centre, side): every place of the family inside it.
	void FamilyFrame(const TArray<FHOLArtGalleryPlace>& Places, const FString& Family, FVector& OutCentre, double& OutWidth)
	{
		OutCentre = FVector::ZeroVector;
		OutWidth = 0.0;
		FVector Low;
		FVector High;
		int Seen = 0;
		for (const FHOLArtGalleryPlace& Place : Places)
		{
			if (Place.Family != Family)
				continue;
			Low = Seen == 0 ? Place.Location : FVector(Math::Min(Low.X, Place.Location.X), Math::Min(Low.Y, Place.Location.Y), 0.0);
			High = Seen == 0 ? Place.Location : FVector(Math::Max(High.X, Place.Location.X), Math::Max(High.Y, Place.Location.Y), 0.0);
			Seen++;
		}
		if (Seen == 0)
			return;
		OutCentre = FVector((Low.X + High.X) * 0.5, (Low.Y + High.Y) * 0.5, 0.0);
		OutWidth = Math::Max(High.X - Low.X, High.Y - Low.Y) + Spacing(Family);
	}

	// AR1-b: moves each family away (whole, its grid kept) until none of its places is within MinDistance of Base.
	void KeepAwayFrom(TArray<FHOLArtGalleryPlace>& Places, FVector Base, double MinDistance)
	{
		TArray<FString> Families;
		Families.Add("Units");
		Families.Add("Buildings");
		Families.Add("Sources");
		for (const FString& Family : Families)
		{
			FVector Centre;
			double Width = 0.0;
			FamilyFrame(Places, Family, Centre, Width);
			FVector Away = FVector(Centre.X - Base.X, Centre.Y - Base.Y, 0.0);
			Away = Away.Size() < 1.0 ? FVector(0.0, -1.0, 0.0) : Away.GetSafeNormal();
			for (int Step = 0; Step < 100; Step++)
			{
				bool bNear = false;
				for (const FHOLArtGalleryPlace& Place : Places)
				{
					if (Place.Family == Family && FVector(Place.Location.X - Base.X, Place.Location.Y - Base.Y, 0.0).Size() < MinDistance)
						bNear = true;
				}
				if (!bNear)
					break;
				for (int K = 0; K < Places.Num(); K++)
				{
					if (Places[K].Family == Family)
						Places[K].Location += Away * 500.0;
				}
			}
		}
	}

	// The gallery as made: planned, then kept away from the local player's Forum (when there is one).
	void PlanAwayFromBase(const FString& Family, TArray<FHOLArtGalleryPlace>& OutPlaces)
	{
		PlanGrid(Family, OutPlaces);
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(Gameplay::GetPlayerController(0));
		FVector Forum;
		if (Controller != nullptr && HOLControllerRead::PointNearMainCentre(Controller, 0.0, 0.0, false, Forum))
			KeepAwayFrom(OutPlaces, Forum, MinBaseDistance());
	}
}

void HOLArtGalleryCommand(const TArray<FString>& Args)
{
	const FString Family = Args.Num() >= 1 ? Args[0] : "All";
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanAwayFromBase(Family, Places);
	if (Places.Num() == 0)
	{
		Log(n"LogHOLMatch", f"Art gallery: family={Family} places=0 (usage hol.Art.Gallery [Units|Buildings|Sources|All])");
		return;
	}
	System::ExecuteConsoleCommand("hol.Combat.Enabled 0");
	// CELEBORN-1125ar6: no forest of the map in the grid: the scenery and the standing wood sources around each family's frame are taken away first
	// (hol.Decor.ClearNear ... Trees), before the gallery's own sources are put.
	TArray<FString> Cleared;
	Cleared.Add("Units");
	Cleared.Add("Buildings");
	Cleared.Add("Sources");
	for (const FString& Each : Cleared)
	{
		FVector Middle;
		double Side = 0.0;
		HOLArtGallery::FamilyFrame(Places, Each, Middle, Side);
		if (Side > 0.0)
			System::ExecuteConsoleCommand(f"hol.Decor.ClearNear {Math::RoundToInt(Middle.X)} {Math::RoundToInt(Middle.Y)} {Math::RoundToInt(0.75 * Side)} Trees");
	}
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
			// AR1-b: a real source of the registry (HOLSourceAdd::AddNeutralSource), everyone's, its table amount.
			bMade = HOLSourceAdd::AddNeutralSource(Place.TypeName, Place.Location, -1) >= 0;
			// The label south of the source by its radius plus a margin: a tall tree (7,5 m) seen from the south hides what is behind it.
			HOLArtGallery::PutLabel(Place, Math::Max(Place.Radius, 30.0) + 250.0, bMade ? "" : "\n(non pose)");
			Sources += bMade ? 1 : 0;
		}
		if (!bMade)
		{
			Refused += 1;
			Log(n"LogHOLMatch", f"Art gallery: refused {Place.Family} {Place.TypeName.ToString()}");
		}
	}
	Log(n"LogHOLMatch", f"Art gallery: family={Family} places={Places.Num()} units={Units} buildings={Buildings} sources={Sources} refused={Refused}");
}

// hol.Art.GalleryFrame <Units|Buildings|Sources> (AR1-b): the camera on the centre of that family's frame (HOLArtGallery::FamilyFrame).
void HOLArtGalleryFrameCommand(const TArray<FString>& Args)
{
	const FString Family = Args.Num() >= 1 ? Args[0] : "Units";
	// CELEBORN-1125ar6: a grid deeper than one frame (Buildings) is shown in two captures: North or South moves the camera a quarter of the side.
	const FString Part = Args.Num() >= 2 ? Args[1] : "";
	TArray<FHOLArtGalleryPlace> Places;
	HOLArtGallery::PlanAwayFromBase("All", Places);
	FVector Centre;
	double Width = 0.0;
	HOLArtGallery::FamilyFrame(Places, Family, Centre, Width);
	if (Width <= 0.0)
	{
		Log(n"LogHOLMatch", f"Art gallery frame: unknown family {Family} (Units, Buildings, Sources)");
		return;
	}
	const double Shift = Part.ToLower() == "north" ? 0.25 * Width : (Part.ToLower() == "south" ? -0.25 * Width : 0.0);
	// The match camera looks towards +X (top of the screen = +X, ag_buildings 1125ar6): the halves are along X.
	Centre.X += Shift;
	const FString PartName = Part.IsEmpty() ? "Whole" : Part;
	System::ExecuteConsoleCommand(f"hol.Camera.Focus {Math::RoundToInt(Centre.X)} {Math::RoundToInt(Centre.Y)}");
	Log(n"LogHOLMatch", f"Art gallery frame: family={Family} part={PartName} centre=({Math::RoundToInt(Centre.X)}, {Math::RoundToInt(Centre.Y)}) side={Math::RoundToInt(Width)}");
}

const FConsoleCommand HOLArtGalleryConsole("hol.Art.Gallery", n"HOLArtGalleryCommand");
const FConsoleCommand HOLArtGalleryFrameConsole("hol.Art.GalleryFrame", n"HOLArtGalleryFrameCommand");
