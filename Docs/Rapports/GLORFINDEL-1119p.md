# GLORFINDEL-1119p : correcteur de script, reprise 14 (2026-11-19)

Worktree HeroesOfLiberty-Glorfindel, branche net-playtest, tip de départ 53c3f92, tip final b99b484.

## Comptes successifs (quick_check -Parts tests)
- 09:23 passage 1 : 1624/1625. Les scripts compilent (aucune erreur de compilation Angelscript). Un seul rouge.
- 09:26 passage 2 (après correction) : 1625/1625 PASS.
- 09:29 rouge vu : 1622/1625 (3 valeurs faussées, 3 tests attendus en échec, les 3 sont tombés).
- 09:31 passage 4 (valeurs remises) : 1625/1625 PASS. Référence main : 1614 (+11 tests neufs).

## Erreur corrigée (1)
- Script/Player/HOLControllerCommands_Test.as:167, Test_ControllerCommandsDamageNear : `AssertFalse(Own.bEnemy, "enemy is compared case-sensitive, as the C++")` avec l'argument "ENEMY". Cause : prémisse fausse du test. Le C++ d'origine (HOLPlayerController.cpp avant 53c3f92) comparait `Args[n] == TEXT("enemy")`, et `FString ==` d'Unreal est insensible à la casse ; le script (`Args[3] == "enemy"`, HOLControllerCommands.as:225 et 238) l'est aussi. Le comportement script = le C++ : pas d'écart de règle. Correction : `AssertTrue(Own.bEnemy, "enemy is compared case-insensitive, as the C++ FString ==")`. Aucun .as de règle modifié.

## Rouge vu (09:29 lancé, résultat 09:31 : 1622/1625)
Fausses valeurs : HOLMatchFlow_Test.as:13 (Allowed -> NoPauseLeft), HOLMarketApply_Test.as:122 dans Test_TributeSettleReasonOrderAndConservation (70 -> 71), HOLControllerCommands_Test.as:161 (50 -> 51). Les 3 tests tombés : Test_MatchFlowPauseAllowedAndRefused, Test_TributeSettleReasonOrderAndConservation, Test_ControllerCommandsDamageNear. Valeurs remises, git diff vide sur les 3 fichiers, relance verte 1625/1625.

## Commit
b99b484 (net-playtest), un fichier : Script/Player/HOLControllerCommands_Test.as. Pas de push.

## Reste / blocage C++
Aucun blocage C++. Rien d'écrit en C++. stray_processes -Stop : aucun processus détaché.

## Anomalies
Aucune erreur de compilation Angelscript sur les trois lots (S1, R1, C1) : les écrits à l'aveugle compilaient. L'horloge Get-Date a donné 09:23 au premier appel (la consigne annonçait un lancement à 09:29) ; arrêt dur 09:44 respecté (fin vers 09:35).
