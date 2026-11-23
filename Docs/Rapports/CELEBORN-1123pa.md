# CELEBORN-1123pa (integ-41, 2026-11-23 15:0x-15:15, sans C++)

Tache : D893 etape 1, les 3 lignes de donnees du palier 1 des voies de commanderie Templiers (Fer / Foi / Pierre), provisoires.

- Commits (integ-41, apres 6985134) : 940570e (test rouge seul, Script/Tests/HOLSpecTier1Units_Test.as, Test_SpecTier1UnitsExist) ; 638a4ed (3 lignes Tools/Data/UnitTypes.csv + Tools/Data/placeholders_map.csv). Les deux finissent par `Agent: CELEBORN-1123pa`.
- Lignes : TempleSergeant (Sergent du Temple, Fer, copie Halberdier : 75 nourriture + 30 bois, 100 PV, MinTier 0), InfirmaryBrother (Frere infirmier, Foi, copie Monk : 100 nourriture + 100 or, 80 PV, MinTier 1), PaviseCrossbowman (Arbaletrier a pavois, Pierre, copie Crossbowman : 80 nourriture + 40 or, 80 PV, MinTier 1) ; colonne Faction = Templars, mannequin, noms d'affichage du GDD (13.7, D133). La voie n'est pas une colonne (ce serait du C++) : litteral dans le test.
- Placeholders.txt (copie principale, non commite) : 3 lignes ajoutees apres « Arbalétrier Aube », doublon « Mur de pierre [à venir] » retire (note dans PlaceholdersInProgress.md). Question Retours\Q-commanderie-palier1.md [A REPONDRE], defaut A (copies gardees).
- `git diff --stat` UnitTypes.csv : +3 / -0 ; placeholders_map.csv : +3 / -0 (L18-a : aucune ligne `-`).

## Champs
- TEST_ROUGE: NON VU. quick_check -Parts tests (15:11, 18 s) : `tests 0/0 FAIL`, `scripts compiled` KO : `Script/AI/HOLAIGarrisonCall.as (26:108) 'GarrisonSlots' is not a member of 'const FHOLAIBuilding'` (champ C++ absent de la DLL du worktree, commit d'un autre agent ; independant de mon test). Aucun test ne tourne tant que la DLL n'est pas reconstruite.
- TEST_ROUGE_SEPARE: oui (940570e seul).
- TESTS_SCRIPT_AJOUTES: 1 (Test_SpecTier1UnitsExist). TESTS_TAUTOLOGIQUES: 0 (lit la vraie table : ReadAIUnitFigures + texte de UnitTypes.csv).
- BUILDS_DEMANDES: 1 (reconstruire la DLL d'integ-41, GarrisonSlots), pas de C++ neuf de ma part. TYPES_A_EXPOSER: aucun.
- UnitTypes.uasset : NON REGENERE. regen_tables.ps1 (15:13) echoue (code 3, « Hot reload failed due to script compile errors », meme GarrisonSlots). A lancer a l'integration apres le build : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\AutoJourney\regen_tables.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, puis `quick_check.ps1 -Parts tests` : attendu le test vert (le test lit ReadAIUnitFigures, donc rouge tant que le .uasset n'est pas regenere).
- Tests qui comptent les unites : Test_UnitTierColumnOfEveryRow (HOLUnitTierRules_Test.as) liste 17 lignes en litteral (18 dans le CSV avant moi, VirtuousKnight absente) : n'est pas casse par les 3 lignes ajoutees. Non verifie a l'execution. quick_check.ps1 : check_placeholders PASS (46 types, 48 entrees, 165 lignes) AVANT mes lignes ; non relance apres.
- make_value_sheet.py --check : 6 erreurs morale.* (HOLMoraleRules.as) et « Valeurs.md perime » : deja la, non liees.
- DIAGNOSTICS_AS: non releve (le script ne compile pas dans ce worktree pour une autre raison). Le test utilise HOLMatchConfigRead::LoadProjectFileText, String.ParseIntoArray, HOLAIOrder::ReadAIUnitFigures : NON COMPILE, a verifier.
- Hors consigne : premier Edit au 9e appel (8 appels de lecture), non au 8e.
- Integ-41 : PAS prete. Elle contient un test rouge attendu (UnitTypes.uasset a regenerer + build de la DLL). Tip : 638a4ed.
