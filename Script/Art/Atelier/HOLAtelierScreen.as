// AR8-b (D929, D930): the light atelier of the published game. Launched with -Atelier, the game goes straight to the atelier: the main
// menu launches a Plain match (UHOLAtelierEntry, "launch"), the match becomes hol.Art.Lab (gallery, scale marks, light) and the atelier
// screen opens ("lab"). Without the argument nothing changes (no button in the menu). Rules: HOLAtelierRules.as.
// The screen: a FIXED type (buildings and sources; a unit is refused, "unités : par l'éditeur"), a .glb / .gltf from the known folder
// <Documents>\HeroesOfLiberty\Atelier\Entrees\ (chosen in the list of that folder, < > and "Rafraîchir"; AR8-c, D931), loaded by
// Interchange's runtime import IN THE PLACE OF THAT TYPE'S MODEL ONLY, yaw and scale, licence, author, status, notes; "Save" writes
// <Documents>\HeroesOfLiberty\Atelier\Envois\<Type>[_<Faction>]_<aaaammjj-hhmm>\ (manifest.json format 1, the model copied as
// model.glb / model.gltf, preview.png), the send Tools\Art\apply_art_send.ps1 applies. Console commands (scenario art_atelier, same
// functions as the buttons):
//   hol.Art.Atelier.Open | .Type <Type> | .Refresh | .Load <name in the list | full path> | .Set <yaw|scale|status|author|faction|file_or_pack|source|licence|url|notes> <value> | .Save

const FConsoleVariable CVar_ArtAtelierState("hol.Art.Atelier.State", 0);

namespace HOLAtelierText
{
	FText Get(const FString& Key)
	{
		if (Key == "Title") return NSLOCTEXT("HOLAtelier", "Title", "Atelier : modèle fixe (bâtiment ou source)");
		if (Key == "Type") return NSLOCTEXT("HOLAtelier", "Type", "Type");
		if (Key == "Choose") return NSLOCTEXT("HOLAtelier", "Choose", "Choisir");
		if (Key == "Folder") return NSLOCTEXT("HOLAtelier", "Folder", "Modèles (.glb / .gltf) dans : {0}");
		if (Key == "File") return NSLOCTEXT("HOLAtelier", "File", "Fichier");
		if (Key == "Load") return NSLOCTEXT("HOLAtelier", "Load", "Charger");
		if (Key == "Yaw") return NSLOCTEXT("HOLAtelier", "Yaw", "Rotation (degrés)");
		if (Key == "Scale") return NSLOCTEXT("HOLAtelier", "Scale", "Échelle");
		if (Key == "Author") return NSLOCTEXT("HOLAtelier", "Author", "Auteur");
		if (Key == "Faction") return NSLOCTEXT("HOLAtelier", "Faction", "Faction (vide = toutes)");
		if (Key == "FileOrPack") return NSLOCTEXT("HOLAtelier", "FileOrPack", "Licence : fichier ou pack");
		if (Key == "Source") return NSLOCTEXT("HOLAtelier", "Source", "Licence : source");
		if (Key == "Licence") return NSLOCTEXT("HOLAtelier", "Licence", "Licence");
		if (Key == "Url") return NSLOCTEXT("HOLAtelier", "Url", "Licence : adresse (url)");
		if (Key == "Notes") return NSLOCTEXT("HOLAtelier", "Notes", "Notes");
		if (Key == "Status") return NSLOCTEXT("HOLAtelier", "Status", "Statut");
		if (Key == "Save") return NSLOCTEXT("HOLAtelier", "Save", "Enregistrer");
		if (Key == "Loaded") return NSLOCTEXT("HOLAtelier", "Loaded", "Modèle chargé à la place de {0} : {1}");
		if (Key == "Saved") return NSLOCTEXT("HOLAtelier", "Saved", "Envoi écrit dans {0}");
		if (Key == "Refresh") return NSLOCTEXT("HOLAtelier", "Refresh", "Rafraîchir");
		if (Key == "NoModels") return NSLOCTEXT("HOLAtelier", "NoModels", "aucun .glb / .gltf dans ce dossier : posez-y votre modèle puis Rafraîchir");
		if (Key == "Copied") return NSLOCTEXT("HOLAtelier", "Copied", "Modèle copié dans l'envoi sous le nom {0}");
		if (Key == "CopyByHand") return NSLOCTEXT("HOLAtelier", "CopyFailed", "Modèle non copié : copiez votre fichier ({1}) dans ce dossier sous le nom {0}");
		if (Key == "CopyExtra") return NSLOCTEXT("HOLAtelier", "CopyExtra", "Copiez aussi à côté les fichiers que le .gltf nomme (.bin, textures).");
		if (Key == "NoPreview") return NSLOCTEXT("HOLAtelier", "NoPreview", "aperçu (preview.png) non écrit");
		if (Key == "Refused") return NSLOCTEXT("HOLAtelier", "Refused", "Non enregistré :");
		// HALDIR-1127ar3: "name : reason" with the culture's separator (en "{0}: {1}", ja "{0}：{1}").
		if (Key == "Pair") return NSLOCTEXT("HOLAtelier", "Pair", "{0} : {1}");
		if (Key == "SendBack") return NSLOCTEXT("HOLAtelier", "SendBack", "Renvoyez ce dossier (ou son zip).");
		return FText::FromString(Key);
	}

