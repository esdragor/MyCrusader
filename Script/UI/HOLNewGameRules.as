// The New game screen's pure rules (lot NG-2, D851, PLAN-ECRANS-1118.md): numbers and arrays in, numbers / booleans / arrays out, no widget.
// Each function names its C++ origin (HOLNewGameScreen.cpp unless said). What lives in a table or a subsystem (the places of a size, the Templars'
// cheat) comes in as a parameter: the screen reads it and passes it.
//
// Codes used by the arguments (the C++ enums' values):
//   slot state (EHOLSlotState): 0 Closed, 1 Human, 2 AI.
//   lobby row state (EHOLLobbySlotState): 0 Open, 1 Closed, 2 Player, 3 AI.
//   lobby state list (LobbyStateNames): 0 Open, 1 Closed, 2 Not ready, 3 Ready, 4 AI.
//   map size (EHOLMapSize): 0 None, 1 Small, 2 Medium, 3 Normal, 4 Large, 5 Huge; the "places by size" array is indexed by it
//   (entry 0, None, is 0), for example 0, 2, 4, 6, 8, 8 (HOLMapSizeRules.cpp, Tools/Data/MapSizes.csv).
//   faction (EHOLFaction): 0 DawnOrder, 1 BlackLegions, 2 DragonChildren, 3 ShadowCircle, 4 FireHeirs, 5 Templars.

namespace HOLNewGameRules
{
	// HOLPlayerColors::NumColors, RandomColor ("Aleatoire" is the index after the last colour).
	const int NumColors = 8;
	const int RandomColor = 8;
	// FHOLMatchConfig::MaxSlots.
	const int MaxSlots = 8;
	// UE::HeroesOfLiberty::UI::NewGame::LobbyAIState: the "IA" entry of the lobby's state list.
	const int LobbyAIState = 4;
	// EHOLFaction::Templars.
	const int FactionTemplars = 5;

	// ---- Colours (cpp:1317-1346) ----

	// HOLNewGameScreen::IsColorTaken: a colour held by ANOTHER slot that is not closed; "Aleatoire" is never taken (E2-2); the slot itself is ignored.
	bool IsColorTaken(const TArray<int>& SlotStates, const TArray<int>& SlotColors, int SlotIndex, int ColorIndex)
	{
		if (ColorIndex == RandomColor)
		{
			return false;
		}
		for (int Other = 0; Other < SlotStates.Num() && Other < SlotColors.Num(); ++Other)
		{
			if (Other != SlotIndex && SlotStates[Other] != 0 && SlotColors[Other] == ColorIndex)
			{
				return true;
			}
		}
		return false;
	}

	// HOLNewGameScreen::FindFreeColor: the first free colour from From, wrapping on NumColors; From itself when every colour is taken.
	int FindFreeColor(const TArray<int>& SlotStates, const TArray<int>& SlotColors, int SlotIndex, int From)
	{
		for (int Tried = 0; Tried < NumColors; ++Tried)
		{
			int Color = (From + Tried) % NumColors;
			if (!IsColorTaken(SlotStates, SlotColors, SlotIndex, Color))
			{
				return Color;
			}
		}
		return From;
	}

	// HOLNewGameScreen::ShowLobby (cpp:598): the colour read from the manager, kept in 0..RandomColor.
	int ClampLobbyColor(int Color)
	{
		return Math::Clamp(Color, 0, RandomColor);
	}

	// HOLNewGameScreen::ShowLobby (cpp:599): the team read from the manager, kept in 0..MaxTeams (FHOLMatchConfig::MaxTeams, a parameter).
	int ClampLobbyTeam(int Team, int MaxTeams)
	{
		return Math::Clamp(Team, 0, MaxTeams);
	}

	// ---- Sizes and places (HOLMapSizeRules.cpp, cpp:788, 958) ----

	// FHOLMapSizeRules::GetNumPlaces(Size): the places of a size, 0 for None or a size the table does not have.
	int NumPlacesOfSize(const TArray<int>& PlacesBySize, int Size)
	{
		if (Size <= 0 || Size >= PlacesBySize.Num())
		{
			return 0;
		}
		return PlacesBySize[Size];
	}

