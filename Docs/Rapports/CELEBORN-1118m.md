# CELEBORN-1118m : fin des tests C++ MatchConfig et Construction (reprise 12, 2026-11-18)

**NON COMPILÉ. Aucun build, aucun quick_check, aucun lancement.** Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `hero-j8-close` (sur main 4a231ac), 3 commits, jamais poussés ni fusionnés. Aucun C++ neuf (BUILDS_DEMANDES: 0) : tout le script passe par des fonctions déjà présentes des bibliothèques de lecture.

## Commits
| sha | contenu |
|---|---|
| 3ec9b28 | retire de `Source\HeroesOfLiberty\Tests\HOLMatchConfigTests.cpp` les tests `Match.PlayerColors` et `Match.NeutralUnitColor` (et l'include `HOLPlayerColors.h`) ; commentaire d'en-tête du `.as` mis à jour |
| 506f632 | ajoute `Test_Construction_Tables` (+ aides `ConSameInts`, `ConInts`, `ConCheckEconomicType`) à la fin de `Script\Tests\HOLConstruction_Test.as` |
| 1e9d0b0 | `git rm Source\HeroesOfLiberty\Tests\HOLConstructionTests.cpp` (commit séparé, facile à revert) |

## A. MatchConfig
Comparaison assertion par assertion (C++ -> `Script\Tests\HOLMatchConfig_Test.as`) :

| test C++ | assertion C++ | test script | verdict |
|---|---|---|---|
| SubsystemFile | fichier écrit | `Test_MatchConfigSubsystemFile` : `RunSubsystemFile` renvoie vrai | équivalent |
| | pas de config au départ | `bNoConfigAtFirst` | équivalent |
| | défaut sans config | `bDefaultWhenNone` | équivalent |
| | fichier lu | `bLoaded` | équivalent |
| | config issue du fichier | `bConfigFromFile` | équivalent |
| | fichier manquant refusé + config gardée | AUCUNE | **non portable** (voir plus bas) |
| PlayerColors | 8 couleurs distinctes, toutes nommées | `Test_MatchConfigPlayerColors` : `GetPaletteDWColors` distinctes, `GetPaletteColorName` non vide | équivalent |
| | l'index NumColors boucle sur 0 | `PaletteLinearColorsEqual(NumColors, 0)` | équivalent |
| NeutralUnitColor | 2 variantes (identité, `(3*s+1)%8`) | `Test_MatchConfigNeutralUnitColor`, mêmes vecteurs | équivalent |
| | propriétaire neutre n'a la couleur d'aucune palette (tolérance 0.001) | `OwnerColorIsPaletteColor(Neutral, ..., 0.001f)` faux pour chaque couleur | équivalent |
| | chaque slot garde sa couleur (égalité exacte) | `OwnerColorIsPaletteColor(Slot, ..., tolérance 0.0f)` | équivalent |

Test C++ gardé : **`Match.SubsystemFile`** (le fichier `HOLMatchConfigTests.cpp` n'est donc PAS supprimé, il passe de 3 tests à 1). Raison : le refus d'un fichier manquant fait écrire à `UHOLMatchSubsystem::LoadFromFile` une ligne Error « cannot read » ; seul le test C++ peut la déclarer attendue (`AddExpectedError`), un test script ne le peut pas, il serait rouge. Le reste de ce test est déjà en double en script. Pour le porter un jour il faudrait une liaison C++ qui déclare l'erreur attendue (pas fait : hors périmètre, build).

Lignes C++ supprimées dans ce commit : 45 (+1 ligne de commentaire d'en-tête).

### Mutations A (à jouer par la session principale, après build)
- PlayerColors : `Source\HeroesOfLiberty\Match\HOLPlayerColors.cpp:19` `FColor(230, 120, 170)` -> `FColor(230, 120, 30)` (rose = orange). Rouge attendu : `Test_MatchConfigPlayerColors` « distinct colours ».
- NeutralUnitColor : `HOLPlayerColors.cpp:57` `FColor(135, 135, 135)` -> `FColor(40, 90, 220)` (gris neutre = bleu, palette 0). Rouge attendu : `Test_MatchConfigNeutralUnitColor` « neutral colour is not palette colour 0 ».
- SubsystemFile (script) : `Source\HeroesOfLiberty\Match\HOLMatchConfigReadLibrary.cpp:253` `bOutLoaded = Match->LoadFromFile(Path);` -> `bOutLoaded = false;`. Rouge attendu : « file read ».
Remettre ensuite les valeurs ; si une mutation ne donne pas de rouge, `git revert 3ec9b28` (rend le test C++ correspondant).

## B. Construction
`Construction.Tables` réécrit en script : `Test_Construction_Tables`, lit les lignes par `HOLConstructionRead::FindBuildingType` (déjà dans la bibliothèque, ainsi que `IsBuildingTypeBuildable`, `GetBuildingTypeCategory`, `GetBuildingTypeAccepts`, `FindSourceTypeInfo`, `GetSettingTileSize`, `GetSettingAntiRushRadius`, `GetSettingBuilderExponent`, `GetWorkPerStep`, `GetWorkPerBuilder`). Ressources en ints (0 Food, 1 Wood, 2 Stone, 3 Gold), catégories (0 Economic, 1 Military, 2 Technological).

| bloc C++ Tables | script |
|---|---|
| tuile 4 m ; rayon anti-rush > 0 ; 3 bâtisseurs ~2x (réglage `BuilderExponent`, pas la constante 0.63 du test Builders) | début de `Test_Construction_Tables` |
| centre principal : non bâti par paysans, 4x4 tuiles | idem |
| House / LumberCamp / MiningCamp / Mill (slot, population, Accepts, coût bois seul, temps > 0, 2x2 tuiles, plus petit que le centre) | `ConCheckEconomicType` x4, mêmes vecteurs |
| Barracks (7 asserts), ArcheryRange (4), Sanctuary (12), Chapel (12), Market (13) | blocs un pour un, mêmes valeurs |
| boucle « bloque le passage, pas de source » (7 types) | boucle sur `TArray<FName>` |
| Farm (13 asserts + ligne de la table des sources : liée, nourriture) | bloc + `FindSourceTypeInfo` |

Équivalences notées : `TestEqual` de flottants -> `ConNear` (tolérance 0.01) ; `Produces == {Monk}` -> `Num()==1 && [0]==n"Monk"` ; `Source.IsNone()` -> `Source == n"None"`.

`Construction.Body` : ses 6 asserts sont tous dans `Test_Construction_BodySettings` (déjà en script) : réglage `BodyInset` (un paysan passe, pas deux), corps d'une maison, forme visible >= 90 %, visible <= corps, forme visible d'une maison. Le C++ ne tenait déjà que ces asserts (le reste est dans `Test_Construction_Body`). Équivalent, aucune assertion perdue.

Résultat : `HOLConstructionTests.cpp` supprimé entièrement (1 commit séparé). Lignes C++ supprimées : 280 environ (fichier entier) ; lignes script ajoutées : 218.

### Mutations B
- Tables : `Config\DefaultGame.ini:94` `BuilderExponent=0.63` -> `1.0`. Rouge attendu : « 3 builders ~2x as fast (D312) » (rapport 3 au lieu de 2). Aucune régénération nécessaire.
- Tables : `Config\DefaultGame.ini:91` `TileSize=400` -> `300`. Rouge attendu : « 4 m tiles » et les empreintes `Grid` (attention : ce réglage touche aussi d'autres tests, ne lire que Tables).
- Tables (données) : `Tools\Data\BuildingTypes.csv:13` (Chapel) `BuildGridSlot` 5 -> 6, puis régénérer la table des types ; rouge attendu : « Chapel: S of the Peasant's grid (slot 5...) ». Plus lourd (régénération du `.uasset`) : à n'utiliser que si les deux premiers ne suffisent pas.
- BodySettings (couvre Body) : `Config\DefaultGame.ini:82` `BodyInset=115` -> `40`. Rouge attendu : « the setting: one Peasant passes, not two ». Et `ini:87` `VisualInset=30` -> `100` : rouge « the visible shape: at least 90 % of a house's tiles ».
Remettre ensuite les valeurs ; si une mutation ne donne pas de rouge, `git revert 1e9d0b0` (rend le C++).

## Champs
- BUILDS_DEMANDES: 0
- TESTS_CPP_RETIRES: 3 (Match.PlayerColors, Match.NeutralUnitColor, Construction.Tables) + Construction.Body (fichier supprimé) = 4 ; Match.SubsystemFile gardé
- TESTS_SCRIPT_AJOUTES: 1 (`Test_Construction_Tables`) ; MatchConfig : 0 (déjà portés par 1117n/o)
- LIGNES_CPP_SUPPRIMEES: ~325 (45 MatchConfig + ~280 Construction)
- LIGNES_MIGREES_PAR_LOT: 218 lignes script
- TYPES_A_EXPOSER: aucun
- Fichier `HOLMatchConfigTests.cpp` : reste 1 test, `HOLConstructionTests.cpp` : supprimé. Le compte du total quick_check baisse de 4 tests C++ et monte de 1 test script (Tables) : les 3 tests MatchConfig script existaient déjà (compte attendu après build à confirmer, un test script par fonction `Test_*`).

## Suspects de compilation script (rien n'a été compilé)
1. `Row.Source == n"None"` : littéral FName `n"None"` supposé égal à `NAME_None` (aucun usage de `.IsNone()` ni `NAME_None` ailleurs dans `Script\`).
2. `FindSourceTypeInfo(Farm.Source, bLinked, FieldResource)` : paramètres `UPARAM(ref) bool&` et `int32&` reçoivent des locales `bool` / `int` ; si le compilateur refuse, déclarer `bool&out` n'est pas possible : passer à des sorties par tableau ou corriger la liaison.
3. Paramètres par défaut d'une fonction retournant `TArray<int>` (`ConInts(int A = -1, int B = -1)`) : autorisé en script (déjà `double Tol = 0.01`), mais appelé `ConInts()` sans argument.
4. `Name.ToString()` sur FName et `Blocking[Index].ToString()` : supposés présents.
5. `T.AssertEquals(int, int, FString)` partout : valeurs int32 (`BuildGridSlot`, `Age`, `MaxHealth`, `PopulationProvided`), pas de flottants dans `AssertEquals`.
6. `Cost.Food/Wood/Stone/Gold` lus directement comme ints sur `FHOLResources` (champs utilisés par `ConRes` déjà).
7. Aucun nom global nouveau ne risque la collision d'unity build (C++ : seulement des suppressions) ; en script `ConSameInts`, `ConInts`, `ConCheckEconomicType`, `Test_Construction_Tables` sont uniques dans le fichier (non vérifié contre les autres `.as` : fonctions globales visibles seulement dans leur module).
