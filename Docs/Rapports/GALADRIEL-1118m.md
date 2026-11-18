# GALADRIEL-1118m : restes de tests C++ (Formation, ForumGrid)

NON COMPILE (aucun build, aucun quick_check, aucun editeur lance). Branche `galadriel-tab` (base main 4a231ac), worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`. Aucun push, aucune fusion.

BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 2 (HeroesOfLiberty.Formation.Assign, HeroesOfLiberty.Formation.Corridor)
TESTS_SCRIPT_AJOUTES: 0 (les ports existaient deja : Test_FMAssign, Test_FMCorridor dans Script\Tests\HOLFormation_Test.as)
LIGNES_CPP_SUPPRIMEES: 194 (fichier Source\HeroesOfLiberty\Tests\HOLFormationTests.cpp entier, git rm)
LIGNES_MIGREES_PAR_LOT: 0 (suppression de doublon verifie, aucun code ecrit)
TYPES_A_EXPOSER: aucun pour A ; pour B (reste en C++), voir plus bas.
Commit : 8277807 "Tests: remove the two C++ Formation tests (Assign, Corridor), every assertion is in Script/Tests/HOLFormation_Test.as (Test_FMAssign, Test_FMCorridor)" (commit separe, il ne contient que le git rm).
Fichier de suivi : aucun touche (EtapeDefini.md, Auto_Journey_File.md, *InProgress.md laisses a la session principale). IN_USE.txt du worktree : libre.

## A. Formation : table assertion C++ -> test script

Aucune liaison manquante : UHOLFormationReadLibrary expose deja AssignPlaces, MakeSquareRows, MakeMixedRows, MakePlaces, MakeCorridor, GetCorridorAt, GetConstant("FileSlideMaxAngle"). Rien a porter.

### Formation.Assign -> Test_FMAssign (HOLFormation_Test.as l. 332-455)
| Assertion C++ (HOLFormationTests.cpp) | Equivalent script |
|---|---|
| "no unit, nothing given" (Num == 0) | l. 338, meme message |
| "a group in shape keeps its shape" (12 places, positions = places - 3000 en X, PlaceOfUnit[u] == u) | l. 356, meme construction |
| 40 essais, RandomStream(7), Count 2..60, ExtraPlaces 1..5 un essai sur 4 : "trial N: a place of its own for each of K units" | `bAllGiven` + AssertTrue, meme message (dans la boucle des essais) |
| "trial N: no paths of one row cross" (Crossings == 0) | l. 453 |
| "trial N: the rows filled front first" (OutOfOrder == 0, tolerance 120.5 cm) | l. 454 |

Ecart connu et documente par le fichier script (en-tete, l. 7) : les tirages ne sont pas les memes que ceux du C++ (le C++ laissait le compilateur choisir l'ordre d'evaluation de `FVector2D(FRandRange, FRandRange)`) ; les 40 essais sont donc d'autres vecteurs, de meme distribution. La garde `Length > 1e-8` du script remplace `GetSafeNormal` (meme sens).

### Formation.Corridor -> Test_FMCorridor + FMMakeCorridor (l. 637-720)
| Assertion C++ | Equivalent script |
|---|---|
| "files inside the free interval", "slope bounded" (apres chaque Make) | FMMakeCorridor l. ~660-670, memes bornes et tolerances |
| "open field: no shift, no squeeze" | l. ~680 |
| "B: no squeeze" | l. ~692 |
| "B: shifted left by what fits" (Shift[25] == -90) | l. 699 |
| "B: the shift starts before the obstacle" (Shift[18] < -1) | present |
| "B: back on the path after it" (Shift[40] ~ 0) | present |
| "A: squeezed in the gap" (Squeeze[25] == 2/3) | l. 704 |
| "A: the squeeze starts before the gap" (Squeeze[19] < 1 - 1e-3) | present |
| "A: reopened after it" (Squeeze[40] ~ 1) | present |
| "cap: shifted by the cap" (Shift[25] == -100) | present (avant l. 710) |
| "cap: squeezed for the rest" (Squeeze[25] == 100/150) | l. 710 |
| "read between samples" (GetCorridorAt a 2550 -> At == -100) | l. 714 |
| "no corridor: no change" (listes vides -> 0 et 1) | present |

Le script ajoute une garde (50 echantillons, sinon "B: 50 samples" faux) : strictement plus strict.

Pas Mass, pas de monde : les deux tests sont des regles pures, rien ne justifiait de garder le C++.

### Mutations (a jouer apres le build, puis remettre la valeur)
Test cote script (la valeur attendue faussee, le test doit passer rouge) :
1. `Script\Tests\HOLFormation_Test.as` l. 699 : `-90.0` -> `-91.0` : rouge attendu "B: shifted left by what fits" (Test_FMCorridor).
2. `Script\Tests\HOLFormation_Test.as` l. 704 : `2.0 / 3.0` -> `0.5` : rouge "A: squeezed in the gap".
3. `Script\Tests\HOLFormation_Test.as` l. 338 : `AssertEquals(0, ...` -> `AssertEquals(1, ...` : rouge "no unit, nothing given" (Test_FMAssign).
4. Mutation de regle (plus forte, optionnelle) : dans `Source\HeroesOfLiberty\Simulation\HOLFormationRules.cpp`, AssignPlaces, inverser l'ordre de tri des rangees : doit rougir "the rows filled front first" (l. 454) sur au moins un essai. A ne jouer que si un build est de toute facon prevu (necessite C++).
Rouge non vu a ce jour (aucun lancement) : TEST_ROUGE: non vu, a jouer par la session principale.

Pour retirer le C++ en toute securite : le total `quick_check tests` doit descendre de 2 (le compte 1525 devient 1523, sauf si les tests script sont comptes a part ; a verifier a la reception).

Suspect de compilation : aucun fichier de code ecrit. Le seul risque : un `.cpp` ou un `.vcxproj` listant explicitement HOLFormationTests.cpp (build unity : le glob de Source\ prend le fichier tout seul, grep fait sur Build.cs sans resultat attendu, mais un build propre le confirmera ; si l'echec est "cannot open HOLFormationTests.cpp", regenerer les fichiers de projet).

## B. HeroesOfLiberty.Specialization.ForumGrid : GARDE en C++ (non porte)

Fichier `Source\HeroesOfLibertyUI\Tests\HOLSpecializationGridTests.cpp` (108 l.) conserve. Il n'est couvert ni par HOLCommandGrid_Test.as (son en-tete dit : "Not read by any test (not exposed): the specialization view (SetSpecialization, GetOpenSpecializationTier)") ni par HOLSpecializationRules_Test.as (regles pures de la planche : Start / Cancel / Advance / IsOpen, pas les cellules de la grille).

Ce qui bloque, precisement :
1. Monde + GameState + planche script : le test cree un `UWorld`, un `AHOLGameState`, et lit `UHOLSpecializationSubsystem::Find(World)` (la sous-classe script `UHOLSpecializationBoardScript`), puis appelle `StartNow`, `AdvanceNow`, `ViewNow` (C++ non-UFUNCTION, `Match\HOLSpecializationSubsystem.h` l. 80-93). Aucun test script ne cree de monde ; la chaine "planche -> FHOLSpecializationView -> grille" ne peut pas etre rejouee sans lui (la remplacer par des champs poses a la main serait une valeur recopiee, pas la vraie chaine).
2. `HOLCommandGrid::SetSpecialization(Context, PlayerTier, View)` et `GetOpenSpecializationTier` ne sont pas exposes dans UHOLCommandGridReadLibrary (le FHOLGridContextView porte deja SpecializationTier / PendingOption / RemainingMs, l. 123-127, et GetCommand les copie, .cpp l. 32-34 ; GetSpecializationCell et GetReportName sont exposes).
3. Les 8 assertions sur le texte de la cellule ("two lines at most", "9 characters", secondes restantes, nom dans l'infobulle) passent par `HOLGameNames::GetSpecializationCellText` / `GetSpecializationName`, dans le module `HeroesOfLibertyUI` (HUD\HOLGameNames.h) : une liaison script demanderait une bibliotheque de lecture dans le module UI, qui n'existe pas.

Portage possible plus tard (lot C++ unique, pour la session principale) : (a) `UHOLCommandGridReadLibrary::SetSpecialization(FHOLGridContextView&, int PlayerTier, FHOLSpecializationView)` ; (b) bibliotheque de lecture UI pour GetSpecializationCellText / GetSpecializationName ; (c) pour la chaine reelle, un test script qui obtient le monde de test (si un harnais de monde script existe) ou lecture de `UHOLSpecializationBoardScript` instanciee directement. Les assertions purement grille (cellules distinctes, Peasant reste en A, tier 0 : rien, Specialize tier 1 option 0/1, Cancel + RemainingMs 30000 une fois lancee, l'autre option reste Specialize, noms de rapport CancelSpecializationT1B / SpecializeT1A, tier 1 final : aucune cellule, tier 2, autre batiment : aucune cellule) sont portables des (a) ; je n'ai pas ecrit un port partiel : le C++ resterait de toute facon, et une erreur de compilation dans HOLCommandGrid_Test.as ferait tomber tout le module de tests script sans pouvoir la voir (aucun build autorise).

Mutation : aucune (rien porte, aucun test script ajoute pour B).

## Anomalies
- Un appel Bash a ete refuse une fois par le classifieur (un hook reecrit les commandes en `rtk ...`) ; relance a l'identique, passe.
- `git rm` + commit enchaines dans le meme appel que des greps : le code de sortie 1 venait d'un `ls` sur un chemin inexistant, pas du commit (8277807 verifie par `git log`).
- Horloge : `Get-Date` PowerShell 19:28 au debut, 19:30 a la fin ; Git Bash `date` concorde ici (19:29).
