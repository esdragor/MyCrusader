APPELS_DECLARES: 11
HEURE_DEBUT/HEURE_FIN: 10:53 / ~10:57
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun code)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (tache docs : releve) ; assertit l'effet : non applicable
COMMITS: 1 (f85b3c0, branche wall-line, docs), merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT: ligne CELEBORN-1116a deja faite par 1121a (bd900a5) -> suivante ; lignes [ ] suivantes = editeur (stealth, regen 7 cartes), bSitesLaidByMaps (interdit) ; prise : SC3. SC3 = C++ + SC2 (table via editeur) : non faisable ; releve classe des emplois de MainCentre ecrit dans SecondaryCentersInProgress.md. Constat : HOLConstructionSubsystem.cpp:102 compare au nom MainCentre, un type SecondaryCentre n'y entre donc pas.
RESTE: ligne SecondaryCentre de BuildingTypes.csv + regeneration de la table (editeur) + test d'accord script/table ; emplois 'tout centre' (HOLConstructionRules.cpp:466 et HOLBuildProcessor.cpp:978 a verifier) ; C++ a signaler au lot : deblocage/garnison du type.
NON VERIFIE: classement = releve de grep, lignes A VERIFIER non lues ; aucun test.
