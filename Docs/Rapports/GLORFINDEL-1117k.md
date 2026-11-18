# GLORFINDEL-1117k : import CSV exposé au script (lot C++ minimal + tests script)

Worktree HeroesOfLiberty-Glorfindel, branche net-playtest, commits 05aae79 (lot C++) puis 18af18b (tests script). Aucun test C++ supprimé, aucun build lancé (le script ne compile qu'après le build : la session principale verra rouge puis vert).

## Lot C++ (Source\HeroesOfLiberty\HOLTableImportReadLibrary.h/.cpp, enveloppes seules, namespace script HOLTableImportRead)
- `ImportTable(Key, File, OutRowNames, OutColumns, OutValues, OutProblems)` : Tools/Data/File importé par `UDataTable::CreateTableFromCSVString` dans une table transitoire de la structure Key ("Market", "Technology", "BuildingType"), rendu en tableaux plats de textes (ExportText : nombre tel quel, liste "(A,B)", bool "True"/"False"). Nom de fichier restreint à un nom simple en .csv.
- `MarketSettingsFromRow(RowName, OutBasePrices, OutStep, OutFeePercent, OutMinPrice, OutMaxPrice, OutRecoverPoints, OutRecoverEverySeconds)` : `HOLMarket::SettingsFromRow` de la ligne importée de Market.csv.
Aucune règle neuve.

## Registre : assertion C++ -> test script (Script\Tests\HOLTableImport_Test.as)
| Test C++ | Assertion | Test script |
|---|---|---|
| Market.Table | import sans problème, Standard (10 nombres MK-Q1), cohérence de chaque ligne (min<max, bases dans les bornes, taxe 0..99, pas>0, retour>0) | Test_TableImportMarketRows (import réel, plus l'analyseur maison) |
| Market.Rules | CSV importé -> SettingsFromRow (bases 80/80/120/100, taxe 30, pas 3, plancher 20, plafond 300) | Test_TableImportMarketSettings (MarketSettingsFromRow, + ligne inconnue = false) |
| Market.Rules | Sell 56/100/77, livre inchangée, pierre 84, 3 lots 56+53+51 -> 71, refus lots/resource/stock, BuyWithGold 100/96/83, 2 lots 96+92 -> 86, pierre 64, refus gold/resource, plancher et plafond | Test_TableImportMarketSettings (règle SCRIPT HOLMarket::Sell / BuyWithGold sur les réglages importés ; la copie C++ de Sell/BuyWithGold garde le devis Slate, non exposée ici) |
| Siege.Technologies | 2 technologies à SiegeWorkshop ; MinTier = GetTier(moteur) ; InPrototype ; Age du bâtiment <= MinTier ; Produces contient le moteur ; GetHealthStat / GetDamageStat (Ram, Mangonel, Spearman) ; Stats contient la stat | Test_TableImportSiegeTechnologies (Age / Produces lus de BuildingTypes.csv importé, non de l'asset UHOLBuildingSettings::FindType : même CSV, pas le même chemin ; Stats et Produces comparés par `Contains` sur le texte exporté) |
| Technologies (Rules / Playable : lecture du CSV) | PloughI / PloughII : Building, MinTier, MinHeroLevel, Requires ; Implemented | Test_TableImportTechnologyRows (lecture seule ; HOLTechnology::Check / IsPlayable restent C++, non exposés : hors de ce lot) |

## Écarts connus
- `Siege.Technologies` lisait l'asset réglé (`UHOLBuildingSettings::FindType`), le test script lit BuildingTypes.csv ; l'asset est fabriqué depuis ce CSV (make_*.py) : écart d'accord à garder en tête avant de retirer le test C++.
- Technologies.Rules / Playable ne sont pas retirables par ce lot (la règle `HOLTechnology::Check` / `IsPlayable` n'est pas exposée) : seule leur lecture de table l'est.
- Rouge à voir par la session principale après build : fausser 80 -> 81 dans Test_TableImportMarketRows / MarketSettings, puis remettre.

## Commande de build
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check -Parts tests` (tests attendus : +4 TableImport).

## Suite GLORFINDEL-1117l (2026-11-17)
- Cause de l'echec Test_TableImportTechnologyRows (MinHeroLevel attendu 0, obtenu -1 ; Implemented attendu False, obtenu "") : la bibliotheque ne ment pas sur les valeurs mais `ExportText_Direct(..., Default = nullptr)` n'ecrit RIEN pour une valeur nulle (0, false). Correctif dans le TEST (b4ceba8) : un champ trouve et vide = 0 / false (TableImportNumber, TableImportBoolIs). Aucun C++ touche pour cela (option propre plus tard : ExportTextItem_Direct dans ImportTable, alors BUILD A RELANCER).
- Passe 1 : quick_check -Parts tests = 1421/1423 (restent les 2 rouges connus NomadFirstForum / NomadReplicated).
- ROUGE vu (1 quick_check, 1417/1423) : 81 au lieu de 80 (MarketRows FoodPrice, MarketSettings food base), "RamHealthX" (SiegeTechnologies), 3 au lieu de 2 (TechnologyRows PloughII MinTier) -> les 4 tests nommes rouges ; remis (git checkout), le vert 1421/1423 est celui de la passe 1 (meme fichier).
- RETIRES (c6f9d63, C++ non recompile) : Market.Table (fichier HOLMarketTableTests.cpp supprime ; equivalent complet : 10 nombres, coherence par ligne = Test_TableImportMarketRows) ; Market.Rules (equivalent complet assertion par assertion = Test_TableImportMarketSettings ; la copie C++ de Sell/BuyWithGold reste couverte par Market.Chain/BookState/Tribute).
- GARDES : Siege.Technologies (le C++ lit l'asset UHOLBuildingSettings::FindType, le script lit BuildingTypes.csv : pas la meme assertion, equivalence NON complete) ; Technologies.Rules / Playable.
- BUILD A RELANCER (la suppression des 2 tests C++ ne se voit qu'apres) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check -Parts tests` (attendu 1419/1421 : -2 tests C++, memes 2 rouges connus).
