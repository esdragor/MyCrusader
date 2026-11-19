# HALDIR-1119y : lot G2b (cycle, sous-groupes, groupes, MergeCommands) en script, NON COMPILE

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `selection`, commit `508ebfc` (2 fichiers neufs, 630 lignes). Aucun build, aucun quick_check, aucun jeu lance. `Crown.umap` reste non suivi. `merge main` non fait (consigne).

## Fichiers
- `Script\Player\HOLCommandGridGroups.as` (`namespace HOLCommandGridGroups`, ne depend PAS de `HOLCommandGridRules::`, n'appelle que des USTRUCT vues de `UHOLCommandGridReadLibrary`).
- `Script\Tests\HOLCommandGridGroups_Test.as` : 5 tests, `Test_CGGSubgroupsSweep`, `Test_CGGMergeSweep`, `Test_CGGCycleSweep`, `Test_CGGDistributionSweep`, `Test_CGGHardValues`.

## Fonctions portees (HOLCommandGrid.cpp 402-557)
GetCellFace, GetCycleRank, IsCategorySelectedByCategory (= IsSelectedByCategory), IsRepeatPress, NextInCycle, IsAloneInGroup, SplitSubgroups, GetCommandPriority, MergeCommands, PickEvenly. En plus : GetNumCells (accesseur, constante locale 12).

Ecarts de forme (decisions identiques) :
- cles int32 dans les vues (bit pattern du uint32) : le tri compare en non signe (`AsUnsigned`), comme le C++ ; le balayage du cycle inclut des cles negatives ;
- tri par insertion (stable) a la place de `Sort` avec lambda (le C++ n'est pas stable, mais rang + cle sont distincts dans le jeu) ;
- `MergeCommands` : cellules des batiments bout a bout (i * 12 + c), comme `HOLCommandGridRead::MergeCommands` (pas de TArray imbrique en script) ; ne peut pas dire un batiment a moins de 12 cellules (le C++ l'accepte) ;
- `GetCycleRank(bBuildable, BuildGridSlot)` et `IsCategorySelectedByCategory(TypeCategory, Category)` prennent les valeurs lues de la ligne `FHOLBuildingTypeRow` (struct non exposee) en parametres.

## Non portees
Aucune de la plage 402-557. Reste au pont : les appelants qui ont une `FHOLBuildingTypeRow` en main (HOLGameScreen, HOLSelection...) lisent la ligne puis passent `IsBuildable()`, `BuildGridSlot`, `Category`.

## Tests (accord contre le C++ par `HOLCommandGridRead::`)
- Sous-groupes : 5461 listes de membres (0 a 6 membres, 4 types dont le centre seul) : Split + Priority.
- Merge : 400 graines x 1 a 4 batiments, priorites avec indices hors bornes et repetes.
- Cycle : 300 graines x 1 a 5 batiments (cles negatives, rangs egaux) x chaque cle + cle absente ; IsRepeatPress 64 x 2 ; GetCellFace ; 9 paires de categories.
- Distribution : 216 files x 4 dernieres cles.
- Valeurs en dur (copiees des vecteurs de `HOLCommandGrid_Test.as:246-263, 343-425, 428-453`, port de `Source\HeroesOfLiberty\Tests\HOLCommandGridTests.cpp`) : cycle, sous-groupes, priorite, fusion mill/forum/barracks, distribution, GetCycleRank.

## Rouge a voir (dans `HOLCommandGridGroups.as`)
Dans `PickEvenly`, remplacer `(Last + Step) % Choices.Num()` par `(Last + Step + 1) % Choices.Num()` : `Test_CGGDistributionSweep` et `Test_CGGHardValues` rouges. Autres : `SortsBefore` `Rank < Rank` -> `>` ; `n"MainCentre"` -> autre nom dans `IsAloneInGroup`.

## Commande a lancer (session principale)
Build `Tools\build.ps1 -LiveCoding=0 -Wait` n'est pas necessaire (script seul, la DLL de la branche doit deja porter `HOLCommandGridReadLibrary` avec les vues FHOLCycleBuildingView etc. : verifier que le binaire est a jour avec `Source\` de `selection`) ; puis `quick_check -Parts` des tests script, filtre `CGG`.

## Champs
- BUILDS_DEMANDES: 0
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 5 (Test_CGGSubgroupsSweep, Test_CGGMergeSweep, Test_CGGCycleSweep, Test_CGGDistributionSweep, Test_CGGHardValues)
- LIGNES_CPP_SUPPRIMEES: 0
- LIGNES_MIGREES_PAR_LOT: ~156 lignes C++ (402-557) portees
- TYPES_A_EXPOSER: aucun pour ce lot (la bibliotheque expose deja tout). Optionnel, pour retirer les parametres de ligne au pont : `static int32 GetCycleRankOfType(FName)` et `IsTypeSelectedByCategory(FName, EHOLBuildingCategory)` existent deja ; rien d'autre a ajouter.
- TEST_ROUGE: NON VU (non compile)
- PROCESSUS_DETACHES: 0 (aucun processus lance)

## NON VERIFIE
Compilation Angelscript de tout (surtout : `int64(0xFFFFFFFF)`, `Math::Min`, `f"..."` avec FString, affectation d'un champ d'element `PerBuilding[0].Command = ...`, noms `Test_CGG*` pris comme tests par la convention `Test_` de `HOLCommandGrid_Test.as`). Les tests de balayage ne passent par le C++ que si la bibliotheque de lecture est compilee dans la DLL.
