# HALDIR-1116p : tests de terrain réalignés (D836 point 1), sans C++

Lancé 22:35, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (branche maps), `CPP: non`, aucun build.

## Appelants (Grep Script, Source, Tools)
`Script\Terrain\HOLTerrainRules.as` : AUCUN appelant du jeu pour les règles suivantes (seuls l'appellent leurs tests de `HOLTerrainRules_Test.as`). Le C++ ne les cite pas (`FHOLTerrainRules` C++ a ses propres noms, vitesse marais/route seulement).

| Règle (script) | Appelant du jeu | Test script | Rouge vu | Décision que cela appelle |
|---|---|---|---|---|
| `HOLTerrain::ForestSpeedPercent` (F4, cavalerie 80 %, engins 70 %) | aucun | `Test_TerrainForestSlowsCavalryAndSiege` | oui, 22:38 (`ForestCavalrySpeedPercent` faussé 80 -> 100) | brancher en D810 ou retirer |
| `HOLTerrain::FordDamageTaken` (F5, gué 120 %) | aucun | `Test_TerrainFordWeakensDefence` | oui, 22:38 (`FordDamageTakenPercent` 120 -> 100) | idem |
| `HOLTerrain::StopsCharge` (+ `Contains`, `bMarshStopsCharge`) | aucun | `Test_TerrainMarshStopsCharge`, `Test_TerrainUnknownZoneKindNeutral` | oui, 22:38 (`bMarshStopsCharge` true -> false) | idem |

Les trois règles ont donc chacune au moins un test script, rouge vu une fois (valeur faussée), puis vert (voir NON VÉRIFIÉ). Le test de la forêt passe par `HOLTerrain::ServerScaledSpeed` (chaîne C++ réelle) : pas de tautologie.

## Rouge vu (L15-5)
Les 3 valeurs faussées d'un coup dans `HOLTerrainRules.as` (non commité, rétabli par `git checkout`) : `quick_check -Parts tests` = 1342/1348, rouges : Test_TerrainFordWeakensDefence, Test_TerrainForestSlowsCavalryAndSiege, Test_TerrainMarshStopsCharge, Test_TerrainUnknownZoneKindNeutral (+ les 2 rouges connus D831 Match.StartingHero.NomadFirstForum / NomadReplicated). Le script a donc été lancé deux fois (rouge, puis vert) et non une : le rouge est exigé par le brief.

## Question posée
`C:\Users\baudo\Documents\MyCrusader\Retours\Q-terrain-regles-sans-appelant.md` [À RÉPONDRE] : A brancher en D810 (recommandé : valeurs déjà tranchées Q1 A + Q2 A) / B retirer les trois règles et 4 tests. Défaut codé : rien changé. Rien décidé ici.

## Obsolescence
`Tools\Editor\maps\eldorado.py` docstring (lignes 15-17) : ne cite plus la copie script supprimée ; dit que la règle de vitesse est celle du serveur (`Map/HOLTerrainRules.cpp`, D836). Commentaire seulement.

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (4 existants, rouge vu) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: aucun ; TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 0.
Suivi à jour : `TerrainForestFordInProgress.md` (1 ligne), `EtapeDefini.md` (1 ligne), `IN_USE.txt`. Placeholders.txt : aucun provisoire ajouté ni retiré.
Branche prête, merge main non fait (inutile : déjà à 8031fd9). `Content\Maps\Crown.umap` non touché (non suivi, pré-existant).