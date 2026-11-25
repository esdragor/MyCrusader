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
	// BOUCHON (commit rouge) : le correctif lit les tables.
	FString FamilyOf(FName TypeName)
	{
		return "";
	}

	// BOUCHON (commit rouge).
	FString FileName(const FString& Family, FName TypeName)
	{
		return "";
	}

	// BOUCHON (commit rouge).
	FHOLArtIconShot Shot(const FString& Family, double Height, double Radius)
	{
		FHOLArtIconShot Out;
		return Out;
	}
}