	// HALDIR-1127ar3: a localized pattern filled, {0} and {1} replaced (the culture's separator kept: fr "{0} : {1}", en "{0}: {1}", ja "{0}：{1}").
	FString Fill(const FString& Pattern, const FString& First, const FString& Second = "")
	{
		return Pattern.Replace("{0}", First).Replace("{1}", Second);
	}

	// The message Key of the current culture with its values in their place.
	FString Format(const FString& Key, const FString& First, const FString& Second = "")
	{
		return Fill(Get(Key).ToString(), First, Second);
	}
}

class UHOLAtelierScreen : UUserWidget
{
	TArray<FString> Buildings;
	TArray<FString> Sources;
	TArray<FString> Units;
	TArray<FString> Fixed;
	int TypeIndex = 0;
	FHOLAtelierSend Send;
	bool bModelLoaded = false;
	UStaticMesh LoadedMesh;
	int Loads = 0;
	// The pieces drawn with the loaded model (restored when the type changes): their mesh, rotation, scale, visibility before.
	TArray<UStaticMeshComponent> Pieces;
	TArray<UStaticMesh> PieceMeshes;
	TArray<FRotator> PieceRotations;
	TArray<FVector> PieceScales;
	TArray<bool> PieceShown;
	// A source has no piece of its own (its chunk draws every source of the map): the model stands on its gallery place.
	AStaticMeshActor SourcePreview;
	FVector ModelSpot;

	UCommonTextBlock TypeText;
	UCommonTextBlock YawText;
	UCommonTextBlock ScaleText;
	UCommonTextBlock StatusText;
	UCommonTextBlock MessageText;
	UEditableTextBox TypeBox;
	// The models of Entrees (names, sorted; HOLAtelier::ListModels) and the one chosen (-1: none).
	TArray<FString> Models;
	int ModelIndex = -1;
	UCommonTextBlock ModelText;
	UEditableTextBox AuthorBox;
	UEditableTextBox FactionBox;
	UEditableTextBox FileOrPackBox;
	UEditableTextBox SourceBox;
	UEditableTextBox LicenceBox;
	UEditableTextBox UrlBox;
	UEditableTextBox NotesBox;

	// --- Construction ---------------------------------------------------------------------------------------------------------
	void Build()
	{
		Buildings = HOLAtelier::TableNames("Buildings");
		Sources = HOLAtelier::TableNames("Sources");
		Units = HOLAtelier::TableNames("Units");
		Fixed = HOLAtelier::FixedTypes(Buildings, Sources);
		Send.MadeWith = HOLVersionRead::Get();
		const int House = Fixed.FindIndex("House");
		TypeIndex = House >= 0 ? House : 0;

		UCanvasPanel Canvas = Cast<UCanvasPanel>(ConstructWidget(UCanvasPanel, n"AtelierRoot"));
		SetRootWidget(Canvas);
		UBorder Panel = Cast<UBorder>(ConstructWidget(UBorder, n"AtelierPanel"));
		Panel.SetBrushColor(FLinearColor(0.05, 0.05, 0.08, 1.0));
		Panel.SetPadding(FMargin(10.0, 8.0, 10.0, 8.0));
		UCanvasPanelSlot PanelSlot = Canvas.AddChildToCanvas(Panel);
		PanelSlot.SetAutoSize(true);
		PanelSlot.SetPosition(FVector2D(16.0, 90.0));
		UVerticalBox Column = Cast<UVerticalBox>(ConstructWidget(UVerticalBox, n"AtelierColumn"));
		Panel.SetContent(Column);

		Column.AddChildToVerticalBox(Label(HOLAtelierText::Get("Title")));

		UHorizontalBox TypeRow = Row(Column);
		AddTo(TypeRow, Label(HOLAtelierText::Get("Type")));
		AddTo(TypeRow, MakeButton(n"AtelierPrevType", "<", n"OnPrevType"));
		TypeText = Label(FText::FromString(""));
		AddTo(TypeRow, Sized(TypeText, 150.0));
		AddTo(TypeRow, MakeButton(n"AtelierNextType", ">", n"OnNextType"));
		TypeBox = Field(n"AtelierTypeBox", TypeRow, 120.0);
		AddTo(TypeRow, MakeButtonText(n"AtelierChooseType", HOLAtelierText::Get("Choose"), n"OnChooseType"));

		Column.AddChildToVerticalBox(Label(FText::FromString(HOLAtelierText::Format("Folder", HOLAtelier::ShownPathHere(HOLAtelier::EntreesDir())))));
		UHorizontalBox FileRow = Row(Column);
		AddTo(FileRow, Label(HOLAtelierText::Get("File")));
		AddTo(FileRow, MakeButton(n"AtelierPrevModel", "<", n"OnPrevModel"));
		ModelText = Label(FText::FromString(""));
		AddTo(FileRow, Sized(ModelText, 300.0));
		AddTo(FileRow, MakeButton(n"AtelierNextModel", ">", n"OnNextModel"));
		AddTo(FileRow, MakeButtonText(n"AtelierRefresh", HOLAtelierText::Get("Refresh"), n"OnRefresh"));
		AddTo(FileRow, MakeButtonText(n"AtelierLoad", HOLAtelierText::Get("Load"), n"OnLoad"));

		UHorizontalBox YawRow = Row(Column);
		AddTo(YawRow, Label(HOLAtelierText::Get("Yaw")));
		AddTo(YawRow, MakeButton(n"AtelierYawMinus15", "-15", n"OnYawMinus15"));
		AddTo(YawRow, MakeButton(n"AtelierYawMinus1", "-1", n"OnYawMinus1"));
		YawText = Label(FText::FromString(""));
		AddTo(YawRow, Sized(YawText, 70.0));
		AddTo(YawRow, MakeButton(n"AtelierYawPlus1", "+1", n"OnYawPlus1"));
		AddTo(YawRow, MakeButton(n"AtelierYawPlus15", "+15", n"OnYawPlus15"));

		UHorizontalBox ScaleRow = Row(Column);
		AddTo(ScaleRow, Label(HOLAtelierText::Get("Scale")));
		AddTo(ScaleRow, MakeButton(n"AtelierScaleMinus10", "-0.1", n"OnScaleMinus10"));
		AddTo(ScaleRow, MakeButton(n"AtelierScaleMinus1", "-0.01", n"OnScaleMinus1"));
		ScaleText = Label(FText::FromString(""));
		AddTo(ScaleRow, Sized(ScaleText, 70.0));
		AddTo(ScaleRow, MakeButton(n"AtelierScalePlus1", "+0.01", n"OnScalePlus1"));
		AddTo(ScaleRow, MakeButton(n"AtelierScalePlus10", "+0.1", n"OnScalePlus10"));

		AuthorBox = LabelledField(Column, "Author", n"AtelierAuthorBox");
		FactionBox = LabelledField(Column, "Faction", n"AtelierFactionBox");
		FileOrPackBox = LabelledField(Column, "FileOrPack", n"AtelierFileOrPackBox");
		SourceBox = LabelledField(Column, "Source", n"AtelierSourceBox");
		LicenceBox = LabelledField(Column, "Licence", n"AtelierLicenceBox");
		UrlBox = LabelledField(Column, "Url", n"AtelierUrlBox");
		NotesBox = LabelledField(Column, "Notes", n"AtelierNotesBox");

		UHorizontalBox StatusRow = Row(Column);
		AddTo(StatusRow, Label(HOLAtelierText::Get("Status")));
		StatusText = Label(FText::FromString(""));
		AddTo(StatusRow, Sized(StatusText, 110.0));
		AddTo(StatusRow, MakeButton(n"AtelierStatus", "final / provisional", n"OnStatus"));

		UHorizontalBox SaveRow = Row(Column);
		AddTo(SaveRow, MakeButtonText(n"AtelierSave", HOLAtelierText::Get("Save"), n"OnSave"));
		MessageText = Label(FText::FromString(""));
		// HALDIR-1127ar4: a long message (the send's folder, a refusal) wraps inside the 560 px instead of running out of the panel.
		Cast<UTextLayoutWidget>(MessageText).SetAutoWrapText(true);
		Column.AddChildToVerticalBox(Sized(MessageText, 560.0));

		Send.Type = Fixed.IsValidIndex(TypeIndex) ? Fixed[TypeIndex] : "";
		Send.Family = HOLAtelier::FamilyOf(Send.Type, Buildings, Sources, Units);
		ShowValues();
		RefreshModels();
	}

