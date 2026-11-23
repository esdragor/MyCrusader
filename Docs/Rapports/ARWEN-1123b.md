# ARWEN-1123b (faction_click joue, S1-c, integ-37, sans C++)

APPELS_DECLARES: 21 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 09:14:50 / 09:23 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++)   LAUNCHCHECK_DEBUT: sans C++ (preflight : "binary not older than any C++ source" OK)
TEST_ROUGE: ebdd211 (herite de ARWEN-1123a, 2097/2099) ; assertit l'effet : oui (ARWEN-1123a) ; aucun test neuf dans cette tache   TEST_ROUGE_SEPARE: oui (ebdd211 puis 53e0fec, 1123a)
COMMITS: 3 (f1af73f scenario vrai clic, 37ae304 variante commentee, 99a9826 AubeInProgress), dernier a 09:22 ; merge_main_fait: non (tache de jeu de scenario, pas de merge demande)
TESTS: preflight faction_click PASS (check 34 s), 0 erreur Angelscript au journal ("no warning or error in the log", sanity audit passed) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: faction_click : preflight PASS, 2 PNG ouverts (Read) et vus, LOGIQUE SEULEMENT (S1-b absent, bEffectCoded faux, pas de fiche E) ; commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 faction_click -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
QUESTIONS: aucune
FAIT / RESTE:
- 1er preflight FAIL : `hol.HUD.Click FactionBox0` -> "HUD click FactionBox0: hidden, greyed or unknown" : UHOLGameScreenScript::ClickWidget (HOLGameScreenScript.as:2672) ne route pas les FactionBox (le .txt de 1123a le supposait). Defaut du scenario, pas du jeu.
- Correctif scenario : `hol.Mouse.Click FactionBox0` (vrai clic Slate : journal "Mouse click FactionBox0 x1 at 93,327: on SButton"), plus de waitlog "HUD click ... done".
- Journal : "LogHOLMatch: FactionAim: refuse soon send false slot -1" (attendu tant que S1-b manque).
- PNG before : barre 100/100 et case "Lumiere sacree : cout 25, bientot" ; PNG after : meme case surlignee (bordure de survol/focus), aucun curseur de visee.
- Variante apres S1-b en commentaire en fin de faction_click.txt (aim 0, clic sol, cast 0 send true slot 0, jauge 75) ; non activee.
- Reste : apres S1-b activer la variante (point de clic sol a mesurer), jouer, fiche E ; option : router FactionBox* dans ClickWidget pour hol.HUD.Click.
NON VERIFIE: le tir au sol (aim 0 / cast 0) et le curseur de visee (S1-b absent) ; aucun son ; le curseur de visee n'a jamais ete vu ; clic realise par injection Slate (hol.Mouse.Click), pas par le curseur du bureau.
PROCESSUS_DETACHES: voir sortie stray_processes en dernier geste (aucun attendu)
ANOMALIES_PROCESSUS: aucune ; 1er preflight : les 20 "Condition failed" des smoke tests moteur (-culture=fr) sont connus et non comptes.
GIT_STATUS_PRINCIPALE: docs voulus seulement (Auto_Journey_File.md, EtapeDefini.md modifies par moi, ligne propre) : oui
