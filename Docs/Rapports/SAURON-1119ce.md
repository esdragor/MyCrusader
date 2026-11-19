# SAURON-1119ce : fermeture des trous L1b / L2 (NON COMPILE, rien lance)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `grid-l2`, commit `7f4d394` (base 1ba65fb). Pas de merge, pas de push. Debut 17:23, fin ~17:31. ~14 appels vus.
APPELS_DECLARES: ~14. BUILDS_DEMANDES: 0. TESTS_CPP_RETIRES: 0. LIGNES_CPP_SUPPRIMEES: 0. TYPES_A_EXPOSER: aucun (les deux lectures existent deja).
TESTS_SCRIPT_AJOUTES: 2 (`Test_CGCoveragePeasantPopulation`, `Test_CGCoverageNomadForumCell`) + 2 tests modifies (`Test_CGNomadAges`, `Test_CGPopulationBlocked`). TESTS_TAUTOLOGIQUES: 0.

## 1. L1b valeurs en dur (Script\Tests\HOLCommandGridKeys_Test.as)
- Lecture de cvar deja en usage dans Script\ : `const FConsoleVariable X("nom", defaut)` puis `X.GetInt()` (Script\AI\HOLAIGarrisonRules.as:12). Ajout `const FConsoleVariable CGKYNomadForumCell("hol.Nomad.ForumCell", 10);` (l.484). Le cvar est enregistre par le C++ (HOLBuildingTypes.cpp:61, defaut 10) et fixe a 2 par Config\DefaultEngine.ini:57.
- `CGKYMakeNomadBuilders` : `Grid.BuildableCells.Add(CGKYNomadForumCell.GetInt())` (l.503) au lieu de 10. `Test_CGNomadAges` : `const int ForumCell = CGKYNomadForumCell.GetInt();` (l.514) utilise pour `PressKey(Grid, 1, ForumCell)`, `ClickAge(Grid, -1, 1, ForumCell)`, `GetAgeBuilding(Grid, 1, ForumCell)`.
- Population : lecture reelle `HOLProductionTables::FindUnitType(n"Peasant", PeasantRow)` (HOLProductionTablesLibrary.h:105, deja utilise par HOLProductionQueue_Test.as:89), `int Places = PeasantRow.Population;` (l.581) + `AssertTrue(bPeasantRow)`.

## 2. L2 couvertures (fichier neuf Script\Tests\HOLCommandGridCoverage_Test.as, valeurs en clair)
- `Test_CGCoveragePeasantPopulation` : `Peasant.Population == 1` (UnitTypes.csv, colonne Population de la ligne Peasant = 1), bloque a 10/10 et libre a 9/10 avec la valeur de la table. Note : `Test_Production_Tables` (HOLProductionQueue_Test.as:~99) assertait deja `AssertEquals(1, Peasant.Population)` : la couverture n'etait donc pas perdue pour la table, ce test la relie a la regle de blocage.
- `Test_CGCoverageNomadForumCell` : cvar `hol.Nomad.ForumCell == 2` (DefaultEngine.ini:57, D831), forum affiche en page II cellule 2, rien en cellule 10, touche 2 construit le forum (`GetAgeBuilding`, `PressKey`).

## FALSIFICATIONS (une ligne chacune, a jouer par la session principale)
- `Test_CGNomadAges` : HOLCommandGridKeys_Test.as:503 `Add(CGKYNomadForumCell.GetInt())` -> `Add(CGKYNomadForumCell.GetInt() + 1)` : touche C, clic et GetAgeBuilding rouges pour le nomade.
- `Test_CGPopulationBlocked` : HOLCommandGridKeys_Test.as:581 `int Places = PeasantRow.Population;` -> `= PeasantRow.Population + 1;` : "9 / 10, one place wanted: room" rouge (ou, dans Tools\Data\UnitTypes.csv, Population du Peasant 1 -> 2 apres regeneration).
- `Test_CGCoveragePeasantPopulation` : HOLCommandGridCoverage_Test.as:19 `AssertEquals(1, Peasant.Population` -> `AssertEquals(2, Peasant.Population`.
- `Test_CGCoverageNomadForumCell` : HOLCommandGridCoverage_Test.as:33 `AssertEquals(2, ForumCell` -> `AssertEquals(3, ForumCell` (ou Config\DefaultEngine.ini:57 `=2` -> `=3` : rouge sur cette ligne ET sur le GetAgeBuilding en cellule 2).

## 3. Includes `Player/HOLCommandGrid.h` retires (4 fichiers)
Grep fichier par fichier (Regicide, Victory, DamnedWarrior, ProductionPopulationEnd) : aucun `HOLCommandGrid`, `FHOLGrid*`, `EHOLGrid*`, `FHOLCommandFeedback`, `EHOLCommandFeedback`, `FHOLTradeGain`, `EHOLAgeEvent`, `FHOLCycleBuilding`, `FHOLQueueChoice`, `FHOLGroupMember`, `FHOLSubgroup`, `FHOLGridCellFace` : 0 occurrence. Transitifs du header (HOLBuildingTypes.h, HOLConstructionRules.h, HOLResources.h, HOLSpecializationView.h, HOLHeroAbilityRules.h, HOLTechnologyRules.h, ICursor.h) : Regicide utilise `HOLBuildingTypeNames` (inclut HOLBuildingTypes.h direct) ; Victory `HOLConstruction::`, `HOLBuildingTypeNames`, `EHOLConstructionState` (inclut HOLBuildingTypes.h, HOLConstructionRules.h, HOLBuilding.h direct) ; `FHOLResources` dans Victory et ProductionPopulationEnd vient de HOLPlayerState.h / HOLProductionRules.h (incluent Match/HOLResources.h) ; DamnedWarrior : aucun symbole transitif ; aucune occurrence de Technology, Specialization, HeroAbility (type), EMouse, ICursor. Aucun include a remettre. Reste inchange : HOLStartingHeroTests.cpp garde son include (usage l.~713).

## NON VERIFIE (rien compile)
- `const FConsoleVariable` au niveau fichier hors namespace (usage vu seulement dans `namespace HOLAIGarrison`) et que le constructeur `("hol.Nomad.ForumCell", 10)` retrouve le cvar C++ existant plutot que d'en creer un second (si le plugin cree un cvar script de meme nom, `GetInt()` rendrait 10 et le test de la ligne 33 du Coverage serait rouge a tort : alors remplacer par `Console::GetVariable`-equivalent / TYPES_A_EXPOSER: `int32 UHOLTestWorldLibrary::GetConsoleInt(FString Name, int32 Default)`).
- `FHOLUnitTypeSummary.Population` (vu dans HOLProductionQueue_Test.as:99) ; `HOLProductionTables::FindUnitType` visible depuis un autre fichier (vu ainsi dans HOLProductionQueue_Test.as:89).
- Valeur 2 du cvar sous l'editeur de test : suppose que Config\DefaultEngine.ini est charge (D831).
- Cellule 2 (E) : `NumCells` > 10 deja suppose par l'ancien test (cellule 10).

## ANOMALIES
- Git avertit LF -> CRLF sur le fichier neuf (sans effet). Commentaire d'en-tete de `CGKYMakeNomadBuilders` cite encore "cell 10" comme defaut C++ (exact : c'est le defaut, la valeur lue est 2).
- PROCESSUS_DETACHES: aucun (rien lance). EtapeDefini.md, Auto_Journey_*.md non touches.