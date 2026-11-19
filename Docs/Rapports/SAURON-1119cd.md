# SAURON-1119cd : lot L2 (retrait des extraits `HOLCommandGrid::` des 5 tests C++ du groupe A) : NON COMPILÉ

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `grid-l2`, base ae64d0e. Commit `1ba65fb` (5 fichiers, +4 / -53). Rien compilé, rien lancé. Pas de merge, pas de push.
BUILDS_DEMANDES: 0 (le lot C++ de la séance compile). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 0. LIGNES_CPP_SUPPRIMEES: 53 (49 nettes; 4 lignes de commentaire ajoutées).
Début 17:07, fin 17:09 (Get-Date). Appels vus : ~21.

## Par fichier (numéros de ligne = HEAD ae64d0e avant retrait)

1. `HOLRegicideTests.cpp` (FHOLRegicideCommandsTest, "Victory.Regicide.Commands") : retiré 288-299 (+ ligne vide 300) et l'include `Player/HOLCommandGrid.h` (l.19). Couvert par `Test_CGKingsOnly` (`HOLCommandGridRules_Test.as:439`), 6 assertions sur 6 (292, 293, 294, 295, 298, 299), mêmes entrées. Gardé : toute la partie `HOLCombat::` (FSeeker, ChooseTarget...) qui suit. -14 lignes.
2. `HOLVictoryTests.cpp` (forum, "grid of a forum down") : retiré 1195-1204 (commentaire + `FHOLGridContext Grid` + 3 `TestTrue`) et l'include (l.25). Couvert par `Test_CGForumDown` (`HOLCommandGridRules_Test.as:466`), 3 assertions sur 3 (1201, 1203, 1204). Gardé : tout le début du test (apparence du forum en ruine, animation). -11 lignes.
3. `HOLDamnedWarriorTests.cpp` : retiré 228-235 (`FHOLGridContext Selection` + 2 `TestTrue` de la grille) et l'include (l.16). Couvert par `Test_CGRageButton` (`HOLCommandGridRules_Test.as:483`), 2 assertions sur 2 (233, 235). Le commentaire est réduit à la partie simulation (IssueRageOrder) + renvoi au test script. Gardé : `IssueRageOrder`, `RageMs`, cool-down, tout le reste. -11 / +2 lignes.
4. `HOLProductionPopulationEndTests.cpp` : retiré 155-157 (`TestTrue "the queue says population full"` avec `HOLCommandGrid::IsPopulationBlocked(Used, Cap, Row->Population)`) + include (l.18). Couvert par `Test_CGPopulationBlocked` (`HOLCommandGridKeys_Test.as:568`) pour la règle (10, 10, 1) plus bornes. Gardé : les asserts de simulation 138, 139, 154 (Used = 10, Cap = 10) ; `Row` reste utilisé (l.119-124). -4 lignes.
5. `HOLStartingHeroTests.cpp` (FHOLStartingNomadFirstForumTest) : retiré 648-662 (déclarations de `FHOLGridContext Grid`, `PressKey` x3, `ClickAge`, 4 assertions) remplacé par un commentaire de 2 lignes renvoyant à `Test_CGNomadAges` (`HOLCommandGridKeys_Test.as:508`). Couverture : 4 assertions sur 4 (655-656 ChosenAge, 657-658 forum key, 659-660 II A verrouillée, 661-662 clic) ; le script teste les deux sièges comme le C++. Gardé : l.647 (le forum est dans la liste du siège), `ForumCell` (utilisé par `Lists`), tout le placement, les coûts et la liste après fondation. -17 / +2 lignes environ.

## RESTE (assertions SANS équivalent script, gardées volontairement)

