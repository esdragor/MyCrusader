# ELROND-1118u : lot NG-5 (mode salon de l'ecran Nouvelle partie) - INCOMPLET, ARRET A LA LIMITE D'APPELS

**NON COMPILE, NON LANCE.** Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers. Aucun C++, aucun build, aucun quick_check. Pas de push, pas de fusion.
J'ai depasse la limite de 28 appels (7 `Edit` refuses faute de `Read` prealable, perdus) : le lot est livre a MOINS DE LA MOITIE. Un agent suivant reprend a partir de ce rapport.
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TESTS_TAUTOLOGIQUES: 0 ; TEST_ROUGE: non fait.

## Fait (fichier `Script\UI\HOLNewGameScreen.as` seulement)
- Membres `LobbyHeading`, `LobbySpectators`, `LobbySlotNames` (8 textes), `LobbyYouText`, `LobbyReadyButton`, `LobbySpectateButton`, `LobbyLaunchButton` : le script garde ce que `AddNewGameText / AddNewGameCellText / AddNewGameButton` renvoient (les membres C++ de la base sont proteges).
- `BuildNewGame` les remplit (modifs d'une ligne chacune, memes appels qu'avant). En-tete du fichier mis a jour. Aucun geste ni `ShowLobby` ecrits.

## Point 3 : MANQUE DE LA BASE (A AJOUTER A LA BASE C++)
`HOLNewGameScreen.h/.cpp` n'a AUCUN evenement pour le script au rafraichissement du salon : `ReadLobby` (cpp:593) et `HandleWritten` (cpp:1208) et `EnterLobby` (cpp:553) appellent `ShowLobby(const FHOLLobby&)` C++ direct.
A ajouter : `UFUNCTION(BlueprintImplementableEvent, Category = "HOL|NewGame") void BP_ShowLobby();` et, dans `ShowLobby`, juste apres `Refresh();` (cpp:646) :
`if (GetClass() != StaticClass()) { BP_ShowLobby(); } else { ...cpp:648-739 inchange... }` ; la suite (cpp:740 : etat Starting, `JoinMatch`, journal) RESTE C++. `Lobby` est deja a jour a cet appel donc `GetLobbyView()` fonctionne. Tant que ce n'est pas fait, le `ShowLobby` C++ (qui trouve les widgets du script par nom) continue d'afficher le salon : rien ne casse.
Autres manques constates : (a) `UHOLButton::SetLabel` n'est pas UFUNCTION : utiliser `UHOLGameMenuScreen::SetButtonLabel(Bouton, FText)` (deja employe dans HOLEndBannerScreen.as). (b) `HOLForumMessages::GetAIName(Slot, Level)` (nom « IA Difficile 3 ») non expose : prevoir `static FText GetNewGameAIName(int Slot, int Level)` sur l'ecran ; sans lui, repli sur `GetNewGameOptions(n"LobbyState")[HOLNewGameRules::LobbyAIState]` ("IA"). (c) textes Leader (« ♛ {0} ») et Spectators (« Spectateurs : {0} ») non couverts par `GetNewGameText` : `// MANQUE:` a poser, concatenation en script en attendant. (d) pas de `HasClient()` : passer `IsLobby()` comme `bHasClient` de `LobbyStateAction` (les envois de la base ne font rien sans client).