	UCommonTextBlock Label(FText Text)
	{
		UCommonTextBlock Shown = Cast<UCommonTextBlock>(ConstructWidget(UCommonTextBlock));
		Shown.SetStyle(UHOLTextStyleHUDLabel);
		Shown.SetText(Text);
		return Shown;
	}

	UHorizontalBox Row(UVerticalBox Column)
	{
		UHorizontalBox Line = Cast<UHorizontalBox>(ConstructWidget(UHorizontalBox));
		Column.AddChildToVerticalBox(Line).SetPadding(FMargin(0.0, 2.0, 0.0, 2.0));
		return Line;
	}

	void AddTo(UHorizontalBox Line, UWidget Child)
	{
		UHorizontalBoxSlot Placed = Line.AddChildToHorizontalBox(Child);
		Placed.SetPadding(FMargin(3.0, 0.0, 3.0, 0.0));
		Placed.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
	}

	USizeBox Sized(UWidget Child, double Width)
	{
		USizeBox Box = Cast<USizeBox>(ConstructWidget(USizeBox));
		Box.SetWidthOverride(Width);
		Box.SetContent(Child);
		return Box;
	}

	UButton MakeButtonText(FName Name, FText Text, FName Handler)
	{
		UButton Button = Cast<UButton>(ConstructWidget(UButton, Name));
		Button.SetBackgroundColor(FLinearColor(0.22, 0.24, 0.34, 1.0));
		Button.SetContent(Label(Text));
		Button.OnClicked.AddUFunction(this, Handler);
		return Button;
	}

	UButton MakeButton(FName Name, const FString& Text, FName Handler)
	{
		return MakeButtonText(Name, FText::FromString(Text), Handler);
	}

	UEditableTextBox Field(FName Name, UHorizontalBox Line, double Width)
	{
		UEditableTextBox Box = Cast<UEditableTextBox>(ConstructWidget(UEditableTextBox, Name));
		AddTo(Line, Sized(Box, Width));
		return Box;
	}

	UEditableTextBox LabelledField(UVerticalBox Column, const FString& Key, FName Name)
	{
		UHorizontalBox Line = Row(Column);
		AddTo(Line, Sized(Label(HOLAtelierText::Get(Key)), 220.0));
		return Field(Name, Line, 320.0);
	}

