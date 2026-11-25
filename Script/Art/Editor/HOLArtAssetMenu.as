// D924 (AR2-a): right-click menus of the Content Browser for the artists (editor only: Script\Art\Editor is not cooked).
// "Check for the game": one green [OK] / red [KO] line per rule of HOLArtChecks in the Output Log, and an on-screen summary.
// "Create the DA_Art of this type": a prompt (TypeName, Faction) creates /Game/Art/<Family>/<Type>/DA_Art_<Type>[_<Faction>]
// through the editor's Python (DataAssetFactory + AssetTools), the selected mesh already set in it.

namespace HOLArtEditor
{
	// The row names of Tools/Data/<File> (first cell of each line after the header).
	TArray<FName> TableNames(FString File)
	{
		TArray<FName> Names;
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, FPaths::ProjectDir() + "Tools/Data/" + File))
			return Names;
		TArray<FString> Lines;
		Text.ParseIntoArray(Lines, "\n");
		for (int Line = 1; Line < Lines.Num(); Line++)
		{
			TArray<FString> Cells;
			Lines[Line].TrimStartAndEnd().ParseIntoArray(Cells, ",", false);
			if (Cells.Num() > 0 && !Cells[0].IsEmpty())
				Names.Add(FName(Cells[0]));
		}
		return Names;
	}

	TArray<FName> AllTypeNames()
	{
		TArray<FName> Names = TableNames("UnitTypes.csv");
		Names.Append(TableNames("BuildingTypes.csv"));
		Names.Append(TableNames("SourceTypes.csv"));
		return Names;
	}

	FString FamilyOf(FName TypeName)
	{
		return HOLArtChecks::Family(TableNames("UnitTypes.csv").Contains(TypeName),
			TableNames("BuildingTypes.csv").Contains(TypeName), TableNames("SourceTypes.csv").Contains(TypeName));
	}

	// Prints the lines (green in the log as Log, red as Error) and a summary on screen; true when all pass.
	bool Report(FString AssetName, const TArray<FHOLArtCheck>& Checks)
	{
		Log("HOL Art check: " + AssetName);
		int Failed = 0;
		for (const FHOLArtCheck& Check : Checks)
		{
			if (Check.bPass)
				Log("  " + Check.Line);
			else
			{
				Error("  " + Check.Line);
				Failed++;
			}
		}
		const bool bPass = HOLArtChecks::AllPass(Checks);
		const FString Summary = AssetName + (bPass ? ": ready for the game" : ": " + Failed + " red line(s), see the Output Log");
		Print(Summary, 8.f, bPass ? FLinearColor::Green : FLinearColor::Red);
		return bPass;
	}

	void AddCommon(TArray<FHOLArtCheck>& Checks, UObject Asset, bool bSkeletal)
	{
		Checks.Add(HOLArtChecks::CheckMeshPrefix(Asset.GetName(), bSkeletal));
		Checks.Add(HOLArtChecks::CheckArtFolder(FPaths::GetPath(Asset.GetPathName())));
	}

	void CheckStaticMesh(UStaticMesh Mesh)
	{
		TArray<FHOLArtCheck> Checks;
		const FBoxSphereBounds Bounds = Mesh.GetBounds();
		Checks.Add(HOLArtChecks::CheckPivot(Bounds.Origin.Z - Bounds.BoxExtent.Z));
		Checks.Add(HOLArtChecks::CheckMaterialSlots(Mesh.GetStaticMaterials().Num()));
		const bool bUnitFolder = Mesh.GetPathName().Contains("/Art/Units/");
		Checks.Add(HOLArtChecks::CheckTriangles(Mesh.GetNumTriangles(0),
			bUnitFolder ? HOLArtChecks::MaxUnitTriangles : HOLArtChecks::MaxBuildingTriangles));
		AddCommon(Checks, Mesh, false);
		Report(Mesh.GetName(), Checks);
	}

	void CheckSkeletalMesh(USkeletalMesh Mesh)
	{
		TArray<FHOLArtCheck> Checks;
		const FBoxSphereBounds Bounds = Mesh.GetBounds();
		Checks.Add(HOLArtChecks::CheckPivot(Bounds.Origin.Z - Bounds.BoxExtent.Z));
		Checks.Add(HOLArtChecks::CheckMaterialSlots(Mesh.GetMaterials().Num()));
		AddCommon(Checks, Mesh, true);
		Report(Mesh.GetName(), Checks);
	}

	void CheckArtVisual(UHOLArtVisual Visual)
	{
		TArray<FHOLArtCheck> Checks;
		Checks.Add(HOLArtChecks::CheckTypeName(Visual.TypeName, AllTypeNames()));
		Checks.Add(HOLArtChecks::CheckArtFolder(FPaths::GetPath(Visual.GetPathName())));
		Checks.Add(HOLArtChecks::CheckLoaded("Mesh", !Visual.Mesh.IsNull(), Visual.Mesh.LoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("SkinnedMesh", !Visual.SkinnedMesh.IsNull(), Visual.SkinnedMesh.LoadSynchronous() != nullptr));
		UAnimSequenceTransformProviderData Provider = Visual.AnimationProvider.LoadSynchronous();
		Checks.Add(HOLArtChecks::CheckLoaded("AnimationProvider", !Visual.AnimationProvider.IsNull(), Provider != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("Icon", !Visual.Icon.IsNull(), Visual.Icon.LoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("Portrait", !Visual.Portrait.IsNull(), Visual.Portrait.LoadSynchronous() != nullptr));
		const int Count = Provider != nullptr ? Provider.GetSequences().Num() : 0;
		Checks.Add(HOLArtChecks::CheckSequence("Idle", Visual.bOverrideIdleSequence, Visual.IdleSequence, Count, false));
		Checks.Add(HOLArtChecks::CheckSequence("Walk", Visual.bOverrideWalkSequence, Visual.WalkSequence, Count, false));
		Checks.Add(HOLArtChecks::CheckSequence("Attack", Visual.bOverrideAttackSequence, Visual.AttackSequence, Count, true));
		Checks.Add(HOLArtChecks::CheckSequence("Fall", Visual.bOverrideFallSequence, Visual.FallSequence, Count, true));
		Checks.Add(HOLArtChecks::CheckSequence("Build", Visual.bOverrideBuildSequence, Visual.BuildSequence, Count, true));
		Report(Visual.GetName(), Checks);
	}

	// Creates the DA_Art through the editor's Python; MeshField = "mesh" / "skinned_mesh" / "" (nothing to set).
	void CreateArtVisual(FName TypeName, FName Faction, FString MeshField, UObject Mesh)
	{
		const FString Path = HOLArtChecks::ArtAssetPath(FamilyOf(TypeName), TypeName, Faction);
		if (Path.IsEmpty())
		{
			Error("HOL Art: " + TypeName.ToString() + " is in no unit, building or source table (Tools/Data): nothing created");
			Print("DA_Art not created: unknown type " + TypeName.ToString(), 8.f, FLinearColor::Red);
			return;
		}
		const FString Folder = FPaths::GetPath(Path);
		const FString Name = FPaths::GetCleanFilename(Path);
		FString Py = "import unreal\n";
		Py += "p='" + Path + "'\n";
		Py += "if unreal.EditorAssetLibrary.does_asset_exist(p):\n  unreal.log_error('HOL Art: '+p+' already exists')\nelse:\n";
		Py += "  f=unreal.DataAssetFactory(); f.set_editor_property('data_asset_class', unreal.HOLArtVisual)\n";
		Py += "  a=unreal.AssetToolsHelpers.get_asset_tools().create_asset('" + Name + "','" + Folder + "',unreal.HOLArtVisual,f)\n";
		Py += "  a.set_editor_property('type_name','" + TypeName.ToString() + "')\n";
		if (Faction != NAME_None)
			Py += "  a.set_editor_property('faction','" + Faction.ToString() + "')\n";
		if (!MeshField.IsEmpty() && Mesh != nullptr)
			Py += "  a.set_editor_property('" + MeshField + "', unreal.load_asset('" + Mesh.GetPathName() + "'))\n";
		Py += "  unreal.EditorAssetLibrary.save_loaded_asset(a)\n  unreal.log('HOL Art: created '+p)\n";
		const bool bRan = UPythonScriptLibrary::ExecutePythonCommand(Py);
		Print((bRan ? "DA_Art: " : "DA_Art not created (Python failed): ") + Path, 8.f, bRan ? FLinearColor::Green : FLinearColor::Red);
	}
}

class UHOLArtStaticMeshMenu : UScriptAssetMenuExtension
{
	default SupportedClasses.Add(UStaticMesh);

	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "Check for the game")
	void CheckForTheGame(UStaticMesh Mesh)
	{
		HOLArtEditor::CheckStaticMesh(Mesh);
	}

	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "Create the DA_Art of this type")
	void CreateArtVisual(UStaticMesh Mesh, FName TypeName, FName Faction)
	{
		HOLArtEditor::CreateArtVisual(TypeName, Faction, "mesh", Mesh);
	}
}

class UHOLArtSkeletalMeshMenu : UScriptAssetMenuExtension
{
	default SupportedClasses.Add(USkeletalMesh);

	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "Check for the game")
	void CheckForTheGame(USkeletalMesh Mesh)
	{
		HOLArtEditor::CheckSkeletalMesh(Mesh);
	}

	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "Create the DA_Art of this type")
	void CreateArtVisual(USkeletalMesh Mesh, FName TypeName, FName Faction)
	{
		HOLArtEditor::CreateArtVisual(TypeName, Faction, "skinned_mesh", Mesh);
	}
}

class UHOLArtVisualMenu : UScriptAssetMenuExtension
{
	default SupportedClasses.Add(UHOLArtVisual);

	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "Check for the game")
	void CheckForTheGame(UHOLArtVisual Visual)
	{
		HOLArtEditor::CheckArtVisual(Visual);
	}

	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "Create the faction variant of this DA_Art")
	void CreateArtVisual(UHOLArtVisual Visual, FName Faction)
	{
		HOLArtEditor::CreateArtVisual(Visual.TypeName, Faction, "", nullptr);
	}
}
