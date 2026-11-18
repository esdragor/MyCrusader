# ELROND-1118v : lot NG-5 (mode salon de l'écran Nouvelle partie, script) - FINI, NON COMPILÉ, NON LANCÉ

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers, commit `140c344` (parent 708a2ba). 2 fichiers : `Script\UI\HOLNewGameScreen.as` (+~250 lignes, 572 au total) et `Script\UI\HOLNewGameScreen_Test.as` (+81). Aucun C++, aucun build, aucun quick_check, aucun lancement. Pas de push, pas de fusion. EtapeDefini.md, Auto_Journey_File.md, *InProgress.md non touchés.
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_NewGameSoloStateGivesTheLineItsFields, Test_NewGameSoloTakenColourGivesWay, Test_NewGameSoloGlobalsGesture) ; TESTS_CPP_RETIRES: 0 ; TEST_ROUGE: non fait (mutations ci-dessous, à poser par la session principale).
Commande : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -LiveCoding=0 -Wait` n'est PAS nécessaire (pas de C++) ; compiler les scripts (lancer l'éditeur / quick_check `-Parts tests`).

## Fait
- `default bScriptOwnsLobbyDisplay = true;` + `UFUNCTION(BlueprintOverride) void ShowLobby()` (surcharge de `BP_ShowLobby`) + `ShowLobbyLine(View, SlotIndex, LobbyStates)`.
- Branches salon de tous les gestes (carte, taille, équipes ensemble, brouillard, mode, options, état, faction, couleur, équipe) : plus de `if (IsLobby()) return;`, envois par SendSettings / SendSlotAIChoice / SendSlotOpen / SendMyChoice. Garde d'envoi : `HasClient()` (pas `IsLobby()`) pour `LobbyStateAction`.
- Ready et Spectateur : aucun abonnement script (la base les câble) ; le script n'en fait que l'affichage.
- Aides ajoutées : `ListIsOn(UHOLDropdown)`.

## Table « ShowLobbyWidgets C++ -> script »
| C++ (cpp:704-800) | Script |
|---|---|
| `HeadingText->SetText(Lobby.Name)` | `LobbyHeading.SetText(FText::FromString(View.Name))` |
| nom de ligne (IA vide / Joueur n / ♛ chef / nom) | `LobbySlotNames[i].SetText(GetNewGameLobbyLineName(i))` |
| `State` 0 ouvert-fermé / IA / 1 / 3 prêt / 2 | `HOLNewGameRules::LobbyListIndex(bRowExists, RowState, bRowReady)` (RowState = `View.Slots[i].State`, 3 = IA) |
| `AIName = GetAIName(i, Level)` | `GetNewGameLobbyAIName(i)` si IA |
| `SlotStates[i]` : SetSelectedIndex / SetIsEnabled(CanToggleSlot) / SetOptionEnabled(2,false),(3,false) / SetShownText(IA) | idem sur `SlotStateLists[i]`, `CanToggleLobbySlot(i)` |
| ligne 0 `SlotYou->SetText(bAI ? AIName : LobbyStateNames()[State])` | `LobbyYouText.SetText(bAI ? AIName : LobbyStates[State])`, `LobbyStates = GetNewGameOptions(n"LobbyState")` |
| `bEditable = bMine \|\| CanEditAISlot(i)` sur faction / couleur / équipe | `HOLNewGameRules::LobbyLineEditable(i, View.YourSlot, CanEditLobbyAISlot(i))` sur `SlotFactionLists/SlotColorLists/SlotTeamLists` |
| 8 réglages du chef `SetIsEnabled(IsYouLeader())` (Carte, Taille, Équipes ensemble, Brouillard, Mode, Nomade, Régicide, Catastrophe ; PAS Sites sacrés) | idem, `IsLobbyYouLeader()`, mêmes 8 listes |
| Prêt : libellé / actif / Collapsed si spectateur | `SetNewGameButtonLabel(n"ReadyButton", GetNewGameText(ReadyButtonShowsUndo(bReady) ? NotReadyAction : ReadyAction))`, `ReadyButtonEnabled(YourSlot, bLobbyOpen)`, `ReadyButtonVisible(bSpectator)` ; `bReady = IsYouReady(ReadyRows, YourSlot)` |
| Spectateur : libellé / actif | `SetNewGameButtonLabel(n"SpectateButton", ... BecomePlayer/BecomeSpectator)`, `SpectateButtonEnabled(bLobbyOpen, bSpectator, LobbyHasOpenSlot())` |
| texte des spectateurs + Collapsed / HitTestInvisible | `GetNewGameSpectatorsText(Spectators, View.Leader)`, visibilité selon `Spectators.Num()` |
| Lancer : visible chef, actif `CanStart() && State == Lobby` | `SetVisibility`, `SetIsEnabled(CanStartLobby() && View.State == 0)` |

Branches salon des gestes (ordre du C++ conservé) : carte = SetConfigMapIndex, Refresh, SendSettings (pas de taille par défaut) ; taille = si `1 <= Index < PlacesBySize().Num()` SetConfigMapSize + SendSettings, puis Refresh ; équipes ensemble / brouillard / mode = SetConfigGlobals, SendSettings, Refresh ; options = SetConfigOptions(4 listes), SendSettings, sans Refresh ; état = `LobbyStateAction(Index, HasClient())` -> 1 SendSlotAIChoice(i,-1,-1,""), 2 SendSlotOpen(i,true), 3 SendSlotOpen(i,false), sans Refresh ; faction = non listée -> Refresh ; ligne non éditable -> Refresh ; SetConfigSlot, Refresh, `IsLobbyAI` ? SendSlotAIChoice(i,-1,-1,Nom) : SendMyChoice(-1,-1,-1,-1,Nom) avec `Nom = GetNewGameFactionName(Index)` ; couleur = SetConfigSlot, Refresh, AI ? SendSlotAIChoice(i,c,-1,"") : SendMyChoice(c,-1,-1,-1,"") ; équipe = SetConfigSlot, sans Refresh, AI ? SendSlotAIChoice(i,-1,t,"") : SendMyChoice(-1,t,-1,-1,"").

## Manques de la base
Aucun bloquant : tout ce que le prédécesseur listait (GetNewGameAIName, textes Leader / Spectators, HasClient, BP_ShowLobby, SetNewGameButtonLabel) existe depuis ARWEN-1118s. Écarts volontaires à signaler :
- Le C++ n'a AUCUNE garde `CanToggleSlot` dans HandleStateChosen (la liste est grisée) : le script non plus (fidélité).
- Les listes de niveau (`SlotLevel<n>`) ne sont pas touchées par ShowLobby (C++ non plus : c'est `Refresh`).
- Pas de `// MANQUE:` posé dans le .as.