	void ShowValues()
	{
		TypeText.SetText(FText::FromString(Send.Type + " (" + Send.Family + ")"));
		YawText.SetText(FText::FromString(HOLAtelier::NumberText(Send.Yaw)));
		ScaleText.SetText(FText::FromString(HOLAtelier::NumberText(Send.Scale)));
		StatusText.SetText(FText::FromString(Send.Status));
	}

	void ShowMessage(const FString& Text)
	{
		MessageText.SetText(FText::FromString(Text));
	}

	// --- Type -------------------------------------------------------------------------------------------------------------------
	// Takes Type when it is fixed; a unit or an unknown type is refused (message, previous type kept). Returns the refusal key or "".
	FString ChooseType(const FString& Type)
	{
		const FString Name = Type.TrimStartAndEnd();
		const FString Refused = HOLAtelier::TypeRefusal(Name, Buildings, Sources, Units);
		if (!Refused.IsEmpty())
		{
			ShowMessage(HOLAtelierText::Format("Pair", Name, HOLAtelier::RefusalText(Refused).ToString()));
			Log(n"LogHOLMatch", f"Atelier: type refused {Refused} {Name}");
			return Refused;
		}
		Restore();
		Send.Type = Name;
		Send.Family = HOLAtelier::FamilyOf(Name, Buildings, Sources, Units);
		TypeIndex = Fixed.FindIndex(Name);
		ShowValues();
		ShowMessage("");
		Log(n"LogHOLMatch", f"Atelier: type {Name} family={Send.Family}");
		if (bModelLoaded)
			Apply();
		return "";
	}

	UFUNCTION()
	void OnPrevType()
	{
		if (Fixed.Num() > 0)
			ChooseType(Fixed[(TypeIndex - 1 + Fixed.Num()) % Fixed.Num()]);
	}

	UFUNCTION()
	void OnNextType()
	{
		if (Fixed.Num() > 0)
			ChooseType(Fixed[(TypeIndex + 1) % Fixed.Num()]);
	}

	UFUNCTION()
	void OnChooseType()
	{
		ChooseType(TypeBox.GetText().ToString());
	}

	// --- Model ------------------------------------------------------------------------------------------------------------------
	// Loads the named file (a bare name: in the known folder) in the place of the chosen type's model. Returns the refusal key or "".
	FString LoadModel(const FString& Name)
	{
		const FString Path = HOLAtelier::ModelPathOf(HOLAtelier::EntreesDir(), Name);
		FString Refused = HOLArtRuntimeLoad::Refusal(Path);
		const FString Lower = Path.ToLower();
		if (Refused.IsEmpty() && !Lower.EndsWith(".glb") && !Lower.EndsWith(".gltf"))
			Refused = "format";
		// The editor binary run with -game (play.ps1, the agents' scenarios): Interchange's material factory makes a MaterialInstanceConstant,
		// which asserts GIsEditor and stops the game (seen 2026-11-27, GLORFINDEL-1127ar2). There only a model without materials is imported
		// (its JSON names no "materials"); the published game imports any glTF / glb.
		if (Refused.IsEmpty() && FPlatformProcess::ExecutableName().Contains("UnrealEditor") && HOLAtelier::HasSwitch(FCommandLine::Get(), "game"))
		{
			FString Text;
			const bool bRead = FFileHelper::LoadFileToString(Text, Path);
			if (!bRead || !Text.Contains("\"asset\"") || Text.Contains("\"materials\""))
				Refused = "editor_game";
		}
		UStaticMesh Mesh = nullptr;
		if (Refused.IsEmpty())
		{
			Loads++;
			Mesh = HOLArtRuntimeLoad::ImportStaticMesh(Path, Loads);
			if (Mesh == nullptr)
				Refused = "import";
		}
		if (!Refused.IsEmpty())
		{
			ShowMessage(HOLAtelierText::Format("Pair", HOLAtelier::ShownPathHere(Path), HOLAtelier::RefusalText(Refused).ToString()));
			Log(n"LogHOLMatch", f"Atelier: load refused {Refused} {Path}");
			return Refused;
		}
		Restore();
		LoadedMesh = Mesh;
		bModelLoaded = true;
		Send.ModelFile = Path;
		if (FileOrPackBox.GetText().ToString().TrimStartAndEnd().IsEmpty())
			FileOrPackBox.SetText(FText::FromString(FPaths::GetCleanFilename(Path)));
		const int Drawn = Apply();
		ShowMessage(HOLAtelierText::Format("Loaded", Send.Type, FPaths::GetCleanFilename(Path)));
		Log(n"LogHOLMatch", f"Atelier: loaded {Path} mesh={Mesh.Name} type={Send.Type} pieces={Drawn}");
		System::ExecuteConsoleCommand(f"hol.Art.Focus {Send.Type} 18");
		return "";
	}

	// Reads the models of Entrees again (the chosen one kept when it is still there, else the first). Returns how many.
	int RefreshModels()
	{
		const FString Kept = Models.IsValidIndex(ModelIndex) ? Models[ModelIndex] : "";
		Models = HOLAtelier::ListModels(HOLAtelier::EntreesDir());
		const int Found = Models.FindIndex(Kept);
		ModelIndex = Found >= 0 ? Found : HOLAtelier::StepIndex(-1, Models.Num(), 1);
		ShowModel();
		FString Names = "";
		for (const FString& Name : Models)
			Names += " " + Name;
		Log(n"LogHOLMatch", f"Atelier: models {Models.Num()}{Names}");
		return Models.Num();
	}

