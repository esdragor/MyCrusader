# ETAPE0-1118m : relecture de la file de migration contre le code (reprise 12, 2026-11-18, lecture seule)

Copie principale, main HEAD 4a231ac. Aucun build, aucun test, aucune édition de code. Méthode : grep ciblé dans `Source\` et `Script\`, existence des fichiers, `git log --grep`, lecture des paragraphes D836 / D845 / D850 du `GDD.md` et de `EtapeDefini.md` l. 5-34. Les numéros de ligne de `Auto_Journey_File.md` sont ceux de la demande.

## (a) Lignes `[ ]` / `[~]` de la file

| Ligne | Verdict | Preuve |
|---|---|---|
| 71 MIG-CPP bascules préparées | PÉRIMÉE | Les bascules Relay, Garrison, Camp, Site, Marché (partiel) sont faites (voir 81-88, 117) ; UnitTier est EN COURS vague 1 (l. 89). Plus de « 3 bascules à 21:45 » à tenir. |
| 81 Relay, sens à trancher | FAITE | Sens tranché : D836 point 3 (script source). `Source\HeroesOfLiberty\Simulation\HOLRelayRules.h` (36 l.) ne garde que `FHOLRelayState`, `DefaultHoldSeconds`/`DefaultDecaySteps`, `FillPercent`, `MakeView` ; `HOLRelayRules.cpp` ne définit plus que `FillPercent` (l. 5) et `MakeView` (l. 10) ; `HOLRelayBoard.h` existe ; `Script\Relays\HOLRelayRules.as` est la source. |
| 82 Relay, copie C++ Step/TerritoryXP supprimée (6bed6a3) | FAITE | Même preuve : plus de `HOLRelay::Step`/`TerritoryXP` dans `Source\` (grep). Commit dans main. |
| 83 Garrison, copie C++ supprimée (0196e7b) | FAITE | `HOLGarrisonRuleLibrary.*` absent ; `Buildings\HOLGarrisonRules.h` (30 l.) = `EKind` + `KindOf` seulement ; `HOLGarrisonBoard.h` + `HOLGarrisonBridgeTests.cpp` ; `Script\Garrison\HOLGarrisonRules.as` source. Ordres Garrison/UngarrisonAll prouvés (D850 point 1, EtapeDefini l. 8). |
| 84 Siege, sens non tranché | FAITE | Tranché D836 point 2 : `Script\Siege\HOLSiegeRules.as` ABSENT (reste `HOLSiegeRules_Test.as`) ; `Siege\HOLSiegeRules.h` s'annonce « the only source », `HOLSiegeCheckLibrary.cpp` expose au script (AI\HOLAISiege.as). |
| 88 Market + Tribute | PÉRIMÉE (tranchée D850 point 2) | Copies C++ `Apply`/`Buy` supprimées (GALADRIEL-1117g, EtapeDefini l. 31 « Market Apply en script (pont) »). `Sell`/`BuyWithGold`/`NewBook` restent C++ : `HOLGameScreen.cpp:2381-2382`, `HOLMarketSubsystem.h:43,80` ; D850 point 2 : l'aperçu HUD reste lu en C++, pas de lot. `HOLTributeRules.cpp` n'existe pas (Tribute = script seul). |
| 89 UnitTier, pont + suppression C++ | EN COURS vague 1 (non évaluée) | Constat seul : `Simulation\HOLUnitTierRules.cpp` (35 l.), `HOLUnitTierRulesLibrary.*` encore présents ; `HOLProductionSubsystem.cpp:448` appelle encore `HOLUnitTier::GetMinTier`. |
| 92 UnitAbility, sens | FAITE | D836 point 2 : `Script\Units\HOLUnitAbilityRules.as` ABSENT (reste `Script\Tests\HOLUnitAbilityRules_Test.as`) ; `HOLUnitAbilityRules.h` C++ lu par Mass + `HOLUnitAbilityReadLibrary.cpp`. Non vérifié : `Tools\Tests\unit_ability_accord_test.py` existe encore, peut être périmé. |
| 96 Diplomacy, sens | FAITE | D836 point 5 : `Script\Diplomacy\HOLDiplomacyRules.as` ABSENT ; C++ `Match\HOLDiplomacyRules.cpp` (239 l.) = source ; `HOLDiplomacyReadLibrary` pour l'interface. |
| 97 Diplomacy, partiel | FAITE | Même preuve (ff5f9ec cité l. 113 : « HOLDiplomacyRules.as supprimé, plus aucun appelant »). Reste un `HOLDiplomacyRules_Test.as` dans `Script\Diplomacy\` : non vérifié s'il lit encore `HOLDiplomacyRead` (probable). |
| 106 D844 forêt sans ralentissement | FAITE | 7f789e0 (tests) + 9187d15 (code) dans main ; `HOLTerrainRules.as` l. 13-17 : défauts à 100 (« D844 »). |
| 107 D843 écran Multijoueur | FAITE | 873935f fusionné dans main, essai joué OK (947ccf7, EtapeDefini l. 22) ; `HOLMultiplayerScreen.as` + test. |
| 108 D841 brancher forêt/gué/charge au Mass | FAITE (réserve) | `HOLTerrainSubsystem.cpp:113`, `HOLCombatProcessor.cpp:724`, `HOLTerrainRules.cpp:76-83` lisent la règle ; EtapeDefini l. 31 : « branché au Mass (rouge non vu) ». Le rouge n'a jamais été vu : à ne pas compter comme preuve. |
| 109 D840 écrans par écran | FAITE | Les 3 écrans du plan existent : `HOLTitleScreen.as` (essai OK fa205a9), `HOLSettingsScreen.as` (essai OK 4e18ee5), `HOLMultiplayerScreen.as` (947ccf7). Le HUD / `HOLGameScreen` (Slate C++) n'est pas dans ce plan. |
| 112 [~] pont sonde RPC | PÉRIMÉE | La sonde a été prouvée (41bd12b « P1d proven ») puis remplacée par la vraie chaîne (lots A-E, L2). Le pont vit en `Source\HeroesOfLiberty\Dev\HOLScriptRpcProbeBridge.cpp`. |
| 113 MIG-C++-FINAL | FAITE | (1)(2) : `HOLDiplomacyRules.as` supprimé (l. 113 elle-même). (3) `HOLCampTable.*` absent ; les 4 commentaires périmés (`HOLDeployRules.h:8`, `.cpp:5`, `HOLExplorationXPRules.h:8`) disent déjà « only in C++ ... D836 ». (4) ponts S2 faits (voir 117). Non vérifié : `HOLDeployRulesTests.cpp:9`. |
| 115 MIG2-S1 socle | FAITE | Interface du menu (`HOLGameMenuRulesBridge.*`), bibliothèques de lecture (Diplomacy, Terrain, Deploy, Relay, Victory, Forum, Faction, Gather : EtapeDefini l. 21) existent. Non vérifié : le décompte exact (~28 fn). |
| 117 MIG2-S2 ponts serveur | FAITE | `HOLGarrisonBoard.h`, `HOLCampBoard.h`, `HOLRelayBoard.h`, `HOLSiteBoardSubsystem.h`, `HOLMarketBridgeTypes.h` + `HOLGarrison/Camp/Market/Relay/Site/UnitTierBridgeTests.cpp`. |
| 118 MIG2-S3 pilote + 18 triviales | FAITE | Lots A-E : df4e6ce, 4854085, 6a13554, 8fc4019/102d0f9/bc0aca0/533ff17, 2913a77/7c01f59/3a0ec7b/c9da2da (36 RPC retirées, main 2506f65, EtapeDefini l. 17) ; L2 : 092592d, 5d8b148 (main 1624d41). Les 5 « grosses charges » restent C++ (voir c). |
| 119 [~] T4 L1 Spé + Héros | FAITE | 0df654a dans main ; RPC retirées (lots B et E). |
| 120 [~] T5 L1 Signaux + chat | FAITE | cecc99c dans main ; RPC retirées (lot C). |
| 121 MIG2-S4 restes de tables | RESTE (décision) | `HOLTechnologyTableTests.cpp` (289 l.) et `HOLSiegeTests.cpp` (269 l.) sont toujours en C++, ~25 champs/fonctions non exposés. D838 ne couvre que la simulation Mass : ces deux fichiers testent des tables, pas Mass. |
| 241 MIG-T6-LOTS | FAITE | Lots A-E dans main 2506f65 (36 RPC), commits ci-dessus. |
| 242 MIG-L2-Ordres | FAITE | 092592d, 8d81190 (16 déclarations `Server` retirées), 5d8b148, e5f3d8b ; reprise 10 (5ff5e96) : lot R dans main 1624d41 ; mesure D848, critère B simulé (D850 point 4). |

## (b) Jumeaux D836

| Règle | Source aujourd'hui | Détail |
|---|---|---|
| Relay | Script (`Script\Relays\HOLRelayRules.as`) + résidu C++ | C++ : `Simulation\HOLRelayRules.h/.cpp` = struct, 2 constantes, `FillPercent`, `MakeView` (vue répliquée). Accord des constantes : `Test_RelayConstantsAgreeWithTheSimulation` (cité par l'en-tête, non relancé). |
| Garrison | Script seul | C++ : `EKind`/`KindOf` (fragments Mass), pas de .cpp. |
| Camp | Script seul | `HOLCampRules.h` = vocabulaire, pas de .cpp ; `HOLCampBoard.h`. |
| Site | Script + résidu C++ | `HOLSiteRules.cpp` (35 l.) = `MakeView`, `ResetCounts`, `AddUnit` ; constantes jumelles tenues par un test script + `Tools\Tests\site_count_accord_test.py`. |
| Market | LES DEUX (jumeaux réels) | `Script\Economy\HOLMarketRules.as` (IA, UI) et `Match\HOLMarketRules.cpp` (120 l. : Sell, BuyWithGold, NewBook, Recover, réglages) d'accord par les mêmes chiffres (`HOLMarketRulesTests.cpp` 334 l. + `HOLMarketRules_Test.as`). `Apply`/`Buy` C++ supprimées. Gardé exprès par D850 point 2. |
| Tribute | Script seul | pas de fichier C++. |
| UnitTier | EN COURS vague 1 | (C++ et script encore tous deux présents, `Test_UnitTierScriptAndSimulationRulesAgree` cité.) |
| Siege | C++ seul | copie script supprimée ; script lit par `HOLSiegeCheck`. |
| Technology | C++ seul (règle) | `HOLTechnologyRules.cpp` 185 l. ; `Script\Technologies\HOLTechnologyRules.as` ne garde que les raisons (en-tête l. 7 : « only the reasons live here »). |
| UnitAbility | C++ seul | `HOLUnitAbilityRules.h` ; script = test via `HOLUnitAbilityReadLibrary`. |
| Diplomacy | C++ seul | copie script supprimée ; lecture par `HOLDiplomacyRead`. |
| Terrain | C++ seul pour la simulation | `HOLTerrainRules.h/.cpp` + `HOLTerrainLibrary.cpp` ; `HOLTerrainRules.as` lit les défauts C++ (`HOLTerrainRead::GetDefault...`) : lecteur + test, pas un jumeau. |
| Deploy | C++ seul | script = `HOLDeployRules_Test.as` sur `HOLDeployRead`. |
| ExplorationXP | C++ seul | pas de script. |
| Formation | EN COURS vague 1 | |
| MatchConfig, Construction, SpecializationGrid, CallN/CallEx/CallCheat | EN COURS vague 1 | non regardés |

Autres C++ purs sans jumeau script (hors périmètre D836) : Forum, Victory (bibliothèques de lecture ajoutées reprise 8).

## (c) RPC dans `HOLPlayerController.h`

Reste 5 déclarations (grep `UFUNCTION(Server|Client`), aucune autre :

| Ligne | RPC | Pourquoi gardée |
|---|---|---|
| 626 | `ClientShowDepositPopups` (Client, Unreliable) | PLAN-1118a §2 « NE PAS retirer » L5 : appelée par pas par `HOLGatherProcessor`, structure non `BlueprintType`, non fiable par choix. |
| 633 | `ServerRequestMatchStats` (Server, Reliable) | idem L5 (réplication) ; une des « 5 grosses charges restent en C++ » de D842 (Q2 A) et D845 point 4. |
| 637 | `ClientReceiveMatchStats` (Client, Reliable) | idem |
| 640 | `ClientMatchStatsRefused` (Client, Reliable) | idem |
| 647 | `ClientReceiveFogMemory` (Client, Reliable) | idem |

`HOLScriptRpcPlayerActor.h:17` : l'acteur script des RPC (attendu, n'est pas du C++ à retirer). Les 16 ordres L2 : plus aucune déclaration `Server` (8d81190). Non vérifié : lecture complète du texte de D842 ; l'association « 5 grosses charges = ces 5 » vient de PLAN-1118a et de l'extrait EtapeDefini l. 29.

## (d) RESTE pour finir la migration

1. Vague 1 en cours (EN COURS, ne pas refaire) : UnitTier (supprimer `HOLUnitTierRules.cpp/.h` + `HOLUnitTierRulesLibrary.*`, appeler le script depuis `HOLProductionSubsystem.cpp:448` et `HOLGameScreen.cpp`), Formation, MatchConfig, Construction, grille de Spécialisation, aides CallN/CallEx/CallCheat. C++ de liaison dans UN lot, ~200-400 l. au total ; aucune décision (D836 points 3 et 6 déjà tranchés).
2. S4 (l. 121) : `HOLTechnologyTableTests.cpp` + `HOLSiegeTests.cpp` (~560 l. de test C++). DÉCISION : les déclarer « restent C++ » (comme D838, gain nul, zéro travail) ou exposer ~25 champs (C++ de liaison ~300 l., puis tests en script).
3. Marché : jumeau Script / C++ voulu (D850 point 2). DÉCISION de forme : le déclarer accepté (accord par vecteurs) pour que « une seule source » ne soit plus faux ; rien à coder.
4. Résidus C++ minuscules de Relay et Site (constantes jumelles, MakeView) : acceptés par construction (vue répliquée) ; mention à ajouter dans la clôture, pas de code.
5. Les 5 RPC C++ (c) : DÉCISION de clôture : confirmer qu'elles restent à jamais C++ (réplication, D842) ; sinon chantier réseau hors migration.
6. Hygiène / clôture papier (script, quelques lignes) : cocher dans `Auto_Journey_File.md` les lignes FAITES/PÉRIMÉES de (a) (21 sur 24), vérifier `unit_ability_accord_test.py` et `HOLDiplomacyRules_Test.as` encore utiles, relire `Placeholders.txt`, `EtapeDefini.md`.
7. Hors migration mais ouvert : critère D846 (gigue et perte sur un vrai réseau) avant sortie publique ; rouge non vu du branchement forêt (l. 108).

## Non vérifié

Contenu des tests d'accord eux-mêmes (non lancés), noms exacts des 28 fonctions S1, `HOLDeployRulesTests.cpp:9`, texte complet de D842, présence du rouge du terrain Mass, état exact des travaux en cours des agents de la vague 1.
