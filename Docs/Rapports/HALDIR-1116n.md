# HALDIR-1116n : BASCULE MIG-J-Terrain (D836 point 1)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `maps`. Sans C++ : aucun build, `Source\` non touché, `Crown.umap` non touché. **Branche prête** (pas de fusion dans `main`).

```
APPELS_DECLARES: 20 (arrêt à 45 vus par moi ; consigne 21:50 respectée)
HEURE_DEBUT/HEURE_FIN: 21:29 / 21:38
PREMIER_BUILD: sans C++   BUILD: sans objet (aucun build demandé)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (suppression, D836 : pas de test rouge ; preuve = Grep sans appelant + compte avant/après + registre ci-dessous). Assertit l'effet : oui pour les tests gardés (forêt passée par la mise à l'échelle C++ réelle)
COMMITS: 2 (c377240 21:36 code+tests, 8a91ef4 21:37 journaux) + merge main f9118fa (docs seulement : aucun fichier Script/Source/Content changé ; Tools seulement brief_commun.md), merge_main_fait: oui
TESTS: quick_check -Parts tests x2 : avant 21:30 1352/1354 (158 s) ; après 21:36 1346/1348 (175 s). Total 1354 -> 1348 = -6 = exactement les 6 tests retirés ; KO identiques avant/après = Match.StartingHero.NomadFirstForum et .NomadReplicated (D831). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (lot de suppression)
QUESTIONS: aucune
FAIT / RESTE: voir plus bas
NON VÉRIFIÉ: voir plus bas
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: (1) quick_check marque toujours les 2 rouges D831 « REGRESSION : pas dans known_reds.txt » (l'énoncé disait « rouge connu ») ; (2) le premier lancement redirigé vers un fichier du scratchpad a échoué (« fichier utilisé par un autre processus »), relancé sans redirection.
GIT_STATUS_PRINCIPALE: M Auto_Journey.md, M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, M Tools/AutoJourney/brief_commun.md, ?? Docs/Priorite_Socle_Factions_2026-11-16.md, ?? Docs/Rapports/ELROND-1116n.md ; inchangée hors docs voulus : oui (mes lignes : Auto_Journey_File.md, EtapeDefini.md ; ce rapport non commité)
```

Champs de migration :

```
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 (le C++ reste seule source)
TESTS_SCRIPT_AJOUTES: 0 ; TESTS_SCRIPT_RETIRES: 6 (registre ci-dessous)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 ; lignes script supprimées : HOLTerrainRules.as +9/-58, HOLTerrainRules_Test.as +19/-171
TYPES_A_EXPOSER: aucun (HOLTerrain::ServerScaledSpeed déjà exposé, utilisé par le test de la forêt)
```

## Preuve « sans appelant » (Grep avant suppression)

Grep `SpeedPercentAt|ScaledSpeed|HOLTerrain::Contains|HOLTerrain::Preset|MarshSpeedPercent|RoadSpeedPercent|FHOLTerrainSettings|FHOLTerrainZone|TerrainServerAndScript` dans `Script\`, `Source\`, `Tools\` :
- `Script\` : seulement `Terrain\HOLTerrainRules.as` et `Terrain\HOLTerrainRules_Test.as`.
- `Source\` : homonymes C++ seulement (`FHOLTerrainRules::GetSpeedPercentAt`, `UHOLTerrainLibrary::ServerScaledSpeed`, struct C++ des zones, cvars) dans Map\HOLTerrain*.h/.cpp, Commands\HOLUnitOrderProcessor.cpp, Tests\HOLTerrainTests.cpp ; le C++ n'appelle pas les fonctions libres du namespace script.
- `Tools\` : seulement les cvars C++ `hol.Terrain.MarshSpeedPercent` / `RoadSpeedPercent` (scénarios, road_group.ps1, cartes) : visent le C++, rien à changer.

## Ce qui est retiré / gardé

- Retiré de `HOLTerrainRules.as` : `SpeedPercentAt`, `ScaledSpeed`, `Preset` (ne servait qu'à choisir la force du marais), réglages `MarshSpeedPercent` et `RoadSpeedPercent` (sans lecteur une fois la copie retirée ; les vrais réglages sont les cvars C++).
- Gardé : `FHOLTerrainSettings` (charge, forêt, gué), `FHOLTerrainZone`, constantes `Marsh/Road/OnFoot/Cavalry/Siege`, `ForestSpeedPercent`, `FordDamageTaken`, `StopsCharge` et **`Contains`** (écart à l'énoncé : `StopsCharge` l'appelle, il ne sert donc pas qu'à la copie).

## Registre des tests script retirés (6)

| Test script retiré | Test C++ qui couvre la même règle |
|---|---|
| `Test_TerrainMarshSlows` | `HeroesOfLiberty.Terrain.Rules` (marais 1000/2000/500, bord 1500, 1501 hors) |
| `Test_TerrainNoZoneAndOverlap` | `HeroesOfLiberty.Terrain.Rules` (aucune zone, chevauchement 200) |
| `Test_TerrainPresetsAndScaledSpeed` | `HeroesOfLiberty.Terrain.Rules` (GetScaledSpeed 1/10, 0, 500) |
| `Test_TerrainRoadSpeedsUp` | `HeroesOfLiberty.Terrain.Rules` (route 5000/5400/5401, marais sur route, 150 %) + `Terrain.WalkSpeed` (monde, Mass) |
| `Test_TerrainServerAndScriptRulesAgree` | accord sans objet (une seule copie) ; règle : `Terrain.Rules` + `Terrain.WalkSpeed` |
| `Test_TerrainServerAndScriptAgreeOnEveryZone` | accord sans objet ; règle : `Terrain.Rules` + `Terrain.WalkSpeed` |

Réécrits (gardés, même nombre) : `Test_TerrainMarshStopsCharge` (réglages par défaut au lieu de `Preset(0)`), `Test_TerrainUnknownZoneKindNeutral` (n'assertit plus la vitesse ; + « un marais par-dessus arrête la charge »), `Test_TerrainForestSlowsCavalryAndSiege` (mise à l'échelle par `HOLTerrain::ServerScaledSpeed`, la règle C++). Inchangé : `Test_TerrainFordWeakensDefence`.

## À signaler sans modifier

- `Tools\Editor\maps\eldorado.py` l. 16-17 (docstring) : « The rule is in Script/Terrain/HOLTerrainRules.as (script) and Map/HOLTerrainRules.cpp (server), checked against each other » : obsolète (D836), commentaire seulement.
- `Tools\Editor\maps\common.py` l. 1213 : commentaire citant `HOLTerrainRules` (vrai côté C++).
- `Tools\Tests\quick_check.ps1` et tests python : aucune ligne ne vise la copie script.

## FAIT / RESTE

- FAIT : bascule MIG-J-Terrain (c377240), journaux (8a91ef4), ligne de la file `[x]`, ligne d'EtapeDefini, merge main (f9118fa), IN_USE libre.
- RESTE : forêt / gué / charge à brancher (D810, en C++ selon la ligne de bascule) ; Déploiement et XP d'exploration (D836 point 1) non traités ici.

## NON VÉRIFIÉ

- La liste nominative des tests après le run n'a pas été comparée nom à nom : la preuve est le compte (-6 exactement) et les mêmes 2 KO ; un .as qui ne compile pas aurait fait disparaître bien plus de tests.
- Pas de run après le merge f9118fa (docs seulement + brief_commun.md).
