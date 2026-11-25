// hol.Art.Try <Type> [N] [Slot] (AR4-a, CELEBORN-1125ar2, ArtistesInProgress.md lot AR4 "Jouer avec"): in a running Plain match, puts N copies of a type
// next to the local player's Forum (a unit: of the slot, selected, the camera on it; a building: placed and finished) and turns on fast build and +1000 resources.
// An unknown type prints the close known types on screen. The place of each copy is the pure rule FreePlaceNearForum (tested by HOLArtTry_Test.as);
// the world chain only reuses routes already exposed to script (SpawnUnitsAround, ServerCheatSpawnBuilding, hol.Cheat.*, hol.Select.BoxGround, hol.Camera.Focus).
namespace HOLArtTry
{
	// Gap kept between the Forum's edge and a placed object, and between two placed objects (cm).
	float Gap()
	{
		return 150.0;
	}

	// The Forum's footprint radius used to keep clear of it (cm); the type table's footprint is not exposed to script.
	float ForumRadius()
	{
		return 1300.0;
	}

	float UnitRadius()
	{
		return 60.0;
	}

	float BuildingRadius()
	{
		return 450.0;
	}

	float FlatDistance(FVector A, FVector B)
	{
		return FVector(A.X - B.X, A.Y - B.Y, 0.0).Size();
	}

	bool IsClear(FVector Place, float ObjectRadius, const TArray<FVector>& Occupied, const TArray<float>& OccupiedRadii)
	{
		for (int Index = 0; Index < Occupied.Num(); ++Index)
		{
			if (FlatDistance(Place, Occupied[Index]) < OccupiedRadii[Index] + ObjectRadius + Gap() - 0.5)
				return false;
		}
		return true;
	}

	// The first free place next to a Forum centred on ForumCenter with radius InForumRadius: on rings around the Forum (the nearest first, from the east,
	// counter-clockwise), the first point whose disc of radius ObjectRadius stays clear (Gap) of every Occupied disc (centre in Occupied, radius in OccupiedRadii).
	// Every ring is one object width plus a gap further out; with no free point on the four rings, the last point of the outer ring.
	FVector FreePlaceNearForum(FVector ForumCenter, float InForumRadius, float ObjectRadius, const TArray<FVector>& Occupied, const TArray<float>& OccupiedRadii)
	{
		const float Step = 2.0 * ObjectRadius + Gap();
		FVector Last = ForumCenter;
		for (int Ring = 0; Ring < 4; ++Ring)
		{
			const float Radius = InForumRadius + Gap() + ObjectRadius + Ring * Step;
			const int Count = Math::Max(6, Math::FloorToInt(6.2831853 * Radius / Step));
			for (int Point = 0; Point < Count; ++Point)
			{
				const float Angle = Math::DegreesToRadians(360.0 * Point / Count);
				const FVector Place = ForumCenter + FVector(Radius * Math::Cos(Angle), Radius * Math::Sin(Angle), 0.0);
				Last = Place;
				if (IsClear(Place, ObjectRadius, Occupied, OccupiedRadii))
					return Place;
			}
		}
		return Last;
	}

	// Letters apart between A and B (Levenshtein, case-blind callers lower first).
	int LettersApart(const FString& A, const FString& B)
	{
		TArray<int> Previous;
		TArray<int> Current;
		for (int J = 0; J <= B.Len(); ++J)
			Previous.Add(J);
		for (int I = 1; I <= A.Len(); ++I)
		{
			Current.Empty();
			Current.Add(I);
			for (int J = 1; J <= B.Len(); ++J)
			{
				const int Cost = A.Mid(I - 1, 1) == B.Mid(J - 1, 1) ? 0 : 1;
				Current.Add(Math::Min(Math::Min(Previous[J] + 1, Current[J - 1] + 1), Previous[J - 1] + Cost));
			}
			Previous = Current;
		}
		return Previous[B.Len()];
	}

	// How close Known is to Typed: 0 the same (case-blind), 1 one starts the other, 2 one holds the other, 3 a few letters apart; -1 far.
	int Closeness(const FString& Typed, const FString& Known)
	{
		const FString A = Typed.ToLower();
		const FString B = Known.ToLower();
		if (A.IsEmpty() || B.IsEmpty())
			return -1;
		if (A == B)
			return 0;
		if (A.StartsWith(B) || B.StartsWith(A))
			return 1;
		if (A.Contains(B) || B.Contains(A))
			return 2;
		if (LettersApart(A, B) <= Math::Max(2, Math::Min(A.Len(), B.Len()) / 4))
			return 3;
		return -1;
	}

	// The known types close to Typed (case-blind prefix, contains, or a few letters apart), the closest first, at most eight.
	TArray<FString> NearTypes(const FString& Typed, const TArray<FString>& Known)
	{
		TArray<FString> Near;
		for (int Level = 0; Level <= 3; ++Level)
		{
			for (const FString& Name : Known)
			{
				if (Near.Num() < 8 && Closeness(Typed, Name) == Level && !Near.Contains(Name))
					Near.Add(Name);
			}
		}
		return Near;
	}

