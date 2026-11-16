# SAURON-1116p (visual, sans C++, 2026-11-16 21:38-21:45)

TACHE : couverture C++ des presets A/B/C de l'XP d'exploration (D809) apres suppression de la copie script (SAURON-1116o).

CONSTAT : HOLExplorationXPTests.cpp ne contient qu'UN test (FogPaysTheHero, l.61) ; il lit `Preset(GetPresetIndex())` (l.131) puis compare a ce que le preset renvoie lui-meme : aucune valeur de preset (A 6/0, B 0/40, C 3/25, tranche 16) n'est assertee nulle part, ni le defaut cvar 0. Aucun autre test C++ ne cite HOLExplorationXP::. Les regles (paie une fois, depart gratuit, mort) n'ont non plus aucun test C++ (SAURON-1116o a retire les 4 tests script qui les couvraient) : a ajouter au prochain lot C++ si voulu (hors tache).

FAIT : Tools\Tests\exploration_xp_presets_test.py (lit HOLExplorationXPRules.h struct FSettings + .cpp Preset + cvar, compare aux valeurs decidees : C 16/3/25 = defaut D809 Q1 C, A 16/6/0, B 16/0/40, cvar 0) ; branche dans quick_check.ps1 partie values (1 ligne de test + commentaire).
TEST_ROUGE: vu a 21:42 : copie temporaire du .cpp avec XPPerPlace 25 -> 26 : "ERREUR preset 0 (C) ... decided ... 25", FAIL, exit 1 ; puis vert : PASS exit 0 sur le worktree.
Note : le test partage les extracteurs du deploye accord supprime (meme technique texte, sans moteur). Les valeurs A/B viennent de Docs\Archive\Retours_2026-11-15\Q-exploration-xp.md (options A/B), C du GDD D809.

BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0 (1 test python values : exploration_xp_presets_test.py)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0
TYPES_A_EXPOSER: aucun
TESTS_TAUTOLOGIQUES: 0
EDITS_REFUSES: 0
NON VERIFIE : quick_check complet (a lancer par la session principale : `Tools\Tests\quick_check.ps1 -Parts values` dans le worktree Sauron, attendu PASS avec une ligne de plus). Seul value_sheet_test + le test lui-meme lances.
RESTE : 4 commentaires C++ (Source\ interdit) : HOLDeployRules.h:8, HOLDeployRules.cpp:5, HOLExplorationXPRules.h:8 (cite encore Script/Heroes/HOLExplorationXPRules.as), HOLDeployRulesTests.cpp:9.