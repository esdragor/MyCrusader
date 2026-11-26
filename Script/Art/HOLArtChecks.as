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

	// AR2-b: PROVISIONAL bounds of an animation clip (Placeholders.txt).
	const float MaxAnimSeconds = 10.f;
	const int MinAnimFrames = 2;

	// The triangle budget of a mesh from its folder: a unit's under /Art/Units/, a building's otherwise.
	int TriangleBudget(FString PathName)
	{
		return PathName.Contains("/Art/Units/") ? MaxUnitTriangles : MaxBuildingTriangles;
	}

	// Projectile / MuzzleSmoke of a DA_Art: empty is fine (a melee unit), set must load and be a StaticMesh.
	FHOLArtCheck CheckShotField(FString Field, bool bSet, bool bLoaded, bool bStaticMesh)
	{
		if (!bSet)
			return Make(true, Field + ": empty, no shot asset");
		if (!bLoaded)
			return Make(false, Field + ": set but does not load");
		return Make(bStaticMesh, Field + (bStaticMesh ? ": a StaticMesh" : ": loads but is not a StaticMesh"));
	}

	// A projectile or a smoke is spawned at the muzzle: its pivot (0, 0, 0) must sit inside its bounds.
	FHOLArtCheck CheckShotPivot(FString Field, FVector BoundsMin, FVector BoundsMax)
	{
		const float Tol = PivotToleranceCm;
		const bool bInside = BoundsMin.X <= Tol && BoundsMin.Y <= Tol && BoundsMin.Z <= Tol
			&& BoundsMax.X >= -Tol && BoundsMax.Y >= -Tol && BoundsMax.Z >= -Tol;
		return Make(bInside, Field + " pivot inside its bounds " + BoundsMin.ToString() + " .. " + BoundsMax.ToString());
	}

	// The type of an asset from its folder: /Game/Art/<Family>/<Type>[/...] (None outside that layout).
	FName TypeFromArtPath(FString PackagePath)
	{
		const FString Padded = PackagePath + "/";
		if (!Padded.StartsWith(ArtRoot))
			return NAME_None;
		// Game / Art / <Family> / <Type> [/ ...]
		TArray<FString> Parts;
		Padded.ParseIntoArray(Parts, "/", true);
		if (Parts.Num() < 4)
			return NAME_None;
		return FName(Parts[3]);
	}

	// An animation plays on the skeleton of its type's SkinnedMesh (MeshSkeleton empty: no DA_Art or no SkinnedMesh).
	FHOLArtCheck CheckSkeleton(FString AnimSkeleton, FString MeshSkeleton)
	{
		if (MeshSkeleton.IsEmpty())
			return Make(false, "skeleton " + AnimSkeleton + ": no SkinnedMesh in the type's DA_Art to compare with");
		return Make(AnimSkeleton == MeshSkeleton, "skeleton " + AnimSkeleton + " is the SkinnedMesh's " + MeshSkeleton);
	}

	FHOLArtCheck CheckAnimLength(float Seconds)
	{
		return Make(Seconds > 0.f && Seconds <= MaxAnimSeconds, "length " + Seconds + " s (0 to " + MaxAnimSeconds + ")");
	}

	FHOLArtCheck CheckAnimFrames(int Frames)
	{
		return Make(Frames >= MinAnimFrames, "frames: " + Frames + " (at least " + MinAnimFrames + ")");
	}

	// "See in the Workshop": the console line to type in a running Plain match.
	FString TryCommand(FName TypeName)
	{
		return "hol.Art.Try " + TypeName.ToString() + " 1";
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
