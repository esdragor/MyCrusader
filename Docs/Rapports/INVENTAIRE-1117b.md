# INVENTAIRE de la migration des tests C++ -> Angelscript (GLORFINDEL-1117i, 2026-11-17, lecture seule)

Base : branche net-playtest + `git merge main` (HEAD 8d8411a). `Source\HeroesOfLiberty\Tests\` : **107 fichiers, 545 tests, 42 557 lignes** (aucun sous-dossier, aucun `.h`). Script : ~105 `*_Test.as`.
Classes : **A** simulation Mass / monde complet (reste C++, D838) ; **B** portable avec une bibliotheque DEJA exposee, equivalent script existant (retirable maintenant apres rouge/vert) ; **C** portable apres exposition d'une bibliotheque minimale ; **D** Slate / UI / replication / actifs / liaison C++ -> script (reste C++).
Methode : comptage `IMPLEMENT_*AUTOMATION_TEST`, includes (Mass, GetWorld, Level, Slate, replication), nom des tests C++ recherche dans `Script\**\*.as`, en-tetes des `*_Test.as` (« port of ... »). Les tailles de bibliotheque sont des ESTIMATIONS d'apres les en-tetes (.h/.cpp) ; non verifiees par un build. Aucun code modifie.

| Classe | Fichiers | Tests | Lignes |
|---|---|---|---|
| A | 52 | 283 | 24 437 |
| B | 9 | 16 | 1 171 |
| C | 18 | 169 | 9 147 |
| D | 28 | 77 | 7 802 |

Retirable maintenant (B, sans C++ neuf) : environ 15 tests / 1 100 lignes (la classe B ci-dessus compte les fichiers entiers, dont certains a verifier). Retirable apres une bibliotheque (C) : 169 tests / 9 147 lignes pour environ 2 000 lignes de bibliotheque C++ (un lot, un build).
Hors tableau : les tests « chaine » de l'IA (`AI.Opening.*`, `AI.Army.*`, `AI.Brain.*`, `AI.Summary.*`) sont les seuls a prouver C++ -> script -> ordres -> files : ils RESTENT en A, leurs regles pures sont deja en `AI\*_Test.as`. Les 8 tests `*.ServerBoard` (Camp, Chat, Garrison, Market, Relay, Signal, Site, Specialization) prouvent le pont C++ -> script (`*BoardScript.as`) : D, ils restent.

## Les 5 taches les plus rentables pour la prochaine vague (enonces d'agent de 15 min)

1. **MIG-B-SWEEP (0 ligne C++, ~15 tests, ~1 100 lignes).** « Lis brief_commun.md. Worktree X. Pour chaque ligne : verifie que le test script equivalent est complet (memes vecteurs), fausse une valeur attendue (rouge vu), remets (vert), puis supprime le test C++ : `HOLSiegeTests.cpp` (4 : Siege.Table/Combat/Technologies/TechnologyHealth -> `Siege\HOLSiegeRules_Test.as`, `Tests\HOLSiege_Test.as`), `HOLMatchStatsTests.cpp` (3 -> `Tests\HOLMatchStats_Test.as`), `HOLFactionChoiceTests.cpp` (1 -> `HOLFactionChoice_Test.as`), `HOLMarketTableTests.cpp` (1 -> `HOLMarketRules_Test.as`), `HOLMapRulesTests.cpp` (1 -> `HOLMapRules_Test.as`, dire si la partie « recherche » reste), `HOLCampRulesTests.cpp` (1 -> `HOLCampRules_Test.as`), `HOLTechnologyTableTests.cpp` (Technologies.Rules -> `HOLTechnologyRules_Test.as`, verifier Playable), `HOLMarketRulesTests.cpp` (Market.Rules seul), `HOLHeroReturnTests.cpp` (Hero.ReturnQuote, ReturnOrderPrice -> `HOLHeroReturnQuote_Test.as`). Un fichier devenu vide est supprime. Pas de build : donne la commande quick_check. » Effort tres faible ; benefice immediat, aucune dependance.
2. **MIG-C-MapSize (~60 lignes C++, 12 tests, ~600 lignes).** « Expose en lecture les fonctions de `HOLMapSizeRules.h` (109 lignes) que `HOLMapSizeTests.cpp` (8) et `HOLMapSizeLobbyTests.cpp` (4) appellent, dans `UHOLMapRulesReadLibrary` (ou `UHOLMapSizeReadLibrary`, modele `HOLMapRulesReadLibrary.h`), ecris `Script\Tests\HOLMapSize_Test.as` (memes vecteurs, rouge vu), liste les fichiers de config (Paths/FileHelper) que les tests lisent et dis si le script sait les lire. Ne supprime les C++ qu'apres vert. Lot C++ unique, pas de build. » Effort faible (meilleur ratio des lots C).
3. **MIG-C-Volcano (~120 lignes C++, 17 tests, 895 lignes).** « `UHOLVolcanoLibrary` expose 5 fonctions ; `HOLVolcanoTests.cpp` (17 tests : CatastropheCycle, LavaSpeed, FlowFront, LavaReach, ObstacleBoxes, Eruption*, DrawFlow, SiteState, MatchSeed, EvacuationPoint, Altar*, FreshDeposit) appelle `HOLVolcanoRules.h` (357 lignes). Liste les fonctions de `HOLVolcano::` appelees par ce fichier et absentes de la bibliotheque, expose-les, ecris `Script\Tests\HOLVolcanoRules_Test.as` ; les cas Altar* sont deja couverts en partie par `HOLAIAltar_Test.as` (n'en refais pas). Rouge vu puis vert, puis retire les C++. » Effort moyen.
4. **MIG-C-PetitesRegles (~200 lignes C++, 19 tests, ~980 lignes).** « Une bibliotheque de lecture pour trois petites regles pures : `HOLHeroXPRules.h` (100 l.) -> `HOLHeroXPTests.cpp` (6), `HOLHealRules.h` (87 l.) + champs lus de `HOLCombatRules` + `UHOLTechnologyCheckLibrary` deja exposee -> `HOLMonkTests.cpp` (5), `HOLGroupMoveRules.h` (47 l.) + `HOLUnitPicking.h` (139 l.) -> `HOLMovementTests.cpp` (8). Ecris les trois `*_Test.as`, rouge vu, vert, puis retire les C++. Un seul lot C++ (un build, a demander a la session principale). » Effort moyen.
5. **MIG-C-Mouvement (~220 lignes C++, 16 tests, ~970 lignes).** « `HOLAvoidRules.h` (194 l.) -> `HOLAvoidTests.cpp` (9) et `HOLSanityRules.h` (317 l.) -> `HOLSanityTests.cpp` (7) : regles pures appelees par entite et par pas (elles restent en C++, on ne les expose qu'en lecture : `UHOLAvoidReadLibrary` / `UHOLSanityReadLibrary`). Ecris les deux `*_Test.as`, rouge vu, vert, retire les C++. » Effort moyen.

Vague d'apres (meme patron, plus lourd, ~50 tests / 3 060 lignes pour ~700 lignes de bibliotheque) : `HOLConstructionTests.cpp` (20 ; `HOLConstructionRules.h` 482 l., `UHOLProductionReadLibrary` deja la), `HOLFormationTests.cpp` (15 ; `HOLFormationRules` 428 l. + 1153), `HOLCommandGridTests.cpp` (15 ; `HOLCommandGrid.h` 488 l.). Puis `HOLForumMessagesTests.cpp` (10, ~100 l.), `HOLManagerClientTests.cpp` (10, ~150 l., JSON), `HOLVisionTests.cpp` (7 tests purs sur 19, ~100 l.), `HOLOrderChainTests.cpp` (1 ; exposer `EndBuildingTarget`, ~25 l.), `HOLMatchConfigTests.cpp` (11 ; verifier d'abord si `FHOLMatchConfig` est deja visible du script, auquel cas c'est du B), `HOLProfaneTests.cpp` (2), `HOLSourceMassifsTests.cpp` (2).

## Tableau trie par benefice (tests retirables / effort de bibliotheque)

| Rang | Lot | Tests retirables | Lignes C++ retirables | Lib a exposer (est.) | Tests par 100 lignes de lib |
|---|---|---|---|---|---|
| 1 | B : sweep (Siege, MatchStats, FactionChoice, MarketTable, MapRules, CampRules, TechnologyTable, MarketRules.Rules, HeroReturn x2) | ~15 | ~1 100 | 0 | sans objet |
| 2 | C : MapSize + MapSizeLobby | 12 | ~600 | ~60 | 20 |
| 3 | C : Volcano rules | 17 | 895 | ~120 | 14 |
| 4 | C : HeroXP + Monk + Movement | 19 | ~980 | ~200 | 9,5 |
| 5 | C : Avoid + Sanity | 16 | 972 | ~220 | 7 |
| 6 | C : ForumMessages | 10 | 460 | ~100 | 10 |
| 7 | C : Construction | 20 | 1 174 | ~250 | 8 |
| 8 | C : CommandGrid | 15 | 926 | ~200 | 7,5 |
| 9 | C : ManagerClient | 10 | 665 | ~150 | 6,7 |
| 10 | C : Formation | 15 | 961 | ~250 | 6 |
| 11 | C : Vision (7 tests purs) | 7 | ~300 | ~100 | 7 |
| 12 | C : OrderChain.BuildingQueue | 1 | 171 | ~25 | 4 |

(Le rang 4 et 5 sont gardes ensemble dans la vague parce que les regles sont deja lues par la meme famille de bibliotheques ; ForumMessages a un meilleur ratio que Avoid + Sanity mais sort du patron « petites regles ».)

## Detail par fichier

Colonnes : fichier | tests | lignes | classe | equivalent script ou bibliotheque exposee, ou raison.

| Fichier | Tests | Lignes | Classe | Note |
|---|---|---|---|---|
| HOLBiomeConfigTests.cpp | 2 | 101 | B | UHOLBiomeReadLibrary + Script/Biomes/*_Test.as ; lit un JSON : a verifier |
| HOLCampRulesTests.cpp | 1 | 41 | B | HOLCampRules_Test.as (Neutrals.Owner) : retirable |
| HOLFactionChoiceTests.cpp | 1 | 34 | B | HOLFactionChoice_Test.as (Net.FactionChoice.Bodies), UHOLFactionChoiceReadLibrary : retirable |
| HOLMapRulesTests.cpp | 1 | 67 | B | HOLMapRules_Test.as + S1dLibraries2 (Map.Rules) ; la partie recherche resterait : verifier |
| HOLMarketTableTests.cpp | 1 | 74 | B | HOLMarketRules_Test.as (Market.Table) : retirable |
| HOLMatchStatsTests.cpp | 3 | 211 | B | HOLMatchStats_Test.as (Rules, Send, Scores, Curves), UHOLMatchStatsReadLibrary : retirable |
| HOLSiegeTests.cpp | 4 | 269 | B | HOLSiegeRules_Test.as + HOLSiege_Test.as (4 noms retrouves), UHOLSiegeCheckLibrary : retirable |
| HOLStartingPeasantsTests.cpp | 1 | 85 | B | UHOLStartingPeasantsReadLibrary ; HOLStartRules_Test.as porte Locations, pas Count : a verifier |
| HOLTechnologyTableTests.cpp | 2 | 289 | B | HOLTechnologyRules_Test.as couvre Technologies.Rules (1 certain, Playable a verifier), UHOLTechnologyCheckLibrary |
| HOLAvoidTests.cpp | 9 | 390 | C | HOLAvoidRules.h 194 l. |
| HOLCommandGridTests.cpp | 15 | 926 | C | HOLCommandGrid.h 488 l. |
| HOLConstructionTests.cpp | 20 | 1174 | C | HOLConstructionRules.h 482 l. + UHOLProductionReadLibrary deja la |
| HOLFormationTests.cpp | 15 | 961 | C | HOLFormationRules.h 428 l. + cpp 1153 l. (RandomStream) |
| HOLForumMessagesTests.cpp | 10 | 460 | C | HOLForumMessages.h 307 l. (culture) |
| HOLHeroXPTests.cpp | 6 | 201 | C | UHOLHeroLibrary partielle ; HOLHeroXPRules.h 100 l. |
| HOLManagerClientTests.cpp | 10 | 665 | C | HOLManagerClientRules (JSON) ; HOLGameMenuRules deja en script |
| HOLMapSizeLobbyTests.cpp | 4 | 190 | C | HOLMapSizeRules + ManagerClientRules (JSON de salon) |
| HOLMapSizeTests.cpp | 8 | 410 | C | HOLMapSizeRules.h 109 l. |
| HOLMatchConfigTests.cpp | 11 | 287 | C | verifier si FHOLMatchConfig est deja visible du script ; sinon exposer ~80 l. ; GameInstance/fichiers |
| HOLMonkTests.cpp | 5 | 248 | C | UHOLTechnologyCheckLibrary deja la ; HealRules.h 87 l. + champs CombatRules |
| HOLMovementTests.cpp | 8 | 529 | C | GroupMoveRules + UnitPicking + VictoryRules (camps) |
| HOLOrderChainTests.cpp | 1 | 171 | C | HOLOrderChain_Test.as : tout porte SAUF EndBuildingTarget (template TWeakObjectPtr) ; exposer ~25 l. |
| HOLProfaneTests.cpp | 2 | 113 | C | CombatRules + table, petit |
| HOLSanityTests.cpp | 7 | 582 | C | HOLSanityRules.h 317 l. |
| HOLSourceMassifsTests.cpp | 2 | 249 | C | HOLGatherRules (RandomStream), ~40 l. |
| HOLVisionTests.cpp | 19 | 696 | C | 7 tests purs (Grid, Disc, Forest, Height, Counts, TreeCut, SightRadius), 12 sur monde/acteurs |
| HOLVolcanoTests.cpp | 17 | 895 | C | UHOLVolcanoLibrary (5 fonctions) ; Altar* en partie dans HOLAIAltar_Test.as |
| HOLAnimationTests.cpp | 4 | 500 | D | AnimSequence / DataTable / actifs |
| HOLBuildingRegistryTests.cpp | 1 | 116 | D | acteurs |
| HOLBuildingTests.cpp | 11 | 1069 | D | acteurs, meshes, collision |
| HOLCameraTests.cpp | 6 | 172 | D | HOLRTSCamera, GameUserSettings |
| HOLCampBridgeTests.cpp | 2 | 104 | D | pont C++ -> HOLCampBoardScript.as |
| HOLCaptureSeqTests.cpp | 1 | 41 | D | outillage de capture |
| HOLChatTransportTests.cpp | 2 | 96 | D | pont C++ -> HOLChatBoardScript.as (+ ThreadLine) |
| HOLDecorClearTests.cpp | 1 | 101 | D | ISM / materiaux |
| HOLFrontEndTests.cpp | 2 | 168 | D | texte localise, GameUserSettings |
| HOLGarrisonBridgeTests.cpp | 1 | 45 | D | pont C++ -> HOLGarrisonBoardScript.as |
| HOLMapColsTests.cpp | 1 | 308 | D | construit un niveau (ULevel + StaticMeshActor) : plus tard, bibliotheque de dispositions de carte (7 tests / ~3 400 lignes pour le groupe) |
| HOLMapFordsTests.cpp | 1 | 334 | D | construit un niveau (ULevel + StaticMeshActor) : plus tard, bibliotheque de dispositions de carte (7 tests / ~3 400 lignes pour le groupe) |
| HOLMapForestTests.cpp | 1 | 160 | D | construit un niveau (ULevel + StaticMeshActor) : plus tard, bibliotheque de dispositions de carte (7 tests / ~3 400 lignes pour le groupe) |
| HOLMapHinterlandTests.cpp | 2 | 582 | D | construit un niveau (ULevel + StaticMeshActor) : plus tard, bibliotheque de dispositions de carte (7 tests / ~3 400 lignes pour le groupe) |
| HOLMapSizeMapTests.cpp | 1 | 139 | D | construit un niveau (ULevel + StaticMeshActor) : plus tard, bibliotheque de dispositions de carte (7 tests / ~3 400 lignes pour le groupe) |
| HOLMapSizeStartTests.cpp | 1 | 103 | D | GameState + StartSlot, monde |
| HOLMapSteppeTests.cpp | 1 | 581 | D | construit un niveau (ULevel + StaticMeshActor) : plus tard, bibliotheque de dispositions de carte (7 tests / ~3 400 lignes pour le groupe) |
| HOLMapTableTests.cpp | 1 | 869 | D | construit un niveau (ULevel + StaticMeshActor) : plus tard, bibliotheque de dispositions de carte (7 tests / ~3 400 lignes pour le groupe) |
| HOLMarketBridgeTests.cpp | 1 | 100 | D | pont C++ -> HOLMarketBoardScript.as |
| HOLNetTests.cpp | 21 | 1224 | D | replication Iris, serialiseurs |
| HOLRelayBridgeTests.cpp | 1 | 68 | D | pont C++ -> HOLRelayBoardScript.as |
| HOLScenarioNoteTests.cpp | 5 | 177 | D | outillage de capture (TestHooks) |
| HOLSignalBoardTests.cpp | 1 | 80 | D | pont C++ -> HOLSignalBoardScript.as |
| HOLSignalGroundMarkTests.cpp | 1 | 87 | D | ISM, controleur |
| HOLSiteBridgeTests.cpp | 1 | 80 | D | pont C++ -> HOLSiteBoardScript.as |
| HOLSourceLookTests.cpp | 1 | 141 | D | ISM / materiaux |
| HOLSourceNavTests.cpp | 1 | 138 | D | composant de navigation |
| HOLSpecializationBoardTests.cpp | 4 | 219 | D | pont C++ -> HOLSpecializationBoardScript.as |
| HOLAIArmyTests.cpp | 4 | 293 | A | Mass / monde complet (D838) |
| HOLAIBrainTests.cpp | 5 | 610 | A | Mass / monde complet (D838) |
| HOLAIBuildOrdersTests.cpp | 3 | 227 | A | Mass / monde complet (D838) |
| HOLAIForumTests.cpp | 5 | 308 | A | Mass / monde complet (D838) |
| HOLAINomadTests.cpp | 2 | 177 | A | Mass / monde complet (D838) |
| HOLAIOpeningTests.cpp | 13 | 817 | A | Mass / monde complet (D838) |
| HOLAIOrdersTests.cpp | 8 | 478 | A | Mass / monde complet (D838) |
| HOLAISlotTests.cpp | 3 | 476 | A | Mass / monde complet (D838) |
| HOLAISummaryTests.cpp | 9 | 554 | A | Mass / monde complet (D838) |
| HOLCampStateTests.cpp | 2 | 315 | A | CampStateDoors_Test.as ne fait que compiler ; l effet reste verifie ici |
| HOLCombatTests.cpp | 19 | 1417 | A | Mass / monde complet (D838) |
| HOLDamnedWarriorTests.cpp | 3 | 304 | A | Mass / monde complet (D838) |
| HOLDeployButtonTests.cpp | 2 | 313 | A | Mass / monde complet (D838) |
| HOLDeployInMatchTests.cpp | 2 | 345 | A | Mass / monde complet (D838) |
| HOLDeployProcessorTests.cpp | 1 | 132 | A | Mass / monde complet (D838) |
| HOLDiplomacyAlliesTests.cpp | 6 | 448 | A | Mass / monde complet (D838) |
| HOLDiplomacyBranchTests.cpp | 5 | 655 | A | Mass / monde complet (D838) |
| HOLDiplomacyStateTests.cpp | 1 | 163 | A | Mass / monde complet (D838) |
| HOLDisconnectDisplayTests.cpp | 4 | 146 | A | Mass / monde complet (D838) |
| HOLDisconnectTests.cpp | 15 | 1168 | A | Mass / monde complet (D838) |
| HOLDuelOrderTests.cpp | 2 | 192 | A | Mass / monde complet (D838) |
| HOLDuelProtectionTests.cpp | 5 | 745 | A | Mass / monde complet (D838) |
| HOLExplorationXPTests.cpp | 1 | 151 | A | Mass / monde complet (D838) |
| HOLFactionRulesTests.cpp | 8 | 795 | A | Mass / monde complet (D838) |
| HOLFarmBuildersTests.cpp | 3 | 406 | A | Mass / monde complet (D838) |
| HOLGarrisonOrderTests.cpp | 7 | 962 | A | Mass / monde complet (D838) |
| HOLGatherTests.cpp | 12 | 1278 | A | Mass / monde complet (D838) |
| HOLHealSleepTests.cpp | 1 | 120 | A | Mass / monde complet (D838) |
| HOLHeroAbilityMatchTests.cpp | 2 | 276 | A | Mass / monde complet (D838) |
| HOLHeroAbilityOrderTests.cpp | 1 | 102 | A | Mass / monde complet (D838) |
| HOLHeroReturnTests.cpp | 15 | 967 | A | Hero.ReturnQuote et ReturnOrderPrice portes (HOLHeroReturnQuote_Test.as) : 2 retirables ; le reste = simulation |
| HOLHeroSheetTests.cpp | 3 | 352 | A | Mass / monde complet (D838) |
| HOLHeroTalentEffectTests.cpp | 3 | 415 | A | Mass / monde complet (D838) |
| HOLHeroTalentOrderTests.cpp | 2 | 134 | A | Mass / monde complet (D838) |
| HOLHeroTests.cpp | 5 | 480 | A | Mass / monde complet (D838) |
| HOLHeroXPMatchTests.cpp | 5 | 393 | A | Mass / monde complet (D838) |
| HOLMarketRulesTests.cpp | 4 | 416 | A | Market.Rules et Tribute (tete pure) portes dans HOLMarketRules_Test.as : 1 retirable ; BookState et Chain (monde) restent |
| HOLProductionPopulationEndTests.cpp | 1 | 173 | A | Mass / monde complet (D838) |
| HOLRegicideTests.cpp | 8 | 498 | A | Mass / monde complet (D838) |
| HOLRelayLayoutTests.cpp | 3 | 133 | A | Mass / monde complet (D838) |
| HOLSignalSpectatorTests.cpp | 1 | 93 | A | Mass / monde complet (D838) |
| HOLSignalThreadTests.cpp | 1 | 58 | A | Mass / monde complet (D838) |
| HOLSiteSubsystemTests.cpp | 10 | 632 | A | Mass / monde complet (D838) |
| HOLSourceGridTests.cpp | 2 | 219 | A | Mass / monde complet (D838) |
| HOLStartingHeroTests.cpp | 9 | 779 | A | Mass / monde complet (D838) |
| HOLTerrainTests.cpp | 4 | 419 | A | Terrain.Rules a un jumeau (HOLTerrainRules_Test.as), les 3 autres sont des pas Mass |
| HOLTierCheatTests.cpp | 1 | 55 | A | Mass / monde complet (D838) |
| HOLUnitTierProductionTests.cpp | 1 | 222 | A | HOLUnitTierProduction_Test.as = moitie decision seulement ; la chaine reelle reste |
| HOLVictoryTests.cpp | 19 | 1734 | A | Mass / monde complet (D838) |
| HOLVisionMatchTests.cpp | 15 | 884 | A | Mass / monde complet (D838) |
| HOLVolcanoLayoutTests.cpp | 6 | 276 | A | Mass / monde complet (D838) |
| HOLVolcanoWorldTests.cpp | 11 | 732 | A | Mass / monde complet (D838) |
