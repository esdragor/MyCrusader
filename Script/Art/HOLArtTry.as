// hol.Art.Try <Type> [N] [Slot] (AR4-a, CELEBORN-1125ar2, ArtistesInProgress.md lot AR4 "Jouer avec"): in a running Plain match, puts N copies of a type
// next to the local player's Forum (a unit: of the slot, selected one frame later, the camera on it; a building: placed and finished; a source: everyone's,
// its table amount, AR4-b) and turns on fast build and +1000 resources.
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

	float SourceRadius()
	{
		return 300.0;
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

	// AR4-b: the selection of the copies waits for the frame after their birth (Mass units are not there in the frame they are asked for).
	bool SelectionDue(int FramesSinceBirth)
	{
		return FramesSinceBirth >= 1;
	}

	// AR4-b (CELEBORN-1125ar5): the box of hol.Select.BoxGround one frame after the birth found no unit (no "Select: box" line in the log, the hero panel stayed):
	// the copies reach the pick candidates only at a later simulation step. The selector boxes again until the own selection holds every copy.
	bool SelectionDone(int OwnSelected, int Wanted)
	{
		return OwnSelected >= Wanted;
	}

	// AR4-b (CELEBORN-1125ar6): art_try captures the shot on an event, not after a fixed wait (12 s = melee, the 1.6 s puff of powder already gone).
	// The first shot is due when a ranged copy (attack range of at least RangedMinRange cm) has an enemy within its range: the selector,
	// once the copies are selected, logs "hol.Art.Try: first shot" then (waitlog of art_try). A melee copy in contact is no shot.
	float RangedMinRange() { return 300.0; }
	bool FirstShotDue(float AttackRange, int EnemiesInRange)
	{
		return AttackRange >= RangedMinRange() && EnemiesInRange > 0;
	}

	// AR4-b: what Type is: "Unit", "Building", "Source" (a row of the source table) or "" (unknown).
	// Read on the three type tables themselves (the paths of hol.Art.Gallery), the same outside a match (tests) and in it.
	bool IsRowOf(const FString& TablePath, const FString& Type)
	{
		UDataTable Table = Cast<UDataTable>(LoadObject(nullptr, TablePath));
		if (Table == nullptr)
			return false;
		TArray<FName> Names;
		Table.GetRowNames(Names);
		return Names.Contains(FName(Type));
	}

	FString KindOf(const FString& Type)
	{
		if (IsRowOf("/Game/Units/UnitTypes.UnitTypes", Type))
			return "Unit";
		if (IsRowOf("/Game/Buildings/BuildingTypes.BuildingTypes", Type))
			return "Building";
		if (IsRowOf("/Game/Resources/SourceTypes.SourceTypes", Type))
			return "Source";
		return "";
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
		const FString Kind = KindOf(Type);
		const bool bUnit = Kind == "Unit";
		const bool bBuilding = Kind == "Building";
		const bool bSource = Kind == "Source";
		if (Kind.IsEmpty())
		{
			TArray<FString> Known;
			AddTableNames("UnitTypes.csv", Known);
			AddTableNames("BuildingTypes.csv", Known);
			AddTableNames("SourceTypes.csv", Known);
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
		const float ObjectRadius = bUnit ? UnitRadius() : (bSource ? SourceRadius() : BuildingRadius());
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
			else if (bSource)
			{
				// AR4-b: a real source, everyone's, its table amount (HOLSourceAdd::AddNeutralSource; -1 on a client).
				if (HOLSourceAdd::AddNeutralSource(FName(Type), Place, -1) < 0)
				{
					Say("hol.Art.Try: sources are made on the server or in solo, once the match runs", true);
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
			// AR4-b: not in this frame (the Mass units asked for are not there yet): the selector selects one frame after the birth.
			const float Margin = 2.0 * ObjectRadius;
			AHOLArtTrySelector Selector = Cast<AHOLArtTrySelector>(SpawnActor(AHOLArtTrySelector::StaticClass(), Centre));
			if (Selector != nullptr)
			{
				Selector.Controller = Controller;
				Selector.Wanted = Count;
				Selector.Command = f"hol.Select.BoxGround {Math::RoundToInt(Low.X - Margin)} {Math::RoundToInt(Low.Y - Margin)} {Math::RoundToInt(High.X + Margin)} {Math::RoundToInt(High.Y + Margin)}";
			}
		}
		Say(f"hol.Art.Try: {Count} {Type} of slot {Slot} next to the Forum around ({Math::RoundToInt(Centre.X)}, {Math::RoundToInt(Centre.Y)}); fast build on, +1000 resources", false);
	}
}

// AR4-b: selects the copies of hol.Art.Try one frame after their birth (HOLArtTry::SelectionDue), then leaves.
class AHOLArtTrySelector : AActor
{
	AHOLPlayerController Controller;
	FString Command;
	int Wanted = 0;
	int Ticks = 0;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		// The first tick may fall in the frame of the birth: frames since the birth = ticks seen - 1.
		Ticks++;
		if (!HOLArtTry::SelectionDue(Ticks - 1))
			return;
		if (Controller == nullptr || Command.IsEmpty())
		{
			DestroyActor();
			return;
		}
		// CELEBORN-1125ar5: the copies reach the pick candidates a few simulation steps after their birth (the first box found none):
		// box again every 6 frames until the own selection holds every copy (HOLArtTry::SelectionDone), 240 frames at most.
		if ((Ticks - 2) % 6 != 0)
			return;
		System::ExecuteConsoleCommand(Command, Controller);
		TArray<int> SelectedIds;
		HOLControllerCommand::OwnSelectedUnitIds(Controller, SelectedIds);
		if (HOLArtTry::SelectionDone(SelectedIds.Num(), Wanted) || Ticks > 240)
		{
			Log(n"LogHOLMatch", f"hol.Art.Try: selected after {Ticks - 1} frame(s): {SelectedIds.Num()} of {Wanted} copies, {Command}");
			// CELEBORN-1125ar6: the selector then watches the copies for the first shot (art_try waits for its line, not a fixed time).
			AHOLArtTryShotWatch Watch = Cast<AHOLArtTryShotWatch>(SpawnActor(AHOLArtTryShotWatch::StaticClass(), GetActorLocation()));
			if (Watch != nullptr)
				Watch.CopyIds = SelectedIds;
			DestroyActor();
		}
	}
}

// AR4-b (CELEBORN-1125ar6): logs "hol.Art.Try: first shot" once, the first frame a ranged copy has an enemy within its attack range
// (HOLArtTry::FirstShotDue; the range is between the bodies' edges, the search radius adds two unit radii); leaves after it or 3600 frames.
class AHOLArtTryShotWatch : AActor
{
	TArray<int> CopyIds;
	int Ticks = 0;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		Ticks++;
		for (int Id : CopyIds)
		{
			const float Range = HOLUnitScript::GetAttackRange(Id);
			TArray<int> EnemyIds;
			TArray<FVector> EnemyPlaces;
			HOLUnitScript::GetEnemiesNear(Id, Range + 2.0 * HOLArtTry::UnitRadius(), EnemyIds, EnemyPlaces);
			if (HOLArtTry::FirstShotDue(Range, EnemyIds.Num()))
			{
				Log(n"LogHOLMatch", f"hol.Art.Try: first shot: copy #{Id} (range {Math::RoundToInt(Range)} cm) has {EnemyIds.Num()} enemy(ies) in range after {Ticks} frame(s)");
				DestroyActor();
				return;
			}
		}
		if (CopyIds.Num() == 0 || Ticks > 3600)
			DestroyActor();
	}
}

void HOLArtTryCommand(const TArray<FString>& Args) { HOLArtTry::Run(Args); }

const FConsoleCommand HOLArtTryConsole("hol.Art.Try", n"HOLArtTryCommand");
