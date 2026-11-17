# ARWEN-1117i : MIG-C-MapSize (lot C++ minimal + tests script, branche d751-root, PAS BUILDE)

Commits (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen) : 5a63f95 (bibliotheque C++), 37965c0 (tests script avec BOUCHON ROUGE : Large side attendu 203 au lieu de 202, ligne marquee `ROUGE-BOUCHON`), 353a19b (bouchon retire = vert). Aucun test C++ supprime.

C++ : `Source\HeroesOfLiberty\Map\HOLMapSizeReadLibrary.h/.cpp` (UHOLMapSizeReadLibrary, namespace script `HOLMapSizeRead`), enveloppes seules de FHOLMapSizeRules, ~150 lignes (pas ~60 : FHOLMatchConfig / FHOLMatchSlot ne sont pas des types script, les tests les construisent ; la bibliotheque prend donc des tableaux). Tailles = entiers (0 None, 1 Small .. 5 Huge) ; creneaux = `States` (0 ferme, 1 humain, 2 IA) + `Teams` (+ `Colors` pour FitSlotsToSize). Aucun nouveau UENUM/USTRUCT. Un build.

Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\build.ps1 -LiveCoding=0 -Wait` puis voir le ROUGE avec le commit 37965c0 (`git checkout 37965c0 -- Script/Tests/HOLMapSize_Test.as`, attendu : echec « Large side » seul, 1 assertion) puis le VERT avec HEAD (353a19b). Les noms `Test_MapSize*` sont decouverts comme `Test_MapRules*` (convention non reverifiee).

## Registre : test C++ -> test script (Script\Tests\HOLMapSize_Test.as)
| Test C++ | Test script | Statut |
|---|---|---|
| HeroesOfLiberty.MapSize.Table | Test_MapSizeTable | porte ; la partie AHOLMapBounds::FitsMaxSize / TileSize n'est PAS portee (limite 220 tuiles ecrite en constante) |
| MapSize.Csv | (aucun) | NON portable : lit Tools/Data/MapSizes.csv (FPaths::ProjectDir + FFileHelper) ; le script n'a pas de lecture de fichier verifiee ; candidat python (comme map_table_test.py) |
| MapSize.Allowed | Test_MapSizeAllowed | porte |
| MapSize.MapFile | Test_MapSizeMapFile | porte (FSoftObjectPath passe en texte) |
| MapSize.DrawPlaces | Test_MapSizeDrawPlaces | porte (12 cas x 2 modes x 200 graines, CheckDraw reecrit en script) |
| MapSize.Config | Test_MapSizeConfigPlaces | PARTIEL : la partie places (GetPlaces fixes/tirees, GetFixedPlaces) ; FromJson/ToJson (aller-retour, version 2 -> 4, Default) RESTE en C++ |
| MapSize.CheckPlaces | Test_MapSizeCheckPlaces | porte |
| MapSize.FitSlots | Test_MapSizeFitSlots | porte |
| MapSize.Lobby.Read, .Send | (aucun) | HORS LOT : appellent FHOLManagerClientRules (ParseLobby, MakeSettingsBody), pas FHOLMapSizeRules ; relevent de MIG-C-ManagerClient |
| MapSize.Lobby.ManagerReference, .ManagerAIReference | (aucun) | HORS LOT : lisent Manager/HOLManager.Tests/Data/*.json + FHOLMatchConfig::FromJson ; restent en C++ |

Bilan : 6 tests portes en entier + 1 en partie sur 12 ; 5 C++ restent (Csv, Lobby x4) + la partie JSON de Config. Retirables apres rouge puis vert : Allowed, MapFile, DrawPlaces, CheckPlaces, FitSlots (Table et Config seulement si l'on accepte de perdre FitsMaxSize / le JSON, ou apres un autre jumeau). Le ratio « 12 tests pour 60 lignes » de l'inventaire est donc ~6 pour ~150.

Fichiers lus par les tests C++ : Tools/Data/MapSizes.csv, Manager/HOLManager.Tests/Data/match_v3_reference.json, match_v3_ai_reference.json ; le script ne sait pas les lire (non verifie).

Champs : BUILDS_DEMANDES: 1 (le lot C++) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 7 (Test_MapSizeTable, Allowed, MapFile, DrawPlaces, ConfigPlaces, CheckPlaces, FitSlots) ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: aucun (tableaux d'entiers) ; TEST_ROUGE: NON VU (pas de build) ; NON VERIFIE : compilation C++ et script (syntaxe : parametres par defaut pour les tableaux, TArray.Contains/FindIndex, T.AssertEquals entiers).
