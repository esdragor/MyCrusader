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
		Checks.Add(HOLArtChecks::CheckMeshPrefix(Asset.GetName().ToString(), bSkeletal));
		Checks.Add(HOLArtChecks::CheckArtFolder(FPaths::GetPath(Asset.GetPathName())));
	}

	void CheckStaticMesh(UStaticMesh Mesh)
	{
		TArray<FHOLArtCheck> Checks;
		const FBoxSphereBounds Bounds = Mesh.GetBounds();
		Checks.Add(HOLArtChecks::CheckPivot(Bounds.Origin.Z - Bounds.BoxExtent.Z));
		Checks.Add(HOLArtChecks::CheckMaterialSlots(Mesh.GetStaticMaterials().Num()));
		Checks.Add(HOLArtChecks::CheckTriangles(Mesh.GetNumTriangles(0), HOLArtChecks::TriangleBudget(Mesh.GetPathName())));
		AddCommon(Checks, Mesh, false);
		Report(Mesh.GetName().ToString(), Checks);
	}

	// AR2-b: what the script binds do not reach (LOD 0 triangles of a SkeletalMesh, skeletons, frame count, soft paths)
	// is read by the editor's Python, one value per line written to Saved/HOLArtProbe.txt; empty when Python failed.
	TArray<FString> Probe(FString Body)
	{
		TArray<FString> Lines;
		const FString File = FPaths::ProjectSavedDir() + "HOLArtProbe.txt";
		FString Py = "import unreal\nout=[]\n";
		Py += "def ld(x):\n  if x is None or isinstance(x, unreal.Object): return x\n";
		Py += "  s=x.export_text() if hasattr(x,'export_text') else str(x)\n  return unreal.load_asset(s) if s and s!='None' else None\n";
		Py += Body;
		Py += "open(r'" + File + "','w').write('\\n'.join(out))\n";
		if (!PythonScript::ExecutePythonCommand(Py))
			return Lines;
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, File))
			return Lines;
		Text.ParseIntoArray(Lines, "\n", false);
		for (int Index = 0; Index < Lines.Num(); Index++)
			Lines[Index] = Lines[Index].TrimStartAndEnd();
		return Lines;
	}

	FVector ParseVector(FString Text)
	{
		TArray<FString> Parts;
		Text.ParseIntoArray(Parts, " ", true);
		if (Parts.Num() < 3)
			return FVector::ZeroVector;
		return FVector(HOLControllerCommands::ParseReal(Parts[0]), HOLControllerCommands::ParseReal(Parts[1]), HOLControllerCommands::ParseReal(Parts[2]));
	}

	void CheckSkeletalMesh(USkeletalMesh Mesh)
	{
		TArray<FHOLArtCheck> Checks;
		const FBoxSphereBounds Bounds = Mesh.GetBounds();
		Checks.Add(HOLArtChecks::CheckPivot(Bounds.Origin.Z - Bounds.BoxExtent.Z));
		Checks.Add(HOLArtChecks::CheckMaterialSlots(Mesh.GetMaterials().Num()));
		// The asset registry tag "Triangles" of a SkeletalMesh is its LOD 0 count.
		TArray<FString> Probed = Probe("d=unreal.EditorAssetLibrary.find_asset_data('" + Mesh.GetPathName() + "')\nout.append(str(d.get_tag_value('Triangles') or ''))\n");
		if (Probed.Num() > 0 && !Probed[0].IsEmpty())
			Checks.Add(HOLArtChecks::CheckTriangles(HOLControllerCommands::ParseInt(Probed[0]), HOLArtChecks::TriangleBudget(Mesh.GetPathName())));
		else
			Checks.Add(HOLArtChecks::Make(false, "triangles (LOD 0): not read (editor Python)"));
		AddCommon(Checks, Mesh, true);
		Report(Mesh.GetName().ToString(), Checks);
	}

	void CheckArtVisual(UHOLArtVisual Visual)
	{
		TArray<FHOLArtCheck> Checks;
		Checks.Add(HOLArtChecks::CheckTypeName(Visual.TypeName, AllTypeNames()));
		Checks.Add(HOLArtChecks::CheckArtFolder(FPaths::GetPath(Visual.GetPathName())));
		// The script bind of TSoftObjectPtr loads through EditorOnlyLoadSynchronous (const, editor only), not LoadSynchronous.
		Checks.Add(HOLArtChecks::CheckLoaded("Mesh", !Visual.Mesh.IsNull(), Visual.Mesh.EditorOnlyLoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("SkinnedMesh", !Visual.SkinnedMesh.IsNull(), Visual.SkinnedMesh.EditorOnlyLoadSynchronous() != nullptr));
		UAnimSequenceTransformProviderData Provider = Visual.AnimationProvider.EditorOnlyLoadSynchronous();
		Checks.Add(HOLArtChecks::CheckLoaded("AnimationProvider", !Visual.AnimationProvider.IsNull(), Provider != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("Icon", !Visual.Icon.IsNull(), Visual.Icon.EditorOnlyLoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("Portrait", !Visual.Portrait.IsNull(), Visual.Portrait.EditorOnlyLoadSynchronous() != nullptr));
		// AR6-a: a sound set in the asset must load (empty = the sound table).
		Checks.Add(HOLArtChecks::CheckLoaded("SelectionSound", !Visual.SelectionSound.IsNull(), Visual.SelectionSound.EditorOnlyLoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("OrderSound", !Visual.OrderSound.IsNull(), Visual.OrderSound.EditorOnlyLoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("AttackSound", !Visual.AttackSound.IsNull(), Visual.AttackSound.EditorOnlyLoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("DeathSound", !Visual.DeathSound.IsNull(), Visual.DeathSound.EditorOnlyLoadSynchronous() != nullptr));
		Checks.Add(HOLArtChecks::CheckLoaded("ConstructionSound", !Visual.ConstructionSound.IsNull(), Visual.ConstructionSound.EditorOnlyLoadSynchronous() != nullptr));
		const int Count = Provider != nullptr ? Provider.GetSequences().Num() : 0;
		Checks.Add(HOLArtChecks::CheckSequence("Idle", Visual.bOverrideIdleSequence, Visual.IdleSequence, Count, false));
		Checks.Add(HOLArtChecks::CheckSequence("Walk", Visual.bOverrideWalkSequence, Visual.WalkSequence, Count, false));
		Checks.Add(HOLArtChecks::CheckSequence("Attack", Visual.bOverrideAttackSequence, Visual.AttackSequence, Count, true));
		Checks.Add(HOLArtChecks::CheckSequence("Fall", Visual.bOverrideFallSequence, Visual.FallSequence, Count, true));
		Checks.Add(HOLArtChecks::CheckSequence("Build", Visual.bOverrideBuildSequence, Visual.BuildSequence, Count, true));
		// AR2-b: Projectile and MuzzleSmoke (soft paths): five lines each = set, loads, StaticMesh, bounds min, bounds max.
		FString Py = "v=unreal.load_asset('" + Visual.GetPathName() + "')\n";
		Py += "for f in ('projectile','muzzle_smoke'):\n";
		Py += "  s=v.get_editor_property(f).export_text()\n  o=unreal.load_asset(s) if s and s!='None' else None\n";
		Py += "  b=o.get_bounding_box() if isinstance(o, unreal.StaticMesh) else None\n";
		Py += "  out += ['1' if s and s!='None' else '0', '1' if o else '0', '1' if b else '0']\n";
		Py += "  out += ['%f %f %f' % (b.min.x, b.min.y, b.min.z) if b else '0 0 0', '%f %f %f' % (b.max.x, b.max.y, b.max.z) if b else '0 0 0']\n";
		TArray<FString> Shot = Probe(Py);
		TArray<FString> Fields;
		Fields.Add("Projectile");
		Fields.Add("MuzzleSmoke");
		for (int Index = 0; Index < Fields.Num(); Index++)
		{
			const int Base = Index * 5;
			if (Shot.Num() < Base + 5)
			{
				Checks.Add(HOLArtChecks::Make(false, Fields[Index] + ": not read (editor Python)"));
				continue;
			}
			const bool bSet = Shot[Base] == "1";
			const bool bStatic = Shot[Base + 2] == "1";
			Checks.Add(HOLArtChecks::CheckShotField(Fields[Index], bSet, Shot[Base + 1] == "1", bStatic));
			if (bSet && bStatic)
				Checks.Add(HOLArtChecks::CheckShotPivot(Fields[Index], ParseVector(Shot[Base + 3]), ParseVector(Shot[Base + 4])));
		}
		Report(Visual.GetName().ToString(), Checks);
	}

	// AR2-b: an AnimSequence under /Game/Art/<Family>/<Type>/: the skeleton of the type's SkinnedMesh, its length, its frames.
	void CheckAnimSequence(UAnimSequence Anim)
	{
		TArray<FHOLArtCheck> Checks;
		const FName TypeName = HOLArtChecks::TypeFromArtPath(FPaths::GetPath(Anim.GetPathName()));
		const FString VisualPath = HOLArtChecks::ArtAssetPath(FamilyOf(TypeName), TypeName, NAME_None);
		if (VisualPath.IsEmpty())
			Checks.Add(HOLArtChecks::Make(false, "no type from the folder " + FPaths::GetPath(Anim.GetPathName()) + " (/Game/Art/<Family>/<Type>/)"));
		FString Py = "a=unreal.load_asset('" + Anim.GetPathName() + "')\nsk=a.get_editor_property('skeleton')\n";
		Py += "out.append(sk.get_path_name() if sk else '')\nms=''\n";
		Py += "if '" + VisualPath + "' and unreal.EditorAssetLibrary.does_asset_exist('" + VisualPath + "'):\n";
		Py += "  m=ld(unreal.load_asset('" + VisualPath + "').get_editor_property('skinned_mesh'))\n";
		Py += "  s=m.get_editor_property('skeleton') if m else None\n  ms=s.get_path_name() if s else ''\n";
		Py += "out.append(ms)\nout.append(str(unreal.AnimationLibrary.get_num_frames(a)))\n";
		TArray<FString> Probed = Probe(Py);
		if (Probed.Num() >= 3)
		{
			Checks.Add(HOLArtChecks::CheckSkeleton(Probed[0], Probed[1]));
			Checks.Add(HOLArtChecks::CheckAnimFrames(HOLControllerCommands::ParseInt(Probed[2])));
		}
		else
			Checks.Add(HOLArtChecks::Make(false, "skeleton and frames: not read (editor Python)"));
		Checks.Add(HOLArtChecks::CheckAnimLength(Anim.GetPlayLength()));
		Checks.Add(HOLArtChecks::CheckArtFolder(FPaths::GetPath(Anim.GetPathName())));
		Report(Anim.GetName().ToString(), Checks);
	}

	// Creates the DA_Art through the editor's Python (UPythonScriptLibrary is bound as the PythonScript:: namespace); MeshField = "mesh" / "skinned_mesh" / "" (nothing to set).
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
		const bool bRan = PythonScript::ExecutePythonCommand(Py);
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

	// AR2-b: starting a PIE Plain match from the script is not bound: the menu gives the console line to type in it.
	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "See in the Workshop")
	void SeeInTheWorkshop(UHOLArtVisual Visual)
	{
		const FString Command = HOLArtChecks::TryCommand(Visual.TypeName);
		Log("HOL Art: start the Plain match (Play), open the console and type: " + Command);
		Print("Play the Plain match, then type in the console: " + Command, 15.f, FLinearColor::Yellow);
	}
}

class UHOLArtAnimSequenceMenu : UScriptAssetMenuExtension
{
	default SupportedClasses.Add(UAnimSequence);

	UFUNCTION(CallInEditor, Category = "HOL Art", DisplayName = "Check for the game")
	void CheckForTheGame(UAnimSequence Anim)
	{
		HOLArtEditor::CheckAnimSequence(Anim);
	}
}
