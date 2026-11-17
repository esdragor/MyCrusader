# SAURON-1117i : MIG-C-MapSize, rouge vu des 7 tests script puis retrait de 5 tests C++ (visual, sans build)

Base : visual = main b3853f8, Get-StaleBinaryProblem vide (binaire a jour), quick_check tests 1417/1419 au depart (2 Nomad D831 connus).

## 1) Rouge puis vert (UNE passe chacune, `quick_check.ps1 -Root <worktree> -Parts tests`)
Une valeur fausse dans chacun des 7 tests de `Script\Tests\HOLMapSize_Test.as` :
- Table : places de Huge 8 -> 9 ; Allowed : defaut pour 8 joueurs 4 -> 5 ; MapFile : `Plain_Large` -> `Plain_Larg` ; DrawPlaces : `3 players on Small: no draw` attend 1 tirage au lieu de 0 ; ConfigPlaces : place fixe du creneau 3 : 3 -> 4 ; CheckPlaces : `Small: two places` attend 3 ; FitSlots : taille par defaut pour 3 joueurs Medium -> Large.
- ROUGE 14:05 : tests 1410/1419, les 7 Test_MapSizeTable / Allowed / MapFile / DrawPlaces / ConfigPlaces / CheckPlaces / FitSlots nommes en echec + les 2 Nomad connus (9 KO = 7 + 2).
- `git checkout -- Script/Tests/HOLMapSize_Test.as` ; VERT 14:10 : tests 1417/1419 (seuls rouges : NomadFirstForum, NomadReplicated, known_reds), "aucune regression".

## 2) Registre : tests C++ retires / gardes (Source\HeroesOfLiberty\Tests\HOLMapSizeTests.cpp)
| Test C++ | Decision | Pourquoi |
|---|---|---|
| MapSize.Allowed | RETIRE | Test_MapSizeAllowed : memes vecteurs (8 joueurs x 5 tailles, defauts, 9 joueurs, 0 joueur, None), rouge vu |
| MapSize.MapFile | RETIRE | Test_MapSizeMapFile : les 6 assertions, rouge vu (FSoftObjectPath passe en texte) |
| MapSize.DrawPlaces | RETIRE | Test_MapSizeDrawPlaces : 12 cas x 2 modes x 200 graines, CheckDraw reecrit, rouge vu |
| MapSize.CheckPlaces | RETIRE | Test_MapSizeCheckPlaces : memes 14 assertions (la config devient States/Teams, meme regle), rouge vu |
| MapSize.FitSlots | RETIRE | Test_MapSizeFitSlots : memes assertions (le creneau deplace compare en etat/equipe/couleur au lieu de `==` du struct), rouge vu |
| MapSize.Table | GARDE | FitsMaxSize / GetTileSize (AHOLMapBounds, UHOLBuildingSettings) non portes : la limite 220 est une constante en script |
| MapSize.Config | GARDE | FromJson/ToJson (version 2 -> 4, aller-retour, MakeDefault, bSacredSites) non portes ; seule la partie places est en script |
| MapSize.Csv | GARDE | lit Tools/Data/MapSizes.csv, pas de lecture de fichier en script |
| MapSize.Lobby.Read / Send / ManagerReference / ManagerAIReference | GARDES | FHOLManagerClientRules et JSON de reference, hors lot (voir CELEBORN-1117l) |

Dans le fichier : MakeSlots et AreTeamsTogether (utilises seulement par les tests retires) supprimes ; CheckDraw garde (Config s'en sert). Aucun fichier vide : pas de `git rm`. Total : 206 lignes C++ supprimees.

## Total attendu apres le build
TESTS_CPP_RETIRES: 5 ; 1419 - 5 = 1414 (1412/1414 avec 2 Nomad connus). Il reste 1417/1419 tant que la dll contient encore les tests.
Build (session principale) : `powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `...\Tools\Tests\quick_check.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Parts tests` (attendu 1412/1414).

Champs : BUILDS_DEMANDES: 0 ; TEST_ROUGE: vu a 14:05 : 1410/1419 (7 Test_MapSize* + 2 Nomad), vert 14:10 : 1417/1419 ; TESTS_TAUTOLOGIQUES: 0 ; LIGNES_CPP_SUPPRIMEES: 206.
Non verifie : la compilation de HOLMapSizeTests.cpp apres retrait (le build de la session principale la verifie ; risque : include devenu inutile seulement, aucune reference restante aux helpers retires).
