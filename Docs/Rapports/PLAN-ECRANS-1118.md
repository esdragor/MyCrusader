# PLAN-ECRANS-1118 : étape 4 de la migration, Nouvelle partie, Statistiques, HUD de jeu (D851, D840, D839)

Rapport de planification en lecture seule (agent PLAN-ECRANS-1118, 2026-11-18, main 84a42b2). Rien n'a été compilé ni modifié hors de ce fichier.
Patron repris de `PLAN-ECRANS-1117.md` (sections 0, 2.2, 3) : base C++ `Abstract, Blueprintable` gardée + `FindScriptClass()` + aides `BlueprintCallable` + `BP_Build<X>` + classe script
`UHOL<X>ScreenScript` + `HOL<X>Screen_Test.as`. Les noms de widgets sont gardés tels quels. Prouvé sur Titre, Réglages, Multijoueur, EndBanner, GameMenu, Confirm.

Chiffres réels relevés : `HOLNewGameScreen.cpp` fait **1403 lignes** (pas 1323), `.h` 196. `HOLGameScreenScript.as` fait **2296 lignes** : une bonne part du HUD est déjà en script (voir §3).

## 1. Nouvelle partie (`HOLNewGameScreen`, mode solo ET mode salon réseau `bLobby`)

### 1.1 Carte du C++ (cpp)

