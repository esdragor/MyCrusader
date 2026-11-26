# FARAMIR-1126m (11:36 - 11:50) : gate Essay valide la commande exacte (L29-5) ; BUG deploy_far : cause vue, rejeu NON FAIT

APPELS_DECLARES: ~33 vus (premier Edit au 15e appel, apres lecture des sources : consigne « avant le 6e appel » non tenue)
HEURE_DEBUT/HEURE_FIN: 11:36:18 / 11:49:43
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: fed0ecd (5 cas FAIL vus a 11:47 sur le gate inchange, script d'extraction FARAMIR_essay_cmd_test.ps1 du scratchpad) ; assertit l'effet : oui (le gate imprime FAIL / PASS sur une vraie fiche E factice)   TEST_ROUGE_SEPARE: oui
COMMITS: 3 sur gate-essay-cmd-1126m (depuis main 99a1306) : fed0ecd test, d5999f0 correctif, b87c493 journal ; dernier a 11:49, merge_main_fait: non (branche creee depuis main)
TESTS: les 5 cas neufs de gate_test.ps1 (essay: commande ...) 5/5 PASS apres correctif (extrait seulement) ; gate_test.ps1 COMPLET NON TERMINE (> 8 min, arrete a la main pour garder l'heure) ; gate reel sur deploy_far/Haldir : 3 FAIL binaire perime + ecart de code, PASS commande exacte valide. TESTS_TAUTOLOGIQUES: 0
ESSAIS: deploy_far NON REJOUE (Haldir occupe par INTEG-1126m, integ-100, binaire du 10:44 plus ancien que Source du 11:30) : LOGIQUE SEULEMENT
QUESTIONS: aucune
FAIT / RESTE:
- FAIT L29-5 : session_gate.ps1 (Essay, avant `$named`) lit la DERNIERE ligne « Lancer UNE commande : » de la fiche : play.ps1 du meme worktree que son -Root, -Root = -Root du gate, scenario present dans Tools\Scenarios ; sinon FAIL ; commande illisible = FAIL.
- deploy_far : la commande donnee (play.ps1 Haldir -Root Haldir) est correcte et le scenario existe ; cause la plus probable = worktree Haldir en rebuild/integration au moment de l'essai (dernier journal scenario_deploy_far.log 25/11 17:19, aucun lancement a 18:3x) ; le gate Essay le signale maintenant FAIL (binaire PLUS ANCIEN que Source). Cause NON prouvee au journal (pas de rejeu).
- RESTE : rejouer `play.ps1 deploy_far -Root <Haldir> -Hidden` apres le build d'INTEG-1126m, lire le journal ; point L29-5 « commandes generees par un seul outil » ; relancer gate_test.ps1 complet ; integrer la branche gate-essay-cmd-1126m (branche prete, aucun push).
NON VERIFIE: gate_test.ps1 complet ; cause reelle du « jamais lance » ; la branche tests-bindings-1125s n'a pas ete touchee (nouvelle branche depuis main) ; la regex de la commande suppose `-File <chemin>\play.ps1 <nom> -Root <chemin>`.
PROCESSUS_DETACHES: aucun detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: gate_test.ps1 complet lance en tache de fond puis tue a la main (processus powershell du worktree Faramir) ; sortie du gate : « Heure PowerShell ... » + erreur ParseExact sur Docs\SessionEnd.txt dans le worktree (hors sujet).
GIT_STATUS_PRINCIPALE: hors docs voulus inchange : oui (ArtistesInProgress.md, Auto_Journey_*.md, SessionEnd.txt, EtapeDefini.md modifies par les sessions ; Retours\Q-sync-branche-prete-1126m.md non cree par moi)