- `HOLStartingHeroTests.cpp` : include `Player/HOLCommandGrid.h` (l.24) et UN appel `HOLCommandGrid::GetAgeBuilding` (lambda `ForumCell` de `FHOLStartingNomadReplicatedTest`, ~l.713-723 après retrait). Raison : ses 6 usages (client avant réplication : pas de forum, après : cellule II C = Build forum non grisée, client standard : pas de forum, après premier forum : cellule fermée, siège repris : plus de cellule) prouvent la chaîne RÉELLE état de jeu répliqué -> `GetBuildableTypesForSlot` -> grille. `Test_CGNomadAges` appelle `GetAgeBuilding` sur un contexte construit à la main : il couvre la règle, pas cette chaîne. Les retirer perdrait 6 assertions : elles restent (à migrer plus tard par un test qui lit la liste directement, ou à garder pour L3). Conséquence : L3a ne peut pas supprimer `HOLCommandGrid::GetAgeBuilding` (et son `*Native`) tant que ce test C++ l'appelle ; il faut le faire passer par `HOLCommandGridRules` exposé en C++ ou conserver le wrapper.
- `Test_CGPopulationBlocked` suppose `Places = 1` ; la valeur de table `Row->Population` du Paysan n'est plus lue nulle part (l'assertion C++ la lisait). Aucun assert de simulation ne la vérifie : écart minime.
- Écart du test script du nomade : la cellule du forum est la constante 10 (C) ; le C++ lisait le cvar `hol.Nomad.ForumCell` (E = 2 dans DefaultEngine.ini). Le lien cvar -> cellule n'est plus testé que par la ligne 647 (`Lists`).
- Le chemin « liste réelle du siège (`GetBuildableTypesForSlot`) -> touches (`PressKey`/`ClickAge`) » du test 5 n'est plus affirmé : le script prend une liste construite à la main.

## Vérification par Grep (fin)

`HOLCommandGrid::` : plus aucun dans Regicide, Victory, DamnedWarrior, ProductionPopulationEnd. Reste : `HOLStartingHeroTests.cpp:713` (voir RESTE). `HOLCommandGrid.h` : reste inclus seulement dans `HOLStartingHeroTests.cpp:24`.

## Nombre de tests C++ inchangé : OUI

Aucun `IMPLEMENT_SIMPLE_AUTOMATION_TEST` supprimé ni renommé (5 fichiers : seuls des corps de `RunTest` raccourcis ; `git diff HEAD~1` ne contient aucune ligne `IMPLEMENT_`). Le total de `quick_check` ne baisse donc pas côté C++.

## NON VÉRIFIÉ (rien compilé)

- Dépendance transitive des includes retirés : `Player/HOLCommandGrid.h` retiré de Regicide, Victory, DamnedWarrior, ProductionPopulationEnd. Si l'un de ces fichiers utilisait par ricochet un type que ce header tire (p. ex. `FHOLBuildable`, `FHOLGridCommand`, `HOLBuildingTypeNames`, ou un type de `HOLUnitView`), la compilation casse : le Grep des types de la grille (`FHOLGridContext`, `EHOLGridCommand`, `FHOLGridKeyResult`, `EHOLGridKeyTarget`) ne trouve plus rien dans ces 4, mais les types tirés par ricochet n'ont pas été cherchés un par un. Si erreur « undeclared identifier » : remettre l'include dans ce fichier (un seul `#include`).
- Les 5 tests script de la ligne de base (ae64d0e) sont supposés VERTS (la tâche le dit) ; non vu.
- Avertissement d'unités : aucune variable locale laissée sans usage selon relecture (`Grid`, `Kings`, `Mixed`, `Selection` partis avec leur bloc ; `List` et `ForumCell` du test 5 restent utilisés) ; non compilé.
- `Test.RunStep` / `Row` : `Row->Population` n'est plus utilisé, mais `Row` l'est toujours (l.120-124).
- Fins de ligne : les fichiers sont retraités par lignes (CR conservés quand présents) ; git avertit « LF will be replaced by CRLF » sur `HOLProductionPopulationEndTests.cpp` (fichier déjà en LF dans l'arbre de travail avant l'édition ; diff de 4 lignes seulement, donc pas de réécriture complète).

## Commande pour la session principale

Dans le lot C++ : `Tools\build.ps1 -LiveCoding=0 -Wait` sur `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, puis `quick_check -Parts tests` ; vérifier que les tests `HeroesOfLiberty.Victory.Regicide.Commands`, `...Victory.Forum...`, `...DamnedWarrior...`, `...Production...PopulationEnd...`, `...StartingHero.NomadFirstForum` / `NomadReplicated` restent verts.

## Anomalies

- Le plan estimait -60 à -80 lignes ; 53 retirées car le 6e usage (`ForumCell`, Replicated) reste.
- `EtapeDefini.md`, `Auto_Journey_File.md`, `IN_USE.txt`, `*InProgress.md` non touchés.
