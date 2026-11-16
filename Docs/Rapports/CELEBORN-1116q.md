# CELEBORN-1116q : lot E3-07 (HOLVersionTests.cpp porte en script)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `hero-j8-close`, commit 753b19b (branche prete, rien de fusionne dans main, merge main non fait).

## Fait
- `Script\Tests\HOLVersion_Test.as` (neuf, 4 tests) contre `HOLVersionRead` et `HOLAccessKeyRead` (bibliotheques S1d). Memes vecteurs que le C++.
- Aucune fonction manquante : Get, GetDev, GetUnknown, Parse, Display, IsAccepted (Version) ; ParseServerKey, IsAccepted, MakeLoginOption (AccessKey) sont exposees. Aucun test C++ non porte.
- Le test C++ `HOLVersionTests.cpp` est GARDE (git rm au lot E3-FIN).

## Registre test C++ -> test(s) script
| Test C++ | Test script |
|---|---|
| HeroesOfLiberty.Version | Test_Version |
| HeroesOfLiberty.VersionDisplay | Test_VersionDisplay |
| HeroesOfLiberty.VersionCheck | Test_VersionCheck |
| HeroesOfLiberty.AccessKey | Test_AccessKey |

## Rouge VU puis vert
- Rouge (22:31 -> 22:34) : une valeur attendue faussee dans chacun des 4 tests (Version "no newline" ...a9 ; VersionDisplay "V 0.3.1" ; VersionCheck message "Versiom mismatch" ; AccessKey "HOLKey=Ab-_08") : `tests 1348/1354`, les 4 `Angelscript.UnitTests.Tests.HOLVersion_Test.Test_*` en echec (plus les 2 Nomad).
- Remise des valeurs (verifie par grep : plus aucune valeur faussee), vert : `quick_check -Parts tests` = `tests 1352/1354`, seuls rouges NomadFirstForum et NomadReplicated (D831, connus). Aucune regression.
- Une seule execution rouge pour les 4 tests (4 fautes distinctes dans une passe) puis une execution verte finale.

## Champs
- BUILDS_DEMANDES: 0
- TESTS_CPP_RETIRES: 0 ()
- TESTS_SCRIPT_AJOUTES: 4 (Test_Version, Test_VersionDisplay, Test_VersionCheck, Test_AccessKey)
- LIGNES_CPP_SUPPRIMEES: 0
- LIGNES_MIGREES_PAR_LOT: 83 (HOLVersionTests.cpp, copie script ; suppression au lot E3-FIN)
- TYPES_A_EXPOSER: aucun
- TESTS_TAUTOLOGIQUES: 0 (chaque assertion passe par la bibliotheque, donc par la regle C++)
- EDITS_REFUSES: 0
- merge main : non fait (DLL a jour avec S1d, mais pas de fusion demandee).

## Suivi mis a jour
Ligne E3-07 de `Auto_Journey_File.md` (cochee), 1 ligne `EtapeDefini.md`, 1 ligne `AngelscriptInProgress.md` (copie de MyCrusader), `IN_USE.txt` (libre, `CPP: non` ; ignore par git, donc hors commit).

## NON VERIFIE
Pas de lancement de jeu (tests d'automatisation seuls). `Version.Get == Dev` suppose un contexte hors paquet (cas des tests d'automatisation).