	// FHOLMapSizeRules::GetNumPlaces(Config): None (no size chosen) leaves all the slots (MaxSlots).
	int NumPlacesOfConfig(const TArray<int>& PlacesBySize, int Size)
	{
		return Size == 0 ? MaxSlots : NumPlacesOfSize(PlacesBySize, Size);
	}

	// FHOLMapSizeRules::IsAllowed: a real size that holds the players (a size too small is greyed, "-" never chosen).
	bool IsSizeAllowed(const TArray<int>& PlacesBySize, int Size, int NumPlayers)
	{
		return Size != 0 && NumPlacesOfSize(PlacesBySize, Size) >= NumPlayers;
	}

	// FHOLMapSizeRules::GetAllowedSizes: the sizes that can hold NumPlayers, smallest first (the table's order).
	TArray<int> AllowedSizes(const TArray<int>& PlacesBySize, int NumPlayers)
	{
		TArray<int> Allowed;
		for (int Size = 1; Size < PlacesBySize.Num(); ++Size)
		{
			if (IsSizeAllowed(PlacesBySize, Size, NumPlayers))
			{
				Allowed.Add(Size);
			}
		}
		return Allowed;
	}

	// FHOLMapSizeRules::GetDefaultSize: the smallest size that holds the players, 0 (None) when none does.
	int DefaultSize(const TArray<int>& PlacesBySize, int NumPlayers)
	{
		TArray<int> Allowed = AllowedSizes(PlacesBySize, NumPlayers);
		return Allowed.Num() == 0 ? 0 : Allowed[0];
	}

	// FHOLMatchConfig::GetNumOccupiedSlots: the slots that are not closed.
	int NumOccupied(const TArray<int>& SlotStates)
	{
		int Count = 0;
		for (int Index = 0; Index < SlotStates.Num(); ++Index)
		{
			if (SlotStates[Index] != 0)
			{
				++Count;
			}
		}
		return Count;
	}

	// FHOLMapSizeRules::FitSlotsToSize on the states only: false (nothing changed) when the players do not fit in NumPlaces; else each occupied slot past
	// the places moves onto the first closed slot below, in order, and the slot it leaves is closed. NumPlaces is already resolved (NumPlacesOfConfig).
	// Outputs Moves: for each slot, where its content goes (itself when it stays), so the screen moves the other fields (colour, team, faction, name).
	bool FitSlotsToSize(TArray<int>& SlotStates, int NumPlacesIn, TArray<int>& Moves)
	{
		Moves.Empty();
		int NumPlaces = Math::Min(NumPlacesIn, SlotStates.Num());
		for (int Index = 0; Index < SlotStates.Num(); ++Index)
		{
			Moves.Add(Index);
		}
		if (NumOccupied(SlotStates) > NumPlaces)
		{
			return false;
		}
		int Free = 0;
		for (int Index = NumPlaces; Index < SlotStates.Num(); ++Index)
		{
			if (SlotStates[Index] == 0)
			{
				continue;
			}
			while (SlotStates[Free] != 0)
			{
				++Free;
			}
			SlotStates[Free] = SlotStates[Index];
			SlotStates[Index] = 0;
			Moves[Index] = Free;
		}
		return true;
	}

	// HOLNewGameScreen::ChooseSize (the rule): false when Size is None or the players do not fit (states untouched); true when the slots were fitted
	// (the screen then sets MapSize, bDrawPlaces = true).
	bool ChooseSize(TArray<int>& SlotStates, const TArray<int>& PlacesBySize, int Size, TArray<int>& Moves)
	{
		if (Size == 0)
		{
			return false;
		}
		return FitSlotsToSize(SlotStates, NumPlacesOfConfig(PlacesBySize, Size), Moves);
	}

	// HOLNewGameScreen::HandleSizeChosen, lobby branch: the leader's size is sent only when the table has it (FHOLMapSizeRules::Find non null).
	bool IsKnownSize(const TArray<int>& PlacesBySize, int Size)
	{
		return Size > 0 && Size < PlacesBySize.Num();
	}

