# DIAG-1123a (2026-11-23, Sauron, integ-55) : regen des tables

APPELS_DECLARES: ~17 (arret a 20)
HEURE_DEBUT/HEURE_FIN: 10:24:26 / 10:30:10
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE / TEST_ROUGE_SEPARE: sans objet (diagnostic)
COMMITS: 0 (pas de vert, donc pas de commit) ; merge_main_fait: non
TESTS: quick_check -Parts tests (quick_check.ps1 du worktree) : 2131/2132 (etait 2130/2132) ; TESTS_TAUTOLOGIQUES: 0

## Regen
`regen_tables.ps1 -Root Sauron` : OK, un seul .uasset modifie : `Content/Buildings/BuildingTypes.uasset` (UnitTypes inchange). Laisse modifie, NON commite (arbre de Sauron : seul ` M Content/Buildings/BuildingTypes.uasset` hors Docs/Integrate_*).
`HOLWallLineRules_Test.Test_WallTypesListsPalisade` passe maintenant.

## Rouge restant (non corrige, rapporte)
`Angelscript.UnitTests.Tests.HOLCommandGrid_Test.Test_CGTables` : echec `panel I: no building on the other cells` (Script/Tests/HOLCommandGrid_Test.as:564-570, journal Saved_Tests/Logs/quick_tests.log:14400).
Cause : test perime, pas table. `EmptyI` (ligne 565-566) contient la case 6, or la ligne 532 du MEME test affirme que la case 6 de la page I est la Palisade (BuildGridSlot=6, Age=0 dans Tools/Data/BuildingTypes.csv ligne 18). Avant la regen la table n'avait pas la Palisade, donc l'assertion de la ligne 524 (16 types) cassait en premier ; apres la regen, la case 6 est occupee et `EmptyI` doit devenir {10, 11}. Correctif d'une ligne (retirer `EmptyI.Add(6)`), a faire par la branche proprietaire du test (celle de la ligne Palisade, 8e6281d), puis commit de la table avec.

## integrate.ps1 et regen
Non : `integrate.ps1` ne lance jamais `regen_tables.ps1` (aucune occurrence de regen / uasset / pythonscript dans le fichier ; il ne fait que cherry-pick des rouges, build.ps1, quick_check, reset a main, merge des verts, build, tests) et un .uasset de table n'est regenere que par un agent a la main (agent_rules.md regle 12) ; les CSV fusionnes donnent donc un .uasset perime, d'ou le FAIL de integ-55.

NON VERIFIE: les parts `net` et `build` apres regen (non lances) ; UnitTypes.uasset (regen sans diff).
PROCESSUS_DETACHES: aucun (stray_processes.ps1 -Stop : "aucun processus detache")
ANOMALIES_PROCESSUS: le quick_check a ete passe en arriere-plan par l'outil apres 180 s (197 s reels) ; aucune boucle d'attente.
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress.md, Auto_Journey_File.md, Auto_Journey_Jour.md, EtapeDefini.md, Docs/Rapports/*) : inchange par moi : oui
IN_USE.txt Sauron : OCCUPE puis libre.