| Plage | Contenu | Destin |
|---|---|---|
| 34-183 | constantes de largeur, `StateNames`, `LevelNames`, `GetShortSizeName` (LOCTEXT) | textes LOCTEXT restent C++ (espace `HOLNewGameScreen`, partagé avec `HOLDiplomacyReadLibrary.cpp:126-136` en NSLOCTEXT et avec `fill_po.py`) ; exposés en `BlueprintPure` |
| 185-222 | `GetSizeName`, `GetLobbyMapName`, `GetMapName` (statiques, appelés par `HOLMultiplayerScreen.cpp:125`), `Open` (212), `Find` (218) | restent C++ |
| 224-340 | `BuildLayout` : titre, ligne carte, taille, grille de 8 lignes (nom, état, faction, pastille, couleur, équipe, niveau IA), mode et options, boutons | **script** |
| 340-460 | `NativeOnInitialized` : liste des cartes, recherche des widgets par nom (`SlotState0`... `SlotLevel7`), abonnements | cartes (`FHOLMapRules`) restent C++ (lecture exposée) ; abonnements en **script** |
| 460-493 | focus, activation, désactivation | `NativeGetDesiredFocusTarget`, `GetColumns` (754) restent C++ (colonnes de navigation, `virtual`) |
| 493-754 | `EnterLobby`, `ReadLobby` (sondage 2 fois par seconde), `ShowLobby` (affichage du salon, lignes des autres joueurs grisées) | `EnterLobby`/`ReadLobby` restent C++ (client manager, réseau) ; `ShowLobby` (566-754, 190 l.) en **script** |
| 770-940 | `SetConfig`, `ChooseSize`, `Refresh` (règles d'activation : taille et places TC3, listes de factions, couleurs) | règles pures et application des widgets en **script** |
| 940-1020 | `HandleMap/Size/TeamsTogether/Fog/Mode/OptionChosen` | **script** (gestes) |
| 1020-1188 | `GetClient`, `SendSettings`, `SendSlotAI`, `ToggleReady`, `ToggleSpectator`, `StartMatch`, `JoinMatch`, `HandleWritten`, `ShowError` | restent C++ : client réseau (`UHOLManagerClient`), écriture dans le salon ; exposés `BlueprintCallable` |
| 1189-1347 | `HandleState/Faction/Color/TeamChosen`, `UpdateFactionLists`, `IsColorTaken`, `FindFreeColor`, `IsLobbyAI` | règles pures en **script** (rouge d'abord) |
| 1348-1403 | `Launch` : graine, `DrawRandomColors`, `Normalize`, `CheckPlaces`, `GetMapFile`, `Match->SetConfig`, config d'entrée `Game`, `OpenLevelBySoftObjectPtr` | reste C++ (monde, entrée, `HOLTestHooks::PendingImposedSeed`) ; `BlueprintCallable` |

### 1.2 Lots ordonnés (7)

Un seul worktree pour tout l'écran (D840) ; les lots 1a, 1b, 2 et 7 touchent des fichiers disjoints et s'écrivent en parallèle dans ce worktree.

**NG-1a : base C++ dans le module UI (C++, ~200 l. ajoutées)**
- Fichiers : `HOLNewGameScreen.h/.cpp`, `HOLDropdown.h/.cpp` (module UI).
- Ajoute `FindScriptClass()` (`BlueprintPure`, copie du patron) ; **Open (cpp:212) garde `StaticClass()` à ce stade** (la base reste concrète, la classe C++ continue de jouer ; pas de basculement ici).
- Ajoute les aides `MakeNewGameColumn`, `AddNewGameText`, `AddNewGameDropdown(Column, Name, Options)`, `AddNewGameButton`, `AddNewGameRow` (largeurs fixées par les constantes cpp:34-50), `BP_BuildNewGame` (`BlueprintImplementableEvent`, appelé à la place de `BuildLayout` quand une classe script est utilisée).
- Événement dynamique de `UHOLDropdown` (`FHOLDropdownChosen(UHOLDropdown*, int32)`, ~12 l. ; à vérifier : l'événement actuel est natif, voir « Non vérifié »).
- `BlueprintCallable` sur la base : `Refresh`, `Launch`, `ToggleReady`, `ToggleSpectator`, `SendSettings`, `SendSlotAI`, `GetClient`-sans-fuite (rien), `ShowError(FText)`, `GetMapNames`, `GetStateNames`, `GetLevelNames`, `GetShortSizeName`.
- Test : aucun script possible avant le build ; vérification par le test de structure du lot 3.

**NG-1b : liaisons de lecture du module principal (C++, ~150 l.)**
- Fichiers : nouveaux `Match\HOLNewGameReadLibrary.h/.cpp` (ou ajouts à une bibliothèque `*ReadLibrary` existante), jamais `HOLNewGameScreen.*` (disjoint de 1a).
- `BlueprintPure` : tailles de carte permises (`FHOLMapSizeRules::CheckPlaces`, `GetMapFile`), couleurs libres, `HOLFactionChoice::IsCheatOn/Sanitize`, liste des cartes de `FHOLMapRules::GetMaps()`, nombre de places d'une carte. Pour l'affichage du salon : `FHOLLobby` en lecture (champs `Leader`, `You`, `Map`, `Size`, `Slots`) via `USTRUCT(BlueprintType)` ou une vue de lecture aplatie `FHOLLobbyView` ; `FHOLMatchConfig` en lecture/écriture de ses champs (si le struct n'est pas déjà un `USTRUCT` exposé).
- Raison D839 : liaison à exposer au script. Pas de règle ajoutée (les règles vont en 2).

**NG-2 : règles pures en script (script, ~220 l.) ; indépendant de 1a/1b**
- Fichier neuf `Script\UI\HOLNewGameRules.as` + `HOLNewGameRules_Test.as` ; toutes les règles prennent des nombres / tableaux (pas de widget).
- Contenu : `IsColorTaken`, `FindFreeColor` (cpp:1317-1347), choix de taille et places (`ChooseSize` 788, `HandleSizeChosen` 958), liste des factions selon la triche des Templiers (`UpdateFactionLists` 1229), états / niveaux IA (`IsLobbyAI` 1061, `HandleStateChosen` 1189), état des boutons Prêt / Spectateur selon le salon.
- Tests rouge d'abord (rouge vu, puis vert) : `Test_ColorTakenIgnoresOwnSlot`, `Test_FindFreeColorSkipsTaken`, `Test_FactionListHidesTemplarsWithoutCheat`, `Test_SizeFitsPlayers` (valeur lue : taille choisie pour 6 joueurs), `Test_StateAIEnablesLevel`.

**NG-3 : mise en page et gestes solo en script (script, ~280 l.) ; dépend de 1a**
- Fichier neuf `Script\UI\HOLNewGameScreen.as` (`class UHOLNewGameScreenScript : UHOLNewGameScreen`, `BP_BuildNewGame`) : cpp:224-460 (titre, ligne carte, grille de 8 lignes, mode, options, boutons) avec **les mêmes noms** (`Heading`, `MapDropdown`, `SizeDropdown`, `TeamsTogetherDropdown`, `FogDropdown`, `ModeDropdown`, `NomadDropdown`, `RegicideDropdown`, `CatastropheDropdown`, `SacredSitesDropdown`, `SlotState<n>`, `SlotFaction<n>`, `SlotColor<n>`, `SlotSwatch<n>`, `SlotTeam<n>`, `SlotName<n>`, `SlotLevel<n>`, `LaunchButton`, `BackButton`, `ReadyButton`, `SpectateButton`, `ErrorText`, `NoteText`) : les scénarios `hol.UI.Choose SlotState1 2` (quick_check.ps1:727), `hol.UI.Text FogDropdown` (fog_shots.ps1:43) s'y accrochent.
- `HOLNewGameScreen_Test.as` : `Test_NewGameScriptClass` (classe trouvée, nommée `HOLNewGameScreenScript`), `Test_NewGameWidgets` (les 9 listes d'options et 8 lignes de places existent), `Test_NewGameSlotCount`. Rouge d'abord : attendre 7 lignes au lieu de 8 une fois.
- Ne touche pas `HOLNewGameScreen.as` en même temps qu'un autre lot (un seul fichier script écrit pour l'écran).

**NG-4 : comportement solo en script (script, ~260 l.) ; dépend de 2, 3 (même fichier `HOLNewGameScreen.as`, donc après 3)**
- cpp:770-1020 (`SetConfig`, `Refresh`, `HandleMap/Size/Fog/Mode/Option`), 1189-1317 (`HandleState/Faction/Color/Team`) appelant les règles de NG-2, et `LaunchButton` -> `Launch()` (C++).
- Tests : `Test_ChooseMapUpdatesConfig`, `Test_SizeGreysTooSmallMaps`, `Test_ColorClickAvoidsTaken`, `Test_LaunchRefusedTooManyPlayers` (effet : `ErrorText` affiche « Trop de joueurs pour cette taille de carte. » ; rouge vu : ne pas afficher l'erreur).
- Les `hol.UI.NewGame.Launch` (HOLUIOpen.cpp:115-140) passent par `SetConfig` + `Launch` : inchangés (C++).

**NG-5 : mode salon réseau en script (script, ~250 l.) ; dépend de 1b et 4 (même fichier)**
- cpp:566-754 (`ShowLobby`) : lignes des autres joueurs grisées, cases Prêt et Spectateurs (`SpectatorsText`), listes d'états propres au salon (`LobbyStateNames`), options réservées au chef ; gestes `HandleState/Color/Team/Faction` en mode salon appellent `SendSlotAI` / `SendSettings` (C++). `EnterLobby` et `ReadLobby` restent C++.
- Tests : `Test_LobbyGreysOthersLines`, `Test_LobbyLeaderOnlyOptions`, `Test_LobbyReadyLabel` (état lu sur les widgets ; salon fabriqué en script depuis la vue de 1b). Ils reprennent le second morceau du test C++ existant (voir 1.3).

**NG-6 : bascule et retrait du C++ (C++ ~ -700 l., ~40 l. ajoutées) ; dépend de 3, 4, 5**
- `UCLASS(Abstract, Blueprintable)` ; `Open` (cpp:212) pousse `FindScriptClass()` ; suppression de `BuildLayout`, `NativeOnInitialized` (partie widgets), `Handle*`, `Refresh`, `ShowLobby`, `UpdateFactionLists`, `IsColorTaken`, `FindFreeColor`, constantes (jamais deux sources). `Launch`, `EnterLobby`, `ReadLobby`, envois réseau et LOCTEXT restent.
- Appelants de l'écran (tous passent par `Open`/`Find`, un seul point à changer) : `HOLTitleScreen.cpp:141` (`OpenNewGame`), `HOLMenuPlayerController.cpp:84`, `HOLMultiplayerScreen.cpp:542` (création de salon : `Open` puis `EnterLobby`), `HOLUIOpen.cpp:62` (`hol.UI.Open NewGame`) et `:122-125` (`hol.UI.NewGame.Launch`, `Find` puis `Open`), `HOLUIFaction.cpp:20` (`Find` + `OnFactionCheatChanged`), `HOLScenario.cpp:123` (étapes intégrées `hol.UI.NewGame.Launch - 0`). `HOLEndBannerScreen.cpp:147` n'est qu'un commentaire.
- `Tools\Tests\quick_check.ps1:3158` : `newgame = "menu=HOLNewGameScreen modal=none"` et `dropdown = ...` deviennent `menu=HOLNewGameScreenScript` (comme `video = "menu=HOLSettingsScreenScript modal=none"` sur la même ligne). `packaged_smoke.ps1:96` : `menu=HOLNewGameScreen` -> `HOLNewGameScreenScript`.
- Test C++ `HOLNewGameSitesBoxTests.cpp` : fait `NewObject<UHOLNewGameScreen>` ; ne marche plus sur une base abstraite -> porté en script (NG-5) puis supprimé dans ce même commit (comme les 12 tests portés de la reprise 12).
- **C'est ce lot qui bascule réellement** (classe script poussée à la place de la classe C++) : build unique du worktree, puis essai.

**NG-7 : scénario, préflight, retours (texte, ~60 l.) ; écrit en parallèle de 2 à 5, joué après 6**
- Voir 1.4. Fichiers `Tools\Scenarios\ecran_nouvelle_partie.txt` + `.look`, `Retours\E-ecran-nouvelle-partie.md` [À ESSAYER].

### 1.3 Tests et scénarios existants qui passent par l'écran

- Test C++ : `Source\HeroesOfLibertyUI\Tests\HOLNewGameSitesBoxTests.cpp`, `HeroesOfLiberty.UI.NewGame.SacredSitesBox` (case « Sites sacrés » cochable en solo, grisée en salon selon la carte ; construit un monde et un joueur local). Aucun autre test C++ ne vise `UHOLNewGameScreen` (le test d'alerte de population en copie seulement le montage, ligne 35). Tests des règles de taille / places dans le module principal : non cherchés.
- Aucun fichier de `Tools\Scenarios\*.txt` ne contient `NewGame` (grep) ; `ai_menu.txt`, `ai_lobby.ps1`, `ecran_multijoueur.*` touchent le menu : à relire au lot NG-6.
- Scripts de test qui ouvrent l'écran (hol.UI.Open NewGame / hol.UI.NewGame.Launch) :
  - `quick_check.ps1` : l.683 (capture `qc_<culture>_newgame`, menu), l.713 (pile de menus Multijoueur puis NewGame), l.726-727 (`hol.UI.Choose SlotState1 2` puis Launch : partie IA du menu `ailaunch`), l.742, 757, 788, 823, 851, 906, 924 (parties « victory », « sides », « fogclick », « audiosel » : `Launch <config> 0`), l.1328-1329 (smoke : `qc_smoke_newgame` + Launch), l.1353 (caméra), **l.3158** (tailles / titres de menu attendus).
  - `packaged_smoke.ps1` l.61-62, 96 ; `faction_shots.ps1` l.38 ; `fog_shots.ps1` l.43 ; `building_crowd.ps1` l.39 ; `crossings.ps1` l.40 ; `lobby_match.ps1` (salon, non lu).

### 1.4 Essai de 3 minutes, Nouvelle partie (scénario `ecran_nouvelle_partie`, à créer)
Séquence : `waitmap MainMenu`, `waitui`, `hol.UI.Open NewGame`, capture, `hol.UI.Choose` d'une taille et d'un état de place, capture, `waitkey` (l'utilisateur joue la grille), notes. Réutiliser `ai_menu.txt` n'est pas suffisant (il lance sans regarder l'écran).
L'utilisateur regarde : (1) la grille de 8 places est la même qu'avant (colonnes alignées, couleur et pastille, rien de coupé en 1280x720 et 1920x1080), les listes se grisent quand une taille ne tient pas le nombre de joueurs ; (2) une partie de bout en bout : changer une couleur déjà prise saute à la suivante, mettre une place en IA fait apparaître le niveau, « Lancer » ouvre la carte ; (3) clavier / manette : navigation entre colonnes et Retour ramènent au titre. Le mode salon se regarde ensuite avec `ecran_multijoueur.ps1` (deux instances) : lignes des autres joueurs grisées, Prêt, chef seul sur les options.

## 2. Statistiques (`HOLStatsScreen`, 428 l. + `Widgets\HOLStatsGraph.cpp`, 167 l.)

Structure : `Open` (52) ; `BuildLayout` (62-112 : onglets `SummaryTab`, `MilitaryTab`, `EconomyTab`, `TimelineTab`, boutons de séries `SeriesWorkers`... `SeriesStock`, `Graph`) ; `NativeOnInitialized` (112), activation / désactivation (143-189), `Close` (189), `NativeOnHandleBackAction` (198), `SelectTab` (211), `SelectSeries` (229), `RefreshTimeline` (242-302), `OnRefused` (302), `Refresh` (308-428 : grille résumé / militaire / économie par joueur). `HOLStatsGraph` : `SetData(TArray<FCurve>, TArray<FMark>)`, `DescribeAxes`, `NativePaint` (lignes, boîte, texte, légende) : **reste C++** (peinture Slate, D839).
Lecture déjà exposée : `UHOLMatchStatsReadLibrary` (`GetNiceStep`, `GetAxisEnd`, `FormatMatchTime`, `GetMilitaryScore`, `GetEconomyScore`, `GetSeriesValue`, `CanReceiveStats`, `MakeChunk`...). **À exposer** : lecture des lignes du sous-système `UHOLMatchStatsSubsystem` (joueurs, résultat, échantillons, évènements) et l'évènement `OnStatsUpdated` en dynamique ; enveloppe `BlueprintCallable` de `UHOLStatsGraph::SetData` (structs `FCurve` / `FMark` en `USTRUCT` ou deux tableaux plats). Noms exacts des getters : non lus.

- **ST-1 (C++, ~170 l.)** : `HOLStatsScreen.h/.cpp` Abstract + `FindScriptClass` + `AddStatsTab`, `AddStatsSeriesRow`, `AddStatsGrid` + `BP_BuildStats` ; `HOLStatsGraph` : `SetCurves/SetMarks` appelables ; `HOLMatchStatsSubsystem.h`: getters de lecture + évènement dynamique. Build unique.
- **ST-2 (script, ~280 l.)** : `HOLStatsScreen.as` : onglets, `Refresh` des trois tableaux (cpp:308-428), séries et `RefreshTimeline` (242-302) ; règle pure de ligne de tableau dans `HOLStatsRules.as` (+ `_Test.as`), rouge d'abord : `Test_StatsSummaryRow`, `Test_StatsSelectTabSwitcher`, `Test_StatsTimelineSeries` (la série choisie fixe le nombre de courbes passées au graphe). Dépend de ST-1.
- **ST-3 (bascule, ~30 l. C++ retirées + scénario)** : `Open` pousse `FindScriptClass` ; retrait de `BuildLayout`, `Refresh*` C++ ; appelants de `Open` : `HOLUIOpen.cpp:91-93` (`hol.UI.Open Stats`) et le bouton « Statistiques » de `HOLGameScreen.cpp:3331` / `HandleStatsClicked` (2842) et `RefreshStatsButton` (2850) ; recherche d'autres appelants non faite.
- **Scénario** : `Tools\Scenarios\ecran_statistiques.txt` (+ `.look`), copie de `victory_win.txt` (partie courte, `hol.Stats.SampleSeconds 2`, victoire) puis `hol.UI.Open Stats`, `hol.UI.Click TimelineTab`, `hol.UI.Click SeriesMilitary`. `hol.Stats.Dump` existe pour le contrôle sur client (lobby_match.ps1:156).
- **L'utilisateur regarde** : (1) les trois tableaux (Résumé, Militaire, Économie) ont les mêmes colonnes et les mêmes nombres qu'avant, un nom dans sa couleur par ligne ; (2) la Chronologie : les 6 séries se changent au clic, le graphe (axes, légende, repères) est identique ; (3) Retour / Échap ferment sans perdre la partie, un spectateur voit l'écran pendant le match.

## 3. HUD de jeu

### 3.1 Ce qui est déjà en script (`Script\UI\HOLGameScreenScript.as`, 2296 l., classe `UHOLGameScreenScript : UHOLScriptScreen`, posée par `UHOLGameScreen` sur son cadre : `ScriptScreen`, `HOLGameScreen.h:166`)
Fenêtre de duel (252-376), **panneau de diplomatie (377-906)**, bannières volcan et sites (907-985, `HOLVolcanoBanner.as`, `HOLSiteBanner.as`), ligne du niveau d'IA (986-1027), écran de pause (1028-1063, `HOLPauseScreen.as`), bloc du héros (1064-1223, `HOLHeroBlock.as`), fenêtre des talents (1224-1351), alerte de population (1373), **minicarte (1385-1590)** avec `HOLMinimapRules.as` (règles) et `UHOLMinimapView` C++ (peinture, terrain, rassemblement des entités : reste C++), règles d'alertes mineures `HOLAlertRules.as`, règles morale `HOLMoraleBlock.as`.
Tests de structure existants : `HOLHUDLayout_Test.as` et un `_Test.as` par pièce.

### 3.2 Ce qui reste dans `HUD\HOLGameScreen.cpp` (3609 l.) : panneaux et plages

| Panneau | Plages (cpp) | Taille | Reste C++ | Scénario d'essai le plus proche |
|---|---|---|---|---|
| Bannières attente + absence | 1144-1211 + mise en page 3343-3366 | ~90 l. | `NativeTick`, liaison à l'état de partie | `disconnect_look.txt`, `disconnect_hold.txt` |
| Message court (D313) + population bloquée | 2737-2827 + 3648-3661 | ~110 l. | rien (texte + minuteur) | `alert_population.txt` |
| Barre du haut (ressources, population, ouvriers inactifs) | 1272-1339, 2059-2082 + 3271-3318 | ~110 l. | marqueurs d'image, lecture de l'état joueur (`BindPlayerState` 659) | `resources_compare.txt`, `starting_peasants.txt`, `idle_wall.txt` |
| Boutons Menu et Statistiques | 2837-2860 + 3319-3342 | ~50 l. | `PushMenu` | `victory_win.txt` (menu) ; après l'écran Stats |
| Bannière d'alerte majeure + fil de messages | 782-1043 + 3367-3380 | ~260 l. | évènements du forum, `FocusAlertSlot` (caméra), clics `FEventReply` | `forum_messages.txt`, `siege_alert.txt`, `alert_population.txt` |
| Tableau des joueurs | 1044-1143 + 3381-3480 | ~200 l. | liste des places en lecture | `spectator_teams.txt`, `forum_lives.txt` |
| Barre des groupes de contrôle | 2083-2202 + 3484-3533 | ~170 l. | clics, touches | `group_march.txt`, `j7_army.txt` |
| File de production et tête | 2622-2704, 2828-2836 + 3662-3709 | ~170 l. | barres de progression, lecture du bâtiment | `building_queue.txt`, `shift_queue.txt`, `siege_production.txt` |
| Panneaux d'âges (médaillons I à IV) | 2504-2621, 2725-2736 + 3727-3778 | ~190 l. | cellules `UHOLGridCell` | `tiers.txt`, `tech_grid.txt` |
| Bloc de sélection (nom, puces, propriétaire, santé, stats, activité) | 1340-2058 + 3545-3640 | ~780 l. | texture de pastille, infobulles de survol | `unit_stats_attack.txt`, `unit_stats_attack_enemy.txt`, `income_tooltip.txt`, `select_type_q.txt`, `enemy_select.txt` |
| Grille de commandes (3x4, A Z E R / Q S D F / W X C V) | 2203-2503, 2705-2724 + 3710-3726 | ~350 l. | `UHOLGridCell` (peinture, infobulle) | `peasant_grid.txt`, `tech_grid.txt`, `siege_grid.txt`, `tech_grid_shot.txt` |
| Outils de test (`Describe*`, `ClickWidget`) | 2861-3191 | ~330 l. | suivent chaque panneau (les `hol.HUD.Click/Report/Hover` s'y accrochent) | tous |
| Squelette (`BuildLayout` 3225-3609, `NativeTick` 543, `Bind*` 659-780, `Refresh` 1227) | | ~600 l. | reste C++ jusqu'au bout (hôte des panneaux) | |

Aussi : `HOLMinimapView.cpp` (683 l., peinture Slate, rendu du terrain, `FocusCamera`, `OrderMove` : **reste C++**), `HOLCheatPanel.cpp` (483 l., panneau de triche de développement ; `HandleCheatClicked`, `FollowServerSettings`) et `HOLScenarioPanel.cpp` (314 l., panneau de scénario de test ; a `HOLScenarioPanelTests.cpp` C++) : outils de développement, dernier rang, non joués par le joueur ; `HOLGameHUD.cpp` (326 l.) : contenu non lu.

### 3.3 Ordre de migration (du plus simple au plus risqué)
Chaque panneau devient un `.as` à lui (vue pure + `_Test.as`, comme `HOLHeroBlock.as`) et `HOLGameScreenScript.as` n'y reçoit qu'un crochet de 5 à 10 lignes (Build / Refresh), **posé en série par le worktree d'intégration** : ce fichier de 2296 lignes est le point de collision.
H-1 bannières attente + absence ; H-2 message court + population bloquée ; H-3 barre du haut + boutons Menu / Statistiques (après l'écran Stats) ; H-4 tableau des joueurs ; H-5 alerte majeure + fil ; H-6 groupes de contrôle ; H-7 file de production ; H-8 panneaux d'âges ; H-9 bloc de sélection en 3 sous-lots (bâtiment, unité et stats, puces de groupe) ; H-10 grille de commandes ; H-11 panneaux de développement (triche, scénario) en dernier, essai non requis du joueur.
Risques HUD : coût par image (`NativeTick` rafraîchit tout, un script par image sur la grille et la sélection est à mesurer : `hol.HUD.Report`), les hooks de test (`ClickWidget` 2938, `Describe` 3033) doivent suivre, les noms de widgets (`TopBar`, `SelectionPanel`, `GridCell02`...) restent identiques, `HUDRects.ps1` / `hud_rects_game_test.ps1` vérifient les rectangles.

### 3.4 Ce que l'utilisateur regarde, par panneau (essai de 3 min, scénario à créer ou réutiliser)
- H-1 : bannière d'attente au bon endroit sous la barre du haut, bannière d'absence sous l'alerte, aucune ne bloque les clics. Réutilise `disconnect_look.txt` (ajouter `ecran_hud_bannieres.txt` si l'attente n'y est pas).
- H-2 : le message court apparaît, s'efface, « population bloquée » s'affiche une fois. `alert_population.txt`.
- H-3 : ressources et population identiques, compteur d'ouvriers inactifs cliquable, boutons Menu et Statistiques cliquables. Nouveau `ecran_hud_hautdroite.txt`.
- H-4 : lignes des joueurs (couleur, vies, phase du forum), mise à jour en direct. `spectator_teams.txt`.
- H-5 : une alerte majeure s'affiche, un clic centre la caméra, le fil garde ses lignes. `siege_alert.txt` + `forum_messages.txt`.
- H-6 : touches de groupes créent et rappellent, la puce « 1 · 9 » suit. `group_march.txt`.
- H-7 : file qui se remplit et se vide, barre de progression, placement ne bouge pas. `building_queue.txt`.
- H-8 : médaillons I à IV, touches A Z E R, apparition au bon âge. `tiers.txt`.
- H-9 : nom, propriétaire, santé, puces de groupe, stats au survol identiques. `unit_stats_attack.txt`.
- H-10 : grille 3x4 : cellules grisées / actives, infobulles, clic = ordre. `peasant_grid.txt`, `tech_grid.txt`.

## 4. Tableau des lots

Rappel : agents Sonnet de 15 min, 25 appels, sans build ; `Haldir` = intégration (crochets dans `HOLGameScreenScript.as`, fusions) ; `Faramir` exclu.

| Lot | Worktree conseillé | C++ | Dépend de | Taille | Essai utilisateur |
|---|---|---|---|---|---|
| NG-1a base C++ UI | Arwen | oui | rien | ~200 l. | non |
| NG-1b liaisons lecture (module principal) | Arwen | oui | rien | ~150 l. | non |
| NG-2 règles pures | Arwen | non | rien (règles à paramètres) | ~220 l. | non |
| NG-7 scénario + .look + retour | Arwen | non | rien (écrit), 6 (joué) | ~60 l. | non |
| NG-3 mise en page script | Arwen | non | NG-1a (build) | ~280 l. | non |
| NG-4 comportement solo | Arwen | non | NG-2, NG-3 | ~260 l. | non |
| NG-5 mode salon | Arwen | non | NG-1b, NG-4 | ~250 l. | non |
| NG-6 bascule + retrait C++ + quick_check.ps1:3158 | Arwen | oui (retrait) | NG-3, 4, 5 | ~-700 / +40 l. | **oui** (NG-7) |
| ST-1 liaisons Stats + abstract | Celeborn | oui | rien | ~170 l. | non |
| ST-2 script + règles | Celeborn | non | ST-1 | ~280 l. | non |
| ST-3 bascule + scénario | Celeborn | oui (retrait) | ST-2 | ~80 l. | **oui** |
| H-1 .. H-2 (une pièce `.as` + test chacun) | Elrond | non | rien (vues pures) ; crochets : Haldir | ~120-200 l. | oui (un essai chacun, D840) |
| H-3 .. H-6 | Elrond puis Galadriel | non | H-1 fusionné ; H-3 après ST-3 | ~150-250 l. | oui |
| H-7 .. H-9 | Galadriel / Glorfindel | non (C++ ensuite) | H-6 | ~190-300 l. | oui |
| H-10 grille de commandes | Sauron | oui | H-9 | ~350 l. | oui |
| H-11 triche / scénario | Glorfindel | non | tout le reste | ~300 l. | non |

Les essais se jouent un à la fois ; l'utilisateur décide s'il regroupe plusieurs petits panneaux (H-1 à H-3) dans un seul essai (proposition, D840 dit un par panneau).

### Première vague à lancer tout de suite (5 lots parallèles, aucun fichier commun)
1. **NG-1a** (Arwen) : `HOLNewGameScreen.h/.cpp`, `HOLDropdown.h/.cpp`.
2. **NG-1b** (Arwen) : nouvelle bibliothèque dans `Source\HeroesOfLiberty\Match\`.
3. **NG-2** (Arwen) : `Script\UI\HOLNewGameRules.as` + `_Test.as` (rouge d'abord ; la session principale lance les tests).
4. **NG-7** (Arwen) : `Tools\Scenarios\ecran_nouvelle_partie.txt/.look`.
5. **ST-1** (Celeborn) : `HOLStatsScreen.*`, `HOLStatsGraph.*`, `HOLMatchStatsSubsystem.*` (fichiers disjoints de Nouvelle partie).
Le C++ de 1a, 1b (même worktree, **un seul build Arwen**) et celui de ST-1 (un build Celeborn) : deux builds dans `Docs\BuildQueue.state`, aucun agent ne compile. Option si la machine est saturée : remettre ST-1 à la vague 2 et garder 4 lots.
Vague 2 : NG-3, ST-2. Vague 3 : NG-4, ST-3-script. Vague 4 : NG-5. Puis NG-6, essai Nouvelle partie.

## 5. Risques principaux
1. Exposition à l'script de `FHOLMatchConfig` et `FHOLLobby` (structs complexes avec `TOptional`, tableaux de slots) : si ce ne sont pas des `USTRUCT` exposables, NG-1b grossit et sort de la limite de 300 lignes.
2. `UHOLDropdown` : événement de choix natif aujourd'hui, à doubler d'un événement dynamique (un fichier de base de plus, utilisé par tous les écrans).
3. Test C++ `SacredSitesBox` incompatible avec une base abstraite : porté puis supprimé dans NG-6 ; `quick_check.ps1:3158` et `packaged_smoke.ps1:96` à ajuster dans le même commit sinon le contrôle de menu échoue.
4. Le mode salon exige deux instances (`ecran_multijoueur.ps1`, `ai_lobby.ps1`, `lobby_match.ps1`) : NG-5 n'est prouvé qu'avec le test de structure, l'essai réseau reste à la main.
5. Textes : l'espace `HOLNewGameScreen` est lu par `HOLDiplomacyReadLibrary.cpp:126-136` et par `fill_po.py` (61 occurrences) : ne pas déplacer les LOCTEXT dans le script.
6. HUD : `HOLGameScreenScript.as` (2296 l.) est le point de collision de tous les panneaux ; coût par image des panneaux lourds (sélection, grille).

## 6. Non vérifié
- `FHOLMatchConfig`, `FHOLLobby`, `FHOLMapRules`, `FHOLMapSizeRules`, `HOLFactionChoice` : exposition au script (USTRUCT, UFUNCTION) non lue.
- Forme exacte de l'événement de `UHOLDropdown` et de `UHOLManagerClient` ; getters de lecture de `UHOLMatchStatsSubsystem` et de son évènement `OnStatsUpdated`.
- Contenu de `HOLGameHUD.cpp`, `HOLCheatPanel.cpp`, `HOLScenarioPanel.cpp` au-delà de leurs signatures ; ce que font `HandleCell0..23` du script de jeu (lignes 1981-2119).
- Corps des fonctions de `HOLNewGameScreen.cpp` (seules les signatures, `Launch` et le début de `BuildLayout` et `NativeOnInitialized` ont été lus) ; les tailles par plage sont des estimations d'après les signatures.
- `ai_menu.txt`, `ai_lobby.ps1`, `ecran_multijoueur.*`, `lobby_match.ps1` : lus par nom seulement ; autres tests du module principal sur les règles de taille / places non cherchés.
- Autres appelants de `UHOLStatsScreen::Open` hors `HOLUIOpen.cpp` : non cherchés. Existence d'un `Saved\Config` par worktree pour l'essai (déjà signalé en 1117).
- Aucun essai d'aucune sorte n'a été lancé ; les noms de scénarios cités comme « le plus proche » sont choisis d'après leur nom, sauf `victory_win.txt` (ligne `quick_check.ps1:757`) et `disconnect_*` (non ouverts).
