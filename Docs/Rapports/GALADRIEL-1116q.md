# GALADRIEL-1116q : lot E3-09 (port de HOLGameNamesTests.cpp en script)

Branche galadriel-tab, commit d647a91, branche prete, merge main non fait. Sans C++, aucun build (DLL a jour).

- Fichier : Script\UI\HOLGameNames_Test.as (2 tests, contre HOLGameNamesRead : GetUnitNameForFaction, GetBuildingName).
- Registre test C++ -> test(s) script : HeroesOfLiberty.UI.GameNames.LegionsLanguages -> Test_GameNamesLegionsNames (Legions != Dawn pour les 7 types ; noms francais ASCII, verifies seulement si la machine est en francais) ; ...ForgeKey -> Test_GameNamesForgeName (nom "Forge" seul).
- NON PORTE : FactionNames (la moitie Overrides depend de E3-06, tests qui creent un monde ; la moitie Legions/Dawn est deja dans Test_GameNamesReadLibrarySmoke) ; changement de langue (FScopedLanguage) et lecture .locres en/ja de LegionsLanguages ; cle et namespace FTextInspector de ForgeKey ; noms accentues (Arbaletrier, Belier : UTF-8 script non verifie).
- TEST_ROUGE: vu a 22:55 : quick_check -Parts tests 1361/1364, Test_GameNamesForgeName rouge (valeur "ForgeROUGE") ; remise ; vert 1362/1364 a 22:58 (seuls rouges : NomadFirstForum, NomadReplicated, D831).
- A noter : au run rouge, "scripts compiled" etait KO aussi (cause non examinee, absente du run vert).
- BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 2 (Test_GameNamesLegionsNames, Test_GameNamesForgeName) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: langue (SetCurrentLanguage), lecture locres, FTextInspector si le reste doit etre porte.
- Suivi : ligne E3-09 de la file, EtapeDefini.md, AngelscriptInProgress.md, IN_USE.txt (libre, CPP: non).