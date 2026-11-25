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
		int Start = -1;
		for (int K = 0; K + Key.Len() <= Text.Len(); K++)
		{
			if (Text.Mid(K, Key.Len()) == Key)
			{
				Start = K + Key.Len();
				break;
			}
		}
		if (Start < 0)
			return 0.0;
		int Stop = Start;
		while (Stop < Text.Len() && Text.Mid(Stop, 1) != "," && Text.Mid(Stop, 1) != ")")
			Stop++;
		return ToNumber(Text.Mid(Start, Stop - Start));
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
		const double Half = Place.Family == "Buildings" ? Place.FootprintY * 0.5 : Math::Max(Place.Radius, 60.0);
		return Place.Location + FVector(0.0, Half + 100.0, 0.0);
	}

	// AR1-c: the ground lines of a building (none for a unit or a source): its footprint outline, the lines of the construction grid
	// inside it (4 m tiles of Tile cm, sub-cells of Tile / SubDivision), and the vertical Height pole at its near right corner.
	void MarkLines(const FHOLArtGalleryPlace& Place, double Tile, int SubDivision, TArray<FVector>& OutStarts, TArray<FVector>& OutEnds, TArray<int>& OutKinds)
	{
		OutStarts.Reset();
		OutEnds.Reset();
		OutKinds.Reset();
		if (Place.FootprintX <= 0.0 || Place.FootprintY <= 0.0 || Tile <= 0.0)
			return;
		const int Sub = SubDivision < 1 ? 1 : SubDivision;
		const double Pitch = Tile / double(Sub);
		const double HalfX = Place.FootprintX * 0.5;
		const double HalfY = Place.FootprintY * 0.5;
		const FVector C = Place.Location;
		// Lines of constant X (across the footprint along Y), the two ends being the outline.
		const int StepsX = Math::RoundToInt(Place.FootprintX / Pitch);
		for (int K = 0; K <= StepsX; K++)
		{
			const double X = C.X - HalfX + Pitch * double(K);
			OutStarts.Add(FVector(X, C.Y - HalfY, C.Z));
			OutEnds.Add(FVector(X, C.Y + HalfY, C.Z));
			OutKinds.Add(K == 0 || K == StepsX ? MarkOutline : (K % Sub == 0 ? MarkTile : MarkSub));
		}
		// Lines of constant Y.
		const int StepsY = Math::RoundToInt(Place.FootprintY / Pitch);
		for (int K = 0; K <= StepsY; K++)
		{
			const double Y = C.Y - HalfY + Pitch * double(K);
			OutStarts.Add(FVector(C.X - HalfX, Y, C.Z));
			OutEnds.Add(FVector(C.X + HalfX, Y, C.Z));
			OutKinds.Add(K == 0 || K == StepsY ? MarkOutline : (K % Sub == 0 ? MarkTile : MarkSub));
		}
		// The Height pole at the near right corner (towards the camera, -X; on the figure's side, +Y).
		if (Place.Height > 0.0)
		{
			const FVector Corner = FVector(C.X - HalfX, C.Y + HalfY, C.Z);
			OutStarts.Add(Corner);
			OutEnds.Add(Corner + FVector(0.0, 0.0, Place.Height));
			OutKinds.Add(MarkHeight);
		}
	}

	// AR1-c (hol.Art.Light): the index of a biome of the game's table (HOLBiome::At(I).Key for I < HOLBiome::Count, any case, or its index),
	// -1 for the map's own look ("default", "map"), -2 for a name the game does not know.
	int BiomeIndexFromName(const FString& Name)
	{
		const FString Wanted = Name.ToLower();
		if (Wanted == "default" || Wanted == "map")
			return -1;
		for (int I = 0; I < HOLBiome::Count; I++)
		{
			if (HOLBiome::At(I).Key.ToLower() == Wanted || f"{I}" == Wanted)
				return I;
		}
		return -2;
	}

	// AR1-c: the 4 m tile of the construction grid (UHOLBuildingSettings::TileSize, HOLBuildingTypes.h; not readable from a
	// namespace function) and the sub-cells shown: hol.Wall.SubDivision, or the 1 m sub-cells of the fine grid (D898 G5) when it is 1.
	double GalleryTileCm()
	{
		return 400.0;
	}

	int GallerySubDivision()
	{
		const int Game = HOLWallLine::SubDivision();
		return Game > 1 ? Game : 4;
	}

	// AR1-d: the outline drawn around each ground label (copies of the text shifted by these offsets, in cm on the label's plane, in
	// LabelOutlineColour) so that a yellow label stays readable on the white of the snow biome.
	FLinearColor LabelOutlineColour()
	{
		return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	}

	void LabelOutlineOffsets(double WorldSize, TArray<FVector2D>& OutOffsets)
	{
		OutOffsets.Reset();
	}

	// AR1-d (hol.Art.Focus <Type>): the close view of one object of the gallery and its 1,80 m figure: the centre between the object
	// and the figure, and the side (cm) the view must show whole. False for a type the gallery does not hold.
	bool FocusOn(const TArray<FHOLArtGalleryPlace>& Places, const FString& Type, FVector& OutCentre, double& OutSide)
	{
		OutCentre = FVector::ZeroVector;
		OutSide = 0.0;
		return false;
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

	// AR1-c: draws the scale marks of every place (old marks removed first): the red 1,80 m figure beside each object, and for a
	// building its footprint outline (cyan), the 4 m tile lines (white), the sub-cell lines (grey) and its Height pole (orange).
	void DrawMarks(const TArray<FHOLArtGalleryPlace>& Places)
	{
		TArray<AHOLArtGalleryMarks> Old;
		GetAllActorsOfClass(Old);
		for (AHOLArtGalleryMarks Each : Old)
		{
			Each.DestroyActor();
		}
		AHOLArtGalleryMarks Marks = Cast<AHOLArtGalleryMarks>(SpawnActor(AHOLArtGalleryMarks::StaticClass(), FVector::ZeroVector));
		if (Marks == nullptr)
			return;
		const double Tile = GalleryTileCm();
		const int Sub = GallerySubDivision();
		UStaticMesh Cube = Cast<UStaticMesh>(LoadObject(nullptr, "/Engine/BasicShapes/Cube.Cube"));
		UStaticMesh Pole = Cast<UStaticMesh>(LoadObject(nullptr, "/Engine/BasicShapes/Cylinder.Cylinder"));
		UMaterialInterface Matte = Cast<UMaterialInterface>(LoadObject(nullptr, "/Game/Rendering/M_MatteColor.M_MatteColor"));
		TArray<FLinearColor> Colours;
		Colours.Add(FLinearColor(0.0f, 0.8f, 1.0f, 1.0f));
		Colours.Add(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
		Colours.Add(FLinearColor(0.45f, 0.45f, 0.45f, 1.0f));
		Colours.Add(FLinearColor(1.0f, 0.45f, 0.0f, 1.0f));
		Colours.Add(FLinearColor(0.9f, 0.05f, 0.05f, 1.0f));
		TArray<UHierarchicalInstancedStaticMeshComponent> Parts;
		for (int Kind = 0; Kind < Colours.Num(); Kind++)
		{
			UHierarchicalInstancedStaticMeshComponent Part = UHierarchicalInstancedStaticMeshComponent::Create(Marks, FName(f"GalleryMark_{Kind}"));
			Part.SetStaticMesh(Kind >= MarkHeight ? Pole : Cube);
			Part.SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Part.SetCollisionProfileName(n"NoCollision");
			Part.SetGenerateOverlapEvents(false);
			Part.SetCastShadow(false);
			UMaterialInstanceDynamic Paint = Part.CreateDynamicMaterialInstance(0, Matte);
			if (Paint != nullptr)
				Paint.SetVectorParameterValue(n"Color", Colours[Kind]);
			Parts.Add(Part);
		}
		int Lines = 0;
		int Figures = 0;
		for (const FHOLArtGalleryPlace& Place : Places)
		{
			TArray<FVector> Starts;
			TArray<FVector> Ends;
			TArray<int> Kinds;
			MarkLines(Place, Tile, Sub, Starts, Ends, Kinds);
			for (int K = 0; K < Kinds.Num(); K++)
			{
				const FVector A = Starts[K];
				const FVector B = Ends[K];
				const FVector Middle = (A + B) * 0.5;
				const double Length = (B - A).Size();
				if (Kinds[K] == MarkHeight)
				{
					Parts[MarkHeight].AddInstance(FTransform(FRotator::ZeroRotator, Middle, FVector(0.25, 0.25, Length / 100.0)), true);
				}
				else
				{
					const double Thick = Kinds[K] == MarkOutline ? 0.3 : (Kinds[K] == MarkTile ? 0.15 : 0.06);
					const bool bAlongX = Math::Abs(B.X - A.X) > Math::Abs(B.Y - A.Y);
					const FVector Scale = bAlongX ? FVector(Length / 100.0, Thick, 0.04) : FVector(Thick, Length / 100.0, 0.04);
					Parts[Kinds[K]].AddInstance(FTransform(FRotator::ZeroRotator, Middle + FVector(0.0, 0.0, 6.0 + 2.0 * double(Kinds[K])), Scale), true);
				}
				Lines++;
			}
			// The 1,80 m figure: a red bar (the engine cylinder is 1 m tall and centred).
			const FVector Foot = ScaleBarBase(Place);
			Parts[4].AddInstance(FTransform(FRotator::ZeroRotator, Foot + FVector(0.0, 0.0, ReferenceHeightCm() * 0.5), FVector(0.35, 0.35, ReferenceHeightCm() / 100.0)), true);
			Figures++;
		}
		Log(n"LogHOLMatch", f"Art gallery marks: figures={Figures} lines={Lines} tile={Math::RoundToInt(Tile)} sub={Sub}");
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
	HOLArtGallery::DrawMarks(Places);
	Log(n"LogHOLMatch", f"Art gallery: family={Family} places={Places.Num()} units={Units} buildings={Buildings} sources={Sources} refused={Refused}");
}

// hol.Art.Light <Biome> (AR1-c): the light, haze, ground and decor of one biome of the game (HOLBiome keys: fresh_grass, dry_steppe,
// autumn, dark_forest, snow, or its index; default = the map's own), through what the game already uses: hol.Biome.Index read by
// UHOLBiomeApplier (Script\Biomes\HOLBiomeApplier.as), which logs "Biome expect: key=..." when it has set it.
void HOLArtLightCommand(const TArray<FString>& Args)
{
	const FString Name = Args.Num() >= 1 ? Args[0] : "";
	const int Index = HOLArtGallery::BiomeIndexFromName(Name);
	if (Index == -2)
	{
		FString Known = "default";
		for (int I = 0; I < HOLBiome::Count; I++)
		{
			Known += ", ";
			Known += HOLBiome::At(I).Key;
		}
		Log(n"LogHOLMatch", f"Art light: unknown biome {Name} (known: {Known})");
		return;
	}
	System::ExecuteConsoleCommand(f"hol.Biome.Index {Index}");
	FString Shown = "default";
	if (Index >= 0)
		Shown = HOLBiome::At(Index).Key;
	Log(n"LogHOLMatch", f"Art light: biome={Shown} index={Index}");
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
const FConsoleCommand HOLArtLightConsole("hol.Art.Light", n"HOLArtLightCommand");