## Table fonction C++ -> methode script (A ECRIRE ; rien n'est fait sauf la premiere ligne)
| C++ | Script prevu | Regle pure / base |
|---|---|---|
| widgets retrouves | membres `Lobby*` (FAIT) | `AddNewGame*` |
| ShowLobby 648-739 | `void ShowLobby()` (methode simple ; quand `BP_ShowLobby` existe : `UFUNCTION(BlueprintOverride)` au-dessus) | `GetLobbyView`, `LobbyListIndex(true, Row.State, Row.bReady)`, `LobbyLineEditable(i, View.YourSlot, Row.bCanEditAI)`, `IsYouReady`, `ReadyButtonEnabled/Visible/ShowsUndo`, `SpectateButtonEnabled/ShowsPlay` |
| HandleMapChosen lobby | `OnMapChosen` : `SetConfigMapIndex` ; pas de taille par defaut (`MapChoiceNeedsDefaultSize(true, ..)` faux) ; `Refresh` ; `SendSettings` | |
| HandleSizeChosen lobby | `OnSizeChosen` : `SetConfigMapSize(Index)`, `SendSettings`, `Refresh` (ordre du C++) | |
| TeamsTogether / Fog / Mode lobby | `SetConfigGlobals(...)`, `SendSettings`, `Refresh` | |
| HandleOptionChosen lobby | lire Nomad / Regicide / Catastrophe / SacredSites par `GetSelectedIndex() == 1`, `SetConfigOptions`, `SendSettings`, pas de Refresh | |
| HandleStateChosen lobby | `LobbyStateAction(Index, IsLobby())` : 1 -> `SendSlotAIChoice(i,-1,-1,"")` ; 2 -> `SendSlotOpen(i,true)` ; 3 -> `SendSlotOpen(i,false)` ; rien d'autre, pas de Refresh | |
| HandleFactionChosen lobby | si `!IsFactionChoosable` : Refresh ; si `!LobbyLineEditable` : Refresh ; `SetConfigSlot`, `Refresh`, puis `IsLobbyAI(i)` -> `SendSlotAIChoice(i,-1,-1,Nom)` sinon `SendMyChoice(-1,-1,-1,-1,Nom)` | `HOLFactionChoiceRead::GetName(EHOLFaction)` (prend l'ENUM : `EHOLFaction(Index)`) |
| HandleColorChosen lobby | `SetConfigSlot`, `Refresh`, `IsLobbyAI` -> `SendSlotAIChoice(i,c,-1,"")` sinon lobby `SendMyChoice(c,-1,-1,-1,"")` | |
| HandleTeamChosen lobby | `SetConfigSlot`, pas de Refresh, `IsLobbyAI` -> `SendSlotAIChoice(i,-1,t,"")` sinon `SendMyChoice(-1,t,-1,-1,"")` | |
| ToggleReady / ToggleSpectator | PAS de geste script : les boutons restent cables sur la base (un appel script ferait un double jeu) | |
| Launch (visible chef, actif si `bCanStart` et `State == 0`) | dans `ShowLobby` | |
Les gestes actuels de l'ecran (NG-4) commencent par `if (IsLobby()) return;` : c'est a remplacer par les branches ci-dessus (`const bool bInLobby = IsLobby();`).

## Tests
Aucun ecrit. Prevus (tous sans reseau, ils passent par `Choose` de `UHOLDropdown`, UFUNCTION qui diffuse `OnChosen`) : `Test_NewGameSoloFogGesture` (creer l'ecran, `Cast<UHOLDropdown>(NGS_Find(Screen, n"FogDropdown")).Choose(2)`, lire `GetConfigJson` + `HOLNewGameRead::DescribeGlobals` : FogMode == 0, et `HOLScreenTest::GetDropdownIndex` == 2 ; mutation : `2 - Index` -> `Index` dans `OnFogChosen`), `Test_NewGameSoloModeGesture` (`Choose(1)` sur ModeDropdown -> mode 1 ; mutation `Index == 1 ? 1 : 0` -> `0`), `Test_NewGameSoloTeamsTogetherGesture` (`Choose(0)` -> `bTeamsTogether` faux ; mutation `Index == 1` -> `true`). Le salon ne se teste PAS sans reseau (pas de `FHOLLobby` constructible depuis le script) : a voir a deux instances.

## Suspects de compilation (de ce qui est fait)
1. `LobbySlotNames.Add(AddNewGameCellText(...))` : `TArray<UCommonTextBlock>` d'objets, meme patron que `SlotStateLists.Add(...)`. 2. `UCommonTextBlock` comme type de membre : deja employe en local dans le fichier. Rien d'autre.

## A regarder a deux instances (quand le reste sera ecrit)
Chef : les 8 listes de reglages actives, Lancer visible, actif seulement quand tous prets ; invite : reglages grises, lignes des autres grisees, la sienne (et les IA du chef) editables ; Pret -> libelle « Plus pret » ; Spectateur -> libelle « Jouer », Pret masque, texte « Spectateurs : ... » ; changer couleur / equipe / faction cote invite arrive chez le chef au sondage suivant ; le chef ferme / ouvre / met une IA sur une place.