## Mutations (une à la fois, à remettre ensuite ; test attendu ROUGE)
1. `Test_NewGameSoloStateGivesTheLineItsFields` : dans `OnSlotStateChosen` (partie solo), supprimer le `Refresh();` final -> les listes faction / couleur gardent leur état grisé / actif d'avant -> les asserts « active » (ou « grisée » selon l'ordre) échouent.
2. `Test_NewGameSoloTakenColourGivesWay` : dans `OnSlotStateChosen`, `SetConfigSlot(SlotIndex, CfgStates[SlotIndex], -1, CfgColors[SlotIndex], -1)` -> `..., -1, -1, -1)` : la couleur n'est pas réécrite, reste celle du slot 0 -> « a colour another slot holds gives way » rouge.
3. `Test_NewGameSoloGlobalsGesture` : (a) dans `OnFogChosen`, `int Fog = 2 - Index;` -> `int Fog = Index;` -> FogMode == 2 au lieu de 0 ; (b) dans `OnTeamsTogetherChosen`, `Index == 1` -> `true` -> bTeamsTogether reste vrai.
Le salon NE SE TESTE PAS sans réseau : pas de manager, pas de `FHOLLobby` constructible depuis le script ; seul l'essai à deux instances le couvre.

## Suspects de compilation (du plus au moins probable)
1. `UFUNCTION(BlueprintOverride) void ShowLobby()` : le C++ a un `ShowLobby(const FHOLLobby&)` privé non-UFUNCTION de même nom (ARWEN suspect 7). Si le script refuse, renommer côté script n'est pas possible (BlueprintOverride lie à `BP_ShowLobby` par le nom `ShowLobby`) : alors soit retirer le préfixe de l'événement C++ (nommer l'événement `ShowLobbyScript`), soit renommer le C++ privé.
2. `const FHOLNewGameLobbyView& View` en paramètre de `ShowLobbyLine` et `View.Slots[i].State` (indexation d'un TArray de structs dans un struct const) : si refusé, copier la ligne dans une variable locale `FHOLNewGameLobbySlotView`.
3. `const TArray<FString> Spectators = View.Spectators;` passé à `GetNewGameSpectatorsText(Spectators, View.Leader)` (statique, `const TArray<FString>&`).
4. `FText AIName;` puis `bAI ? AIName : LobbyStates[State]` (ternaire de deux FText ; si refusé : if / else).
5. `SendSlotAIChoice(SlotIndex, -1, -1, "")` / `SendMyChoice(..., "")` : littéral `""` vers `const FString&` (attendu OK).
6. `TArray<UHOLDropdown> Settings; Settings.Add(MapList)` avec membres éventuellement nuls (Add d'un nullptr, test `!= nullptr` ensuite).
7. Tests : `UHOLDropdown.Choose(int)` appelé depuis le script ; `Screen.SetConfigSlot(...)` / `Screen.Refresh()` sur `UHOLNewGameScreen` ; `HOLNewGameRead::DescribeGlobals` (6 sorties, même appel que `ReadConfig`) ; `T.AssertTrue(!HOLScreenTest::IsWidgetEnabled(...))`.
8. Comportement : si `Choose` refuse une option grisée ou ne diffuse pas quand la valeur est inchangée, les tests 1 et 2 seraient rouges à tort (ils partent de `Choose(0)` puis `Choose(2)` / `Choose(1)`, donc de valeurs changeantes ; le slot 1 doit être dans les places de la taille par défaut). Si la config de test a par défaut un slot 0 non occupé, le test 2 est vide : il vérifie `HeldColour < 8` avant.

## À regarder à deux instances (salon)
Chef : 8 listes de réglages actives, Sites sacrés inchangé (actif selon la carte), Lancer visible, actif seulement quand tous prêts, ligne 0 « Vous »/état ; invité : réglages grisés, lignes des autres grisées, sa ligne et les IA du chef éditables ; Prêt -> « Plus prêt » ; Spectateur -> « Jouer », Prêt masqué, « Spectateurs : ... » avec couronne ; UNE seule trame par clic sur Prêt / Spectateur (le script ne s'y abonne pas) ; couleur / équipe / faction changées côté invite arrivent chez le chef au sondage suivant (et côté IA du chef : SendSlotAIChoice) ; le chef ferme / ouvre / met une IA sur une place (liste d'états à 5 entrées : Ouvert, Fermé, Pas prêt, Prêt, IA ; Pas prêt / Prêt grisés) ; le chef change la taille : les places au-delà se ferment après la lecture suivante ; en-tête = nom du salon.
Journal attendu : lignes `Lobby: ... sent` à chaque geste, une par geste.
