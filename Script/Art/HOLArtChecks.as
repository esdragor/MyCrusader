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
		Check.Line = (bPass ? "[OK] " : "[KO] ") + Line;
		return Check;
	}

	FHOLArtCheck CheckPivot(float BoundsMinZ)
	{
		const bool bPass = Math::Abs(BoundsMinZ) <= PivotToleranceCm;
		return Make(bPass, "pivot on the ground: bottom of the bounds at " + BoundsMinZ + " cm (tolerance " + PivotToleranceCm + ")");
	}

	FHOLArtCheck CheckMaterialSlots(int Slots)
	{
		const bool bPass = Slots >= 1 && Slots <= MaxMaterialSlots;
		return Make(bPass, "material slots: " + Slots + " (1 to " + MaxMaterialSlots + ")");
	}

	FHOLArtCheck CheckTriangles(int Triangles, int Budget)
	{
		return Make(Triangles <= Budget, "triangles (LOD 0): " + Triangles + " (budget " + Budget + ")");
	}

	FHOLArtCheck CheckMeshPrefix(FString AssetName, bool bSkeletal)
	{
		const FString Prefix = bSkeletal ? "SKM_" : "SM_";
		return Make(AssetName.StartsWith(Prefix), "name " + AssetName + " starts with " + Prefix);
	}

	FHOLArtCheck CheckArtFolder(FString PackagePath)
	{
		const bool bPass = (PackagePath + "/").StartsWith(ArtRoot);
		return Make(bPass, "folder " + PackagePath + " under " + ArtRoot);
	}

	FHOLArtCheck CheckTypeName(FName TypeName, const TArray<FName>& KnownTypes)
	{
		const bool bPass = TypeName != NAME_None && KnownTypes.Contains(TypeName);
		return Make(bPass, "TypeName " + TypeName.ToString() + " in the unit, building or source tables");
	}

	FHOLArtCheck CheckLoaded(FString Field, bool bSet, bool bLoaded)
	{
		if (!bSet)
			return Make(true, Field + ": empty, falls back to the next level");
		return Make(bLoaded, Field + (bLoaded ? ": loads" : ": set but does not load"));
	}

	FHOLArtCheck CheckSequence(FString Field, bool bOverride, int Sequence, int SequenceCount, bool bOptional)
	{
		if (!bOverride)
			return Make(true, Field + " sequence: not overridden, the CSV's");
		const bool bLeftOut = Sequence == -1 && bOptional;
		const bool bPass = bLeftOut || (Sequence >= 0 && Sequence < SequenceCount);
		return Make(bPass, Field + " sequence " + Sequence + " in the provider's " + SequenceCount + " sequences");
	}

	// The family folder of a type: Units, Buildings or Sources (empty when the type is in no table).
	FString Family(bool bUnit, bool bBuilding, bool bSource)
	{
		if (bUnit)
			return "Units";
		if (bBuilding)
			return "Buildings";
		if (bSource)
			return "Sources";
		return "";
	}

	// /Game/Art/<Family>/<Type>/DA_Art_<Type>[_<Faction>] (D924).
	FString ArtAssetPath(FString FamilyName, FName TypeName, FName Faction)
	{
		if (FamilyName.IsEmpty() || TypeName == NAME_None)
			return "";
		const FString Type = TypeName.ToString();
		FString Name = "DA_Art_" + Type;
		if (Faction != NAME_None)
			Name += "_" + Faction.ToString();
		return ArtRoot + FamilyName + "/" + Type + "/" + Name;
	}

	bool AllPass(const TArray<FHOLArtCheck>& Checks)
	{
		for (const FHOLArtCheck& Check : Checks)
		{
			if (!Check.bPass)
				return false;
		}
		return true;
	}
}
