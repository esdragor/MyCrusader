// AR5-a (ArtistesInProgress.md, GLORFINDEL-1125ar3) : regles de la cabine d'icones hol.Art.Icon.
// La famille vient des VRAIES tables de types (la ligne source est la premiere ligne de SourceTypes, lue ici), le nom du
// fichier en decoule ; le cadrage est le meme pour toute une famille et suit la hauteur des bornes (l'objet entier tient
// dans le champ de la camera : angle calcule depuis la position rendue par Shot, pas recopie).

FName ArtIconFirstRow(const FString& Path)
{
	TArray<FName> Names;
	UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, Path));
	if (Table != nullptr)
	{
		Table.GetRowNames(Names);
	}
	return Names.Num() > 0 ? Names[0] : NAME_None;
}

void Test_ArtIconFileNameFollowsTheTableFamily(FUnitTest& T)
{
	const FString UnitFamily = HOLArtIconRules::FamilyOf(n"Arquebusier");
	const FString UnitFile = HOLArtIconRules::FileName(UnitFamily, n"Arquebusier");
	T.AssertEquals(FString("T_Unit_Arquebusier.png"), UnitFile, "Arquebusier (UnitTypes) -> T_Unit_Arquebusier.png");

	const FString BuildingFamily = HOLArtIconRules::FamilyOf(n"Barracks");
	const FString BuildingFile = HOLArtIconRules::FileName(BuildingFamily, n"Barracks");
	T.AssertEquals(FString("T_Building_Barracks.png"), BuildingFile, "Barracks (BuildingTypes) -> T_Building_Barracks.png");

	const FName Source = ArtIconFirstRow("/Game/Resources/SourceTypes.SourceTypes");
	T.AssertTrue(Source != NAME_None, "SourceTypes has a row");
	const FString SourceFamily = HOLArtIconRules::FamilyOf(Source);
	const FString SourceFile = HOLArtIconRules::FileName(SourceFamily, Source);
	T.AssertEquals(f"T_Source_{Source.ToString()}.png", SourceFile, "first SourceTypes row -> T_Source_<Type>.png");

	const FString Unknown = HOLArtIconRules::FamilyOf(n"NoSuchTypeAR5");
	const FString UnknownFile = HOLArtIconRules::FileName(Unknown, n"NoSuchTypeAR5");
	T.AssertTrue(Unknown.IsEmpty() && UnknownFile.IsEmpty(), "a type in no table has no family and no file");
}

// Cosine of the angle between the camera's forward axis (from its pitch and yaw) and the ray from the camera to Point.
double ArtIconCosTo(const FHOLArtIconShot& Shot, FVector Point)
{
	const FVector Ray = (Point - Shot.CameraOffset).GetSafeNormal();
	const double P = Math::DegreesToRadians(Shot.Rotation.Pitch);
	const double Y = Math::DegreesToRadians(Shot.Rotation.Yaw);
	const FVector Ahead = FVector(Math::Cos(P) * Math::Cos(Y), Math::Cos(P) * Math::Sin(Y), Math::Sin(P));
	return Ray.X * Ahead.X + Ray.Y * Ahead.Y + Ray.Z * Ahead.Z;
}

void Test_ArtIconShotSameFramingPerFamilyScaledByHeight(FUnitTest& T)
{
	const FHOLArtIconShot Low = HOLArtIconRules::Shot("Buildings", 600.0, 300.0);
	const FHOLArtIconShot High = HOLArtIconRules::Shot("Buildings", 1200.0, 600.0);
	T.AssertTrue(Low.FieldOfView > 0.0 && Low.Distance > 0.0, "a building shot has a field of view and a distance");
	T.AssertTrue(Low.Rotation.Pitch < -5.0, "the camera looks down on the object");
	T.AssertTrue(Math::Abs(Low.Rotation.Pitch - High.Rotation.Pitch) < 0.001 && Math::Abs(Low.Rotation.Yaw - High.Rotation.Yaw) < 0.001
		&& Math::Abs(Low.FieldOfView - High.FieldOfView) < 0.001, "same angle and field for every building");
	T.AssertTrue(Low.Distance > 0.0 && Math::Abs(High.Distance / Math::Max(Low.Distance, 0.001) - 2.0) < 0.01,
		"twice the bounds height = twice the camera distance");

	// The whole object (foot and top of its bounds) is inside the field, for each family.
	TArray<FString> Families;
	Families.Add("Units");
	Families.Add("Buildings");
	Families.Add("Sources");
	for (int K = 0; K < Families.Num(); K++)
	{
		const FHOLArtIconShot Shot = HOLArtIconRules::Shot(Families[K], 400.0, 150.0);
		const double Half = Math::Cos(Math::DegreesToRadians(Shot.FieldOfView * 0.5));
		const double Foot = ArtIconCosTo(Shot, FVector(0.0, 0.0, 0.0));
		const double Top = ArtIconCosTo(Shot, FVector(0.0, 0.0, 400.0));
		T.AssertTrue(Shot.Distance > 0.0 && Foot > Half && Top > Half, f"{Families[K]}: foot (cos {Foot}) and top (cos {Top}) inside half field (cos {Half})");
	}
	const FHOLArtIconShot UnitA = HOLArtIconRules::Shot("Units", 180.0, 40.0);
	const FHOLArtIconShot UnitB = HOLArtIconRules::Shot("Units", 180.0, 40.0);
	T.AssertTrue(UnitA.Distance > 0.0 && UnitA.CameraOffset.Equals(UnitB.CameraOffset, 0.01), "two units of the same bounds get the same frame");
}