	void ShowModel()
	{
		if (Models.IsValidIndex(ModelIndex))
			ModelText.SetText(FText::FromString(f"{Models[ModelIndex]} ({ModelIndex + 1}/{Models.Num()})"));
		else
			ModelText.SetText(HOLAtelierText::Get("NoModels"));
	}

	// Chooses a model of the list by its name and loads it. Returns the refusal key or "" (a name not in the list: "missing").
	FString PickModel(const FString& ModelName)
	{
		RefreshModels();
		const int Found = Models.FindIndex(ModelName.TrimStartAndEnd());
		if (Found < 0)
		{
			ShowMessage(HOLAtelierText::Format("Pair", ModelName, HOLAtelier::RefusalText("missing").ToString()));
			Log(n"LogHOLMatch", f"Atelier: load refused missing {ModelName} (not in the list)");
			return "missing";
		}
		ModelIndex = Found;
		ShowModel();
		Log(n"LogHOLMatch", f"Atelier: picked {Models[ModelIndex]} {ModelIndex + 1}/{Models.Num()}");
		return LoadModel(Models[ModelIndex]);
	}

	UFUNCTION()
	void OnPrevModel()
	{
		ModelIndex = HOLAtelier::StepIndex(ModelIndex, Models.Num(), -1);
		ShowModel();
	}

	UFUNCTION()
	void OnNextModel()
	{
		ModelIndex = HOLAtelier::StepIndex(ModelIndex, Models.Num(), 1);
		ShowModel();
	}

	UFUNCTION()
	void OnRefresh()
	{
		RefreshModels();
	}

	UFUNCTION()
	void OnLoad()
	{
		if (!Models.IsValidIndex(ModelIndex))
		{
			ShowMessage(HOLAtelier::RefusalText("empty").ToString());
			return;
		}
		LoadModel(Models[ModelIndex]);
	}

	// Draws the loaded model in the place of the chosen type: the first visible piece of each building of the type takes the model
	// (rotation yaw, scale), its other pieces are hidden; a source: the model on the type's gallery place. Returns the pieces drawn.
	int Apply()
	{
		Restore();
		if (!bModelLoaded || LoadedMesh == nullptr)
			return 0;
		int Drawn = 0;
		if (Send.Family == "Buildings")
		{
			// The type of a building: the standing buildings' list (AHOLBuilding::TypeName is not readable from the script).
			TArray<FIntPoint> Cells;
			TArray<FName> Types;
			TArray<int> Owners;
			TArray<AHOLBuilding> Standing;
			TArray<bool> Finished;
			HOLConstructionRead::GetStandingBuildingCells(1, Cells, Types, Owners, Standing, Finished);
			TArray<AHOLBuilding> Seen;
			for (int Index = 0; Index < Standing.Num(); Index++)
			{
				AHOLBuilding Building = Standing[Index];
				if (Building == nullptr || Seen.Contains(Building) || Types[Index].ToString() != Send.Type)
					continue;
				Seen.Add(Building);
				bool bFirst = true;
				TArray<UActorComponent> Found = Building.GetComponentsByClass(UStaticMeshComponent);
				for (UActorComponent Component : Found)
				{
					UStaticMeshComponent Piece = Cast<UStaticMeshComponent>(Component);
					if (Piece == nullptr || Piece.StaticMesh == nullptr || !Piece.IsVisible())
						continue;
					Pieces.Add(Piece);
					PieceMeshes.Add(Piece.StaticMesh);
					PieceRotations.Add(Piece.RelativeRotation);
					PieceScales.Add(Piece.RelativeScale3D);
					PieceShown.Add(true);
					if (bFirst)
					{
						Piece.SetStaticMesh(LoadedMesh);
						ModelSpot = Building.GetActorLocation();
						bFirst = false;
						Drawn++;
					}
					else
					{
						Piece.SetVisibility(false);
					}
				}
			}
		}
		else if (Send.Family == "Sources")
		{
			TArray<FHOLArtGalleryPlace> Places;
			HOLArtGallery::PlanAwayFromBase("All", Places);
			FVector Centre;
			double Side = 0.0;
			if (HOLArtGallery::FocusOn(Places, Send.Type, Centre, Side))
			{
				SourcePreview = Cast<AStaticMeshActor>(SpawnActor(AStaticMeshActor::StaticClass(), Centre));
				if (SourcePreview != nullptr)
				{
					SourcePreview.StaticMeshComponent.SetMobility(EComponentMobility::Movable);
					SourcePreview.StaticMeshComponent.SetStaticMesh(LoadedMesh);
					ModelSpot = Centre;
					Drawn++;
				}
			}
		}
		Turn();
		return Drawn;
	}

	// Rotation and scale of the drawn model (the manifest's yaw = MeshYawOffset, scale = MeshScale).
	void Turn()
	{
		for (int K = 0; K < Pieces.Num(); K++)
		{
			if (Pieces[K] != nullptr && Pieces[K].StaticMesh == LoadedMesh)
			{
				Pieces[K].SetRelativeRotation(FRotator(0.0, Send.Yaw, 0.0));
				Pieces[K].SetRelativeScale3D(FVector(Send.Scale, Send.Scale, Send.Scale));
			}
		}
		if (SourcePreview != nullptr)
		{
			SourcePreview.SetActorRotation(FRotator(0.0, Send.Yaw, 0.0));
			SourcePreview.SetActorScale3D(FVector(Send.Scale, Send.Scale, Send.Scale));
		}
		ShowValues();
	}

