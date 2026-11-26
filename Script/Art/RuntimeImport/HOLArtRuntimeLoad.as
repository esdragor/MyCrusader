// B0 (D928, ArtistesInProgress.md): feasibility trial of the light atelier, NOT a tool. hol.Art.RuntimeLoad <file> [Buildings]
// imports a model file (FBX, glTF / glb) from the disk WHILE THE GAME RUNS, through Interchange's runtime import
// (UInterchangeManager::ImportAsset into a package of memory under /Game/RuntimeImport), then writes to the log what came out
// (one line per object: its class, its name; for a static mesh its vertices, its material slots and their materials).
// With "Buildings": every static mesh component of every building of the map draws the first static mesh imported instead
// (the body AHOLBuilding::BuildFinalMesh made: the real drawing path of a building). Developer trial only; nothing saved.

namespace HOLArtRuntimeLoad
{
	// Why File is not imported: "" when it can be tried, "empty" (no path), "format" (not .fbx / .glb / .gltf / .obj), "missing" (no such file).
	// The command refuses before Interchange is asked (tested by HOLArtRuntimeLoad_Test.as).
	FString Refusal(const FString& File)
	{
		if (File.TrimStartAndEnd().IsEmpty())
			return "empty";
		const FString Lower = File.ToLower();
		if (!Lower.EndsWith(".fbx") && !Lower.EndsWith(".glb") && !Lower.EndsWith(".gltf") && !Lower.EndsWith(".obj"))
			return "format";
		if (!FPaths::FileExists(File))
			return "missing";
		return "";
	}

	// The name of the package folder for File (its base name, letters and digits only).
	FString PackageNameOf(const FString& File)
	{
		FString Base = File;
		int Slash = -1;
		for (int K = 0; K < Base.Len(); K++)
		{
			if (Base.Mid(K, 1) == "/" || Base.Mid(K, 1) == "\\")
				Slash = K;
		}
		if (Slash >= 0)
			Base = Base.Mid(Slash + 1);
		// An explicit list: in the packaged trial of 2026-11-26 the test "upper case differs from lower case" kept no letter
		// (every name came out "Model").
		const FString Allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		FString Out = "";
		for (int K = 0; K < Base.Len(); K++)
		{
			const FString C = Base.Mid(K, 1);
			if (C == ".")
				break;
			if (Allowed.Contains(C))
				Out += C;
		}
		return Out.IsEmpty() ? "Model" : Out;
	}

}

void HOLArtRuntimeLoadCommand(const TArray<FString>& Args)
{
	if (Args.Num() == 0)
	{
		Log(n"LogHOLMatch", "RuntimeLoad: usage hol.Art.RuntimeLoad <file> [Buildings]");
		return;
	}
	const FString File = Args[0];
	const FString Refused = HOLArtRuntimeLoad::Refusal(File);
	if (!Refused.IsEmpty())
	{
		Log(n"LogHOLMatch", f"RuntimeLoad: refused {Refused}: {File}");
		return;
	}
	const bool bBuildings = Args.Num() > 1 && Args[1] == "Buildings";
	const FString Name = HOLArtRuntimeLoad::PackageNameOf(File);
	const FString ContentPath = f"/Game/RuntimeImport/{Name}";
	Log(n"LogHOLMatch", f"RuntimeLoad: start {File} -> {ContentPath}");

	UInterchangeManager Manager = UInterchangeManager::GetInterchangeManagerScripted();
	if (Manager == nullptr)
	{
		Log(n"LogHOLMatch", "RuntimeLoad: FAIL no Interchange manager");
		return;
	}
	UInterchangeSourceData Source = UInterchangeManager::CreateSourceData(File);
	if (Source == nullptr)
	{
		Log(n"LogHOLMatch", "RuntimeLoad: FAIL no source data");
		return;
	}
	FImportAssetParameters Params;
	Params.bIsAutomated = true;
	TArray<UObject> Imported;
	const float Start = Gameplay::GetRealTimeSeconds();
	const bool bOk = Manager.ImportAsset(ContentPath, Source, Params, Imported);
	const float Took = Gameplay::GetRealTimeSeconds() - Start;
	Log(n"LogHOLMatch", f"RuntimeLoad: import returned {bOk}, {Imported.Num()} object(s), {Took} s");

	UStaticMesh FirstMesh = nullptr;
	int Meshes = 0;
	int Skeletal = 0;
	int Anims = 0;
	int Textures = 0;
	int Materials = 0;
	for (UObject Object : Imported)
	{
		if (Object == nullptr)
			continue;
		Log(n"LogHOLMatch", f"RuntimeLoad: object {Object.Class.Name} {Object.Name}");
		UStaticMesh Mesh = Cast<UStaticMesh>(Object);
		if (Mesh != nullptr)
		{
			Meshes++;
			if (FirstMesh == nullptr)
				FirstMesh = Mesh;
			const FBoxSphereBounds Bounds = Mesh.GetBounds();
			Log(n"LogHOLMatch", f"RuntimeLoad: static mesh {Mesh.Name} slots {Mesh.StaticMaterials.Num()} extent {Bounds.BoxExtent}");
			for (int Slot = 0; Slot < Mesh.StaticMaterials.Num(); Slot++)
			{
				UMaterialInterface Material = Mesh.GetMaterial(Slot);
				const FString MaterialPath = Material != nullptr ? Material.GetPathName() : FString("none");
				Log(n"LogHOLMatch", f"RuntimeLoad: slot {Slot} {Mesh.StaticMaterials[Slot].MaterialSlotName} material {MaterialPath}");
			}
		}
		if (Cast<USkeletalMesh>(Object) != nullptr)
			Skeletal++;
		if (Cast<UAnimSequence>(Object) != nullptr)
			Anims++;
		if (Cast<UTexture>(Object) != nullptr)
			Textures++;
		if (Cast<UMaterialInterface>(Object) != nullptr)
			Materials++;
	}
	Log(n"LogHOLMatch", f"RuntimeLoad: summary {Name} static {Meshes} skeletal {Skeletal} anims {Anims} textures {Textures} materials {Materials}");

	if (!bBuildings)
		return;
	if (FirstMesh == nullptr)
	{
		Log(n"LogHOLMatch", "RuntimeLoad: Buildings: no static mesh to put");
		return;
	}
	TArray<AHOLBuilding> Buildings;
	GetAllActorsOfClass(Buildings);
	int Swapped = 0;
	for (AHOLBuilding Building : Buildings)
	{
		TArray<UStaticMeshComponent> Pieces;
		Building.GetComponentsByClass(Pieces);
		for (UStaticMeshComponent Piece : Pieces)
		{
			if (Piece.StaticMesh == nullptr || !Piece.IsVisible())
				continue;
			Piece.SetStaticMesh(FirstMesh);
			Swapped++;
		}
	}
	Log(n"LogHOLMatch", f"RuntimeLoad: Buildings {Name}: {Swapped} piece(s) of {Buildings.Num()} building(s) now draw {FirstMesh.Name}");
}

const FConsoleCommand HOLArtRuntimeLoadConsole("hol.Art.RuntimeLoad", n"HOLArtRuntimeLoadCommand");
