# GALADRIEL-1118p : lot NG-2, regles pures de l'ecran Nouvelle partie en script

Joueur : aucun changement visible (regles seulement, pas encore branchees a l'ecran). Branche `galadriel-tab`, commit `8bb97d4`, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`. AUCUN test lance, AUCUN build, aucun fichier de `Source\` touche.
Fichiers : `Script\UI\HOLNewGameRules.as` (namespace `HOLNewGameRules`), `Script\UI\HOLNewGameRules_Test.as`. BUILDS_DEMANDES: 0. TESTS_CPP_RETIRES: 0. LIGNES_CPP_SUPPRIMEES: 0.
TESTS_SCRIPT_AJOUTES: 6 (Test_ColorTakenIgnoresOwnSlot, Test_FindFreeColorSkipsTaken, Test_FactionListHidesTemplarsWithoutCheat, Test_SizeFitsPlayers, Test_StateAIEnablesLevel, Test_LobbyButtonsAndRows). TYPES_A_EXPOSER: aucun.

Codes : etat de slot 0 Closed / 1 Human / 2 AI ; ligne de salon 0 Open / 1 Closed / 2 Player / 3 AI ; liste d'etats du salon 0 Open / 1 Closed / 2 Pas pret / 3 Pret / 4 IA ; taille 0 None, 1 Small .. 5 Huge (tableau `PlacesBySize` indexe par la taille : 0,2,4,6,8,8) ; faction 0 Dawn .. 5 Templars. `bCheat` et `PlacesBySize` sont des PARAMETRES (aucune liaison appelee).

## Table C++ -> script (toutes dans `HOLNewGameRules::`)

| C++ | script |
|---|---|
| IsColorTaken (cpp:1317) | `bool IsColorTaken(const TArray<int>& SlotStates, const TArray<int>& SlotColors, int SlotIndex, int ColorIndex)` |
| FindFreeColor (1334) | `int FindFreeColor(SlotStates, SlotColors, int SlotIndex, int From)` |
| ShowLobby 598-599 (clamp couleur, equipe) | `int ClampLobbyColor(int)`, `int ClampLobbyTeam(int, int MaxTeams)` |
| FHOLMapSizeRules::GetNumPlaces(Size / Config) | `int NumPlacesOfSize(const TArray<int>& PlacesBySize, int Size)`, `int NumPlacesOfConfig(PlacesBySize, int Size)` (None = 8) |
| IsAllowed, GetAllowedSizes, GetDefaultSize | `bool IsSizeAllowed(PlacesBySize, Size, NumPlayers)`, `TArray<int> AllowedSizes(PlacesBySize, NumPlayers)`, `int DefaultSize(PlacesBySize, NumPlayers)` |
| GetNumOccupiedSlots | `int NumOccupied(const TArray<int>& SlotStates)` |
| FitSlotsToSize (MapSizeRules.cpp:223) | `bool FitSlotsToSize(TArray<int>& SlotStates, int NumPlaces, TArray<int>& Moves)` (Moves[i] = destination du slot i, pour deplacer couleur / equipe / faction / nom) |
| ChooseSize (cpp:788) | `bool ChooseSize(TArray<int>& SlotStates, PlacesBySize, int Size, TArray<int>& Moves)` |
| HandleSizeChosen salon (965) | `bool IsKnownSize(PlacesBySize, int Size)` |
| HandleMapChosen 947, SetConfig 780 | `bool MapChoiceNeedsDefaultSize(bool bLobby, int CurrentSize)` |
| HandleStateChosen solo (1211-1226) | `bool ApplyStateChosen(TArray<int>& SlotStates, TArray<int>& SlotColors, int SlotIndex, int Value, int NumPlaces)` |
| HandleStateChosen salon (1191-1209) | `int LobbyStateAction(int Value, bool bHasClient)` : 1 IA, 2 ouvrir, 3 fermer, 0 rien |
| IsLobbyAI (1061) | `bool IsLobbyAI(bool bLobby, const TArray<int>& LobbyRowStates, int SlotIndex)` |
| ShowLobby 594, 636 | `int SlotStateOfLobbyRow(bool, int)`, `int LobbyListIndex(bool bRowExists, int RowState, bool bReady)` |
| Refresh 837, 855, 871, 884-909 | `LevelColumnShown(bLobby, SlotStates)`, `SlotStateListEnabled(SlotIndex, NumPlaces)`, `LevelCellShown(bLevelColumn, SlotState)`, `SlotFieldsActive(SlotState)` |
| HOLFactionChoice IsListed / IsChoosable / GetListed / Sanitize | `IsFactionListed`, `IsFactionChoosable`, `TArray<int> ListedFactions(bool bCheat)`, `SanitizeFaction(int, bool)` |
| UpdateFactionLists 1232, HandleFactionChosen 1261, Refresh 877 | `FactionListsNeedRebuild(bHasLast, bLast, bCheat)`, `CanTakeFaction`, `FactionNeedsShownText` |
| ShowLobby 679-692 (Pret / Spectateur) | `IsYouReady(const TArray<bool>&, int Yours)`, `ReadyButtonEnabled(Yours, bLobbyOpen)`, `ReadyButtonVisible(bSpectator)`, `ReadyButtonShowsUndo(bReady)`, `SpectateButtonEnabled(bLobbyOpen, bSpectator, bHasOpenSlot)`, `SpectateButtonShowsPlay(bSpectator)`, `LobbyLineEditable(SlotIndex, Yours, bCanEditAISlot)` |

## Non transpose (et pourquoi)
- `Lobby.CanToggleSlot`, `CanEditAISlot`, `IsYouSpectator`, `HasOpenSlot`, `FindYourSlot`, `IsYouLeader` : methodes de `FHOLLobby`, lues par l'ecran puis passees en booleens ; leur regle reste en C++ (hors lot).
- Les effets (`SendSlotAI`, `SetSlot`, `UpdateMe`, `Launch`, `SetConfig` qui appelle `Normalize`) : reseau / monde, restent C++.
- `FitSlotsToSize` ne deplace que les etats ; l'ecran applique `Moves` aux autres champs (le C++ copie le slot entier, nom vide a la fermeture).
- Alpha de la pastille fermee (`ClosedSwatchOpacity`) : valeur d'habillage, laissee a l'ecran.

## Mutations pour voir rouge (une a la fois, dans `Script\UI\HOLNewGameRules.as`, recompiler le script)
1. Test_ColorTakenIgnoresOwnSlot : dans `IsColorTaken`, `Other != SlotIndex` devient `true` (ligne `if (Other != SlotIndex && ...`) : le 1er Assert (couleur 3 de son propre slot) passe a vrai.
2. Test_FindFreeColorSkipsTaken : dans `FindFreeColor`, `(From + Tried) % NumColors` devient `(From + Tried + 1) % NumColors` : le premier Assert rend 5 au lieu de 4.
3. Test_FactionListHidesTemplarsWithoutCheat : dans `IsFactionListed`, `|| bCheat` devient `|| !bCheat` : `ListedFactions(false).Num()` rend 1 de plus (6 au lieu de 5).
4. Test_SizeFitsPlayers : dans `IsSizeAllowed`, `>= NumPlayers` devient `> NumPlayers` : `DefaultSize(Places, 6)` rend 4 au lieu de 3.
5. Test_StateAIEnablesLevel : dans `ApplyStateChosen`, `Value != 0 &&` devient `Value == 0 &&` : `Colors[2]` reste 0 au lieu de 2. (Variante : `LevelCellShown`, `SlotState == 2` devient `== 1`.)
6. Test_LobbyButtonsAndRows : dans `SpectateButtonEnabled`, `(!bSpectator || bHasOpenSlot)` devient `(bSpectator || bHasOpenSlot)` : "un joueur peut toujours quitter" passe a faux.

## Suspects de compilation (non lances)
- `TArray<bool>` en `const TArray<bool>&` (IsYouReady) : devrait passer ; sinon passer des `int`.
- `const int` de namespace (NumColors...) : meme patron que `HOLAlertRules::PopulationRepeatSeconds`.
- `Math::Clamp(int,int,int)` / `Math::Min(int,int)` : surcharges entieres supposees presentes.
- `TArray<int>& Moves` en sortie, `Moves.Empty()` : supposes ; sinon `Reset()`.
- Un nom de test ou d'aide en double ferait tomber la compilation : verifie par grep, `NGR_` et les 6 `Test_` n'existent que dans ce fichier.
- Commande a lancer : la voie script habituelle de la session principale (quick_check -Parts tests / script) ; aucun build.

## Avancement
Reste : brancher ces regles dans l'ecran script (lots suivants NG-3...), l'autre agent exposant en C++ les lectures de table (places, triche). Charges : ~24 appels d'outils vus, fin 20:3x (arret dur 20:51 non atteint). IN_USE.txt du worktree : libre. PROCESSUS_DETACHES: 0 (aucun lance).