	// HOLNewGameScreen::HandleMapChosen / SetConfig: solo, a configuration with no size takes the default one for the players
	// (in a lobby the manager's size is kept).
	bool MapChoiceNeedsDefaultSize(bool bLobby, int CurrentSize)
	{
		return !bLobby && CurrentSize == 0;
	}

	// ---- Slot state (cpp:1189-1227, 836-866) ----

	// HOLNewGameScreen::HandleStateChosen, solo branch: false (nothing changes, the list is greyed) for a slot past the places of the size; else the
	// slot takes the state, and a slot that is no longer closed on a colour another one holds takes the first free one.
	bool ApplyStateChosen(TArray<int>& SlotStates, TArray<int>& SlotColors, int SlotIndex, int Value, int NumPlaces)
	{
		if (SlotIndex >= NumPlaces)
		{
			return false;
		}
		SlotStates[SlotIndex] = Value;
		if (Value != 0 && IsColorTaken(SlotStates, SlotColors, SlotIndex, SlotColors[SlotIndex]))
		{
			SlotColors[SlotIndex] = FindFreeColor(SlotStates, SlotColors, SlotIndex, SlotColors[SlotIndex]);
		}
		return true;
	}

	// HOLNewGameScreen::HandleStateChosen, lobby branch: what is sent. 1 = the AI (SetSlotAI), 2 = the slot open, 3 = the slot closed, 0 = nothing
	// (values 2 and 3 are filled by the manager; no client, nothing sent). The AI is tested before the client, as in the C++.
	int LobbyStateAction(int Value, bool bHasClient)
	{
		if (Value == LobbyAIState)
		{
			return 1;
		}
		if (bHasClient && Value <= 1)
		{
			return Value == 0 ? 2 : 3;
		}
		return 0;
	}

	// HOLNewGameScreen::IsLobbyAI: a lobby slot the leader has given to an AI (lobby only, the index valid, row state AI).
	bool IsLobbyAI(bool bLobby, const TArray<int>& LobbyRowStates, int SlotIndex)
	{
		return bLobby && SlotIndex >= 0 && SlotIndex < LobbyRowStates.Num() && LobbyRowStates[SlotIndex] == 3;
	}

	// HOLNewGameScreen::ShowLobby (cpp:594): the match slot state of a lobby row (Player -> Human, AI -> AI, the rest Closed); no row = Closed.
	int SlotStateOfLobbyRow(bool bRowExists, int RowState)
	{
		if (!bRowExists)
		{
			return 0;
		}
		return RowState == 2 ? 1 : RowState == 3 ? 2 : 0;
	}

	// HOLNewGameScreen::ShowLobby (cpp:636): the entry of the lobby's state list (0 Open, 1 Closed, 2 Not ready, 3 Ready, 4 AI).
	int LobbyListIndex(bool bRowExists, int RowState, bool bReady)
	{
		if (!bRowExists || RowState == 0)
		{
			return 0;
		}
		if (RowState == 3)
		{
			return LobbyAIState;
		}
		if (RowState != 2)
		{
			return 1;
		}
		return bReady ? 3 : 2;
	}

	// HOLNewGameScreen::Refresh (cpp:855): in solo the state list of a slot is changeable only inside the places of the size.
	bool SlotStateListEnabled(int SlotIndex, int NumPlaces)
	{
		return SlotIndex < NumPlaces;
	}

	// HOLNewGameScreen::Refresh (cpp:837): the level column shows in solo, and in a lobby only with an AI in it.
	bool LevelColumnShown(bool bLobby, const TArray<int>& SlotStates)
	{
		if (!bLobby)
		{
			return true;
		}
		for (int Index = 0; Index < SlotStates.Num(); ++Index)
		{
			if (SlotStates[Index] == 2)
			{
				return true;
			}
		}
		return false;
	}

	// HOLNewGameScreen::Refresh (cpp:871): a slot's level list shows only for an AI slot, when the column shows.
	bool LevelCellShown(bool bLevelColumn, int SlotState)
	{
		return bLevelColumn && SlotState == 2;
	}

	// HOLNewGameScreen::Refresh (cpp:884, 890, 909): the faction, colour and team fields are changeable / visible only for an occupied slot (solo);
	// the colour swatch's alpha too (ClosedSwatchOpacity otherwise).
	bool SlotFieldsActive(int SlotState)
	{
		return SlotState != 0;
	}

