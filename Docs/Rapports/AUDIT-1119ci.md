# AUDIT-1119ci : les `Ask*` qui rendent `false` pour une autre cause que « pas de planche »

Lecture seule, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` (integ-20). Aucun build, aucune édition de code.
Remarque de méthode : j'ai dépassé les 25 appels d'outils (environ 28) pour finir la lecture des appelants ; l'heure (18:07) reste dans la limite.

## Verdict en une ligne

Aucun second cas de la famille « entrée plus étroite que prévu » (gravité A) trouvé dans ce qui a été lu. Un cas de gravité B à risque de plantage (`HOLPlayerController.cpp:1952-1953`, `NextInCycle` rend `INDEX_NONE` puis `Buildings[Index]`), non vérifié jusqu'au bout (voir « non lu »).

## Quelles planches sont déjà dans `main`

| Planche | Dans `main` ? |
|---|---|
| Ordres du joueur (`UHOLPlayerOrdersBoard`) | oui |
| Construction (`UHOLConstructionBoard`) | oui |
| Production (`UHOLProductionBoard`) | oui |
| Fin de partie / match flow / pause (`UHOLMatchFlowSubsystem`) | oui |
| Grille de commandes (`UHOLCommandGridBoard`) | NON : integ-20 seulement (repli retiré par L3a) |
| Camp, Garrison, Relay, Site, Marché, Signal, Spécialisation, UnitTier | autres ponts, pas de `Ask*` : hors périmètre de cet audit (voir « non lu ») |

`AskToSurrender` (`HOLGameMenuScreen.cpp:105`) et `AskToQuit` n'ont rien à voir avec une planche : ce sont des boîtes de confirmation de l'interface.

## Mécanisme commun

- Grille : `FindAny()` (`HOLCommandGridBoard.cpp:32-53`) rend `nullptr` si `!GEngine`, hors fil de jeu, ou si aucun contexte de monde n'a de planche. Toutes les `Ask*` de la grille rendent `true` dès qu'une planche existe : les conversions (`ToRule`, `FromRule`, `ToKeyResult`) ne peuvent pas échouer. Une seule exception existait, `AskMergeCommands`, corrigée (7b85be8, lignes 252-263 : remplissage par `NoCommand`).
- Ordres : `Find(World)` (`HOLPlayerOrdersBoard.cpp:8-17`) rend `nullptr` si `World` nul ou hors fil de jeu ; `FindAny` idem (`:19-33`). Les `Ask*` rendent `-1`. Appelant : `Verdict` (`HOLPlayerOrders.cpp:53-61`), `-1` = refus + journal « script board absent » une fois.
- Construction, production, match flow : `Find(World)` seulement sur `World` nul ; PAS de test `IsInGameThread` (`HOLConstructionBoard.cpp:7-15`, `HOLMatchFlowSubsystem.cpp:7-15`). Hors fil de jeu, le script serait appelé de force au lieu de rendre `-1`.
- Types de monde : toutes les planches lues n'existent que pour `EWorldType::Game` et `PIE` (`DoesSupportWorldType`). Un monde d'éditeur ou de prévisualisation n'a jamais de planche.
- Deux mondes (client PIE + serveur) : `FindAny` prend la première planche trouvée parmi les contextes. Les réponses de la grille sont des fonctions pures des arguments (aucun état lu dans le monde) : sans effet. Les `Ask*` avec `World` (ordres, construction, production, match flow) utilisent le monde de l'appelant : correct. `AskIsAcceptedAnywhere` (`HOLPlayerOrders.cpp:67`, `World` nul) prend la première planche : règle pure `(PlayerIndex, NumUnits)`, sans effet.

## Tableau

Légende effet : « C++ » = repli encore là ; « refus » = action non faite et journalisée ; « neutre » = valeur par défaut.

| Planche | Ask* | condition de `false` / `-1` autre que « pas de planche » (fichier:ligne) | appelant (fichier:ligne) | effet aujourd'hui | gravité | correctif proposé |
|---|---|---|---|---|---|---|
| Grille | AskMergeCommands | entrée plus étroite que 12 cases (`HOLCommandGridBoard.cpp:246-271`, ancien test de largeur) | `HOLPlayerController.cpp:2267` via `HOLCommandGrid.cpp:415` | CORRIGÉ 7b85be8 : grille vide quand le sous-groupe actif est plus étroit que le groupe | (A, corrigé) | rien ; ajouter un test « sous-groupe plus étroit que le groupe » |
| Grille | les 20 autres Ask* (GetCommand, GetOrder, IsAgeReached, IsAgeOpen, GetAgeBuilding, PressKey, ClickAge, GetChosenAgeAfter, CheckProduction, CheckBuild, Describe, NextInCycle, PickEvenly, SplitSubgroups, IsAloneInGroup, IsRepeatPress, GetCommandPriority, IsSelectedByCategory, Blocks, GetCycleRank) | seule cause : `FindAny` nul = pas de GEngine, hors fil de jeu, aucun monde avec planche (`HOLCommandGridBoard.cpp:35-52`) ; aucun test de largeur, de tableau vide, d'indice ni de pointeur dans les corps | `HOLCommandGrid.cpp:98-426` (une fonction enveloppe par Ask) | neutre (grille vide, âges verrouillés, `INDEX_NONE`) + journal `LogNoBoardOnce` ; seulement sans planche ou hors fil de jeu | B (pas d'entrée anormale connue) | séparer dans `LogNoBoardOnce` « hors fil de jeu » de « pas de planche » |
| Grille | AskNextInCycle | rend `INDEX_NONE` quand la planche est absente (ou, côté script, quand la liste est vide : non lu) | `HOLPlayerController.cpp:1952-1953` : `Buildings[Index]` sans test de `INDEX_NONE` | sans planche : indice -1, plantage ou lecture hors tableau (`HOLPlayerController.cpp:1952-1953`). Avec planche : dépend du script (non lu) | B, à risque de plantage | tester `Index == INDEX_NONE` avant `Buildings[Index]` (retour sans effet) ; lire `HOLPlayerController.cpp:1925-1947` pour voir si une liste vide est déjà écartée |
| Grille | AskPickEvenly | `INDEX_NONE` si planche absente | `HOLPlayerController.cpp:2709-2710` | gérée (`nullptr`) : aucun producteur choisi, touche sans effet | B | rien |
| Ordres | 17 Ask* (IsAccepted, IsAcceptedAnywhere, Move, Attack, AttackBuilding, AttackMove, Stop, ToggleDeploy, Gather, Build, DropOff, Place, QueueProduction, MarketTrade, SetRally, Garrison, UngarrisonAll) | `Find` nul : `World` nul ou `!IsInGameThread()` (`HOLPlayerOrdersBoard.cpp:11`) ; toute réponse `< 0` est prise pour « planche absente » | `HOLPlayerOrders.cpp:53-61` (`Verdict`), appels 64-249 | refus + journal « PlayerOrders: <ordre> refused, script board absent » une fois par monde et par ordre ; l'ordre n'est pas donné. Si un ordre part d'un autre fil (IA ?) : refus silencieux au sens du joueur, jamais de repli | B (sauf si un appelant IA est hors fil de jeu : non vérifié, serait A) | vérifier le fil des appelants de `HOLPlayerOrders::*` (IA, Mass) ; distinguer « mauvais fil » de « pas de planche » dans le journal ; ne pas utiliser `-1` pour un refus de règle côté script |
| Ordres | Gather, Build, DropOff (pré-test `AskIsAccepted` puis 2e Ask) | un `AskIsAccepted` réussi suivi d'un `AskX` refusé a déjà fait marcher les soldats (`SendNonWorkersTo`, `HOLPlayerOrders.cpp:142, 164, 189`) | idem | effet partiel : les non-ouvriers marchent, les ouvriers n'obéissent pas | C | rien, ou inverser l'ordre |
| Construction | AskPlace | `-1` si planche absente ; valeur hors `0-5` ramenée à `-1` (`HOLConstructionSubsystem.cpp:473-475`) | `HOLConstructionSubsystem.cpp:448-487` | refus + « Construction: Place refused, script board absent » (`:484`) ALORS QU'UNE VALEUR HORS PLAGE vient d'une planche présente : MESSAGE TROMPEUR | B (message) | ne pas journaliser « absent » quand `Find(World)` est non nul (`:434`) ; dire « verdict hors plage » |
| Construction | AskCancel | planche absente => `Verdict = 1` (`:545`) | `HOLConstructionSubsystem.cpp:536-551` | l'annulation est ignorée ; journal d'erreur « absent » puis ligne Display « not its building under construction » : MESSAGE TROMPEUR | C | idem |
| Construction | AskQueueRefund | `-1` si absente | `HOLConstructionSubsystem.cpp:596-606` | la file de production d'un bâtiment détruit n'est pas remboursée (bâtiment tombé, forum tombé) | B (sans planche seulement) | rien |
| Construction | AskDestroy | `-1` si absente | `HOLConstructionSubsystem.cpp:636-649` | bâtiment détruit quand même, sans remboursement ni ruine (prévu, journalisé une fois) | C | rien |
| Construction | AskQueueTotal | `false` si planche absente (`HOLConstructionBoard.cpp:49-58`) | `HOLConstructionSubsystem.cpp:623` (valeur de retour ignorée) | le total calculé en C++ par `HOLProduction::Cancel` (`:621`) reste : repli C++ encore là | C | rien |
| Construction | AskStockAfterPlace, AskCancelRefund | planche absente : stock rendu tel quel / ressources à zéro (`HOLConstructionBoard.cpp:24-41`) | `HOLConstructionSubsystem.cpp:453, 554, 662` | neutre ; pas de remboursement d'une annulation sans planche (`:554`) | C | rien |
| Construction | AskRubble | `-1` si absente | `HOLConstructionSubsystem.cpp:675-676` | pas de ruine (`> 0` requis) | C | rien |
| Production | AskOrderGate, AskRally, AskRallySite, AskQueue, AskCancel, AskStockAfterCancel | planche absente seulement (en-têtes lus ; corps `HOLProductionBoard.cpp` NON lus) ; `AskQueue` : réponse hors `1-8` retombe sur `HOLProduction::TryEnqueue` (`HOLProductionSubsystem.cpp:511-513`) : repli C++ sûr | `HOLProductionSubsystem.cpp:311, 346, 372, 475, 544, 554` | `-1` : ordre sauté (`continue`) + « LogBoardAbsentOnce » ; `AskRallySite < 0` : drapeau au sol (`:371-380`) | B | rien |
| Match flow | AskFallGate, AskKingGate, AskDefeatGate | `World` nul ; `-1` => `false`, rien ne se passe. Les index d'emplacement sont validés AVANT l'appel (`bSlotValid`), puis le tableau indexé APRÈS si la porte rend `1` : correct tant que la règle script refuse quand `bSlotValid` est faux (non lu) | `HOLGameState.cpp:407, 450, 690` | sans planche : forum qui ne tombe pas, roi sans effet, défaite jamais prononcée (la partie ne se termine pas) | B | tester `bSlotValid` aussi côté C++ avant d'indexer |
| Match flow | AskMatchEnd | `-1` si absente ; `States` et `Teams` ont la largeur de `LaunchConfig.Slots` (`HOLGameState.cpp:743-749`), pas `MaxSlots` : largeur utile pour la règle script (non lue) | `HOLGameState.cpp:770-775` | sans planche : la partie ne finit jamais, journal « absent » | B | contrôler la largeur côté script |
| Match flow | AskCanPause, AskCanResume, AskPausesLeftAfterPause | `-1` si absente ; l'appel client `CanPauseNow` passe `IsPauseFree()` (= `NM_Standalone`), identique au serveur (`HOLGameState.h:734`) : cohérent | `HOLGameState.cpp:1513, 1524, 1543` ; `HOLPauseLibrary.cpp:64, 78` | pause refusée ; journal « no script match flow board (HOLMatchFlowBoard.as not compiled) » ; le bouton Pause est grisé côté client | B ; message à vérifier : même texte pour `-1` venant d'une réponse script hors plage (non lu) | idem |
| Match flow | AskStanceGate, AskVisionGate, AskEvaluateMatchEnd | `-1` si absente | `HOLGameState.cpp:1682-1683, 1718-1719, 1705-1706` | repli C++ encore là (garde en ligne) : sans risque | C | rien |

## Réponse aux points demandés

2. Conditions de `false` autres que « pas de planche » : seulement (a) hors fil de jeu (grille et ordres), (b) `World` nul (ordres, construction, production, match flow), (c) l'ancienne largeur de `AskMergeCommands`. Pas de test d'enum hors plage, de pointeur nul ni d'indice dans les corps lus.
3. Aucun autre cas où l'entrée peut être plus étroite que prévue ne ressort des corps lus. Le seul indice hors tableau par valeur de retour est `HOLPlayerController.cpp:1953`.
4. Messages trompeurs : `HOLConstructionSubsystem.cpp:484` (valeur hors plage rapportée comme « script board absent »), `HOLConstructionSubsystem.cpp:543-549` (annulation), `HOLCommandGrid.cpp:51` (« no script grid board » aussi quand la vraie cause est « hors fil de jeu »), `HOLPlayerOrders.cpp:49` (idem).
5. Classement : A = 0 nouveau ; B = 10 lignes du tableau, dont une à risque de plantage ; C = le reste.

## Ce que je n'ai PAS lu

- Les corps de `HOLProductionBoard.cpp` (seulement les signatures) et 13 des 17 corps de `HOLPlayerOrdersBoard.cpp` (lignes 60-145 : une ligne chacun, même motif d'après les signatures).
- Tous les scripts `.as` (`HOLCommandGridBoard`, `HOLPlayerOrdersBoard`, `HOLConstructionBoard`, `HOLProductionBoard`, `HOLMatchFlowBoard`) : ce que la règle script fait de tableaux vides, d'indices hors bornes, de `Priority` plus court que `PerBuilding`, de `States` / `Teams` de largeur différente. C'est là que peut se cacher un second cas de la famille.
- `HOLPlayerController.cpp` autour de 1925-1947 (garde de liste vide avant `NextInCycle`) et 2240-2267 (construction de `PerBuilding`, déjà expliquée par le correctif).
- Le fil d'exécution des appelants de `HOLPlayerOrders::*` (IA, processeurs Mass) : si l'un tourne hors fil de jeu, ses ordres sont refusés.
- Les autres planches : Camp, Garrison, Relay, Site (`HOLCampBoard`, `HOLGarrisonBoard`, `HOLRelayBoard`, `HOLSiteBoardSubsystem`), Marché, Signal, Spécialisation, UnitTier (leurs corps, leurs appelants `HOLCampSubsystem.cpp:238`, `HOLGarrisonSubsystem.cpp:157`, `HOLRelaySubsystem.cpp:244, 262`, `HOLSiteSubsystem.cpp:319, 390, 438`). Je n'ai pas vérifié qu'elles sont 14 au total.
- `HOLBoardLog.cpp` (journal de présence « Board: <nom> script »).
- Les tests : `HOLSpecializationGridTests.cpp` crée un monde `EWorldType::Game` à la main ; je n'ai pas vérifié si les tests de grille C++ ou de script tournent avec une planche (sans planche, la grille répond « neutre » : un test de grille hors monde échouerait en silence).
- Un montage client PIE + serveur n'a pas été exercé : seule la lecture du code.
