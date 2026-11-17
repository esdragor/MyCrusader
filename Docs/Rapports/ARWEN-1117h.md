# ARWEN-1117h : palier d'unité (HOLUnitTier), une seule source ?

Worktree `HeroesOfLiberty-Arwen`, branche `d751-root`, `git merge main` : déjà à jour. Aucun build, aucun test lancé. Lecture seule du code ; **rien supprimé**.

## Sens D836 pour cette règle
Point (3) : « Camps, Relais, Sites, **Palier d'unité** : le **script devient la source** après le lot de liaisons ; le serveur l'appelle par un pont et le C++ de la règle est supprimé ; d'ici là les deux copies restent, tenues par les tests d'accord. » Point (6) : le palier des héros / Roi / mannequins = colonne `MinTier` de `UnitTypes.csv`, « seule vérité ; le script lit la colonne ». Donc le sens est « script source », la suppression est permise en principe ; elle est refusée ici par les preuves manquantes ci-dessous.

## Registre

| Élément | Décision | Pourquoi |
|---|---|---|
| `HOLUnitTierRules.{h,cpp}` : `HOLUnitTier::Check` + `EBlock` (C++ ~35 lignes) | **GARDÉ** | Appelants vivants (grep `HOLUnitTier::` dans `Source\`) : `HOLGameScreen.cpp` l. 2279-2280 (grille Slate, module `HeroesOfLibertyUI`, grise une unité de palier trop bas), `HOLUnitTierSubsystem.cpp` (repli du pont l. 24 ET implémentation par défaut `CheckUnitTier_Implementation` l. 49), `HOLUnitTierBridgeTests.cpp` (boucle de comparaison pont / règle C++). Le retrait demande de brancher la grille Slate sur le pont (monde du `PlayerController`) : C++ neuf non compilable ici, et le pont n'a jamais été vu vert en C++ (voir preuves). |
| `HOLUnitTier::GetMinTier` (lecture de la colonne `MinTier`) | **GARDÉ, et à garder** | C'est la liaison de lecture de la colonne (D836 point 6), pas une copie de règle. Seul garde-fou entre la table recopiée du script (`MinTierOf`) et le CSV. |
| `HOLUnitTierRulesLibrary.{h,cpp}` (`CheckUnit`, `GetUnitMinTier`) | **GARDÉ** | Sert uniquement aux 3 tests d'accord script ; supprimable avec la règle C++ (inventaire 1117b), mais `GetUnitMinTier` doit survivre (ou être remplacé) car c'est le seul accès script à la colonne. À réduire alors à `GetUnitMinTier` seul. |
| Repli C++ du pont (`UHOLUnitTierSubsystem::CheckUnit`, sans board) | **GARDÉ** | Il s'appuie sur `HOLUnitTier::Check` ; sans lui, un monde sans script ne refuserait plus aucune unité (aucun test ne dit ce qu'on veut dans ce cas). |
| Tests d'accord script (`Test_UnitTierScriptAndSimulationRulesAgree`, `...AgreesOnEveryRowOfTheTable`, `Test_UnitTierSiegeEnginesLikeCpp`) | **GARDÉS** | Seuls tests qui comparent la table `MinTierOf` du script à la colonne du CSV (`ColumnDifferences`, `RowsFound == 16`). Sans équivalent nommé qui lise la colonne, les retirer laisse la table du script (copie typée) sans garde. |
| `HOLUnitTierBridgeTests.cpp` (`HeroesOfLiberty.UnitTier.ServerBoard`) | **GARDÉ** | Teste la chaîne C++ pont -> board script, sans équivalent script possible. Seule sa boucle finale (comparaison à `HOLUnitTier::Check`) serait à remplacer par des valeurs littérales. |
| `HOLUnitTierProductionTests.cpp` (`HeroesOfLiberty.Production.UnitTier`) | **GARDÉ** | Ne duplique pas la règle (il inclut `HOLUnitTierRules.h` sans appeler `HOLUnitTier::`) ; il mesure la vraie chaîne de production. Le port script `HOLUnitTierProduction_Test.as` est PARTIEL (SAURON-1117f : « le test C++ reste »). |
| Tests de la règle en script (`Table`, `OpensSoldiers`, `ReasonOrder`, `AgreesWithTheSiegeRule`, `Monotone`) | gardés (sans doublon C++) | Aucun test C++ de la règle pure n'existe : rien à retirer. |

## Preuves (journal `AngelscriptInProgress.md`)
- Rouge puis vert vus, règle script : ELROND-1116n (D836 point 6, `...AgreesOnEveryRowOfTheTable`, rouge 1351/1354 puis vert 1352/1354, bd64095) ; ELROND-1116q (`Test_UnitTierSiegeEnginesLikeCpp`, rouge 1359/1363, vert 1361/1363, « ServerBoard vert », 5108aa3) ; SAURON-1117f (`HOLUnitTierProduction_Test.as`, rouge 1393/1396, vert 1394/1396).
- Après ces verts, deux lots ont touché la règle ou ses tests **sans build** : ARWEN-1116r (pont serveur, « NON compilé », rouge à voir = bouchon `return Available`) et ELROND-1117i (`HOLUnitTierRules_Test.as` redirigé de `HOLSiege::X` vers `EHOLSiegeBlock`, « PAS BUILDÉ »). L'état courant de la règle script et du pont n'a donc pas été vu vert.
- Aucun test script équivalent nommé ne lit la colonne `MinTier` sans passer par `HOLUnitTierRules::GetUnitMinTier` (C++) : la copie typée `MinTierOf` n'est pas la lecture de la colonne voulue par D836 point 6. La suppression du C++ la ferait dériver sans alarme (ex. une unité ajoutée au CSV).
- La grille Slate (client) lit le C++ ; l'IA (`HOLAIArmy.as`) lit `MinTierOf`. Deux lecteurs de plus que le serveur.

## Ce qui reste pour une seule source (ordre)
1. Lot de build : compiler le pont (`UHOLUnitTierSubsystem`), voir `HeroesOfLiberty.UnitTier.ServerBoard` rouge (bouchon dans `HOLUnitTierBoardScript.as`) puis vert, et les tests `HOLUnitTier*` script verts.
2. Script : `MinTierOf` lit `HOLUnitTierRules::GetUnitMinTier` (colonne, -1 = hors table) au lieu de la table typée ; test script de la colonne (16 lignes) rouge puis vert. Les Trebuchet / Cannon (sans ligne) donneraient alors NotListed d'eux-mêmes ; `HasRowInUnitTypes` disparaît.
3. C++ (un seul lot) : `HOLGameScreen.cpp` appelle `UHOLUnitTierSubsystem::CheckUnit(PlayerController->GetWorld(), ...) == 2` ; `HOLUnitTierRules.{h,cpp}` supprimés (constantes de raisons dans l'en-tête du pont, `GetMinTier` à la bibliothèque) ; repli du pont retiré (ou NotListed) ; `HOLUnitTierRulesLibrary` réduite à `GetUnitMinTier` ; boucle finale de `ServerBoard` en valeurs littérales ; journal de production : afficher le palier du joueur et non `UnitMinTier`.
4. Retirer les 3 tests d'accord script (devenus sans objet) seulement après 1 et 2 verts.
