# BILAN-CPP-1119 : ce qui reste en C++ après la migration (lecture seule, HALDIR-1119cn, 18:48-18:55)

Code mesuré : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche integ-21 (tête `d70c184`). Aucune édition, aucun build.
MESURÉ = compté par commande aujourd'hui (lignes = `ReadAllLines().Length`, fichiers `.cpp` / `.h` / `.as`). ESTIMÉ = jugement ou repris d'un plan. Les chiffres de fonctions de `HOLPlayerController.cpp` et `HOLGameState.cpp` sont mesurés par plage entre deux débuts de fonction (une plage peut englober du code qui n'est pas la fonction, voir § 3a).

## 1. Totaux et tableau par dossier

| Ensemble | Fichiers | Lignes | Statut |
|---|---|---|---|
| C++ hors tests (`Source\`, `.cpp` + `.h` + 6 `.cs` de build, ~137 l.) | 537 | 119 510 | MESURÉ |
| C++ de tests (`HeroesOfLiberty\Tests` 98 fichiers 36 505 l. + `HeroesOfLibertyUI\Tests` 8 fichiers 893 l.) | 106 | 37 398 | MESURÉ |
| Script hors tests (`Script\`, `.as`) | 239 | 49 123 | MESURÉ |
| Script de tests (`Script\Tests`) | 104 | 21 417 | MESURÉ |
| Script total | 343 | 70 540 | MESURÉ |

Rapport script / C++ : 70 540 / (119 510 + 37 398) = 0,45 (MESURÉ, calculé). Hors tests : 49 123 script contre 119 510 C++.

### Module HeroesOfLiberty (hors tests)

| Dossier | .cpp (fich. / l.) | .h (fich. / l.) | Total l. |
|---|---|---|---|
| (racine du module) | 6 / 1 265 | 7 / 296 | 1 561 |
| AI | 6 / 1 372 | 6 / 999 | 2 371 |
| Buildings | 14 / 4 756 | 14 / 2 510 | 7 266 |
| Commands | 6 / 6 025 | 9 / 1 449 | 7 474 |
| Dev | 10 / 3 320 | 7 / 563 | 3 883 |
| Map | 19 / 3 866 | 19 / 1 872 | 5 738 |
| Match | 38 / 8 337 | 48 / 6 059 | 14 396 |
| Net | 15 / 4 486 | 16 / 1 967 | 6 453 |
| Player | 13 / 9 463 | 14 / 3 010 | 12 473 |
| Presentation | 8 / 1 929 | 9 / 648 | 2 577 |
| Resources | 7 / 1 348 | 7 / 689 | 2 037 |
| Settings | 1 / 97 | 1 / 96 | 193 |
| Siege | 4 / 539 | 4 / 482 | 1 021 |
| Simulation | 57 / 21 511 | 66 / 9 854 | 31 365 |
| StressTest | 1 / 1 969 | 1 / 246 | 2 215 |
| Technologies | 4 / 385 | 4 / 347 | 732 |
| Tests (à part) | 96 / 35 744 | 2 / 761 | 36 505 |

### Module HeroesOfLibertyUI

| Dossier | .cpp (fich. / l.) | .h (fich. / l.) | Total l. |
|---|---|---|---|
| (racine) | 1 / 6 | 1 / 6 | 12 |
| Dev | 7 / 1 024 | 1 / 31 | 1 055 |
| HUD | 15 / 5 273 | 16 / 1 974 | 7 247 |
| Input | 1 / 35 | 1 / 29 | 64 |
| Menu | 2 / 99 | 2 / 45 | 144 |
| Net | 3 / 895 | 3 / 281 | 1 176 |
| Screens | 9 / 4 193 | 9 / 1 573 | 5 766 |
| Style | 2 / 224 | 2 / 262 | 486 |
| Widgets | 8 / 1 106 | 8 / 562 | 1 668 |
| Tests (à part) | 7 / 831 | 1 / 62 | 893 |

Il n'y a pas d'autre module C++ dans `Source\` (3 fichiers `*.Target.cs` et les `Build.cs` seulement). MESURÉ.

### Script par dossier de `Script\` (fichiers / lignes, MESURÉ)

AI 43 / 13 326 ; UI 72 / 13 880 ; Player 18 / 3 996 ; Duel 10 / 2 134 ; Biomes 11 / 2 068 ; Audio 13 / 1 796 ; Net 3 / 1 722 ; Economy 6 / 1 138 ; Diplomacy 4 / 1 035 ; Match 2 / 516 ; Commands 5 / 986 ; Visual 4 / 982 ; Neutrals 8 / 919 ; Talents 5 / 814 ; autres dossiers (Buildings, Chat, Fog, Garrison, Hero, Input, Morale, Nomad, Orders, Relays, SecondaryCentres, Siege, Signals, Sites, Specialization, Technologies, Terrain, Units) 1 à 5 fichiers chacun, 31 à 446 l. ; Tests 104 / 21 417.

Dans `Commands\` (C++, 7 474 l.), `HOLUnitOrderProcessor.cpp` seul pèse 4 495 l. (Mass) ; le reste de la couche commande (`HOLCommandScriptLibrary`, `HOLPlayerOrders*`, `HOLCommandSubsystem`) fait 1 929 l. cpp. Plus aucun `*Native(` dans le C++ hors tests : le retrait natif de la grille (L2-L3b) est donc bien dans integ-21 (MESURÉ : 0 occurrence).

## 2. Les 25 plus gros fichiers `.cpp` hors tests

Marqueurs comptés par fichier entier (MESURÉ) : M = `UMassProcessor|FMass|UMassObserver`, R = `DOREPLIFETIME|UFUNCTION(Server|Client|NetMulticast|GetLifetimeReplicatedProps`, W = widgets UMG (`UUserWidget|UImage|UTextBlock|UCanvasPanel|WidgetTree`). La colonne Raison est un JUGEMENT (ESTIMÉ) fondé sur ces marqueurs et l'en-tête, pas sur le corps.

| # | Fichier | Lignes | Marqueurs | Raison de rester en C++ |
|---|---|---|---|---|
| 1 | Player\HOLPlayerController.cpp | 5 393 | R 3, Script 20 | Mixte : entrée / sélection (Slate-base côté joueur), RPC (R 3), consoles ; contient des `Request<X>` / `Do<X>` = RÈGLE DE JEU encore en C++ : candidat partiel (§ 3a) |
| 2 | Commands\HOLUnitOrderProcessor.cpp | 4 495 | M 148 | Mass (par entité et par pas) |
| 3 | Simulation\HOLCombatProcessor.cpp | 3 261 | M 27 | Mass |
| 4 | Simulation\HOLSimulationProcessors.cpp | 2 756 | M 59 | Mass |
| 5 | HeroesOfLibertyUI\HUD\HOLGameScreen.cpp | 2 012 | W 46, Slate 9, Script 83 | Widget de base ; le contenu de `RefreshGrid` est un candidat (ESTIMÉ -140 net, plan C6) |
| 6 | StressTest\HOLStressTestSubsystem.cpp | 1 969 | Perf 158, M 37 | Mesure de performance |
| 7 | Match\HOLGameState.cpp | 1 823 | R 28, Ask 10, Script 15 | Réplication (155 l.) + décisions encore C++ (§ 3b) : candidat partiel |
| 8 | Simulation\HOLGatherProcessor.cpp | 1 520 | M 30 | Mass |
| 9 | Simulation\HOLUnitSubsystem.cpp | 1 497 | M 41 | Mass (naissance / registre d'unités) |
| 10 | Player\HOLSelectionMarkers.cpp | 1 453 | Tick 2, Rules 9 | Rendu des marqueurs de sélection (par image, matériaux / plans du moteur) : visuel, non classé par D839 ; NON JUGÉ sans lire le corps |
| 11 | HeroesOfLibertyUI\Screens\HOLNewGameScreen.cpp | 1 413 | W 53, Rules 20 | Widget de base ; décisions déjà par `*ReadLibrary` / script (NG-1..6b) ; reste la construction des widgets |
| 12 | Buildings\HOLBuilding.cpp | 1 247 | R 15 | Acteur répliqué (réplication) |
| 13 | Simulation\HOLFormationRules.cpp | 1 153 | aucun | Règle pure appelée par `HOLUnitOrderProcessor` (Mass) et `HOLSelectionMarkers` ; lue par script via `HOLFormationReadLibrary` : reste car appelée par pas |
| 14 | Simulation\HOLBuildProcessor.cpp | 1 056 | M 17 | Mass |
| 15 | HeroesOfLibertyUI\HUD\HOLScriptScreen.cpp | 1 049 | W 19, Script 94 | Widget de base + liaison script |
| 16 | HeroesOfLibertyGameModeBase.cpp | 1 043 | Subsystem 47, Rules 24 | Orchestration de partie (démarrage, connexion) ; règles de jeu mêlées : candidat, NON JUGÉ sans corps |
| 17 | Map\HOLVisionSubsystem.cpp | 1 009 | Perf 4, Rules 18 | Calcul de visibilité par pas (appelé par entité et par pas, ESTIMÉ) |
| 18 | Dev\HOLBuildingCrowdTest.cpp | 898 | M 4 | Outil de test de la simulation Mass (D838) |
| 19 | Presentation\HOLInterpolatedInstancesProcessor.cpp | 878 | M 60 | Mass (présentation par pas) |
| 20 | Dev\HOLTestSequence.cpp | 876 | Perf 4 | D853 |
| 21 | Simulation\HOLParallelMassProcessors.cpp | 863 | M 110 | Mass |
| 22 | Player\HOLControllerCommandLibrary.cpp | 831 | aucun | Liaison exposée au script (`UHOLControllerCommandLibrary`) |
| 23 | Simulation\HOLHeroSubsystem.cpp | 791 | M 3, 9 consoles, Subsystem 111 | Sous-système du héros : mélange registre et règles ; NON JUGÉ sans corps (candidat possible) |
| 24 | Net\HOLUnitReceptionSubsystem.cpp | 761 | M 22, Perf 4 | Réplication des unités / Mass |
| 25 | Buildings\HOLConstructionRules.cpp | 748 | aucun | Règle pure appelée par `HOLUnitOrderProcessor` (Mass), `HOLBuilding`, `HOLAIOrders`, `HOLProductionSubsystem` et lue par script (`HOLConstructionReadLibrary`) |

Lignes des 25 : somme MESURÉE = 40 795 (ESTIMÉ : 20 fichiers sur 25 ont une raison D839 claire ; 3 restent « non jugés » : n° 10, 16, 23 ; n° 1 et 7 sont mixtes).

## 3. Candidats restants, chiffrés

### 3a. `HOLPlayerController.cpp` (5 393 l., MESURÉ ; 215 fonctions détectées, 4 689 l. dans des fonctions, 705 l. avant la première = en-tête, consoles, cvars)

| Famille | Fonctions | Lignes (MESURÉ par plages) | Remarque |
|---|---|---|---|
| `Request<X>` | 51 | 529 (~10 l. chacune) | Appelants C++ non recomptés (ESTIMÉ : C11) |
| `Do<X>` (corps serveur) hors triche | 29 | 437 | 43 corps `Do<X>` au total confirmés (MESURÉ) ; 4 seulement mentionnent `Board|Script|Library` dans leur corps |
| `Do<X>` triche (`DoCheat*`) | 14 | 593 en plage, dont ~300 sont le `namespace HOLRpcScriptRoute_Haldir1117n` (L3723-4025) qui suit `DoCheatHoldPresence` (corps réel ~25 l., plage 328) | Corps `Do<X>` réels : 702 l. mesurées hors HoldPresence + ~25 = ~727 l. pour 43 corps |
| Sélection / souris / groupes de contrôle (`Select*`, `Handle*`, `ControlGroup`, `Purge`, `Describe`) | 33 | 965 | Entrée joueur |
| Grille / contexte (`ExecuteGridCommand` 217, `GetGridCommands` 91, `GetUnitsContext` 59, ...) | 8 | 453 | |
| Ordres au sol (`OrderAt` 179, `AttackMoveAt` 44, ...) | 3 | 258 | |
| Placement (fantôme) | 7 | 188 | |
| Cycle de vie (Setup, BeginPlay, PlayerTick) | 4 | 231 | |
| RPC `_Implementation` / `Server` / `Client` | 7 | 120 | Réplication, reste |
| Autres (59 fonctions) | 59 | 915 | Non ventilé |
| Consoles | 23 `FAutoConsoleCommand` + 6 cvars | ~600 l. de L100 à L705 (ESTIMÉ : plage du préambule) | 5 sont des rapports, 4 des gestionnaires d'entrée ; Group / Army / Category / Hero / Subgroup non attribués à un lot |

Total des consoles `FAutoConsoleCommand` dans tout le C++ hors tests : 129 (PlayerController 23, HeroSubsystem 9, UIOpen 8, UIHUD 6, SourceSubsystem 6, GameModeBase 4, CheatRules 4, Sanity 4, ...), MESURÉ.

Réponse chiffrée à la question ouverte sur les `Do<X>` : brut maximal migrable ~727 l. (43 corps) + 529 l. (51 `Request<X>`) = ~1 256 l. brutes. Le net est plus petit (liaison et enveloppe de remplacement restent) : le plan C3-C5 l'estime à -440 net pour l'option A (ESTIMÉ, non vérifié). Les corps `DoCheat*` (hors HoldPresence) pèsent ~265 l. : risque réseau nul (triches de dev) ; les `Do` de jeu (déplacements, production, héros, tribut) : risque RÉSEAU.

### 3b. `HOLGameState.cpp` (1 823 l., MESURÉ ; 99 fonctions, 1 776 l. dans des fonctions)

| Famille | Fonctions | Lignes | Décide quelque chose ? |
|---|---|---|---|
| Réplication (`OnRep_*`, `GetLifetimeReplicatedProps` 33, multicast, EndPlay, steps) | 15 | 155 | non (reste, D839) |
| Forum (`AdvanceForums` 99, `HandleForumFall` 44, `HandleKingDeath` 25, `SkipForumPhase` 29, `SetForumPresenceOverride` 30, état, présence) | 21 | 425 | oui : AdvanceForums est appelé par pas ; la porte de chute / roi est déjà au script (S2c) |
| Fin de partie / défaite (`EvaluateMatchEnd` 58, `DefeatSlot` 49, `CollapseSlot` 39, `ApplyPendingDefeats` 23, `InitVictory` 39) | 8 | 243 | oui (verdict déjà par la planche script : -1 = refus journalisé) |
| Absence / reconnexion / stock de siège (`GiveSlotStock` 58, `FindSlotOnLogin` 28, `DefeatExpiredAbsences` 36, ...) | 23 | 498 | oui ; S4 abandonné |
| Diplomatie (`ApplyStance` 37, `ApplyVision` 24, `ApplyDiplomacyOrders` 22, ...) | 9 | 164 | oui ; repli S1 `EvaluateEnd >= 0 ? ... : Change.bFormed` vers L1705 (ESTIMÉ ~2 l., à relire) |
| Pause (`RequestPause` 32, `RequestResume` 23, ...) | 7 | 116 | verdict déjà par `FHOLPauseRules` / script (S1) |
| Marché (livre) | 3 | 39 | stockage |

Somme des familles « décide » (forum + fin + absence + diplo + pause + marché) = 1 485 l. brutes (les familles se recoupent un peu : somme des 7 familles 1 640 l. contre 1 776 l. de fonctions). Ce chiffre est un PLAFOND : ces fonctions appliquent un verdict déjà rendu par script (EvaluateEnd, FallGate, KingGate, DefeatGate, pause) ; la part restée « décision » n'est pas comptable sans lire les corps.

### 3c. Fichiers `*Rules.cpp` encore en C++

33 fichiers `*Rules.cpp`, 8 631 l. au total (MESURÉ). Les trois nommés :

| Fichier | Lignes | Appelants hors tests (MESURÉ par Grep) | Lecture par script |
|---|---|---|---|
| `Match\HOLForumRules.cpp` | 553 | `HOLBuilding.cpp`, `HOLVisionSubsystem.cpp`, `HOLAISummary.cpp`, `HOLForumMessages.cpp`, `HOLDiplomacyReadLibrary.cpp` | oui : `HOLForumRulesReadLibrary`, `HOLForumMessagesReadLibrary` |
| `Match\HOLVictoryRules.cpp` | 152 | `HOLUnitOrderProcessor.cpp` (Mass), `HOLVisionSubsystem.cpp`, `HOLDisconnectRules.cpp`, `HOLForumMessages.cpp` | oui : `HOLVictoryReadLibrary` |
| `Match\HOLDisconnectRules.cpp` (`FHOLDisconnectRules`) | 131 | `HOLGameState.cpp`, `HeroesOfLibertyGameModeBase.cpp`, `HOLManagerClient.cpp`, `HOLAIBrain.h` | lecture par script via une `*ReadLibrary` (une des Small / StartRules, non précisé) |

Les plus gros autres : `HOLFormationRules` 1 153, `HOLConstructionRules` 748, `HOLGatherRules` 661, `HOLManagerClientRules` 631, `HOLVisionRules` 572, `HOLCombatRules` 471, `HOLFactionRules` 434, `HOLVolcanoRules` 422. 62 fichiers `*Library.cpp` / `*ReadLibrary.cpp` (9 106 l.) exposent déjà ces règles au script en lecture (MESURÉ). Donc ces règles sont déjà « lues par script » : le script ne les redéfinit pas. Les migrer = les réécrire en script ET faire appeler le script par leurs appelants Mass (`HOLUnitOrderProcessor`, `HOLCombatProcessor`, `HOLGatherProcessor`) : contraire à D839 pour ces appelants. Migrables sans toucher Mass : ESTIMÉ seulement les règles dont tous les appelants sont des fonctions par évènement (Forum, Disconnect, ManagerClientRules, FactionRules, MapRules, StartRules), soit au plus ~1 900 l. brutes (ESTIMÉ, somme des 6 fichiers : 553 + 131 + 631 + 434 + 101 + 89 = 1 939 ; MESURÉ pour les lignes, ESTIMÉ pour « sans appelant par pas » : `HOLForumRules` est appelé par `HOLVisionSubsystem` (par pas), `HOLFactionRules` par `HOLProductionSubsystem`).

### 3d. Tests C++ restants

| Mesure | Fichiers | Lignes | Statut |
|---|---|---|---|
| Tous les tests C++ (`Tests\` des deux modules) | 106 | 37 398 | MESURÉ |
| Appelant `ServeBirthRequests|RequestUnits|FMass` | 40 | 21 166 | MESURÉ (restent par D838 / D854) |
| Appelant l'un de ces motifs ou `UHOLUnitSubsystem` | 42 | 22 723 | MESURÉ |
| Sans ces motifs | 66 | 16 232 | MESURÉ |

Dans les 66 : `HOLTestWorldLibrary.cpp` 1 114 + `.h` 443 et `HOLGameStateTestLibrary.cpp` 643 sont l'infrastructure des tests script (restent par construction, ~2 200 l.). Plus gros restants : `HOLBuildingTests` 1 021 (partie propriétaire du Foundation, gravats, navmesh... selon ETAPE0), `HOLMapTableTests` 869, `HOLManagerClientTests` 665, `HOLSanityTests` 582, `HOLMapHinterland/SteppeTests` 582 / 581, `HOLMovementTests` 529, `HOLAnimationTests` 500, `HOLVisionTests` 462. Combien de ces 66 sont portables : NON MESURÉ (ESTIMÉ du plan : Victory ~45 %, StartingHero ~40 %, AIOrders ~30 %).

## 4. Verdict

1. Le C++ hors tests fait 119 510 l. ; ce que D839 garde est mesurable en gros : Mass (UnitOrderProcessor, Combat, Simulation / Parallel / Gather / Build processors, UnitSubsystem, InterpolatedInstances : 16 326 l. pour 8 fichiers, MESURÉ ; 17 087 avec UnitReception), mesure (StressTest 1 969), widgets (GameScreen, NewGame, ScriptScreen : 4 474), réplication (GameState part, Building, Reception), liaisons (`*Library.cpp` 9 106 l.), D853 / D838 (TestSequence 876, CrowdTest 898).
2. Règle de jeu migrable sans toucher D839, par famille : `Do<X>` + `Request<X>` du contrôleur 1 256 l. brutes, net ESTIMÉ -440 (risque RÉSEAU sauf les `DoCheat*` ~265 l., risque nul) ; décisions de `HOLGameState` jusqu'à 1 485 l. brutes en plafond, net ESTIMÉ très inférieur (verdicts déjà au script ; risque RÉSEAU, partie multijoueur) ; `RefreshGrid` ~140 net (risque VISUEL, ESTIMÉ) ; `Select.Group/Army/Category/Hero/Subgroup` ~60 net (risque nul, dev) ; camera / placement C10-3 ~90 net (nul, 158 scénarios à rejouer).
3. Les règles `*Rules.cpp` (8 631 l.) sont déjà lues par script ; seules ~1 900 l. brutes sont candidates sans Mass et seulement si leurs appelants par pas disparaissent : RÉSEAU ou nul selon la règle, non comptable sans lire les corps.
4. Tests : 16 232 l. de tests C++ sans Mass (dont ~2 200 l. d'infrastructure) : portables en partie, risque joueur nul, mais portage historiquement surestimé ; 21 166 l. restent par D838 / D854.
5. NON comptable sans lire les corps : fichiers n° 10 (SelectionMarkers 1 453), 16 (GameModeBase 1 043), 23 (HeroSubsystem 791), la part « décision » réelle de GameState, la ventilation des 915 l. « autres » du contrôleur, les 5 consoles Select non attribuées, le nombre de tests des 66 réellement portables ; le net de tout retrait (liaison laissée en place). Conclusion honnête : ce qui reste de « règle de jeu » migrable est de l'ordre de 1 000 à 1 500 l. NETTES (ESTIMÉ, fourchette large), surtout `Do` / `Request`, GameState et les tests ; l'étape 5 est donc finie pour les règles (planches, grille, production, fin de partie) et il ne reste que des familles à gain net faible et à risque réseau ou rejeu élevé : à l'utilisateur de décider si elles valent un build.

## 5. Ce qui n'est pas mesuré

- Aucun corps de fonction lu : les raisons du § 2 viennent des marqueurs et des en-têtes (ESTIMÉ). 4 fichiers « non jugés ».
- `DoCheatHoldPresence` : plage 328 l. englobe le `namespace HOLRpcScriptRoute_Haldir1117n` (~300 l.) ; ce namespace est une liaison RPC vers le script (infrastructure), pas un corps `Do`.
- Aucun comptage d'appelants de chaque `Request<X>`, aucun test exécuté, aucun binaire vérifié (l'état « construit » d'integ-21 vient du journal, pas de cette mesure).
- Consoles du contrôleur : la plage L100-L705 inclut des cvars et des gestionnaires d'entrée ; le poids exact des 23 commandes n'est pas ventilé.
- Les 33 `*Rules.cpp` n'ont pas été classés un par un entre « appel par pas » et « appel par évènement ».