	// The type's pieces as they were before the model.
	void Restore()
	{
		for (int K = 0; K < Pieces.Num(); K++)
		{
			if (Pieces[K] == nullptr)
				continue;
			Pieces[K].SetStaticMesh(PieceMeshes[K]);
			Pieces[K].SetRelativeRotation(PieceRotations[K]);
			Pieces[K].SetRelativeScale3D(PieceScales[K]);
			Pieces[K].SetVisibility(PieceShown[K]);
		}
		Pieces.Empty();
		PieceMeshes.Empty();
		PieceRotations.Empty();
		PieceScales.Empty();
		PieceShown.Empty();
		if (SourcePreview != nullptr)
		{
			SourcePreview.DestroyActor();
			SourcePreview = nullptr;
		}
	}

	void SetYaw(double Value)
	{
		double Yaw = Value;
		while (Yaw > 180.0)
			Yaw -= 360.0;
		while (Yaw <= -180.0)
			Yaw += 360.0;
		Send.Yaw = Yaw;
		Turn();
		Log(n"LogHOLMatch", f"Atelier: yaw {HOLAtelier::NumberText(Send.Yaw)}");
	}

	void SetScale(double Value)
	{
		Send.Scale = Math::Max(0.01, Math::RoundToInt(Value * 100.0) / 100.0);
		Turn();
		Log(n"LogHOLMatch", f"Atelier: scale {HOLAtelier::NumberText(Send.Scale)}");
	}

	UFUNCTION() void OnYawMinus15() { SetYaw(Send.Yaw - 15.0); }
	UFUNCTION() void OnYawMinus1() { SetYaw(Send.Yaw - 1.0); }
	UFUNCTION() void OnYawPlus1() { SetYaw(Send.Yaw + 1.0); }
	UFUNCTION() void OnYawPlus15() { SetYaw(Send.Yaw + 15.0); }
	UFUNCTION() void OnScaleMinus10() { SetScale(Send.Scale - 0.1); }
	UFUNCTION() void OnScaleMinus1() { SetScale(Send.Scale - 0.01); }
	UFUNCTION() void OnScalePlus1() { SetScale(Send.Scale + 0.01); }
	UFUNCTION() void OnScalePlus10() { SetScale(Send.Scale + 0.1); }

	UFUNCTION()
	void OnStatus()
	{
		Send.Status = Send.Status == "final" ? "provisional" : "final";
		ShowValues();
	}

	// One field by its manifest name (the console's hol.Art.Atelier.Set); false for an unknown field.
	bool SetField(const FString& Name, const FString& Value)
	{
		if (Name == "yaw") { SetYaw(String::Conv_StringToDouble(Value)); return true; }
		if (Name == "scale") { SetScale(String::Conv_StringToDouble(Value)); return true; }
		if (Name == "status") { Send.Status = Value.TrimStartAndEnd(); ShowValues(); return true; }
		UEditableTextBox Box = nullptr;
		if (Name == "author") Box = AuthorBox;
		else if (Name == "faction") Box = FactionBox;
		else if (Name == "file_or_pack") Box = FileOrPackBox;
		else if (Name == "source") Box = SourceBox;
		else if (Name == "licence") Box = LicenceBox;
		else if (Name == "url") Box = UrlBox;
		else if (Name == "notes") Box = NotesBox;
		if (Box == nullptr)
			return false;
		Box.SetText(FText::FromString(Value));
		return true;
	}

