# HALDIR-1116q - E3-13 part regle (HOLMapRulesTests.cpp, Map.Rules)

- Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche maps, commit 088dc49. Branche prete, merge main non fait.
- Fichier : Script\Tests\HOLMapRules_Test.as (neuf, script, D839). Aucun C++, aucun build.
- BUILDS_DEMANDES: 0
- TESTS_CPP_RETIRES: 0 (HeroesOfLiberty.Map.Rules reste, git rm au lot MIG-C++-FINAL)
- TESTS_SCRIPT_AJOUTES: 3 (Test_MapRulesByKey, Test_MapRulesByAsset, Test_MapRulesDisplayName)
- LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~45 (part Map.Rules)
- TYPES_A_EXPOSER: aucun de plus
- TEST_ROUGE: vu a 22:55 : quick_check -Parts tests 1362/1365, echec Angelscript.UnitTests.Tests.HOLMapRules_Test.Test_MapRulesByKey (valeur "La Steppe" faussee en "La Steppe FAUX"), puis remise
- Vert : 22:58, tests 1363/1365, seuls rouges NomadFirstForum et NomadReplicated (D831, known_reds). TESTS_TAUTOLOGIQUES: 0 (chaine reelle : bibliotheque -> FHOLMapRules).

## Registre test C++ -> test(s) script
| Test C++ | Test script |
|---|---|
| HeroesOfLiberty.Map.Rules | Test_MapRulesByKey, Test_MapRulesByAsset, Test_MapRulesDisplayName |
| HeroesOfLiberty.Map.Csv | Tools\Tests\map_table_test.py (python, 32b6154) |

## Non porte (la bibliotheque ne l'expose pas)
- FTextInspector : cle Map_Plain / Map_Steppe, namespace HOLMaps, source string (ToString le remplace pour le nom francais seulement).
- FText::IdenticalTo (Plain's name is the table's text) et IsCultureInvariant (nom inconnu non traduit).
- Forme FSoftObjectPath (la bibliotheque prend le chemin en texte) ; FindByAsset(FSoftObjectPath()) porte comme chemin "".
- GetSiteCount / FScopedSiteCount : absents du test Map.Rules et de la bibliotheque.
Consequence : le test C++ ne peut etre supprime sans perdre ces controles de cle/namespace de texte, a trancher au lot MIG-C++-FINAL.

## NON VERIFIE
- Les ToString() du nom d'affichage dependent de la culture du processus de test (passe ici, culture de quick_check) ; merge main non fait (DLL a jour avec Source de main non verifie).
Commande de re-verification : Tools\Tests\quick_check.ps1 -Parts tests (dans le worktree).
PROCESSUS_DETACHES: non lance (aucun processus demarre hors quick_check)