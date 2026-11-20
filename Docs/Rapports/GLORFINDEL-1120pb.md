NON COMPILE (script seul, aucun build, aucun quick_check)

APPELS_DECLARES: 14 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 15:45 / 15:50 (Get-Date)
PREMIER_BUILD: sans C++ (C++ non touche)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 7b061b3 (commit seul, test reecrit Test_SpecializationThirdGridCellForThreeOptions) ; assertit l'effet : oui (la commande de grille Specialize et l'option de chaque case 5/6/7 pour les Templiers, 6/7 pour l'Aube, aucune apres) ; rouge NON VU a l'execution (analyse : Templiers case 5 < FirstOption 6 donc pas Specialize avant le correctif)
COMMITS: 2 (7b061b3 test, 4b771a1 correctif), dernier 15:48, merge_main_fait: oui (Already up to date)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (la case attendue vient du cvar Base = GetSpecializationCell(0), la regle testee est celle de GetCommandWithOptions)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- D868 fait dans la REGLE SCRIPT : Script\Player\HOLCommandGridRules.as GetCommandWithOptions : FirstOption = GetSpecializationCell(0) - (OptionCount - SpecializationOptions), fin = FirstOption + OptionCount ; Templiers 5 6 7 (cvar 6), 2 options inchange 6 7, la case 8 (recherche) reste visible.
- ECART a l'enonce : `GetSpecializationCell` (HOLCommandGrid.cpp:63) n'est PAS modifie : il ne recoit que l'option, pas le nombre d'options, donc une ligne C++ ne peut pas distinguer Templiers/autres sans changer la signature (liaison UFUNCTION + appelants). Le compte est connu du script (OptionCount) : correctif en script (D839), zero C++ neuf.
- Test existant mis a jour : Test_SpecializationThirdGridCellForThreeOptions (ancienne case 8 supposee) ; Test sweep HOLCommandGridSweeps_Test.as:276 (Cell == GetSpecializationCell(Option), Option 0..1, contexte 2 options) et test C++ HOLSpecializationGridTests.cpp (2 options) restent valides ; scenarios specialization_* (Aube, case D) inchanges ; aucune supposition de l'ancienne case 8 trouvee dans les scenarios (grep).
- RESTE : libelle de la 3e voie (GetSpecializationName C++, lot C++) ; GetCommand (sans OptionCount) garde 2 options : l'appelant du jeu doit passer par GetCommandWithOptions avec la faction du siege pour que les Templiers voient 5 6 7 en vrai (liaison non faite, ELROND-1120e/f) ; commentaires de HOLCommandGrid.h (l.322-327) et de l'entete de HOLCommandGridRules.as (l.5) mentionnent encore 2 options / cvar 6.
NON VERIFIE: tout a l'execution : compilation script (DIAGNOSTICS_AS non relus faute d'editeur), test rouge puis vert, absence d'autre appelant supposant la case 8 (grep sur Script, Source, quick_check, scenarios seulement) ; L18-d et L19-4 non identifies par nom.
PROCESSUS_DETACHES: voir sortie du dernier geste (aucun lance par moi)
ANOMALIES_PROCESSUS: edit_line.ps1 -After refuse avec un texte accentue / "E-specialization_cheat = OK" ; la ligne EtapeDefini a ete ajoutee apres "DOCS-1120s" (ligne 7 d'EtapeDefini.md), a deplacer si mal placee.
DIAGNOSTICS_AS: non releve (pas d'editeur)
GIT_STATUS_PRINCIPALE: docs seuls modifies (SpecializationInProgress.md, EtapeDefini.md, Auto_Journey_File.md, rapport) : oui