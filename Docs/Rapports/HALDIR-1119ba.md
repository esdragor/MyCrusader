# HALDIR-1119ba : lot S2a reduit (DefeatGate + MatchEndVerdict), reprise 16, 2026-11-19

NON COMPILE, NON EXECUTE. Branche `s2a` du worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, commit `910bfc5` (depart integ-17 848ce80). Impact joueur : aucun attendu (memes defaites, memes journaux `Victory:`), re-branchement de deux portes.

```
APPELS_DECLARES: ~15 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 15:22 / ~15:30 (Get-Date PowerShell)
PREMIER_BUILD: sans build (consigne : la session principale compile)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (consigne : aucun build)
TEST_ROUGE: non vu (aucune execution). Falsification a faire voir : ci-dessous. assertit l'effet : non pour la porte (predicat pur, valeur calculee par la regle), oui pour le verdict (masque gagnant calcule par HOLVictoryRules::Evaluate via HOLVictoryRead)
COMMITS: 1, 910bfc5 a ~15:27, merge_main_fait: non (consigne : aucun merge)
TESTS: 4 tests script ajoutes, non lances ; TESTS_TAUTOLOGIQUES: 0 declare (les agreements comparent a la regle C++ via HOLVictoryRead et a la chaine || d'origine ; voir limite en NON VERIFIE)
BUILDS_DEMANDES: 1 (lot C++ de la seance : bridge + GameState)   TESTS_CPP_RETIRES: 0   TESTS_SCRIPT_AJOUTES: 4 (Test_MatchFlowDefeatGate, Test_MatchFlowMatchEndVerdict, Test_MatchFlowDefeatGateAgreement, Test_MatchFlowMatchEndVerdictAgreement)
LIGNES_CPP_SUPPRIMEES: 0   LIGNES_MIGREES_PAR_LOT: ~55 (porte 6 l. + verdict 20 l. de regle, en script ; le C++ reste en repli)   TYPES_A_EXPOSER: aucun (entiers, booleens, TArray<int32>)
ESSAIS: aucun   QUESTIONS: aucune   PROCESSUS_DETACHES: aucun lance
```

## Fait