	// The first column of a type table of Tools/Data (Name), header skipped; empty when the file is not there (a packaged game).
	void AddTableNames(const FString& FileName, TArray<FString>& Names)
	{
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, FPaths::ProjectDir() + "Tools/Data/" + FileName))
			return;
		TArray<FString> Lines;
		Text.ParseIntoArray(Lines, "\n", true);
		for (int Index = 1; Index < Lines.Num(); ++Index)
		{
			TArray<FString> Cells;
			Lines[Index].ParseIntoArray(Cells, ",", false);
			const FString Name = Cells.Num() > 0 ? Cells[0].TrimStartAndEnd() : "";
			if (!Name.IsEmpty() && !Name.StartsWith("#"))
				Names.Add(Name);
		}
	}

	void Say(const FString& Line, bool bWarning)
	{
		Print(Line, 8.0);
		if (bWarning)
			HOLScriptLog::LogWarning(n"LogHOLMatch", Line);
		else
			Log(n"LogHOLMatch", Line);
	}

	AHOLPlayerController LocalController()
	{
		TArray<AHOLPlayerController> Controllers;
		GetAllActorsOfClass(Controllers);
		for (AHOLPlayerController Controller : Controllers)
		{
			if (Controller.IsLocalController())
				return Controller;
		}
		return nullptr;
	}

	void Run(const TArray<FString>& Args)
	{
		if (Args.Num() == 0)
		{
			Say("Usage: hol.Art.Try <Type> [N=1] [Slot=0]", true);
			return;
		}
		const FString Type = Args[0];
		const int Count = Math::Clamp(Args.Num() >= 2 ? HOLControllerCommands::ParseInt(Args[1]) : 1, 1, 50);
		const int Slot = Args.Num() >= 3 ? HOLControllerCommands::ParseInt(Args[2]) : 0;
		FHOLUnitTypeSummary Row;
		const bool bUnit = HOLProductionTables::FindUnitType(FName(Type), Row);
		const bool bBuilding = !bUnit && HOLCommandGridRead::IsTypeInTable(FName(Type));
		if (!bUnit && !bBuilding)
		{
			TArray<FString> Known;
			AddTableNames("UnitTypes.csv", Known);
			AddTableNames("BuildingTypes.csv", Known);
			AddTableNames("SourceTypes.csv", Known);
			TArray<FString> Sources;
			AddTableNames("SourceTypes.csv", Sources);
			if (Sources.Contains(Type))
			{
				Say(f"hol.Art.Try: {Type} is a resource source; placing a source is not wired yet (AR4-a RESTE)", true);
				return;
			}
			const TArray<FString> Near = NearTypes(Type, Known);
			FString List;
			for (const FString& Name : Near)
				List += (List.IsEmpty() ? "" : ", ") + Name;
			if (List.IsEmpty())
				List = "(none)";
			Say(f"hol.Art.Try: unknown type {Type}; close types: {List}", true);
			return;
		}
		AHOLPlayerController Controller = LocalController();
		FVector Forum;
		if (Controller == nullptr || !HOLControllerRead::PointNearMainCentre(Controller, 0.0, 0.0, false, Forum))
		{
			Say("hol.Art.Try: start a Plain match first (no local player or no Forum)", true);
			return;
		}
		System::ExecuteConsoleCommand("hol.Cheat.FastBuild 1", Controller);
		System::ExecuteConsoleCommand("hol.Cheat.Resources 1000", Controller);

		TArray<FVector> Occupied;
		TArray<float> Radii;
		Occupied.Add(Forum);
		Radii.Add(ForumRadius());
		TArray<AHOLBuilding> Buildings;
		GetAllActorsOfClass(Buildings);
		for (AHOLBuilding Building : Buildings)
		{
			if (FlatDistance(Building.GetActorLocation(), Forum) > 1.0)
			{
				Occupied.Add(Building.GetActorLocation());
				Radii.Add(BuildingRadius());
			}
		}
		const float ObjectRadius = bUnit ? UnitRadius() : BuildingRadius();
		FVector Low = Forum;
		FVector High = Forum;
		FVector Sum = FVector::ZeroVector;
		AHOLRpcPlayer Rpc = bBuilding ? Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(Controller)) : nullptr;
		for (int Copy = 0; Copy < Count; ++Copy)
		{
			const FVector Place = FreePlaceNearForum(Forum, ForumRadius(), ObjectRadius, Occupied, Radii);
			Occupied.Add(Place);
			Radii.Add(ObjectRadius);
			if (bUnit)
			{
				if (!HOLControllerRead::SpawnUnitsAround(Controller, 1, FName(Type), Slot, 1.0, Place))
				{
					Say("hol.Art.Try: units are made on the server or in solo, once the match runs", true);
					return;
				}
			}
			else if (Rpc != nullptr)
				Rpc.ServerCheatSpawnBuilding(Type, Slot, Place);
			Low = Copy == 0 ? Place : FVector(Math::Min(Low.X, Place.X), Math::Min(Low.Y, Place.Y), 0.0);
			High = Copy == 0 ? Place : FVector(Math::Max(High.X, Place.X), Math::Max(High.Y, Place.Y), 0.0);
			Sum += Place;
		}
		const FVector Centre = Sum / Count;
		// The camera first: hol.Select.BoxGround projects its ground corners on the screen, off-screen corners select nothing (art_try 1125ar2).
		System::ExecuteConsoleCommand(f"hol.Camera.Focus {Math::RoundToInt(Centre.X)} {Math::RoundToInt(Centre.Y)}", Controller);
		if (bUnit)
		{
			const float Margin = 2.0 * ObjectRadius;
			System::ExecuteConsoleCommand(f"hol.Select.BoxGround {Math::RoundToInt(Low.X - Margin)} {Math::RoundToInt(Low.Y - Margin)} {Math::RoundToInt(High.X + Margin)} {Math::RoundToInt(High.Y + Margin)}", Controller);
		}
		Say(f"hol.Art.Try: {Count} {Type} of slot {Slot} next to the Forum around ({Math::RoundToInt(Centre.X)}, {Math::RoundToInt(Centre.Y)}); fast build on, +1000 resources", false);
	}
}

void HOLArtTryCommand(const TArray<FString>& Args) { HOLArtTry::Run(Args); }

const FConsoleCommand HOLArtTryConsole("hol.Art.Try", n"HOLArtTryCommand");
