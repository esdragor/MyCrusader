# SAURON-1119cc : lot L1b (tests script, forum nomade, ages, population)

NON COMPILE, rien lance (pas de build, pas de quick_check, pas d'editeur). Branche `grid-l1b` du worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, base 28dfc85. merge main non fait. Rien pousse.

HEURE_DEBUT/HEURE_FIN: 17:02 / ~17:05. APPELS_DECLARES: ~25. BUILDS_DEMANDES: 0. TESTS_CPP_RETIRES: 0. LIGNES_CPP_SUPPRIMEES: 0. TYPES_A_EXPOSER: aucun.
TESTS_SCRIPT_AJOUTES: 2 (`Test_CGNomadAges`, `Test_CGPopulationBlocked`). TESTS_TAUTOLOGIQUES: 0 (chaque valeur est calculee par `HOLCommandGridKeys::` / `HOLCommandGridRules::`).

## Commits (un seul fichier touche : `Script\Tests\HOLCommandGridKeys_Test.as`, +~100 lignes en fin de fichier)
- 02d5b79 Command grid tests (L1b): Nomad forum ages page II keys and click, population blocked at 10/10 ...
- b0a2906 Command grid tests (L1b): compare the Nomad flags with AssertTrue (AssertEquals(bool,bool) : aucun precedent dans Script\, remplace par `AssertTrue(bNomad == (...))`).

## Tests

### Test_CGNomadAges (commentaire `HeroesOfLiberty.CommandGridKeys.NomadAges`)
Helper `CGKYMakeNomadBuilders(bool bNomad)` : contexte des batisseurs, ReachedAge 0, buildables : House (age 0, cellule 0), Market (age 1, cellule 0, pas always-open), et pour le nomade seulement MainCentre (age 1, cellule 10 = C, always-open), comme `UHOLBuildingSettings::GetBuildableTypesForSlot` (HOLBuildingTypes.cpp:72-82). Boucle sur les deux sieges (standard, nomade). Assertions :
- `PressKey(Grid, -1, 1)` : ChosenAge 1 (nomade) / -1 (standard), Target Age seulement pour le nomade. (C++ HOLStartingHeroTests.cpp:655-656)
- `PressKey(Grid, 1, 10)` : Build + TypeName MainCentre pour le nomade, jamais pour le standard. (C++ :657-658)
- `PressKey(Grid, 1, 0)` : pas de Build (le Marche, II A, reste verrouille au palier 0), ChosenAge 1 conserve. (C++ :659-660)
- `ClickAge(Grid, -1, 1, 10)` : meme resultat que la touche (Build MainCentre nomade, rien en standard), meme ChosenAge que KeyC pour le nomade. (C++ :661-662)
- `HOLCommandGridRules::GetAgeBuilding(Grid, 1, 10)` : cellule du forum montree (Command != None, TypeName MainCentre) seulement pour le nomade, jamais grisee ; cellule Marche (1, 0) grisee. (C++ :726, lambda ForumCell de FHOLStartingNomadReplicatedTest 717-727)
- `GetChosenAgeAfter(1, Placed) == -1`, `(1, PlacedKept) == 1` (ajout hors C++ 648-661, meme famille).
Ecart assume : la cellule du forum est la constante 10 (C, defaut C++ de `hol.Nomad.ForumCell`) ; le C++ lit le cvar (E = 2 dans DefaultEngine.ini, D831) : le script ne lit pas le cvar, le contexte est construit a la main. La tache disait « A de la maison » ; le C++ dit « II A (the Market) » : j'ai mis le Marche en II A (le TypeName est sans effet sur le resultat, la cellule est grisee).

### Test_CGPopulationBlocked (`HeroesOfLiberty.CommandGridKeys.PopulationBlocked`)
`HOLCommandGridKeys::IsPopulationBlocked` EXISTE en script (HOLCommandGridKeys.as:136). Couvre HOLProductionPopulationEndTests.cpp:156-157 (Used 10, Cap 10, Places = `Row->Population` du Paysan, pris egal a 1 : valeur de la table NON relue). Assertions : (10,10,1) bloque ; (9,10,1) libre ; (9,10,2) bloque ; (10,10,0) libre ; (11,10,1) bloque. Note : `HOLCommandGridKeys_Test.as` avait deja (9,10,2) et (9,10,1) dans `Test_CGKYHardValues`; le cas ajoute de fait est le plein 10/10 du C++ et les bornes.

## FALSIFICATION (rouge a voir, une par test ; recherche par texte, lignes ~480-585 du fichier)
- `Test_CGNomadAges` : dans `CGKYMakeNomadBuilders` (`Script\Tests\HOLCommandGridKeys_Test.as`, ~l.490-500) mettre la cellule du forum a 9 au lieu de 10 (les 3 `Grid.BuildableCells.Add(10)`... un seul : celui du MainCentre) : `KeyC`, `Clicked` et `GetAgeBuilding(Grid, 1, 10)` rougissent pour le nomade. Alternative : permuter les cellules attendues dans `PressKey(Grid, 1, 10)` et `PressKey(Grid, 1, 0)` (C et A).
- `Test_CGPopulationBlocked` : attendre « bloque » a `Used = Cap - 1` : changer l'assertion `AssertFalse(bRoomForOne, ...)` en `AssertTrue` (rouge), ou dans `HOLCommandGridKeys.as:138` remplacer `>` par `>=` (rouge sur (9,10,1)... et sur le test existant).
Les lignes exactes ne sont pas relues apres ecriture : chercher `AssertTrue(bNomad == (PageTwo` (l.528) et `bRoomForOne`.

## NON VERIFIE (rien compile)
- `T.AssertTrue(bNomad == (PageTwo.Target == EHOLGridKeyTargetKind::Age), f"{Name}: ...")` : comparaison bool == bool et f-string avec `{Name}` (FString) : syntaxe supposee d'apres le reste du fichier (`f"..."` avec ints), non vue compilee avec un FString.
- `FString Name = "standard";` puis `Name = "nomad";` : supposee valide.
- `Grid.BuildableAlwaysOpen.Add(false)` : champ `TArray<bool> BuildableAlwaysOpen` de `FHOLGridContextView` (HOLCommandGridReadLibrary.h:107) supposee exposee BlueprintReadWrite comme les autres (utilise ainsi ligne 80 du meme fichier : OK).
- `KeyC.TypeName == n"MainCentre"` (FName), `Building.Command != EHOLGridCommandKind::None`, `Building.bGreyed` : membres vus dans HOLCommandGridKeys.as:50,75-78.
- Test sur le nom du cvar : aucun.
- Les noms de tests (`Test_*`) sont decouverts par prefixe comme les voisins ; le compte attendu de `quick_check` tests script : +2.
Commande a lancer par la session principale : `quick_check -Parts <partie des tests script>` sur le worktree Sauron apres compilation des scripts (aucun build C++ requis).

## ANOMALIES
- Un message systeme a signale que `HOLCommandGridKeys_Test.as` avait change sur disque : c'etait mon propre remplacement PowerShell (AssertEquals bool -> AssertTrue).
- Pas de `git status` de la copie principale lance. PROCESSUS_DETACHES: aucun (aucun processus lance).