- `Script\Match\HOLMatchFlowBoard.as` (+43) : `HOLMatchFlow::MatchNotOver()` (256), `DefeatGateOpen(bAuthority, bMatchOver, bReasonNone, bSlotValid, bDefeated, bCampOccupied)`, `MatchEndVerdict(States, Teams, DefeatedMask, bDiplomacyMode, AlliedWinnerMask)` (regle des camps lue par `HOLVictoryRead::Evaluate`, puis alliance D373 si mode diplomatie et masque defait != 0) ; surcharges `DefeatGateAnswer`, `MatchEndAnswer` dans `UHOLMatchFlowBoardScript`.
- `Script\Match\HOLMatchFlow_Test.as` (+121) : les 4 tests ci-dessus (chaque branche de la porte ; 1c1, camp unique, equipes 1+2, diplomatie ; balayage des 64 combinaisons de la porte contre la chaine `||` d'origine ; balayage des 256 masques x 3 configurations x modes contre `HOLVictoryRead::Evaluate`).
- Pont `HOLMatchFlowSubsystem.h/.cpp` (+27) : `AskDefeatGate` (1 ouvert, 0 ferme, -1 pas de planche), `AskMatchEnd` (masque 0..255 fini, `MatchNotOver` = 256 pas fini, -1 pas de planche), 2 `BlueprintNativeEvent` proteges, defaut -1.
- `HOLGameState.cpp` (+52 / -2) : `DefeatSlot` (porte, ligne ~650) et `EvaluateMatchEnd` (verdict, ligne ~714) interrogent la planche, REPLI C++ garde (porte inline en branche `: (...)`, `Result` C++ calcule d'abord). Le verdict script remplace `Result` ; un ecart avec la regle C++ ecrit `Victory: the script verdict (%d) differs from the C++ rule ...` (Warning) : temoin d'accord a l'execution.
- Ecart au plan : le plan disait `-1` = pas fini pour `MatchEndVerdict` ; `-1` est la convention « pas de planche » du patron S1, donc « pas fini » = 256.

## Constat de thread (fichier:ligne)

- Branches : `DefeatSlot` et `EvaluateMatchEnd` seulement (le lot reduit). Appelants de `DefeatSlot` : `HOLGameState.cpp:456,460` (`ApplyPendingDefeats`, `OnStepEnded`), `:439` (nomades), `:1135` (absences), `HOLScriptRpcPlayerActor.cpp:95` (RPC serveur), `HOLPlayerController.cpp:4138` (console), `HOLSiteSubsystem.cpp:428` (dans `OnStepEnded` du sous-systeme, evenement de pas) : tous fil de jeu ; aucun depuis un processeur Mass. `EvaluateMatchEnd` : `DefeatSlot` `:681` et l'alliance `:1641`.
- NON branches (hors lot) : `HandleKingDeath` est appele depuis `HOLCombatProcessor.cpp:652` (`KillUnit`, via le lambda `Kill` `:1300-1311`, hors des `ParallelFor` `:1246, 1527, 1579`) ; le processeur declare `bRequiresGameThreadExecution = true` (`HOLCombatProcessor.cpp:252`), donc fil de jeu. `HandleForumFall` : `HOLConstructionSubsystem.cpp:581` (dans `DamageBuilding`), lui-meme appele depuis le combat `HOLCombatProcessor.cpp:1364, 2528`. Donc probablement fil de jeu pour les deux, mais le corps des `ParallelFor` n'a pas ete lu en entier : a confirmer avant de brancher les portes 1 et 3 (pas fait ici).

## Falsification pour le rouge a voir

Fichier `Script\Match\HOLMatchFlowBoard.as`, fonction `DefeatGateOpen` : remplacer `!bMatchOver` par `true` dans le `return`. Tests attendus ROUGES : `Test_MatchFlowDefeatGate` (le cas « the match is over: no more defeat ») et `Test_MatchFlowDefeatGateAgreement` (32 combinaisons sur 64 fausses). Les 3 autres restent verts. Remettre puis un seul commit vert. Preuve que la planche decide (partie net) : avec cette falsification un `DefeatSlot` apres la fin du match passe, ce que le repli C++ refuse ; sinon aucune preuve que le chemin script est pris.

## Commandes pour la session principale

1. `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1 -LiveCoding=0 -Wait` (C++ : pont + `HOLGameState.cpp` ; UHT genere les 2 evenements).
2. `quick_check -Parts tests` (4 tests script neufs + `HeroesOfLiberty.Victory.*` C++ inchanges, qui passent par la planche), puis `-Parts net,victory` (journaux `Victory: slot 1 defeated`, `Victory: match over, winners 0` ; aucune ligne `script verdict ... differs`).
3. Rouge : falsification ci-dessus, `-Parts tests`, N-4/N attendu... (4 neufs : 2 rouges attendus), puis remettre.

## NON VERIFIE

- Rien compile ni execute : syntaxe Angelscript de la surcharge `MatchEndAnswer(const TArray<int>& ...)` (BlueprintOverride d'un `BlueprintNativeEvent` a tableaux const ref : non vu ailleurs dans le depot), UHT des deux evenements, `LaunchConfig.Slots[i].State/.Team` (noms pris de `HOLVictoryReadLibrary.cpp`), `Allied.Winners` en `int32`.
- Teams = tous 0 en mode diplomatie (le C++ `GetCamp` « toujours seul » en diplomatie, `HOLVictoryRules.h` doc) : `HOLVictoryRules.cpp` non lu ; le Warning de desaccord le dira a l'execution.
- Les deux « accords » sont des accords de formulation (script vs regle C++ lue ; script vs chaine `||` d'origine) : l'accord reel de bout en bout est celui des tests C++ `Victory.GameState/Collapse/Forum.Fall` et de la partie net, qui traversent maintenant la planche (non lances).
- Cas `-1` du `Verdict` : le repli C++ n'est pris que sans planche ; avec planche le script est la source.

## Reste

- Rouge VU puis commit vert unique ; retrait du repli C++ (porte inline, `Result` C++, bloc `Gone/Allied` duplique) dans un lot ulterieur, pas ici.
- S2b (`AdvanceForums`) ne migre pas (chemin chaud, par pas). Portes `FallGate` / `KingGate` : a brancher apres confirmation du fil (ParallelFor).
- Placeholders.txt : rien d'ajoute. Journal `*InProgress.md` / `EtapeDefini.md` / `IN_USE.txt` : non touches (consigne).