	// ---- Factions (cpp:1229-1256, HOLFactionChoice.cpp) ----

	// HOLFactionChoice::IsListed: every faction but the Templars, which the cheat lists.
	bool IsFactionListed(int Faction, bool bCheat)
	{
		return Faction != FactionTemplars || bCheat;
	}

	// HOLFactionChoice::IsChoosable: the Dawn and the Black Legions always, the Templars with the cheat, the three others never (listed, greyed).
	bool IsFactionChoosable(int Faction, bool bCheat)
	{
		if (Faction == 0 || Faction == 1)
		{
			return true;
		}
		if (Faction == FactionTemplars)
		{
			return bCheat;
		}
		return false;
	}

	// HOLFactionChoice::GetListed (UpdateFactionLists): the factions of the list, in the enum's order, the Templars only with the cheat.
	TArray<int> ListedFactions(bool bCheat)
	{
		TArray<int> Listed;
		for (int Faction = 0; Faction <= FactionTemplars; ++Faction)
		{
			if (IsFactionListed(Faction, bCheat))
			{
				Listed.Add(Faction);
			}
		}
		return Listed;
	}

	// HOLFactionChoice::Sanitize (Launch): a Templars slot without the cheat is played as the Dawn.
	int SanitizeFaction(int Faction, bool bCheat)
	{
		return Faction == FactionTemplars && !bCheat ? 0 : Faction;
	}

	// HOLNewGameScreen::UpdateFactionLists (cpp:1232): the lists are rebuilt only the first time or when the cheat changed.
	bool FactionListsNeedRebuild(bool bHasLastCheat, bool bLastCheat, bool bCheat)
	{
		return !(bHasLastCheat && bLastCheat == bCheat);
	}

	// HOLNewGameScreen::HandleFactionChosen (cpp:1261): a faction the list greys is never taken from a stale list.
	bool CanTakeFaction(int Faction, bool bCheat)
	{
		return IsFactionChoosable(Faction, bCheat);
	}

	// HOLNewGameScreen::Refresh (cpp:877): a line on a faction the list does not hold shows its name all the same.
	bool FactionNeedsShownText(int Faction, bool bCheat)
	{
		return !IsFactionListed(Faction, bCheat);
	}

	// ---- Lobby buttons (cpp:679-692) ----

	// ShowLobby: your row is ready (Yours = your slot, -1 without one; ReadyRows = the lobby rows' ready flags).
	bool IsYouReady(const TArray<bool>& ReadyRows, int Yours)
	{
		return Yours >= 0 && Yours < ReadyRows.Num() && ReadyRows[Yours];
	}

	// ShowLobby: the Ready button is changeable when you hold a slot and the lobby has not started (State == Lobby).
	bool ReadyButtonEnabled(int Yours, bool bLobbyOpen)
	{
		return Yours != -1 && bLobbyOpen;
	}

	// ShowLobby (SP1, D642): a spectator has no Ready button.
	bool ReadyButtonVisible(bool bSpectator)
	{
		return !bSpectator;
	}

	// ShowLobby: the Ready button's label is "Plus pret" (true) when you are ready, "Pret" (false) otherwise.
	bool ReadyButtonShowsUndo(bool bReady)
	{
		return bReady;
	}

	// ShowLobby: the Spectator button is changeable while the lobby has not started, and a spectator needs an open slot to take a place back.
	bool SpectateButtonEnabled(bool bLobbyOpen, bool bSpectator, bool bHasOpenSlot)
	{
		return bLobbyOpen && (!bSpectator || bHasOpenSlot);
	}

	// ShowLobby: the Spectator button's label is "Jouer" (true) for a spectator, "Spectateur" (false) for a player.
	bool SpectateButtonShowsPlay(bool bSpectator)
	{
		return bSpectator;
	}

	// ShowLobby: your line (and an AI's line the leader holds) is the only changeable one for faction, colour and team.
	bool LobbyLineEditable(int SlotIndex, int Yours, bool bCanEditAISlot)
	{
		return SlotIndex == Yours || bCanEditAISlot;
	}
}
