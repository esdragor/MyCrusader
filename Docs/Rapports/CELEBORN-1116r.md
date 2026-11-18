# CELEBORN-1116r : lot E3-02 (HOLFactionChoiceTests.cpp porte en script)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `hero-j8-close`, commit 28ddc82 (branche prete, merge main non fait, rien de fusionne).

## Fait
- `Script\Tests\HOLFactionChoice_Test.as` (neuf, 3 tests) contre `HOLFactionChoiceRead` / `FHOLClickCounter` (UHOLFactionChoiceReadLibrary, S1D-1116c). Memes vecteurs que le C++ (boucle bCheat en deux appels d'une aide, tableau des factions grisees).
- Test C++ `HOLFactionChoiceTests.cpp` GARDE (git rm au lot MIG-C++-FINAL). Les 3 `Test_` s'ajoutent au test de fumee existant `Test_FactionChoiceReadLibrarySmoke` (HOLS1dLibraries2_Test.as, inchange).

## Registre test C++ -> test(s) script
| Test C++ | Test script |
|---|---|
| HeroesOfLiberty.Match.FactionChoice.List | Test_FactionChoiceList |
| HeroesOfLiberty.Match.FactionChoice.Names | Test_FactionChoiceNames |
| HeroesOfLiberty.Match.FactionChoice.TripleClick | Test_FactionChoiceTripleClick |
| HeroesOfLiberty.Net.FactionChoice.Bodies | NON PORTE (appelle FHOLManagerClientRules::MakeMeBody, MakeAISlotBody, ParseLobby : fonctions du manager hors bibliotheque ; a porter quand une bibliotheque de lecture du manager existera) |

## Rouge VU puis vert
- Rouge (22:53 -> 22:56) : une valeur faussee par test (List : 6 au lieu de 5 entrees sans triche ; Names : "Templar" au lieu de "Templars" ; TripleClick : 3 au lieu de 2 triples) : `tests 1360/1365`, les 3 `Angelscript.UnitTests.Tests.HOLFactionChoice_Test.Test_*` en echec + 2 Nomad.
- Valeurs remises (grep verifie), vert (22:58) : `quick_check -Parts tests` = `tests 1363/1365`, seuls rouges NomadFirstForum et NomadReplicated (D831). Aucune regression.

## Champs
- BUILDS_DEMANDES: 0
- TESTS_CPP_RETIRES: 0 ()
- TESTS_SCRIPT_AJOUTES: 3 (Test_FactionChoiceList, Test_FactionChoiceNames, Test_FactionChoiceTripleClick)
- LIGNES_CPP_SUPPRIMEES: 0
- LIGNES_MIGREES_PAR_LOT: ~95 (3 des 4 tests du .cpp de 122 l.)
- TYPES_A_EXPOSER: bibliotheque de lecture du manager (MakeMeBody, MakeAISlotBody, ParseLobby) pour le test Bodies
- TESTS_TAUTOLOGIQUES: 0
- EDITS_REFUSES: 0
- PROCESSUS_DETACHES: voir sortie stray_processes (aucun lance hors quick_check)
- merge main : non fait.

## Suivi
Ligne E3-02 de `Auto_Journey_File.md` cochee, 1 ligne `EtapeDefini.md`, 1 ligne `AngelscriptInProgress.md`, `IN_USE.txt` libre (`CPP: non`, ignore par git). Placeholders.txt : non concerne (aucun provisoire ajoute ou retire).

## NON VERIFIE
Pas de lancement de jeu (tests d'automatisation seuls). Les 16 fichiers non suivis Tools\Scenarios\c28_*, c31_* non touches.
