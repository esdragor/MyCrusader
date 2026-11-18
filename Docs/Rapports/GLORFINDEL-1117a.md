# GLORFINDEL-1117a : lot L17-05 (HOLMinimapAlertRingTests + HOLMinimapTerrainTests)

Verdict : NON PORTE, "C++ bibliotheque requise". Aucune regle testee n'est lisible du script.

## Verification (Grep)
- `UHOLMinimapView::AlertDiameter` : defini dans `Source\HeroesOfLibertyUI\HUD\HOLMinimapView.cpp` l.319 (declare `.h` l.187, membre statique de la classe Slate/UMG). Aucune copie dans `Script\UI\HOLMinimapRules.as` ni ailleurs en .as.
- `HOLMinimapTerrain::GetSourcesSignature` : defini dans `HOLMinimapView.cpp` l.91 (declare `.h` l.19, `HEROESOFLIBERTYUI_API`). Pas de copie .as. Elle depend de `FHOLSource::IsStanding()` et de `HashCombineFast` (hash UE).
- Les deux tests lisent aussi les cvars `hol.Minimap.AlertShrinkSeconds/AlertSize/AlertSeconds` (valeurs par defaut 0.75 / 24 / 6) : lisibles par script via la lecture de console variable si exposee, a verifier ; le script a `HOLMinimap::AlertSeconds` (const) seulement.

## Fonctions a exposer (lot C++ suivant)
Module : `HeroesOfLibertyUI` (header pur a creer : `HUD\HOLMinimapMath.h/.cpp`, sans Slate, puis liaison script) ; consommateurs C++ : `HOLMinimapView.cpp` l.224, 674 (AlertDiameter), l.397 (signature).
1. `float HOLMinimapMath::AlertDiameter(float Age, float Period, float Size)` : Clamp(Period,0.2,10), Phase = Fmod(Max(Age,0),Period)/Period, Lerp(Size, Min(8,Size), Phase). Fonction libre pure ; `UHOLMinimapView::AlertDiameter` en devient un renvoi.
   - Alternative sans C++ : ecrire la regle en .as dans `HOLMinimap::AlertDiameter` (D839 : code neuf = script) et que le Slate l'appelle via le script ; mais le Slate C++ la rappelle a chaque Paint, donc il faut la liaison. A trancher par le lot.
2. `uint32 HOLMinimapTerrain::GetSourcesSignature(TConstArrayView<FHOLSource> Sources)` : exposer en script comme `HOLMinimapTerrain::GetSourcesSignature(const TArray<FHOLSource>&)` (ou `int32`, UINT non lisible en AS : renvoyer `int32`/`uint`), avec `FHOLSource` (TypeIndex, Resource, Location, Remaining, IsStanding) deja exposee ou a exposer.
Tests a porter ensuite dans `Script\UI\HOLMinimapRules_Test.as` (existant : `Script\Tests\HOLMinimapRules_Test.as`, a verifier le bon chemin) : Test_MinimapAlertRing (8 assertions de valeur + decroissance stricte) et Test_MinimapTerrainSources (7 assertions de signature) ; rouge VU puis vert, puis retrait des 2 .cpp de test.

## Champs
- BUILDS_DEMANDES: 0
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 0
- LIGNES_CPP_SUPPRIMEES: 0
- LIGNES_MIGREES_PAR_LOT: 0
- TYPES_A_EXPOSER: HOLMinimapMath::AlertDiameter (ou regle .as), HOLMinimapTerrain::GetSourcesSignature, FHOLSource (IsStanding)
- TEST_ROUGE: sans objet (rien ecrit)
- TESTS_TAUTOLOGIQUES: 0
- quick_check: non lance (aucun test ecrit)
- Commit net-playtest : Docs AngelscriptInProgress seulement ; merge main non fait
- NON VERIFIE : exposition des cvars en script ; chemin exact du fichier _Test.as (Script\Tests\ ou Script\UI\) 
- PROCESSUS_DETACHES: 0 (aucun lance)
