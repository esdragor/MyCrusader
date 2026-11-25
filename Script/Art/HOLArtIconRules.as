// AR5-a (ArtistesInProgress.md, GLORFINDEL-1125ar3) : regles pures de la cabine d'icones hol.Art.Icon (HOLArtIconBooth.as).
// - FamilyOf : la famille d'un type lue dans les VRAIES tables (UnitTypes, BuildingTypes, SourceTypes), jamais une liste a la main.
// - FileName : T_Unit_<Type>.png, T_Building_<Type>.png, T_Source_<Type>.png (le nom de l'asset Texture2D a importer ensuite).
// - Shot : camera FIXE par famille (meme tangage, meme cap, meme champ), distance a l'echelle de la hauteur des bornes.

struct FHOLArtIconShot
{
	// Camera position relative to the object's ground point, its rotation and its field of view (degrees).
	FVector CameraOffset;
	FRotator Rotation;
	double FieldOfView = 0.0;
	// The point the camera looks at, relative to the object's ground point (middle of the bounds height).
	FVector LookAt;
	double Distance = 0.0;
}

namespace HOLArtIconRules
{
	const double Pitch = -25.0;
	// The camera stands south-west of the object and looks north-east (three-quarter view, same for every type).
	const double Yaw = 45.0;
	const double FieldOfView = 30.0;
	// Room around the object inside the frame (1.0 = the bounds touch the edge).
	const double Margin = 1.3;

	FString TablePath(const FString& Family)
	{
		if (Family == "Units")
			return "/Game/Units/UnitTypes.UnitTypes";
		if (Family == "Buildings")
			return "/Game/Buildings/BuildingTypes.BuildingTypes";
		return "/Game/Resources/SourceTypes.SourceTypes";
	}

