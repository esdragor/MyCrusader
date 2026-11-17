# GLORFINDEL-1117f : E3-11 Test_SignalAltClick en script + signals_altclick mapSize Small

APPELS_DECLARES: ~28 (au-dela de 25 : arret sur la fin de tache)
HEURE_DEBUT/HEURE_FIN: 10:26:03 / 10:37
PREMIER_BUILD: sans C++   BUILD: sans build (binaire du worktree a jour)   LAUNCHCHECK_DEBUT: vide a 10:27 (Get-StaleBinaryProblem)
TEST_ROUGE: vu a 10:30 : tests 1390/1393, Test_SignalAltClick seul nouveau rouge (valeur faussee : glisser vers le haut attendu Defend au lieu d'Attack) ; commit f7ec5f8 = test vert (le rouge n'a pas ete commite seul) ; assertit l'effet : oui (valeurs calculees par HOLSignalInput::KindOfAltClick / Expire / ColorOf via HOLSignalInputRead)
COMMITS: 2 sur net-playtest (f7ec5f8 test, cb41fd6 scenario json), dernier 10:35, merge_main_fait: non (rien a fusionner, main 1cacfa9 deja dedans)
TESTS: quick_check -Parts tests : rouge 1390/1393, vert 1391/1393 (2 rouges connus NomadFirstForum / NomadReplicated, known_reds) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: signals_altclick : preflight PASS (10:36), 2 PNG ouverts et vus (losange blanc barre d'une croix au sol, carre blanc sur la minicarte, "Vous : Signal" ; puis tout disparu) ; geste interne (non reel) = LOGIQUE + PNG ; commande : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\play.ps1 signals_altclick -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
QUESTIONS: aucune
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 1 (Test_SignalAltClick dans Script\Tests\HOLSignalInput_Test.as)
LIGNES_CPP_SUPPRIMEES: 0   LIGNES_MIGREES_PAR_LOT: 63 (HOLSignalInputTests.cpp)   TYPES_A_EXPOSER: aucun de plus

REGISTRE (nom C++ -> test script) :
- HeroesOfLiberty.Signals.AltClick (Source\HeroesOfLiberty\Tests\HOLSignalInputTests.cpp, FHOLSignalAltClickTest) -> Angelscript.UnitTests.Tests.HOLSignalInput_Test.Test_SignalAltClick. Couvre les memes assertions (clic bref = neutre, 6 directions de glisser 0/72/144/216/288/350 = Attack/Defend/NeedHelp/Rally/Scout/Attack, Expire 14.9/15.0/17.0 = 2/1 Rally/0, ColorOf Attack != Neutral). Le .cpp est retirable (git rm) dans le prochain lot C++, rien supprime ici.
- Non portes (hors tache) : HOLRelationIconsTests.cpp, HOLFactionTechnologyTests.cpp (E3-10 / filtre, bibliotheques HOLRelationIconsRead / HOLFactionTechnologyRead jamais compilees ni testees, voir GLORFINDEL-1117e).

FAIT / RESTE : test script vert contre la vraie bibliotheque (MakeSignal remplit la liste, les champs de FHOLReceivedSignal sont lisibles) ; json signals_altclick avec mapSize Small ; E-signals_altclick.md a re-lister par la session principale.
NON VERIFIE: le test C++ et le test script tournent-ils tous deux verts ensemble : le total 1391/1393 compte les deux ? (le C++ n'a pas ete lance a part) ; E-signals_altclick.md non reecrit ; les 2 ecrans (E-ecran-multijoueur / reglages) non touches.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: un Start-Process mal forme (argument -RedirectStandardOutput passe au script preflight) lance par erreur a 10:34 ; stray_processes n'a rien trouve ensuite ; l'arret manuel a ete refuse par le classifieur et n'etait plus necessaire
GIT_STATUS_PRINCIPALE: docs seulement (AngelscriptInProgress, Auto_Journey_File, EtapeDefini, Docs\Rapports, fichiers Retours\ d'autres sessions) : oui
