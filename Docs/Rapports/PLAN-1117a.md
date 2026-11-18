# PLAN-1117a : ce qui reste a migrer de Source\ hors simulation Mass, et les 12 prochains lots

Agent de planification, lecture seule du code (aucun build, aucun test, aucun commit). Ecrit le 2026-11-17 vers 00:10, base `main` 111d6b6.
Sources : `Docs\Rapports\PLAN-1116-E3.md`, `Docs\Rapports\S1D-1116a.md`, section « Prochaine seance de migration » d'`Auto_Journey_File.md`,
GDD D830, D836 a D839 (D838 : les tests de la simulation Mass restent en C++), et des mesures par script sur `Source\` et `Script\` de `main`.

Methode de mesure (pour relire les chiffres) : lignes brutes (`split` sur fin de ligne, blancs compris) sauf mention. Categorie d'un fichier non-test =
recherche de mots-cles dans son texte (Mass = `Mass`, monde = `UWorld|GetWorld|Subsystem|AActor`, reseau = `Replicat|NetDriver|Http|Socket`,
Slate = `Slate|SWidget|UUserWidget|SNew`). Categorie d'un test = `UWorld/CreateWorld/GEngine` (monde), `UMassEntitySubsystem/FMassEntityManager` (Mass),
`FSlate` (Slate). Ce sont des indices : un mot dans un commentaire compte.

## En bref

- `Source\` = 150 800 lignes brutes (139 000 non vides) : 105 800 de code, 45 000 de tests (126 fichiers `*Tests.cpp`, dont 2 dans HeroesOfLibertyUI et 1 dans Simulation).
- Tests : 17 400 lignes touchent Mass (29 fichiers : restent, D838) ; 11 900 creent un monde (44 fichiers : restent) ; 131 Slate ; **15 600 en 52 fichiers n'ont ni monde, ni Mass, ni Slate**.
  Parmi ces 52 : environ 3 000 lignes chargent une carte `.umap` (`LoadObject<UWorld>`), 1 100 sont des tests de regles lues par Mass, d'autres lisent des assets : il reste
  **une trentaine de fichiers (environ 6 000 lignes) vraiment portables**, dont 4 deja ouverts (E3-04 reste, E3-06, E3-10, E3-11) et 7 ports partiels a finir au lot MIG-C++-FINAL.
- Code : 35 400 lignes de fichiers touchant Mass (Simulation 18 700, Commands 5 000, Match 2 800, Presentation 2 000, Net 1 900, Buildings 1 100, Map 700, Dev 900, StressTest 2 200) : restent en C++.
- HeroesOfLibertyUI : 17 600 lignes, dont `HOLGameScreen.cpp` 4 458 a lui seul. Candidats etape 4 : ecrans de 100 a 700 lignes.
- Rien d'urgent n'est « regle pure sans appelant Mass » en grande quantite : environ 4 000 lignes (tableau 2).
- Les 12 lots proposes : 6 sans C++, 6 avec bibliotheque C++ (une seule compilation regroupee, comme MIG-C++-FINAL).

## 1. Inventaire

### 1.a Tests C++ « purs » pas encore portes (aucun monde, Mass, Slate)

Les 4 lots deja ouverts dans `Auto_Journey_File.md` passent d'abord : E3-04 (reste Count), E3-06 (FactionTechnology), E3-10 (RelationIcons), E3-11 (SignalInput).
Ports partiels dont le test C++ reste : FactionChoice (Bodies), MapRules, MarketTable (import UE), GameNames x3, ManagerErrorText (ReadFailure), StartingPeasants.Count.

Taille = lignes brutes du test. « Regle » = fichier C++ teste. Appelants : Mass = lu par un processeur Mass, serveur = sous-systeme/etat, UI = ecran C++.

| Test (lignes) | Regle testee (appelants) | Categorie | Migrable maintenant ? |
|---|---|---|---|
| HOLRelayRing (40) | HOLRelayRules + FHOLRelayView (serveur) | pur | oui apres lib : `RelayLibrary` n'expose pas MakeView/FillPercent (L17-07) |
| HOLCampRules (42) | HOLCampSubsystem (`Step`), Victory | pur (tire un Subsystem) | non : la regle est dans un sous-systeme monde |
| HOLCaptureSeq (42), HOLScenarioNote (178) | outillage d'essai Dev (TestSequence, TestHooks) | pur | non : c'est le harnais de test, reste C++ |
| HOLMinimapAlertRing (42), HOLMinimapTerrain (55) | HOLMinimapView (UI, 919 l.) ; `HOLMinimapRules.as` existe | pur | a verifier : oui si la regle testee est celle du script (L17-05) |
| HOLSignalInput (63) | HOLSignalInput (controleur) | pur | ouvert : E3-11, apres P1b-P1d |
| HOLHeroAbility (102) | HOLHeroAbilityRules (Mass x2, serveur, UI) | pur | oui apres lib (L17-08) ; `HeroLibrary` a deja 2 fonctions de capacite |
| HOLBiomeConfig (102), HOLMapSizeLobby (191), HOLMatchConfig (288), HOLManagerClient (666) | ManagerClientRules (JSON), MatchConfig (partout) | pur (JSON/CSV) | non maintenant : bibliotheque lourde (JSON) ; apres decision sur la couche reseau du manager |
| HOLProfane (114), HOLAnimation (501), HOLMonk (249) | CombatRules, UDataTable, assets AnimSequence | pur (tables UE, assets) | non : lisent des assets UE ou des tables Mass |
| HOLSourceNav (139), HOLSourceGrid (220), HOLSourceMassifs (250) | SourceNavComponent (UObject), GatherRules (Mass) | pur | non : composant UE / regle lue par Mass |
| HOLGameMenu (143, 7 tests) | HOLGameMenuRules ; jumeau script deja la (`HOLGameMenuRules.as`, `_Test.as`) | pur | oui sans C++ : verifier la couverture puis retirer (L17-04) |
| HOLDiplomacyRules (163) | HOLDiplomacyRules ; `DiplomacyReadLibrary` (43 fonctions) existe | pur | oui sans C++ (L17-01) |
| HOLHeroXP (202) | HOLHeroXPRules (Mass x2) + UDataTable + CombatRules | pur | apres lib lourde (16 fonctions) : non cette vague |
| HOLProduction (208) | HOLProductionRules (serveur, aucun Mass) + UDataTable | pur | apres lib (12 fonctions + file) : L17-12 |
| HOLMatchStats (234) | HOLMatchStats (serveur, controleur, ecran Stats) | pur | apres lib (22 fonctions, structure a fusion de blocs) : L17-11 |
| HOLOrderChain (261) | HOLOrderChainRules (Mass x3) | pur | apres lib (struct + enum + 2 fonctions) : L17-09 |
| HOLDeployRules (296) | HOLDeployRules (Mass x4 seul) | pur | apres lib (L17-10) ; la regle reste en C++ |
| HOLSiege (310, 5 tests) | HOLSiegeRules (Mass x1, serveur, IA) ; `SiegeCheckLibrary` existe | pur | partiel sans C++ : Tiers/Technologies/Health oui, Combat non (L17-02) |
| HOLAvoid (391) | HOLAvoidRules (Mass x3 seul) | pur | non : 30 fonctions + lambdas + Sanity, rien d'autre que Mass |
| HOLMapSize (411) | HOLMapSizeRules (serveur, scenario) + CSV | pur | apres lib ; partie CSV en python (facon P) : lot suivant |
| HOLTechnologyTable (445, 3 tests) | HOLTechnologyRules (Mass x3) + tables CSV | pur (table) | partiel sans C++ : tests de table en python (L17-03) ; TechnologyRules (26 fonctions) reste C++ |
| HOLForumMessages (461) | HOLForumMessages (FText, culture) | pur | non : localisation UE (FText/`.locres`) |
| HOLMovement (530), HOLSanity (583), HOLCommandGrid (927), HOLFormation (962), HOLConstruction (1175) | GroupMove/UnitPicking, SanityRules, CommandGrid, FormationRules (Mass), ConstructionRules (Mass x5) | pur | non : regles chaudes lues par Mass, ou 15 a 20 tests de 30 fonctions |
| HOLVolcano (896, 17 tests) | VolcanoRules/Layout + UDataTable | pur | non maintenant : 3 appelants monde, tables UE |
| HOLMapSteppe (582), HOLMapHinterland (583), HOLMapFords (335), HOLMapCols (309), HOLMapForest (161), HOLMapSizeMap (140), HOLMapTable (870) | cartes `.umap` (`LoadObject<UWorld>`) | pur en apparence, **asset carte** | non : lisent les cartes de l'editeur, restent C++ |

### 1.b Regles C++ pures sans appelant Mass (candidates a passer en script)

« Mass » = un fichier `*Processor.cpp` ou `SimulationProcessors` la cite. Compte par recherche du nom de fichier (hors tests, hors elle-meme). Aucune de ces regles ne se
migre sans un pont ou une bibliotheque : l'appelant serveur/UI est en C++ (D839 : le code neuf est en script, pas l'existant).

| Regle (lignes .cpp+.h) | Appelants hors Mass | Copie script existante | Migrable maintenant ? |
|---|---|---|---|
| HOLMarketRules (168+?) | ProductionSubsystem, GameState, MarketSubsystem | HOLMarketRules.as 233 l. (double !) | non : double a resoudre d'abord (R9, MIG-C++-FINAL) |
| HOLSiteRules (45) | SiteSubsystem (pas serveur), SiteBoard | HOLSiteRules.as 130 l. (double) | non : idem |
| HOLTerrainRules (54+41) | TerrainSubsystem | HOLTerrainRules.as 90 l. (double) | non : le sous-systeme est lu par Mass |
| HOLMapSizeRules (249+110) | Scenario, GameState, MatchConfig, ManagerClientRules | aucune | apres lib, avec MapSize (partie CSV python) |
| HOLMatchStats (238) | MatchStatsSubsystem, PlayerController, StatsScreen | aucune | apres lib : L17-11 |
| HOLUnitPicking (240+140) | GameState, PlayerController (clic joueur) | aucune | non maintenant : appele a chaque clic, chemin chaud client |
| HOLDisconnectRules (132) | AIBrain, GameState, ManagerClient, GameMode | aucune | apres lib (5 appelants) |
| HOLSourceGrid (177+62) | AISummary, SourceSubsystem, GatherRules | aucune | non : GatherRules est lue par Mass |
| HOLProductionRules (93) | ProductionSubsystem, Building, AISummary | aucune | apres lib : L17-12 |
| HOLPauseRules (77+52) | GameState, PauseLibrary (lib existe) | aucune | oui sans C++ pour tests de pause (aucun test C++ propre) |
| HOLVolcanoRules (423) + HOLVolcanoLayout (193) | VolcanoSubsystem, LavaObstacle | `HOLVolcano...` 1 reference | non : serveur, a revoir apres la bibliotheque Volcano |
| HOLVisionRules (573) | VisionSubsystem, FogRender, Building | aucune | non : rendu du brouillard par case, chemin chaud |
| HOLForumRules (554) + HOLForumMessages (547) | AISummary, Building, VisionSubsystem, GameState | aucune | non maintenant : FText/culture pour les messages ; ForumRules a 18 appelants |
| HOLExplorationXPRules (88) | VisionSubsystem (boucle par case) | aucune (script retire) | non : pont dans une boucle chaude, a mesurer |
| HOLRelayLayout (75), HOLSiteLayout (106) | RelaySubsystem ; SiteLayout : aucun appelant hors tests (recherche par nom de fichier) | aucune | SiteLayout : suppression a verifier si seul l'essai l'appelle |
| HOLGameMenuRules (68+89) | HUD, GameScreen, PlayerController, ForumMessages | HOLGameMenuRules.as 52 l. (double) | oui sans C++ pour le test (L17-04) ; la regle suit l'etape 4/5 |
| HOLSignalInput (52) | PlayerController, ForumMessages | HOLSignalRules.as 136 l. | ouvert E3-11 |

Regles lues par Mass (restent en C++, source unique) : FormationRules 1 583, ConstructionRules 749, CombatRules 1 263, AvoidRules 337, DeployRules 275, HeroXPRules 222, HealRules 190,
GroupMoveRules 230, OrderChainRules 57, UnitAnimation 115, TechnologyRules 185, UnitAbilityRules (lib existante), HeroAbilityRules 180.

### 1.c Ecrans et widgets de HeroesOfLibertyUI, petits d'abord (candidats etape 4)

Classe : base = classe C++ parente de widgets script (reste C++) ; ecran = candidat. Script existant : `HOLEndBannerScreen.as` 78, `HOLGameMenuScreen.as` 105, `HOLPauseScreen.as` 71,
`HOLSiteBanner.as`, `HOLVolcanoBanner.as`, `HOLGameScreenScript.as` 2 162, hote `HOLScriptScreen.cpp` 574.

| Fichier (lignes .h+.cpp) | Categorie | Test C++ associe | Migrable maintenant ? |
|---|---|---|---|
| HOLMenuGameMode (26), HOLUIFaction (31), HOLUICapture (51), HOLUIClick (64), HOLUIKey (93) | Dev (essais interface) | — | non : outillage d'essai |
| HOLScreenStack (40), HOLButton (70), HOLQueueItem (72), HOLRotator (103), HOLFonts (81), HOLUIInputData (66) | Slate/UMG (base) | — | non : classes de base dont heritent les widgets script |
| HOLRelationIcons (37) | pur (textures) | HOLRelationIcons (39) | ouvert E3-10 |
| HOLHUDLayoutRules (55) + ReadLibrary (90) | pur, deja en bibliotheque | HOLHUDLayoutTests porte | fait : reste `HOLHUDLayoutRules` pour HOLScriptScreen |
| HOLConfirmDialog (107) | ecran (UCLASS) | aucun | oui sans C++ si le patron GameMenu/EndBanner suffit (L17-06) |
| HOLTitleScreen (206) | ecran, Slate | HOLFrontEnd (169, lit des fichiers) | non maintenant : Slate, titre du jeu |
| HOLStatsGraph (222) | widget Slate (dessin) | — | non : peinture Slate |
| HOLUIMouse (303 Dev) | Dev + Slate | HOLUIMouse (131, Slate) | non |
| HOLEndBannerScreen (333) | ecran, script existe | — | en cours : le script existe (78 l.), reste a retirer le C++ |
| HOLScreen (379), HOLRootLayout (189), HOLUIStyles (409) | base ecran, style | — | non : base |
| HOLScenarioPanel (403) | panneau monde | HOLScenarioPanel (289, monde) | non : le test cree un monde |
| HOLGameHUD (418), HOLGridCell (419), HOLDropdown (444) | widget Slate | — | non maintenant |
| HOLGameMenuScreen (438) | ecran, script existe (105 l.) | HOLGameMenu (143) | en cours (L17-04) |
| HOLGameNames (496) | localisation (`FText`) | HOLGameNames (138) | non : `.locres` |
| HOLStatsScreen (516), HOLMultiplayerScreen (560) | ecrans (monde, reseau) | — | apres etape 4 petite taille |
| HOLScriptScreen (574) | hote du script | — | non : c'est le pont |
| HOLCheatPanel (634), HOLSettingsScreen (713) | ecrans (Slate) | — | plus tard (etape 4, lots a 2 passes) |
| HOLManagerClient (857), HOLManagerErrorText (189+136) | reseau | ManagerClient (666), ManagerErrorText porte | non : transport HTTP/WebSocket |
| HOLMinimapView (919) | ecran (monde, Slate) | MinimapAlertRing (42), MinimapTerrain (55) | partiel : L17-05 |
| HOLNewGameScreen (1 648) | ecran (monde, reseau) | NewGameSitesBox (93, monde) | non maintenant |
| HOLGameScreen (4 458) | ecran principal | PopulationAlertThread (83, monde) | non : decoupage en lots de 150 a 250 lignes autour de `HOLGameScreenScript.as` |

## 2. Les 12 prochains lots

Regle commune : un worktree par lot, un agent Sonnet, 15 minutes. Le C++ regle ne disparait que la ou aucun appelant C++ ne reste ; sinon la regle C++ reste la **seule** source
et le test passe en script par la bibliotheque (jamais de copie de regle en script). Les tests C++ portes sont retires au lot MIG-C++-FINAL, avec la compilation.
Les 4 lots deja ouverts (E3-04 reste, E3-06, E3-10, E3-11) passent avant.

### 2.a Sans C++ (a lancer des maintenant, en parallele)

| Lot | Fichier(s) | A exposer | Premier test rouge | Suppression C++ associee (ou disparait la copie) |
|---|---|---|---|---|
| L17-01 | `HOLDiplomacyRulesTests.cpp` (163 l., 1 test) -> `Script\Tests\HOLDiplomacyRules_Test.as` | rien : `HOLDiplomacyReadLibrary` (43 fonctions) | `Test_DiplomacyRulesMatrix`, une valeur d'alliance faussee une fois | git rm du test au lot MIG-C++-FINAL ; la regle reste en C++ (vision, forum, serveur, `Ally` dans HealProcessor) ; la copie script `HOLDiplomacyRules.as` a deja ete retiree |
| L17-02 | `HOLSiegeTests.cpp` (310 l.) parties Tiers, Technologies, Health -> `Script\Tests\HOLSiege_Test.as` | rien : `SiegeCheckLibrary` expose GetTier, InPrototype, GetHealthStat, GetDamageStat, FindEngine | `Test_SiegeTiers`, une valeur de palier faussee | Tests Tiers/Technologies/Health retires du `.cpp` ; **double existante** `Script\...\HOLSiegeRules.as` (98 l.) a retirer si `HOLAISiege.as` lit la bibliotheque ; la partie Combat (FSeeker) reste en C++ |
| L17-03 | `HOLTechnologyTableTests.cpp` (445 l.) tests TechnologyTable + TechnologyPlayable -> `Tools\Tests\technology_table_test.py` dans quick_check | rien (facon P : lit `Technologies.csv`, `BuildingTypes.csv`) | copie de `Technologies.csv` avec une ligne en moins, le test doit echouer | 2 des 3 tests retires du `.cpp` ; `TechnologyRulesTest` (26 fonctions, Mass x3) reste C++ |
| L17-04 | `HOLGameMenuTests.cpp` (143 l., 7 tests) : verifier la couverture par `Script\UI\HOLGameMenuRules_Test.as` (41 l.) | rien | un test manquant (par exemple EndBannerClosed) ajoute et fausse une fois | git rm du `.cpp` quand les 7 sont couverts ; `HOLGameMenuRules.cpp/.h` (68+89) reste : 5 appelants C++ de l'UI ; **jumeau** `HOLGameMenuRules.as` a resorber a l'etape 4 |
| L17-05 | `HOLMinimapAlertRingTests.cpp` (42) + `HOLMinimapTerrainTests.cpp` (55) -> `Script\UI\HOLMinimapRules_Test.as` | rien si la regle testee est celle de `HOLMinimapRules.as` ; sinon passer au rang C++ | `Test_MinimapAlertRing`, une valeur faussee | git rm des 2 tests ; `HOLMinimapView` C++ garde ses appels ; **a verifier d'abord** que la regle n'est pas dans `HOLMinimapView.cpp` (alors lot C++) |
| L17-06 | `HOLConfirmDialog.cpp/.h` (107 l.) -> `Script\UI\HOLConfirmDialog.as` sur le patron `HOLGameMenuScreen`/`HOLEndBannerScreen` (hote `HOLScriptScreen`) | rien si le patron suffit | script de smoke : le dialogue rend ses deux boutons (rouge = texte d'un bouton faux) | `HOLConfirmDialog.cpp/.h` supprimes ; aucun test C++ ; la ligne « dialogue de confirmation » ne figure pas dans `Placeholders.txt`, a relire |

### 2.b Avec C++ (bibliotheque a ecrire, une seule compilation pour les 6)

Modele : `HOLDiplomacyReadLibrary` (`UBlueprintFunctionLibrary`, statiques `UFUNCTION(BlueprintPure)`), ecrits dans un worktree commun puis UN build, ensuite 6 tests script en parallele.

| Lot | Fichier(s) | A exposer en C++ | Premier test rouge | Suppression C++ associee |
|---|---|---|---|---|
| L17-07 | `HOLRelayRingTests.cpp` (40) -> `Script\Tests\HOLRelayRing_Test.as` | `URelayLibrary::MakeView` + `FillPercent` (struct `FHOLRelayView`, ~25 l.) | `Test_RelayRing`, une valeur du pourcentage faussee | test C++ retire ; la **double existante** `HOLRelayRules.as` (127 l.) retiree, la regle C++ (17 l.) reste seule source (serveur `Step`) |
| L17-08 | `HOLHeroAbilityTests.cpp` (102, 3 tests) -> `Script\Tests\HOLHeroAbility_Test.as` | `UHeroAbilityReadLibrary` : unlock, cast, damage (~40 l.) | `Test_HeroAbilityUnlock`, un niveau faux | test C++ retire ; `HOLHeroAbilityRules` reste C++ (Mass x2) : source unique |
| L17-09 | `HOLOrderChainTests.cpp` (261, 2 tests) -> `Script\Tests\HOLOrderChain_Test.as` | `FHOLOrderChainStep`, enum de pas, `IsCurrentOrderOver`, `MaySeek` (~60 l.) | `Test_OrderChains`, une etape fausse | test C++ retire ; `HOLOrderChainRules` reste C++ (Mass x3) |
| L17-10 | `HOLDeployRulesTests.cpp` (296, 9 tests) -> `Script\Tests\HOLDeployRules_Test.as` | `UDeployReadLibrary` : 9 fonctions et enums (~90 l.) | `Test_DeployWhoCanDoWhat`, un cas faux | tests C++ retires ; `HOLDeployRules` (158+117) reste C++ (Mass x4) : source unique |
| L17-11 | `HOLMatchStatsTests.cpp` (234, 4 tests) -> `Script\Tests\HOLMatchStats_Test.as` | bibliotheque de lecture (22 fonctions, structure a blocs, `MakeChunk`/`MergeChunk`) ~150 l. | `Test_MatchStatsScores`, un score faux | tests C++ retires ; la regle (238) reste C++ tant que le serveur l'appelle ; a decider ensuite : pont serveur vers script |
| L17-12 | `HOLProductionTests.cpp` (208, 3 tests) -> `Script\Tests\HOLProduction_Test.as` | bibliotheque `Production` : TryEnqueue/Cancel/Advance/GetProgress/PopulationCap (~120 l., file en `FHOLQueue`) | `Test_ProductionQueue`, une file fausse | tests C++ retires ; `HOLProductionRules` (93) reste C++ (3 appelants serveur) ; plus cher : a lancer en dernier |

Ordre : L17-01 a 06 d'abord (vague 1), puis ecrire les 6 bibliotheques (une vague, un worktree commun), UN build, puis tests script L17-07 a 12 (vague 3).

## 3. Estimation de ce qui reste

- **Lignes C++ hors Mass** : 150 800 brutes au total, moins 17 400 de tests Mass = **environ 133 000** (116 000 non vides), dont code 105 800 (fichiers touchant Mass 35 400), tests monde 11 900, tests purs 15 600, Slate 131.
  Si l'on retire aussi le code Mass (35 400) : environ 98 000 lignes hors Mass.
- **Ce qui est vraiment migrable** (pas de monde, pas de Mass, pas de Slate) : environ 6 000 lignes de tests (une trentaine de fichiers), 4 000 lignes de regles pures sans appelant Mass
  (a condition d'avoir bibliotheque ou pont), et 5 000 a 6 000 lignes d'ecrans (etape 4 : ConfirmDialog, EndBanner, GameMenu, Stats, Multiplayer, Settings, CheatPanel,
  puis `HOLGameScreen` en morceaux). Soit environ 15 000 lignes (11 % du total).
- **Vagues de 5 a 7 agents** : tests purs restants (une trentaine de fichiers, deux lots par fichier au pire) : 5 vagues dont 2 de builds ; regles pures vers script avec ponts : 3 vagues
  (chaque pont = un build de 10 minutes environ) ; etape 4 UI (une vingtaine de lots, deux passes pour les gros ecrans) : 4 vagues ; fusions et MIG-C++-FINAL : 2 vagues.
  **Total : de 12 a 14 vagues, soit 4 a 5 heures**, si les agents tiennent 15 minutes et que le build tient.
- **Ne se migrera PAS** :
  - tests Mass (17 400, D838) et tests qui creent un monde (11 900) : le harnais de simulation reste C++ ;
  - simulation et processeurs Mass (35 400 lignes, D836) ;
  - transport reseau (Net 4 900, ManagerClient, replication, RPC du controleur) ;
  - base Slate/UMG (HOLScreen, RootLayout, Style, Button, Rotator, Dropdown, GridCell, StatsGraph) : les widgets script en heritent ;
  - tests de carte `.umap` (3 000 lignes) et d'assets (Animation 501) : lisent des fichiers de l'editeur ;
  - localisation `FText` (ForumMessages 547+461, GameNames 496) ;
  - outillage Dev et StressTest (5 900) et harnais de test (`TestSequence`, `TestHooks`) ;
  - regles chaudes lues par Mass (FormationRules, ConstructionRules, CombatRules, AvoidRules, DeployRules, HeroXPRules...) : le C++ reste la seule source ; les tests passent en script par bibliotheque, sinon restent.
- Les 7 « parties non portees » deja listees dans E3-FIN (Bodies, MatchLossText ReadFailure, FTextInspector, import UE de MarketTable...) restent en C++ tant que ces types sont invisibles du script.

## 4. Non verifie

- Les categories viennent de mots-cles dans le texte : un `UWorld` ou `Subsystem` dans un commentaire classe un test a tort ; les cartes `.umap` sont reperees par `LoadObject<UWorld>`.
- Aucun test n'a ete lu en entier : « 1 test » = nombre de `IMPLEMENT_SIMPLE_AUTOMATION_TEST`. Les listes de fonctions appelees (DeployRules 9, MatchStats 22, Siege 18, Production 12...) sont des comptes de recherche, pas lus.
- Pour L17-01, que toutes les fonctions appelees par `HOLDiplomacyRulesTests.cpp` soient dans `DiplomacyReadLibrary` (43 fonctions) : non verifie (la recherche n'a trouve qu'un seul nom d'espace).
- Pour L17-02, que `SiegeCheckLibrary` couvre exactement les appels du test (noms identiques, signatures non comparees).
- Pour L17-05, que les tests Minimap testent la regle du script et non `HOLMinimapView.cpp` : non verifie.
- Pour L17-06, que le patron `HOLScriptScreen` convient a un dialogue modal et qui appelle `HOLConfirmDialog` : non verifie.
- Les appelants « Mass » = recherche du nom de fichier dans les fichiers `*Processor*` ; un appel indirect (par un sous-systeme) n'y est pas (cas Terrain, constate par PLAN-1116-E3).
- Un `HOLSiteLayout` sans appelant hors tests peut etre appele par un autre nom ou n'etre qu'un resultat de recherche incomplet.
- Les durees (vagues, heures) supposent 15 minutes par agent, un build de 10 minutes (598 s au pilote chat) et aucune reprise ; l'ordre de grandeur est plausible, pas mesure.
- Le script d'inventaire a ete lance deux fois avec des definitions de ligne differentes (brute / non vide) : les totaux sont a 10 % pres selon la base.
- Chiffres de `Tools\Tests` et de la configuration Angelscript : pas lus (le module `HeroesOfLibertyUI` est-il visible du script : deja note non verifie par S1D-1116a).