	bool TableHas(const FString& Family, FName TypeName)
	{
		UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, TablePath(Family)));
		if (Table == nullptr)
			return false;
		TArray<FName> Names;
		Table.GetRowNames(Names);
		return Names.Contains(TypeName);
	}

	// "Units", "Buildings", "Sources", or "" when the type is in no table (first table wins).
	FString FamilyOf(FName TypeName)
	{
		if (TableHas("Units", TypeName))
			return "Units";
		if (TableHas("Buildings", TypeName))
			return "Buildings";
		if (TableHas("Sources", TypeName))
			return "Sources";
		return "";
	}

	FString Prefix(const FString& Family)
	{
		if (Family == "Units")
			return "T_Unit_";
		if (Family == "Buildings")
			return "T_Building_";
		if (Family == "Sources")
			return "T_Source_";
		return "";
	}

	FString FileName(const FString& Family, FName TypeName)
	{
		const FString Head = Prefix(Family);
		if (Head.IsEmpty() || TypeName == NAME_None)
			return "";
		return Head + TypeName.ToString() + ".png";
	}

	// AR5-b: a unit's bounds from its REAL mesh (the type's DA_Art first, then the UnitTypes row; skinned mesh before the static
	// one, as drawn by Mass) times its MeshScale (DA_Art override, then the row). False when the type has no mesh.
	bool UnitBounds(FName TypeName, double& OutHeight, double& OutRadius)
	{
		OutHeight = 180.0;
		OutRadius = 40.0;
		FString Skinned;
		FString Static;
		double Scale = 1.0;
		UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, TablePath("Units")));
		if (Table == nullptr)
			return false;
		TArray<FName> Names;
		Table.GetRowNames(Names);
		const int Row = Names.FindIndex(TypeName);
		if (Row < 0)
			return false;
		// The row may have lost its visual columns (AR0-d2 moves them to the DA_Art): a missing column reads empty.
		TArray<FString> SkinnedColumn = Table.GetColumnAsString(n"SkinnedMesh");
		TArray<FString> MeshColumn = Table.GetColumnAsString(n"Mesh");
		TArray<FString> ScaleColumn = Table.GetColumnAsString(n"MeshScale");
		if (Row < SkinnedColumn.Num())
			Skinned = SkinnedColumn[Row];
		if (Row < MeshColumn.Num())
			Static = MeshColumn[Row];
		if (Row < ScaleColumn.Num())
			Scale = String::Conv_StringToDouble(ScaleColumn[Row]);
		const FString Type = TypeName.ToString();
		UHOLArtVisual Art = Cast<UHOLArtVisual>(LoadObject(nullptr, f"/Game/Art/Units/{Type}/DA_Art_{Type}.DA_Art_{Type}"));
		if (Art != nullptr)
		{
			if (!Art.SkinnedMesh.IsNull())
				Skinned = Art.SkinnedMesh.ToString();
			if (!Art.Mesh.IsNull())
				Static = Art.Mesh.ToString();
			if (Art.bOverrideMeshScale)
				Scale = Art.MeshScale;
		}
		FVector Low;
		FVector High;
		USkeletalMesh Body = Skinned.IsEmpty() || Skinned == "None" ? nullptr : Cast<USkeletalMesh>(LoadObject(nullptr, Skinned));
		UStaticMesh Mesh = Static.IsEmpty() || Static == "None" ? nullptr : Cast<UStaticMesh>(LoadObject(nullptr, Static));
		if (Body != nullptr)
		{
			const FBoxSphereBounds Bounds = Body.GetBounds();
			Low = Bounds.Origin - Bounds.BoxExtent;
			High = Bounds.Origin + Bounds.BoxExtent;
		}
		else if (Mesh != nullptr)
		{
			const FBox Box = Mesh.GetBoundingBox();
			Low = Box.Min;
			High = Box.Max;
		}
		else
		{
			return false;
		}
		if (Scale <= 0.0 || High.Z <= 0.0)
			return false;
		OutHeight = High.Z * Scale;
		const double Wide = Math::Max(Math::Max(Math::Abs(Low.X), Math::Abs(High.X)), Math::Max(Math::Abs(Low.Y), Math::Abs(High.Y)));
		OutRadius = Math::Max(Wide * Scale, 1.0);
		return true;
	}

	// AR5-d: a building type's bounds from its BuildingTypes row (Footprint, Height), not from the placed actor (AR5-c cut the
	// Barracks): Height = its Height column, Radius = half the diagonal of its footprint (the corners stay in the frame).
	bool BuildingBounds(FName TypeName, double& OutHeight, double& OutRadius)
	{
		OutHeight = 100.0;
		OutRadius = 100.0;
		UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, TablePath("Buildings")));
		if (Table == nullptr)
			return false;
		TArray<FName> Names;
		Table.GetRowNames(Names);
		const int Row = Names.FindIndex(TypeName);
		TArray<FString> Footprints = Table.GetColumnAsString(n"Footprint");
		TArray<FString> Heights = Table.GetColumnAsString(n"Height");
		if (Row < 0 || Row >= Footprints.Num() || Row >= Heights.Num())
			return false;
		const FVector2D Foot = HOLArtGallery::FootprintOf(Footprints[Row]);
		const double Top = String::Conv_StringToDouble(Heights[Row]);
		if (Foot.X <= 0.0 || Foot.Y <= 0.0 || Top <= 0.0)
			return false;
		OutHeight = Top;
		OutRadius = 0.5 * Math::Sqrt(Foot.X * Foot.X + Foot.Y * Foot.Y);
		return true;
	}
	// Fixed angle and field for the family; the distance makes the bounds (Height tall, Radius wide) fill the frame.
	FHOLArtIconShot Shot(const FString& Family, double Height, double Radius)
	{
		FHOLArtIconShot Out;
		Out.Rotation = FRotator(Pitch, Yaw, 0.0);
		Out.FieldOfView = FieldOfView;
		const double Tall = Math::Max(Height, 1.0);
		const double Wide = Math::Max(Radius, 1.0);
		// Half the size seen by the camera: the bounding sphere of the box (Height x 2 Radius), so any angle fits.
		const double HalfSize = Math::Sqrt(Tall * Tall * 0.25 + Wide * Wide);
		Out.Distance = HalfSize * Margin / Math::Tan(Math::DegreesToRadians(FieldOfView * 0.5));
		Out.LookAt = FVector(0.0, 0.0, Tall * 0.5);
		const double P = Math::DegreesToRadians(Pitch);
		const double Y = Math::DegreesToRadians(Yaw);
		const FVector Ahead = FVector(Math::Cos(P) * Math::Cos(Y), Math::Cos(P) * Math::Sin(Y), Math::Sin(P));
		Out.CameraOffset = Out.LookAt - Ahead * Out.Distance;
		return Out;
	}
}
