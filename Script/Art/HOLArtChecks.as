// D924 (AR2-a): the pure rules an artist's asset must pass before the game takes it, outside Script\Art\Editor so the
// tests run them in any world. The editor menus (Script\Art\Editor\HOLArtAssetMenu.as) read the asset and call these.
// Budgets: PROVISIONAL named constants (Placeholders.txt), to tune once real art arrives.

struct FHOLArtCheck
{
	UPROPERTY()
	bool bPass = false;

	UPROPERTY()
	FString Line;
}

namespace HOLArtChecks
{
	// Pivot on the ground: the bottom of the bounds sits at 0, within this many centimetres.
	const float PivotToleranceCm = 2.f;
	// PROVISIONAL budgets (Placeholders.txt).
	const int MaxMaterialSlots = 4;
	const int MaxUnitTriangles = 8000;
	const int MaxBuildingTriangles = 40000;
	const FString ArtRoot = "/Game/Art/";

	FHOLArtCheck Make(bool bPass, FString Line)
	{
		FHOLArtCheck Check;
		Check.bPass = bPass;
		Check.Line = Line;
		return Check;
	}

	FHOLArtCheck CheckPivot(float BoundsMinZ)
	{
		return Make(false, "");
	}

	FHOLArtCheck CheckMaterialSlots(int Slots)
	{
		return Make(false, "");
	}

	FHOLArtCheck CheckTriangles(int Triangles, int Budget)
	{
		return Make(false, "");
	}

	FHOLArtCheck CheckMeshPrefix(FString AssetName, bool bSkeletal)
	{
		return Make(false, "");
	}

	FHOLArtCheck CheckArtFolder(FString PackagePath)
	{
		return Make(false, "");
	}

	FHOLArtCheck CheckTypeName(FName TypeName, const TArray<FName>& KnownTypes)
	{
		return Make(false, "");
	}

	FHOLArtCheck CheckLoaded(FString Field, bool bSet, bool bLoaded)
	{
		return Make(false, "");
	}

	FHOLArtCheck CheckSequence(FString Field, bool bOverride, int Sequence, int SequenceCount, bool bOptional)
	{
		return Make(false, "");
	}

	// The family folder of a type: Units, Buildings or Sources (empty when the type is in no table).
	FString Family(bool bUnit, bool bBuilding, bool bSource)
	{
		return "";
	}

	// /Game/Art/<Family>/<Type>/DA_Art_<Type>[_<Faction>] (D924).
	FString ArtAssetPath(FString FamilyName, FName TypeName, FName Faction)
	{
		return "";
	}

	bool AllPass(const TArray<FHOLArtCheck>& Checks)
	{
		return false;
	}
}
