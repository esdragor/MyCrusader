# HALDIR-1116m : jumeaux Terrain et Site (migration D830/D835, séance du 2026-11-16 soir)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `maps`. Sans C++ : aucun build, `Source\` non touché. **Branche prête** (pas de fusion dans `main`).

```
APPELS_DECLARES: 41 (arrêt à 45 vus par moi)
HEURE_DEBUT/HEURE_FIN: 20:11 / 20:36
PREMIER_BUILD: sans C++   BUILD: sans objet (aucun build demandé)   LAUNCHCHECK_DEBUT: sans C++ (DLL 19:06 annoncée à jour, non relancé)
TEST_ROUGE: Terrain : rouge VU a 20:22 : tests 1326/1329, KO Test_TerrainServerAndScriptAgreeOnEveryZone (pourcentage de route faussé +1 côté serveur) ; vert a 20:27 ; commit dd4123c. Site : rouges VUS a 20:27 : tests 1326/1330, KO Test_SiteScriptAndSimulationRulesAgree (DecaySteps + 1 donné à la simulation) et Test_SiteVictoryRuleVectors (VictorySteps - 1 donné à la simulation) ; vert a 20:31 (1328/1330) ; commit 9221ff1. Assertit l'effet : oui (l'accord compare l'état/la vitesse calculés des deux côtés).
COMMITS: 2 (dd4123c 20:27, 9221ff1 20:31) + merge main 3259c56 (20:32, docs seulement : aucun fichier Source/Script/Tools/Content changé), merge_main_fait: oui
TESTS: quick_check -Parts tests x4 : référence 20:14 1326/1328 (229 s) ; rouge Terrain 1326/1329 (210 s) ; vert Terrain + rouges Site 1326/1330 (207 s) ; vert final 20:31 1328/1330 (199 s). Les 2 KO restants = HeroesOfLiberty.Match.StartingHero.NomadFirstForum et .NomadReplicated (D831, connus de main ; quick_check dit « REGRESSION » car absents de known_reds.txt). Total : 1328 -> 1330 (+2), jamais en baisse. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (lots de tests purs)
QUESTIONS: aucune (le sens des jumeaux est proposé dans les lignes de bascule, pas tranché)
FAIT / RESTE: voir plus bas
NON VÉRIFIÉ: voir plus bas
PROCESSUS_DETACHES: voir la fin du rapport
ANOMALIES_PROCESSUS: aucune vue ; les commandes quick_check de plus de 180 s sont passées en arrière-plan d'elles-mêmes (attendues par une boucle de lecture du fichier de sortie, sans second lancement)
GIT_STATUS_PRINCIPALE: voir la fin du rapport
```

Champs de migration :

```
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 (aucun : la suppression appartient à la bascule)
TESTS_SCRIPT_AJOUTES: 2 (Test_TerrainServerAndScriptAgreeOnEveryZone, Test_SiteVictoryRuleVectors) + 1 étendu (Test_SiteScriptAndSimulationRulesAgree)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: MIG-J-Terrain : 0 ligne de règle migrée, +69 lignes de test script (accord) ; MIG-J-Site : 0 ligne de règle migrée, +75/-3 lignes de test script (accord étendu + portage de Sites.VictoryRule)
DUREE_PAR_LOT: MIG-J-Terrain 20:17-20:27 (10 min, dont 2 runs de 210/207 s) ; MIG-J-Site 20:23-20:31 (8 min, dont 2 runs de 207/199 s, le premier partagé avec le vert Terrain) ; relevé + référence 20:11-20:17
TYPES_A_EXPOSER: sens proposé (C++ reste) : Terrain : FHOLTerrainZone en USTRUCT + UHOLTerrainLibrary::ServerSpeedPercentAtZones (plusieurs zones ; la bibliothèque n'accepte qu'une zone, le cas « marais sur route » n'est donc pas comparable aujourd'hui) ; au branchement de D810 : ServerForestSpeedPercent, ServerFordDamageTaken, ServerStopsCharge. Site : aucun (StepSite, CountSiteUnits, VictoryStepSite suffisent). Sens script (non proposé) : FHOLSiteState, FHOLSiteVictoryState en USTRUCT + un pont abstrait (modèle HOLSignalSubsystem / P1b).
```

## Registre des tests, nom à nom

### Terrain

