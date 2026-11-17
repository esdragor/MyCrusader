# ELROND-1117r : port en script des tests purs HOLFactionRulesTests.cpp / HOLMarketRulesTests.cpp

Verdict : AUCUN test portable sans C++ neuf. Aucun fichier modifié, aucun commit (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` propre, tip fe31a39). NON COMPILÉ / rien à compiler.

APPELS_DECLARES: 11 (lecture seule)
HEURE_DEBUT/HEURE_FIN: 22:08 / ~22:13
PREMIER_BUILD: sans C++   BUILD: sans objet (aucun fichier touché)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun test écrit) ; assertit l'effet : sans objet
COMMITS: 0, merge_main_fait: non (rien à fusionner, worktree déjà à fe31a39 = main)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 0
TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; BUILDS_DEMANDES: 0
ESSAIS: aucun
QUESTIONS: aucune

## Analyse des 8 tests de HOLFactionRulesTests.cpp (HeroesOfLiberty.Match.Factions.*)

| Test C++ | Pur ? | Verdict |
|---|---|---|
| Tables (FHOLFactionTablesTest) | Oui, au fond (parseurs CSV, FTable, Allows, ResolveKey, ApplyOverrides) | NON PORTABLE : rien de `HOLFactionRules` n'est exposé au script |
| RealChain | Non (FWorld : monde, sous-systèmes, bâtiments, ordres) | reste en C++ (D838) |
| Zombie | Non (monde, production, naissances) | reste en C++ |
| Roster | Non (monde, ordres, coûts payés, naissances) | reste en C++ |
| ClientModel | Non (UHOLUnitReceptionSubsystem, modèles clients) | reste en C++ |
| Preload | Non (monde, gabarits de Mass) | reste en C++ |
| Corpse | Non (Mass : HOLCorpse::Lay, FMassEntityManager) | reste en C++ (D838) |
| WarriorGrid | Non (monde, ordres de production) | reste en C++ |

Constat sur l'exposition : `Source\HeroesOfLiberty\Match\HOLFactionRules.h` est un `namespace HOLFactionRules` sans AUCUN `UCLASS` ni `UFUNCTION` ; `FHOLStartRules::GetWorkerTypeName / GetHeroTypeName / GetMainCentreTypeName` sont des statiques C++ simples, et `HOLStartRulesReadLibrary.h` n'expose que `GetMissing` et `ShouldStart`. Les seuls tests script voisins qui touchent les factions (`HOLFactionChoice_Test.as`, `HOLFactionTechnology_Test.as`) passent par leurs propres bibliothèques de lecture (`HOLFactionChoiceReadLibrary`, `HOLFactionTechnologyReadLibrary`), pas par `HOLFactionRules`.

Écrire le test « Tables » en script sans l'exposition obligerait à reparser les CSV en script : cela ne testerait pas les parseurs C++ (test tautologique, refusé par la règle L12-5). Je ne l'ai donc pas écrit.

## Analyse des 3 tests de HOLMarketRulesTests.cpp

- Market.BookState : lit une propriété répliquée d'AHOLGameState (`FindPropertyByName`, `GetLifetimeReplicatedProps`) et crée un monde : réplication + monde, reste en C++.
- Market.Chain : monde, ordres de commande, sous-système de production, état de jeu : reste en C++.
- Market.Tribute : « Tribute Chain », même chaîne avec monde : reste en C++.
- Déjà couvert en script (le fichier C++ le dit lui-même) : `Market.Rules` retiré (couvert par `Test_TableImportMarketSettings` dans `Script\Tests\HOLTableImport_Test.as` et `Test_MarketRulesVectors`), `Market.Apply` retiré (`Test_MarketApplySellIsSell` / `Test_MarketApplyBuyIsBuyWithGold` / `Test_MarketApplyRefusals`). `Script\Tests\HOLTributeRules_Test.as` couvre déjà `Test_TributeReceived / Test_TributeCheck / Test_TributeMake`. Il ne reste donc aucune règle pure non couverte dans ce fichier.

## TYPES_A_EXPOSER

- `HOLFactionRules::ParseRoles`, `ParseOverrides`, `FTable::{FindRoleType, ResolveKey, HasFaction, Allows, HasOverride, ApplyOverrides}`, `GetBuiltInTable` : une bibliothèque `UHOLFactionRulesReadLibrary` (BlueprintPure, texte CSV en entrée, valeurs scalaires en sortie, par exemple `FindRoleTypeFromText(RolesText, Faction, Role)`, `AllowsFromTexts(...)`, `CountParseProblems(...)`) permettrait d'écrire « Tables » en script. À mettre dans le prochain lot C++ (un seul build).
- `FHOLStartRules::GetWorkerTypeName / GetHeroTypeName / GetMainCentreTypeName` (config + slot) : à ajouter à `UHOLStartRulesReadLibrary` (la config se construit déjà par `HOLMatchConfigReadLibrary`).
- `ApplyOverrides(..., FHOLUnitTypeRow::StaticStruct(), ...)` (réflexion sur UStruct) : probablement à laisser en C++.

## Tests C++ prêts à retirer

Aucun (le test « Tables », seul candidat, n'a pas de jumeau script).

## NON VÉRIFIÉ
Rien n'a été exécuté. L'absence d'exposition est établie par grep sur `Source\HeroesOfLiberty` (UFUNCTION / UCLASS dans `HOLFactionRules.h`, `HOLStartRules.h` et `HOLStartRulesReadLibrary.h`) et sur `Script\` (aucun appel à `HOLFactionRules` hors commentaires), pas par le journal d'un chargement de script.

PROCESSUS_DETACHES: aucun (aucun processus lancé)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relancé ; aucun fichier de code touché ; seul `EtapeDefini.md` et ce rapport sont écrits dans la copie principale (docs voulus).
