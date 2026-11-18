# ARWEN-1117a : lot L17-02 (HOLSiegeTests.cpp, parties Tiers / Technologies / Health)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root, commit e57e270 (branche prete, merge main non fait, aucun C++).

## Constat
Les trois parties etaient DEJA portees en script (MIG-J-Siege-a, SOCLE-1116a / ARWEN-1116p) dans `Script\Siege\HOLSiegeRules_Test.as` et `Script\Tests\HOLSocleLibraries_Test.as`. Rien n'a ete duplique ; seuls les cas non couverts ont ete ajoutes dans `Script\Tests\HOLSiege_Test.as`.

## Registre : test C++ -> test(s) script
| Test C++ (HOLSiegeTests.cpp) | Test(s) script |
|---|---|
| Siege.Tiers (paliers, prototype, raisons, ordre, FindEngine, DescribeBlock) | Test_SiegeEngineTiers, Test_SiegePrototypeSet, Test_SiegeCheckOneReason, Test_SiegeCheckReasonOrder, Test_SiegeFindEngine, Test_SiegeCheckLibraryVectors (HOLSocleLibraries_Test.as) ; NEUF : Test_SiegeTiers (nom -> meme index, meme palier, somme 1+2+3+3) |
| Siege.Technologies (table reelle) | Test_SiegeTechnologies ; NEUF : Test_SiegeTechnologyTiersNotBeforeTheWorkshop (MinTier >= palier du Belier, port de « pas avant son batiment ») |
| Siege.TechnologyHealth | Test_SiegeTechnologyHealth ; NEUF : Test_SiegeHealthRounding (arrondi au plus proche, pourcentage negatif) |
| Siege.Combat | Test_SiegeCombatVectors (IsInArea, TakesUnitsOn) ; ChooseTarget reste C++ |
| Siege.Table | Test_SiegeTableVectors (partie lisible) |

## Rouge VU
Test_SiegeTiers, somme attendue faussee (10 au lieu de 9) : `quick_check -Parts tests` 00:20:58 = 1366/1369, echec `Angelscript.UnitTests.Tests.HOLSiege_Test.Test_SiegeTiers` (+ les 2 rouges connus D831). Remise a 9 : 00:23:29 = 1367/1369, seuls rouges NomadFirstForum et NomadReplicated.

## Reste en C++ (non porte), avec raison
- Combat : `HOLCombat::ChooseTarget` (FSeeker / FCandidate, Mass) ; `IsInArea` et `TakesUnitsOn` ne sont testes que sur leur copie script.
- Health : `HOLCombat::GetMaxHealth(..., percent)` et `GetDamage(..., percent)` sur les vraies lignes (400 -> 440 -> 480, 45 -> 54) : aucune bibliotheque script ; le script teste la copie `HOLSiege::HealthWithTechnology` / `AttackWithTechnology`, pas la regle C++. Lot C++ a prevoir : bibliotheque HOLCombat (GetMaxHealth, GetDamage).
- Technologies : `Row->Stats.Contains(...)`, `Workshop->Age`, `Workshop->Produces` (FHOLAITechnology et FHOLAITypeFigures ne les exposent pas).
- Table : age, grille, empreinte, vie, categorie, Produces, marchable, vue du batiment ; type d'attaque, attaque sur batiments, AreaRadius, vue, armures siege / heros ; balayage « seul le Mangonel a une zone ».
- Les tests C++ ne sont PAS supprimes (suppression groupee au lot C++ suivant).

## HOLSiegeRules.as (98 l.) : NON retire
Appelants restants (Grep) : `Script\AI\HOLAISiege.as` (HOLSiege::Available, EHOLSiegeEngine), `Script\Units\HOLUnitTierRules.as` (commentaires + numeros de raison), `HOLUnitTierRules_Test.as` (HOLSiege::Available/TierTooLow/NoWorkshop/CannotPay), `HOLSocleLibraries_Test.as`, `HOLSiegeRules_Test.as`, `HOLSiege_Test.as` (EngineAt, EngineCount, HealthWithTechnology, AttackWithTechnology, IsInArea, TakesUnitsOn, PrototypeEngines, OpenedEngines, IsAvailable). Ce n'est plus un jumeau de regle mais l'enum et les numeros de raison du script plus quelques helpers purs ; le retirer demande de deplacer enum et constantes (decision de design, pas ce lot).

## Champs
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 3 (Test_SiegeTiers, Test_SiegeTechnologyTiersNotBeforeTheWorkshop, Test_SiegeHealthRounding)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~60 (HOLSiege_Test.as), le reste deja migre
TYPES_A_EXPOSER: bibliotheque HOLCombat (GetMaxHealth, GetDamage avec pourcentage de technologie) ; champs Stats de FHOLAITechnology, Age / Produces de FHOLAITypeFigures
TEST_ROUGE: vu a 00:20 : 1366/1369 (Test_SiegeTiers)
TESTS_TAUTOLOGIQUES: 0 (les 3 passent par HOLSiegeCheck / la table reelle ; Test_SiegeHealthRounding teste la copie script, dit)
NON VERIFIE: build C++ (aucun, DLL a jour) ; merge main non fait ; processus detaches non verifies (voir stray_processes).