	// --- Save -------------------------------------------------------------------------------------------------------------------
	// Writes the send folder; every refusal is listed before anything is written. Returns the folder written, "" when refused.
	FString Save()
	{
		Send.Author = AuthorBox.GetText().ToString();
		Send.Faction = FactionBox.GetText().ToString().TrimStartAndEnd();
		Send.FileOrPack = FileOrPackBox.GetText().ToString();
		Send.Source = SourceBox.GetText().ToString();
		Send.Licence = LicenceBox.GetText().ToString();
		Send.Url = UrlBox.GetText().ToString();
		Send.Notes = NotesBox.GetText().ToString();
		const TArray<FString> Refusals = HOLAtelier::SaveRefusals(Send, bModelLoaded, Buildings, Sources, Units);
		if (Refusals.Num() > 0)
		{
			FString Text = HOLAtelierText::Get("Refused").ToString();
			FString Keys = "";
			for (const FString& Key : Refusals)
			{
				Text += "\n- " + HOLAtelier::RefusalText(Key).ToString();
				Keys += " " + Key;
			}
			ShowMessage(Text);
			Log(n"LogHOLMatch", f"Atelier: save refused{Keys}");
			return "";
		}
		// A second save in the same minute gets its own folder (_2, _3...): a send is never mixed with another one.
		const FString Base = HOLAtelier::EnvoisDir() + HOLAtelier::SendFolderName(Send.Type, Send.Faction, FDateTime::Now());
		FString Folder = Base;
		int Again = 2;
		while (FPaths::DirectoryExists(Folder))
		{
			Folder = Base + f"_{Again}";
			Again++;
		}
		const FString ModelName = HOLAtelier::SendModelName(Send.ModelFile);
		const bool bManifest = FFileHelper::SaveStringToFile(HOLAtelier::ManifestJson(Send), Folder + "/manifest.json", FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		// The model copied as model.glb / model.gltf (AR8-c, D931: UHOLArtVisualRegistry::CopyArtFile, byte for byte).
		const bool bCopied = !HOLAtelier::CopyModelToSend(Send.ModelFile, Folder).IsEmpty();
		// A .gltf that names other files (.bin, textures): those are not copied, the message says so.
		bool bExtra = false;
		if (ModelName == "model.gltf")
		{
			FString Gltf;
			if (FFileHelper::LoadFileToString(Gltf, Send.ModelFile))
				bExtra = Gltf.Contains("\"uri\"") && Gltf.Replace("\"uri\":\"data:", "").Replace("\"uri\": \"data:", "").Contains("\"uri\"");
		}
		const bool bPreview = CapturePreview(Folder);
		FString Text = HOLAtelierText::Format("Saved", HOLAtelier::ShownPathHere(Folder));
		if (bCopied)
			Text += "\n" + HOLAtelierText::Format("Copied", ModelName);
		else
			Text += "\n" + HOLAtelierText::Format("CopyByHand", ModelName, HOLAtelier::ShownPathHere(Send.ModelFile));
		if (bExtra)
			Text += "\n" + HOLAtelierText::Get("CopyExtra").ToString();
		if (!bPreview)
			Text += "\n" + HOLAtelierText::Get("NoPreview").ToString();
		Text += "\n" + HOLAtelierText::Get("SendBack").ToString();
		ShowMessage(Text);
		Log(n"LogHOLMatch", f"Atelier: saved {Folder} manifest={bManifest} model_copied={bCopied} preview={bPreview} type={Send.Type} yaw={HOLAtelier::NumberText(Send.Yaw)} scale={HOLAtelier::NumberText(Send.Scale)}");
		return Folder;
	}

	UFUNCTION()
	void OnSave()
	{
		Save();
	}

	// preview.png: a scene capture of the model on its place (512 px), written in the send folder.
	bool CapturePreview(const FString& Folder)
	{
		if (LoadedMesh == nullptr)
			return false;
		const double Radius = Math::Max(LoadedMesh.GetBounds().SphereRadius * Send.Scale, 150.0);
		const FVector Target = ModelSpot + FVector(0.0, 0.0, Radius * 0.4);
		const FVector Eye = Target + FVector(-Radius * 1.5, -Radius * 1.5, Radius * 1.3);
		const FRotator Look = (Target - Eye).Rotation();
		ASceneCapture2D Camera = Cast<ASceneCapture2D>(SpawnActor(ASceneCapture2D::StaticClass(), Eye, Look));
		if (Camera == nullptr)
			return false;
		USceneCaptureComponent2D Capture = Camera.CaptureComponent2D;
		Capture.bCaptureEveryFrame = false;
		Capture.bCaptureOnMovement = false;
		Capture.CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		Capture.FOVAngle = 50.0;
		Capture.TextureTarget = Rendering::CreateRenderTarget2D(512, 512, ETextureRenderTargetFormat::RTF_RGBA8);
		Capture.CaptureScene();
		Rendering::ExportRenderTarget(Capture.TextureTarget, Folder, "preview.png");
		Camera.DestroyActor();
		return FPaths::FileExists(Folder + "/preview.png");
	}
}

// --- Entry: -Atelier ------------------------------------------------------------------------------------------------------------
// hol.Art.Atelier.State (one run): 0 nothing done, 1 the atelier's match launched from the main menu, 2 the atelier open.
class UHOLAtelierEntry : UScriptWorldSubsystem
{
	float Elapsed = 0.0;
	FString Step = "?";
	UHOLAtelierScreen Screen;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		Elapsed += DeltaTime;
		if (Step == "?")
		{
			const FString Map = GetWorld().GetName().ToString();
			const int State = CVar_ArtAtelierState.GetInt();
			const bool bDone = Map.Contains("MainMenu") ? State >= 1 : State >= 2;
			Step = HOLAtelier::EntryStep(Map, HOLAtelier::IsAsked(FCommandLine::Get()), bDone);
			if (!Step.IsEmpty())
				Log(n"LogHOLMatch", f"Atelier: -Atelier on {Map}: {Step}");
		}
		if (Step == "launch" && Elapsed > 2.0)
		{
			Step = "";
			HOLAtelierOpen::LaunchMatch();
		}
		else if (Step == "lab" && Elapsed > 6.0 && HOLArtLab::LocalController() != nullptr)
		{
			Step = "";
			HOLAtelierOpen::Open();
		}
	}
}

namespace HOLAtelierOpen
{
	// The atelier's match (the main menu's New game launch, as the scenarios do): Plain, Small, the artist and one AI (hol.Art.Lab turns combat off).
	void LaunchMatch()
	{
		const FString File = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + "Atelier/atelier_match.json");
		const FString Json = "{ \"map\": \"/Game/Maps/Plain.Plain\", \"mapSize\": \"Small\", \"slots\": [ { \"state\": \"Human\", \"colorIndex\": 0 }, { \"state\": \"AI\", \"colorIndex\": 1 } ] }";
		FFileHelper::SaveStringToFile(Json, File, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		CVar_ArtAtelierState.SetInt(1);
		Log(n"LogHOLMatch", f"Atelier: launch {File}");
		System::ExecuteConsoleCommand(f"hol.UI.NewGame.Launch {File} 0");
	}