| C++ | Script en regard | Sort proposé |
|---|---|---|
| `HeroesOfLiberty.Terrain.Rules` (16 assertions, pur) | `Test_TerrainMarshSlows`, `Test_TerrainNoZoneAndOverlap`, `Test_TerrainPresetsAndScaledSpeed`, `Test_TerrainRoadSpeedsUp` (mêmes vecteurs : 1000/2000/500, 1500, 1501, chevauchement 200, route 5000/5400/5401, marais sur route 5000/5300, GetScaledSpeed 1/10, 0, 500) ; le vecteur « the road's own percent is read » (150) est porté dans `Test_TerrainServerAndScriptAgreeOnEveryZone` | reste en C++ si le C++ reste la règle (proposé) |
| `HeroesOfLiberty.Terrain.WalkSpeed` (monde, Mass) | aucun | reste en C++ |
| `HeroesOfLiberty.UI.Minimap.TerrainSources` (UI) | aucun | reste en C++ |
| (aucun) | `Test_TerrainMarshStopsCharge`, `Test_TerrainUnknownZoneKindNeutral`, `Test_TerrainForestSlowsCavalryAndSiege`, `Test_TerrainFordWeakensDefence` | règles sans copie C++ (voir l'écart) |
| accord | `Test_TerrainServerAndScriptRulesAgree` (existant), `Test_TerrainServerAndScriptAgreeOnEveryZone` (neuf) | vert |

**Écart réel noté (pas de rouge commité)** : la forêt (F4, `ForestSpeedPercent`), le gué (F5, `FordDamageTaken`) et la charge arrêtée par le marais (`StopsCharge`) n'existent qu'en script ; `FHOLTerrainRules` ne connaît que marais et route, `UHOLTerrainLibrary` n'a aucune fonction pour eux, et **aucun code du jeu ne les appelle** (ni C++ ni script hors tests) : D810 n'est pas branché. Plus largement, la copie script de la vitesse (`SpeedPercentAt`, `ScaledSpeed`) n'a aucun appelant hors tests ; la règle vivante est le C++.

Appelants C++ de la règle Terrain : `Commands\HOLUnitOrderProcessor.cpp` l. 151-153 (`GetTerrainShare`), l. 499 (vitesse voulue par marcheur et par pas), l. 2234, 3558, 3800 (`GetSubsystem<UHOLTerrainSubsystem>`) ; `Map\HOLTerrainSubsystem.cpp` l. 14, 19, 36, 91 ; tests `HOLMapForestTests.cpp` l. 105, `HOLMapTableTests.cpp` l. 673 (`HOLTerrainKind`).

### Site

| C++ (`Tests\HOLSiteSubsystemTests.cpp`) | Script en regard | Sort proposé |
|---|---|---|
| `HeroesOfLiberty.Sites.VictoryRule` (pur, `HOLSite::VictoryStep`) | `Test_SiteVictoryRuleVectors` (neuf : mêmes vecteurs, équipes 7/7/9/10, script et simulation comparés à chaque pas ; 5 pas du C++ remplacés par `VictorySteps()` du script) | retirable après la bascule |
| `Sites.SubsystemViews`, `.SubsystemOffByDefault`, `.SubsystemBox`, `.HolyColumn`, `.SubsystemTake`, `.SubsystemKeepAndRetake`, `.SubsystemGold`, `.SubsystemSameTwice`, `.SubsystemVictory`, `.SubsystemCount` (10, monde) | aucun | restent en C++ |
| (aucun test C++ pur de `HOLSite::Step`) | 11 tests de règle dans `HOLSiteRules_Test.as` (`Test_SiteTakenByAHolyUnit` ... `Test_SiteTakerOfAnotherCampStartsFromZero`) | règle couverte par l'accord |
| accord | `Test_SiteScriptAndSimulationRulesAgree` (étendu : 16 128 -> 44 800 cas ; propriétaires 1 et 3 ; équipes 500/1000+siège comme le sous-système sous Diplomatie ; moins d'équipes que de sièges ; unités de sièges 9 et -1), `Test_SiteConstantsAgreeWithTheSimulation`, `Test_SiteVictoryScriptAndSimulationAgree` | verts |

Aucun écart réel trouvé sur Site (le vert du premier run sans valeur faussée le montre).

Appelants C++ de la règle Site : `Simulation\HOLSiteSubsystem.cpp` l. 25 (`DefaultHoldSeconds`), 296 (`ResetCounts`), 311 (`AddUnit`, dans `ForEachLivingUnitWithType`), 332 (`Step`, une fois par pas et par site, avec les équipes relatives au propriétaire sous Diplomatie) ; `VictoryStep`/`GoldStep` dans le même sous-système. Côté script, seul `Script\UI\HOLGameScreenScript.as` l. 945 lit `HOLSiteRules::GetVictoryCount` (lecture de l'état répliqué, C++) : la copie script `HOLSite::Step` n'a aucun appelant hors tests.

## Lignes de bascule écrites (file, « Lot C++ de fin de séance »)

- `BASCULE MIG-J-Terrain` : sens proposé C++ reste (appelé par un processeur Mass par marcheur et par pas), copie script retirée, forêt/gué/charge à écrire en C++ au branchement de D810.
- `BASCULE MIG-J-Site` : sens proposé ce soir C++ reste (pont trop coûteux sans le socle de l'étape 1), à revoir après le pilote 1.
Les deux : **non tranché** (R9), décision de l'utilisateur à la sortie de séance.

## FAIT / RESTE

- FAIT : MIG-J-Terrain (dd4123c), MIG-J-Site (9221ff1), relevé des appelants, registre, 2 lignes de bascule, lignes de la file et d'EtapeDefini, merge main (3259c56).
- RESTE : aucune autre ligne Haldir lançable ce soir (les autres sont C++ ou éditeur, « attend la migration D830 », Q2 = A).

## NON VÉRIFIÉ

- `Get-StaleBinaryProblem` non lancé (sans C++, DLL 19:06 donnée à jour par l'énoncé) ; la référence 1326/1328 identique à celle de main le soutient.
- Le comportement de `HOLSiteVictory::Step` sur `VictorySteps()` réels n'est comparé au C++ que via la bibliothèque (pas de lecture du `.cpp` de `VictoryStep`).
- Les appels l. 2234/3558/3800 de `HOLUnitOrderProcessor.cpp` relevés par Grep, non lus.
- Les cas « plusieurs zones » (marais sur route) ne sont pas comparables : la bibliothèque ne prend qu'une zone.

## PROCESSUS_DETACHES (stray_processes -Stop, dernier geste, 20:33)

```r
aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
```

## GIT_STATUS_PRINCIPALE

```r
M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M EtapeDefini.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ARWEN-1116m.md
?? Docs/Rapports/ELROND-1116m.md
?? Docs/Rapports/ETAPE0-1116-MIG.md
?? Docs/Rapports/GLORFINDEL-1116m.md
?? Docs/Rapports/HALDIR-1116m.md
```
inchangée hors docs voulus : oui (Auto_Journey_File.md, EtapeDefini.md : mes lignes ; ce rapport non commité)

