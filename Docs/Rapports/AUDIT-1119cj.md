# AUDIT-1119cj : production, ordres du joueur et fil d'exécution

Lecture seule, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` (integ-20). Suite de `AUDIT-1119ci.md`. Aucun build, aucune édition de code. Appels d'outils : 24.

## Verdict

- Aucun cas A. Les 6 corps `Ask*` de la production et les 17 de la planche des ordres sont de simples relais : ils rendent `-1` si `Find` est nul, sinon la réponse du script. Aucun test d'indice, de largeur, de pointeur ni d'enum dans ces corps.
- Fil : aucun appelant hors fil de jeu trouvé, ni pour les ordres, ni pour la construction, la production et la fin de partie. Le seul code parallèle repéré (3 `ParallelFor` du processeur de combat) ne touche aucun de ces points d'entrée dans les lambdas lus.

## Tableau

| planche | Ask* | condition (fichier:ligne) | appelant (fichier:ligne) | effet | gravité | correctif proposé |
|---|---|---|---|---|---|---|
| Production | AskOrderGate | `World` nul ou aucune planche : `-1` (`HOLProductionBoard.cpp:9-14, 21`). Pas de test de fil. | `HOLProductionSubsystem.cpp:311` | `-1` : l'ordre est sauté (`continue`) + journal « absent » une fois ; rien n'est payé | B | rien |
| Production | AskQueue | idem (`HOLProductionBoard.cpp:47`) | `HOLProductionSubsystem.cpp:475`, repli `:511-513` sur `HOLProduction::TryEnqueue` (lu au rapport ci, pas relu) | rien n'est payé tant que le verdict est `-1` ou hors `1-8` ; si le repli C++ est bien là, l'unité est quand même mise en file | B | rien |
| Production | AskCancel | idem (`:27`) | `HOLProductionSubsystem.cpp:544` | `-1` : annulation sans effet (pas de remboursement non plus, car le verdict est testé avant `AskStockAfterCancel`) | B | rien |
| Production | AskStockAfterCancel | planche absente : rend `Stock + Cost` (`HOLProductionBoard.cpp:33-38`) : remboursement intégral, jamais perdu | `HOLProductionSubsystem.cpp:554` | neutre ; le remboursement se fait même sans planche | C | rien |
| Production | AskRally, AskRallySite | idem (`:53, :59`) | `HOLProductionSubsystem.cpp:346, 372` | `AskRally -1` : ordre de ralliement sauté ; `AskRallySite -1` : drapeau au sol (`:371-380`, lu au rapport ci) | B | rien |
| Ordres | AskIsAccepted, AskIsAcceptedAnywhere | `World` nul ou `!IsInGameThread()` : `-1` (`HOLPlayerOrdersBoard.cpp:11, 21`) | `HOLPlayerOrders.cpp` (`Verdict`) ; `HOLAIOrders.cpp:223` | refus + journal une fois | B | séparer « mauvais fil » de « pas de planche » dans le journal |
| Ordres | AskMove, AskAttack, AskAttackBuilding, AskAttackMove, AskStop, AskToggleDeploy | même condition (`:47-83`) ; `AskMove` reçoit `FrontLine` / `bFrontWidth`, simple relais | `HOLPlayerOrders.cpp:64-249`, `HOLAIOrders.cpp:95, 149, 156, 166`, `HOLVolcanoSubsystem.cpp:395` | refus journalisé ; l'ordre n'est pas donné | B | idem |
| Ordres | AskGather, AskBuild, AskDropOff, AskPlace | même condition (`:85-107`) | `HOLAIOrders.cpp:88, 236, 244`, `HOLPlayerOrders.cpp:142-189` | refus ; effet partiel connu au rapport ci (soldats déjà partis) | B / C | idem |
| Ordres | AskQueueProduction, AskMarketTrade, AskSetRally | même condition (`:109-125`) | `HOLAIOrders.cpp:251, 259`, `HOLPlayerOrders.cpp` | refus ; aucune unité payée : le contrôle de coût est dans la règle de production (`AskQueue`), plus loin | B | idem |
| Ordres | AskGarrison, AskUngarrisonAll | même condition (`:127-137`) | `HOLPlayerOrders.cpp` ; `HOLCommandScriptLibrary.cpp:395` et `HOLControllerCommandLibrary.cpp:454` appellent `TakesGarrisonOrder` (corps non lu : je n'ai pas vérifié s'il passe par `AskGarrison`) | refus journalisé | B | vérifier `TakesGarrisonOrder` |

Lecture des 17 corps d'ordres : `HOLPlayerOrdersBoard.cpp:35-137`, tous du même motif (`Find`, puis `Board ? Board->XAnswer(...) : -1`). Implémentations par défaut : `-1` partout (`:154-169`), donc une classe de script absente ou ne surchargeant pas la fonction donne un refus, jamais un plantage.

## Fil d'exécution

Les deux planches testent `IsInGameThread()` dans `Find` (ordres) ; la production, la construction et la fin de partie ne le testent pas.

Appelants des ordres (`HOLPlayerOrders::*`, 80 occurrences dans 12 fichiers, dont 38 en `Tests/`) :

- `HOLAIOrders.cpp` (88, 95, 149, 156, 166, 223, 236, 244, 251, 259 ; l'évacuation `:103`) : bibliothèque de fonctions (`UBlueprintFunctionLibrary`, `HOLAIOrders.h:6`) appelée par le script de l'IA. Le script Angelscript s'exécute sur le fil de jeu. Preuve de lecture : Grep `ParallelFor|AsyncTask|Async(|FRunnable|ENamedThreads|FQueuedThreadPool` sur tout `Source/HeroesOfLiberty` hors tests ne rend AUCUN fichier de `AI/`, ni de `Player/`, ni `HOLPlayerOrders*`.
- `HOLVolcanoSubsystem.cpp:395` : `EvacuateOwner`, appelée seulement par `HOLAIOrders.cpp:103` (Grep) : même fil que l'IA.
- `HOLPlayerController.cpp` (18 appels) : gestionnaires d'entrée et de RPC du contrôleur : fil de jeu (aucun `ParallelFor` ni `Async` dans le fichier, même Grep).
- `HOLCommandScriptLibrary.cpp:395`, `HOLControllerCommandLibrary.cpp:454` : `TakesGarrisonOrder`, appelé depuis le script ou l'interface : fil de jeu.
- Processeurs Mass : le Grep des noms `HOLPlayerOrders|DamageBuilding|DestroyBuilding|RefundProductionQueue|HandleForumFall|HandleKingDeath|DefeatSlot|EvaluateMatchEnd|RequestPause|ConstructionSubsystem|ProductionSubsystem|MatchFlow` sur `Presentation/*.cpp`, `HOLNetUnitDisplay.cpp`, `HOLParallelMassProcessors.cpp`, `HOLAvoidanceHoldProcessors.cpp`, `HOLSimulationProcessors.cpp`, `HOLHealProcessor.cpp`, `HOLDeployProcessor.cpp` et `AI/*.cpp` ne rend que `HOLAIOrders.cpp`. Aucun processeur parallèle (`ParallelForEachEntityChunk` dans `HOLSimulationProcessors.cpp:345, 1210, 2735`, `HOLParallelMassProcessors.cpp:58, 453, 659`) n'appelle un ordre.

Appelants de la construction, de la production et de la fin de partie (points 4) :

- `UHOLConstructionSubsystem::ApplyOrders` et `UHOLProductionSubsystem::ApplyOrders` (donc `Place`, `Cancel`, files, ralliement) : branchés sur `OnStepStarted` (`HOLConstructionSubsystem.cpp:387`, `HOLProductionSubsystem.cpp:170`), diffusé par `HOLSimulationSubsystem.cpp:333` (`RunStep`), lui-même lancé par `OnPrePhysicsPhaseStarted` (`:303-320`), une phase de tick du monde : fil de jeu. Je n'ai pas lu le branchement du délégué de phase ni les corps de `OnStepStarted` pour confirmer qu'ils appellent `ApplyOrders`.
- `DamageBuilding` (donc `AskQueueRefund`, `AskDestroy`, `AskRubble`, `HandleForumFall`) : `HOLCombatProcessor.cpp:1364, 2528` ; `HOLVolcanoSubsystem.cpp:505` ; `HOLPlayerController.cpp:3641, 4423` (cheats, RPC). Le processeur de combat pose `bRequiresGameThreadExecution = true` (`HOLCombatProcessor.cpp:252`), donc `Execute` est sur le fil de jeu. Ses trois `ParallelFor` (`:1246-1255`, `:1527-1531`, `:1579-...`) : j'ai lu les deux premiers en entier (appels `GatherNearby`, `CountBodiesAround` seulement) et 14 lignes du troisième (lecture de `Fighters`, sans appel de règle) ; la ligne 1364 est HORS du premier (`ParallelFor` fermé en 1255). Le commentaire de `HOLGameState.cpp:405, 448` dit la même chose pour 1364 et 2528.
- `HandleKingDeath` : `HOLCombatProcessor.cpp:652` (même `Execute`, fil de jeu). `DefeatSlot` : `HOLSiteSubsystem.cpp:428`, `HOLScriptRpcPlayerActor.cpp:95` (RPC), `HOLPlayerController.cpp:3592`, `HOLGameState.cpp:493, 497, 1200`. `EvaluateMatchEnd` : `DefeatSlot` (`:727`) et `ApplyStance` (`:1706`). `RequestPause` / `RequestResume` : RPC. Tous de fil de jeu par construction (tick de sous-système, RPC, processeur à fil de jeu forcé).
- `DestroyBuilding` : `HOLGameState.cpp:1279` (`CollapseSlot`, depuis `DefeatSlot`) : fil de jeu.

Conclusion : pas d'appelant hors fil de jeu pour les ordres, ni pour les trois planches sans test de fil. Le risque reste théorique : si un jour un `ParallelFor` ou un `AsyncTask` appelle ces fonctions, la planche des ordres refuserait en silence (journal « script board absent »), les trois autres appelleraient le script depuis un fil de travail.

Correctif proposé (B, prévention) : ajouter le même test `IsInGameThread()` dans `Find` de la construction, de la production et de la fin de partie, avec un `ensure` ou un journal « mauvais fil » distinct de « planche absente ».

## Ce que je n'ai PAS lu

- Les corps des fonctions de `HOLConstructionSubsystem.cpp` autres que celles du rapport ci, et `OnStepStarted` / `ApplyOrders` de construction et de production : « appelé depuis un tick du monde » est déduit de `HOLSimulationSubsystem.cpp:303-333`, pas de ces corps.
- Le branchement de `OnPrePhysicsPhaseStarted` (qu'il s'agisse bien d'un délégué de tick sur le fil de jeu).
- Les 18 appels de `HOLPlayerController.cpp` un par un ; seule l'absence de `ParallelFor` / `Async` dans le fichier est établie.
- La fin du lambda `ParallelFor` `HOLCombatSearchAhead` (`HOLCombatProcessor.cpp:1579` au-delà de 14 lignes) ; les `ParallelFor` de `HOLSimulationProcessors.cpp:1829` et `HOLParallelMassProcessors.cpp:659` (leur corps ne figure pas dans les fichiers qui contiennent les noms cherchés, vu par Grep).
- Les scripts `.as` (planches et IA), en particulier le tick de l'IA ; « Angelscript tourne sur le fil de jeu » est une propriété du moteur, pas vérifiée dans ce dépôt.
- Les autres modules que `HeroesOfLiberty` (`HeroesOfLibertyUI`, etc.) comme appelants possibles ; `TakesGarrisonOrder` ; les tests (`Tests/`, 38 appels d'ordres) : s'ils appellent hors fil de jeu, ils seraient refusés.
- La couche de repli de `AskQueue` (`HOLProductionSubsystem.cpp:511-513`) n'a pas été relue.
