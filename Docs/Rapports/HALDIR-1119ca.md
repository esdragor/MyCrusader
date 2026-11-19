# HALDIR-1119ca : portes FallGate et KingGate (S2c), reprise 16 vague 16-B, 2026-11-19

NON COMPILE, NON EXECUTE. Branche `s2a` du worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, commit `07c67b9` (sur 910bfc5). Impact joueur : aucun attendu (memes refus de chute de forum et de mort de roi, memes journaux `Victory:` / `Regicide:`), re-branchement de deux portes.

```
APPELS_DECLARES: ~14 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 16:18 / ~16:21 (Get-Date PowerShell)
PREMIER_BUILD: sans build (consigne : la session principale compile)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (consigne)
TEST_ROUGE: non vu (aucune execution). Falsification ci-dessous. assertit l'effet : non pour les portes (predicats purs, valeur calculee par la regle) ; l'effet est couvert par les tests C++ Victory.Forum.Fall / Regicide.Death qui traversent maintenant la planche
COMMITS: 1, 07c67b9 a 16:20, merge_main_fait: non (consigne)
TESTS: 4 tests script ajoutes, non lances ; TESTS_TAUTOLOGIQUES: 0 declare (les 2 accords comparent a la chaine || d'origine ecrite dans le test ; voir limite en NON VERIFIE)
BUILDS_DEMANDES: 1 (lot C++ de la seance : pont + HOLGameState.cpp, avec 910bfc5)   TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 4 (Test_MatchFlowFallGate, Test_MatchFlowKingGate, Test_MatchFlowFallGateAgreement, Test_MatchFlowKingGateAgreement)
LIGNES_CPP_SUPPRIMEES: 0   LIGNES_MIGREES_PAR_LOT: ~12 (2 portes de 6 clauses, en script ; le C++ reste en repli)   TYPES_A_EXPOSER: aucun (booleens, entier)
ESSAIS: aucun   QUESTIONS: aucune   PROCESSUS_DETACHES: aucun lance
```

## Constat de fil : OUI, les deux sur le fil de jeu, sans doute possible

Question : `HandleKingDeath` et `HandleForumFall` sont-ils atteignables depuis l'interieur d'un `ParallelFor` (fil de travail) ? **Non.**

Maillons lus (`Source\HeroesOfLiberty\...`) :

- Le processeur : `Simulation\HOLCombatProcessor.cpp:252` `bRequiresGameThreadExecution = true` (constructeur 240-253) : `Execute` (`:779`) tourne sur le fil de jeu.
- Les trois `ParallelFor` du fichier (seuls du processeur) et leur corps entier :
  - `:1246-1256` (`HOLCombatAhead`) : corps = `GatherNearby(...)` (`:346-387`, lecture de la grille, ecrit `Out` par slot) et le lambda `ContactReachOf` (`:1189-1194`, arithmetique pure). Aucun appel de jeu.
  - `:1527-1531` (`HOLCombatPlaces`) : corps = `CountBodiesAround(...)` (`:421`, grep de `GameState|Kill|Damage|Subsystem|Cast<|GetWorld|UE_LOG` sur `:421-485` : vide), ecrit `AheadPlaces[Other]`.
  - `:1579-1663` (`HOLCombatSearchAhead`, fin de lambda trouvee a `:1663`) : lit `Fighters`, appelle `FindFighter` (lambda `:1146`, lecture de tables), `HOLCombat::*` purs (`GetSearchRadius`, `IsInRange`, `KeepsLastAttacker`, `IsRangedAttack`, `ChooseTarget`), `IsAutoFoe` (membre `:474`), `CountBodiesAround`. Grep de `Kill|FinishDying|DamageBuilding|HandleKing|HandleForum|GameState|Construction|Units->` sur `:1579-1663` : aucun resultat ; test des 15 lambdas definis en `:779-1579` (`Kill`, `Touch`, `AwardKillXP`, `MarkBuildingGone`, `Mark`, `PlacesAround`...) appeles dans les trois corps : seuls `FindFighter` (PF3) et `ContactReachOf`, `GatherNearby` (PF1) sont utilises.
- Chaine de `HandleKingDeath` : appelant unique `HOLCombatProcessor.cpp:652` (`KillUnit`, `:635`). Appels de `KillUnit` : `:663` (dans `FinishDying`), `:1310` (lambda `Kill`). Appels de `FinishDying` : `:1015`, `:1688` ; appels de `Kill` : `:1435`, `:1473`, `:3112`. Tous hors des plages `1246-1256`, `1527-1531`, `1579-1663`. Aucun autre appelant dans `Source` (grep de tout `Source`, tests exclus).
- Chaine de `HandleForumFall` : appelant unique `Buildings\HOLConstructionSubsystem.cpp:581` (`DamageBuilding`, `:564`). Appelants de `DamageBuilding` : combat `HOLCombatProcessor.cpp:1364` et `:2528` (hors des plages des `ParallelFor`), `Player\HOLPlayerController.cpp:4187` (console), `:4969` (`DoCheatDamageBuilding`, RPC serveur), `Simulation\HOLVolcanoSubsystem.cpp:505` (pas du volcan, `TActorIterator`). Aucune utilisation de `ParallelFor`/`Async` dans `HOLVolcanoSubsystem.cpp` ni dans les controleurs ; les seuls fichiers de `Source` qui utilisent `ParallelFor`/`AsyncTask` : `HOLCombatProcessor.cpp`, `HOLAvoidanceHoldProcessors.cpp`, `HOLParallelMassProcessors.cpp`, `HOLSimulationProcessors.cpp`, et les trois derniers ne contiennent ni `HandleKing`, `HandleForum`, `DamageBuilding`, `GetGameState`, `DestroyBuilding` (grep vide).
- Conclusion : un appel de script depuis ces deux portes est sur le fil de jeu (comme `DefeatSlot`, `AskEvaluateMatchEnd`).