	// The lab (hol.Art.Lab: gallery and scale marks, light, fast construction, combat off) then the screen. Also hol.Art.Atelier.Open.
	void Open()
	{
		AHOLPlayerController Controller = HOLArtLab::LocalController();
		UHOLAtelierEntry Entry = UHOLAtelierEntry::Get();
		if (Controller == nullptr || Entry == nullptr)
		{
			Log(n"LogHOLMatch", "Atelier: open refused (no local player: a Plain match first)");
			return;
		}
		CVar_ArtAtelierState.SetInt(2);
		if (Entry.Screen == nullptr)
		{
			// The steps of hol.Art.Lab without its on-screen help (it would cover the screen): combat off, fast construction, gallery and marks, light.
			const TArray<FString> Steps = HOLArtLab::Steps(HOLArtLab::DefaultBiome());
			for (const FString& Line : Steps)
				System::ExecuteConsoleCommand(Line, Controller);
			UHOLAtelierScreen Screen = Cast<UHOLAtelierScreen>(WidgetBlueprint::CreateWidget(UHOLAtelierScreen, Controller));
			Screen.Build();
			Screen.AddToViewport(40);
			Entry.Screen = Screen;
			// The folder of the models exists from the first opening (a note in it says what goes there).
			FFileHelper::SaveStringToFile("Posez ici vos modeles .glb / .gltf (batiments et sources), puis choisissez-les dans la liste de l'atelier (bouton Rafraichir).\n", HOLAtelier::EntreesDir() + "LISEZMOI.txt", FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
			Screen.RefreshModels();
		}
		System::ExecuteConsoleCommand("hol.Art.GalleryFrame Buildings South", Controller);
		Log(n"LogHOLMatch", f"Atelier: open entrees={HOLAtelier::EntreesDir()} envois={HOLAtelier::EnvoisDir()}");
	}

	UHOLAtelierScreen Screen()
	{
		UHOLAtelierEntry Entry = UHOLAtelierEntry::Get();
		if (Entry == nullptr || Entry.Screen == nullptr)
		{
			Log(n"LogHOLMatch", "Atelier: no screen (hol.Art.Atelier.Open first)");
			return nullptr;
		}
		return Entry.Screen;
	}

	FString Joined(const TArray<FString>& Args, int From)
	{
		FString Out = "";
		for (int K = From; K < Args.Num(); K++)
			Out += (K > From ? " " : "") + Args[K];
		return Out;
	}
}

void HOLArtAtelierOpenCommand(const TArray<FString>& Args)
{
	HOLAtelierOpen::Open();
}

void HOLArtAtelierTypeCommand(const TArray<FString>& Args)
{
	UHOLAtelierScreen Screen = HOLAtelierOpen::Screen();
	if (Screen != nullptr)
		Screen.ChooseType(HOLAtelierOpen::Joined(Args, 0));
}

void HOLArtAtelierLoadCommand(const TArray<FString>& Args)
{
	UHOLAtelierScreen Screen = HOLAtelierOpen::Screen();
	if (Screen == nullptr)
		return;
	// A name: chosen in the list of Entrees (as the < > buttons do); a full path: loaded from there (developers).
	const FString Name = HOLAtelierOpen::Joined(Args, 0).TrimStartAndEnd();
	if (Name.Contains(":") || Name.StartsWith("/") || Name.StartsWith("\\"))
		Screen.LoadModel(Name);
	else
		Screen.PickModel(Name);
}

void HOLArtAtelierRefreshCommand(const TArray<FString>& Args)
{
	UHOLAtelierScreen Screen = HOLAtelierOpen::Screen();
	if (Screen != nullptr)
		Screen.RefreshModels();
}

void HOLArtAtelierSetCommand(const TArray<FString>& Args)
{
	UHOLAtelierScreen Screen = HOLAtelierOpen::Screen();
	if (Screen == nullptr || Args.Num() < 1)
		return;
	const FString Value = HOLAtelierOpen::Joined(Args, 1);
	const bool bSet = Screen.SetField(Args[0], Value);
	Log(n"LogHOLMatch", f"Atelier: set {Args[0]} {bSet}");
}

void HOLArtAtelierSaveCommand(const TArray<FString>& Args)
{
	UHOLAtelierScreen Screen = HOLAtelierOpen::Screen();
	if (Screen != nullptr)
		Screen.Save();
}

const FConsoleCommand HOLArtAtelierOpenConsole("hol.Art.Atelier.Open", n"HOLArtAtelierOpenCommand");
const FConsoleCommand HOLArtAtelierTypeConsole("hol.Art.Atelier.Type", n"HOLArtAtelierTypeCommand");
const FConsoleCommand HOLArtAtelierRefreshConsole("hol.Art.Atelier.Refresh", n"HOLArtAtelierRefreshCommand");
const FConsoleCommand HOLArtAtelierLoadConsole("hol.Art.Atelier.Load", n"HOLArtAtelierLoadCommand");
const FConsoleCommand HOLArtAtelierSetConsole("hol.Art.Atelier.Set", n"HOLArtAtelierSetCommand");
const FConsoleCommand HOLArtAtelierSaveConsole("hol.Art.Atelier.Save", n"HOLArtAtelierSaveCommand");
