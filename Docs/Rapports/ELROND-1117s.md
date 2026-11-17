# ELROND-1117s : UHOLFactionRulesReadLibrary + port script du test Match.Factions.Tables

NON COMPILÉ (aucun build, aucun quick_check, par consigne). Commit 9915937 sur `tiers` (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`), branche prête. `git merge main` fait en premier geste (sans build, DLL périmée).

APPELS_DECLARES: 10
HEURE_DEBUT/HEURE_FIN: 23:54 / 23:57
PREMIER_BUILD: sans build (demandé)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans objet (aucun lancement)
TEST_ROUGE: sans objet (rien n'a tourné) ; assertit l'effet : oui (valeurs calculées par les vrais parseurs et FTable C++, jamais recopiées)
COMMITS: 1 (9915937), merge_main_fait: oui (au début)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (le script ne réécrit aucun parseur : il passe le texte CSV aux vrais parseurs C++)
TESTS_SCRIPT_AJOUTES: 5 (Test_FactionTablesRealFiles, Test_FactionTablesBuiltIn, Test_FactionTablesFixture, Test_FactionTablesStartRoles, Test_FactionTablesShape) dans `Script\Tests\HOLFactionRules_Test.as`
TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~100 (le corps de FHOLFactionTablesTest) ; BUILDS_DEMANDES: 1 (lot groupé de la session principale)
ESSAIS: aucun   QUESTIONS: aucune

## Fonctions C++ ajoutées (`Source\HeroesOfLiberty\Match\HOLFactionRulesReadLibrary.h/.cpp`, `UHOLFactionRulesReadLibrary`, espace script `HOLFactionRulesRead`)

FTable n'est pas un type script : chaque appel reçoit le TEXTE CSV, construit la table avec les vrais `ParseRoles` / `ParseOverrides`, puis interroge le vrai `FTable`.
- Parseurs : `CountRoleRows`, `CountRoleProblems`, `GetRoleCell(text,row,col)`, `CountOverrideRows`, `CountOverrideProblems`, `GetOverrideCell(text,row,col)`.
- FTable : `FindRoleType`, `HasFaction`, `ResolveKey`, `Allows`, `HasOverride`.
- ApplyOverrides sur une `FHOLUnitTypeRow` par défaut (réflexion UStruct faite côté C++) : `ApplyOverridesToUnitRow` (nombre écrit), `CountApplyOverridesProblems`, `ApplyOverridesToUnitRowReadInt(…, IntField)`.
- Table intégrée : `CountBuiltInRoleRows`, `CountBuiltInOverrideRows`, `FindBuiltInRoleType`, `BuiltInHasOverrideRowOf(text,row)`.
- Chaîne de match : `GetSlotStartType(roles, overrides, slot, "Worker|Hero|MainCentre")` : config de MakeConfig du test (slot 0 Dawn, 1 Légions, reste fermé), table posée par `SetTableForTests` le temps de l'appel (RAII), puis `FHOLStartRules::Get*TypeName`.
Les fichiers réels sont lus en script par `HOLMatchConfigRead::LoadProjectFileText` (existant).

## Suspects de compilation
1. `FindFProperty<FIntProperty>` + `GetPropertyValue_InContainer` : include `UObject/UnrealType.h` ajouté ; `FHOLUnitTypeRow` via `Simulation/HOLUnitTypes.h` (il y a deux `MaxHealth` dans ce fichier, lignes 215 et 429 : vérifier que `FHOLUnitTypeRow` est bien incluse par ce header).
2. `HOLFactionRules::FTable` copie par valeur depuis `MakeTable` / `GetBuiltInTable` (TArray de structs, copiable) ; `Table.ApplyOverrides` est `const` : OK.
3. `FHOLStartRules::GetMainCentreTypeName` : supposé public (le test C++ l'appelle) ; `EHOLSlotState::AI` / `Closed` de `Match/HOLMatchConfig.h`.
4. Côté script : chaînes littérales adjacentes concaténées sur plusieurs lignes dans des `const FString` globales (`FixtureRolesText`) ; si le plugin refuse, passer par `+`. `AssertEquals` sur des FString : utilisé ailleurs, devrait passer. Nom d'espace `HOLFactionRulesRead` déduit de `UHOLFactionRulesReadLibrary` (comme `HOLFactionChoiceRead`).
5. Dérive voulue : la table « Bad » du test C++ est montée ligne à ligne ; le script passe les deux mêmes lignes par le parseur CSV (équivalent, un peu plus de chaîne testée).

## Test C++ retirable
`HeroesOfLiberty.Match.Factions.Tables` (FHOLFactionTablesTest, HOLFactionRulesTests.cpp l.191-286) : tous ses vecteurs sont repris (sections 1 à 4, y compris les 5 vérifications de rôles de slot). Aucun retrait fait ; à retirer après rouge vu (faussser une valeur, p.ex. `"Zombie"` -> `"Zomby"`) puis vert. Les 7 autres tests restent en C++ (D838). Commande à lancer après le build groupé : `quick_check -Parts tests` (puis la partie script).

## NON VÉRIFIÉ
Tout : ni compilation C++, ni chargement du script, ni exécution. Les chemins `Tools/Data/Factions.csv` / `FactionOverrides.csv` relatifs à `ProjectDir` supposés comme dans `HOLMatchConfig_Test.as`.

PROCESSUS_DETACHES: aucun (stray_processes.ps1 : « aucun processus detache »)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relancé ; aucun code touché dans la copie principale (seul ce rapport écrit)