## Fait (07c67b9)

- `Script\Match\HOLMatchFlowBoard.as` (+31) : `HOLMatchFlow::FallGateOpen(bAuthority, bPrimaryForum, bSlotValid, bDefeated, bPendingForum, bCampOccupied)`, `KingGateOpen(bAuthority, bMatchOver, bSlotValid, bDefeated, bPendingKing, bCampOccupied)` (signatures du plan section 2, point 1 et 3) ; surcharges `FallGateAnswer`, `KingGateAnswer` dans `UHOLMatchFlowBoardScript`.
- `Source\HeroesOfLiberty\Match\HOLMatchFlowSubsystem.h/.cpp` (+24) : `AskFallGate`, `AskKingGate` (1 ouvert, 0 ferme, -1 pas de planche), 2 `BlueprintNativeEvent` proteges + `_Implementation` par defaut -1.
- `Source\HeroesOfLiberty\Match\HOLGameState.cpp` (+20 / -4) : `HandleForumFall` (`:385`) et `HandleKingDeath` (apres) interrogent la planche ; REPLI C++ garde (chaine `||` d'origine en branche `: (...)`). Les arguments sont proteges par `bSlotValid &&` (pas de `1 << Slot` ni `IsSlotDefeated` sur un slot invalide, ce que le `||` d'origine evitait par court-circuit).
- `Script\Match\HOLMatchFlow_Test.as` (+64) : `Test_MatchFlowFallGate`, `Test_MatchFlowKingGate` (chaque branche), `Test_MatchFlowFallGateAgreement`, `Test_MatchFlowKingGateAgreement` (64 combinaisons contre la chaine `||` d'origine).
- Non touche : `AdvanceForums`, `FallPendsDefeat` (plan point 2, `Lives <= 0` : une comparaison, non branche, hors consigne).

## Falsification pour le rouge a voir

Fichier `Script\Match\HOLMatchFlowBoard.as`, fonction `FallGateOpen` : retirer `&& !bPendingForum` du `return`. Tests attendus ROUGES : `Test_MatchFlowFallGate` (cas Â« a forum defeat already pending Â») et `Test_MatchFlowFallGateAgreement` (16 combinaisons sur 64 fausses). Verts : `KingGate`, `KingGateAgreement`.
Seconde falsification : `KingGateOpen`, remplacer `!bMatchOver` par `true` : rouges `Test_MatchFlowKingGate` (cas Â« the match is over Â») et `Test_MatchFlowKingGateAgreement` (32/64). Remettre, puis un seul commit vert. Preuve a l'execution que la planche decide : avec la 1re falsification, deux chutes du meme forum principal dans le meme pas passent (la garde ne les refuse plus), alors que le repli C++ les refuse ; sans cette falsification aucune preuve que le chemin script est pris (le repli -1 masque une planche absente).

## Commandes pour la session principale (a lancer, rien lance ici)

1. `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1 -LiveCoding=0 -Wait` (C++ : pont + `HOLGameState.cpp`, lot commun avec 910bfc5 ; UHT genere les 4 evenements natifs).
2. `quick_check -Parts tests` (4 tests script neufs de ce lot + 4 de 910bfc5 + `HeroesOfLiberty.Victory.*` / Regicide C++ inchanges, qui passent par la planche : `Victory.Forum.Fall`, `Regicide.Death`), puis `-Parts net,victory`.
3. Rouge : falsifications ci-dessus, `-Parts tests`, puis remettre.

## NON VERIFIE

- Rien compile ni execute : UHT des deux evenements, syntaxe Angelscript des surcharges (copie de la forme de `DefeatGateAnswer`, 6 booleens, risque faible), interaction avec le lot 910bfc5 non compile non plus (si le build de 910bfc5 casse, celui-ci casse avec).
- Les deux accords sont des accords de formulation (script vs chaine `||` ecrite dans le test) ; l'accord reel de bout en bout est celui des tests C++ `Victory.Forum.Fall` et `Regicide.Death` (non lances).
- Le fil : lu statiquement (grep + lecture des corps) ; je n'ai pas lu `AssistNeighbours` (`:486-617`) en entier, mais aucun `KillUnit(` n'y figure (grep : seuls `:663` et `:1310`), donc la chaine ne passe pas par lui. `Kill` est un lambda `[&]` : il ne peut etre appele que dans `Execute`, et les appels (`:1435, 1473, 3112`) sont hors des trois lambdas de `ParallelFor`.
- Un lanceur d'ordre exterieur au combat qui tuerait un roi (`KillUnitsOfOwner`, `CollapseSlot`) n'appelle pas `HandleKingDeath` (grep : unique appelant `:652`).
- Si une version future ajoute un 4e `ParallelFor` dans le processeur, rien n'empeche d'y mettre `Kill` : la garde de fil est une lecture statique, pas un `check(IsInGameThread())` (on peut en ajouter un dans `AskFallGate`/`AskKingGate` au lot de retrait, si voulu).

## Reste

- Rouge VU puis commit vert ; retrait du repli C++ (chaines `||` des deux portes, comme celle de `DefeatSlot`) au lot de retrait ; `FallPendsDefeat` et S2b hors lot.
- Placeholders.txt : rien d'ajoute. Journal `*InProgress.md` / `EtapeDefini.md` / `IN_USE.txt` : non touches (consigne).

## Anomalies

- Aucune. `Content\Maps\Crown.umap` non suivi, non touche. Aucun processus lance.
