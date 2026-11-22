# SAURON-1122b (2026-11-22)
APPELS_DECLARES: 19 (vus)
HEURE_DEBUT/HEURE_FIN: 16:33 / 16:48
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (outil PowerShell)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 756cf78, vu a 16:44 (gate : FAIL L22-6 x2, DIAG compte 6 au lieu de 5 ; filtre : ROUGE (1)) ; assertit l'effet : oui   TEST_ROUGE_SEPARE: oui
COMMITS: 2 (756cf78 rouge, fc6230a vert), dernier 16:47, merge_main_fait: non
TESTS: main_code_filter_test VERT (2 cas L22-6 neufs) ; section Close de gate_test (reconstituee : en-tete + lignes 470-fin, 10 cas) 10 PASS 0 FAIL ; gate_test.ps1 COMPLET non relance apres correctif (>8 min, arret dur) ; TESTS_TAUTOLOGIQUES: 0
CAUSE : l'ancienne regle exemptait seulement les branches integ-* ENCORE extraites dans un worktree HeroesOfLiberty-* autre qu'Arwen ; une fois le worktree d'un agent DIAG passe sur une autre branche, ses integ-N redevenaient « session principale ».
CORRECTIF : git ne stocke pas d'auteur distinct (meme auteur partout) mais le journal HEAD de chaque worktree (<common>/worktrees/<nom>/logs/HEAD) dit ou un commit a ete fait. MainCodeFilter.ps1 : Get-LogCommitShas, Get-DeclaredWorktrees (colonne worktree de '## Agents du <date>'), Get-AgentCommitShas ; session_gate.ps1 l.340-: remplace $mcAgentInteg par $mcAgentShas, saute les commits faits dans un worktree declare (Arwen jamais exempte). Seuil MaxMainCodeCommits inchange (3).
RESIDUEL (non desserre, par prudence) : un worktree d'agent supprime (git worktree remove) perd son journal : ses commits redeviennent comptes ; un agent qui commite hors de son worktree declare est compte.
NON VERIFIE: gate_test.ps1 complet ; gate reel sur le depot principal (le « 25 fois » n'a pas ete rejoue) ; fins de ligne/BOM : fichiers sans BOM, CRLF conserves (parse sans erreur).
PROCESSUS_DETACHES: voir stray_processes ci-dessous
ANOMALIES_PROCESSUS: un 1er appel PS (commit+stray) est sorti en 255 sans effet, refait ; un gate_test complet lance en avant-plan a depasse 180 s (arrete par moi pour lancer la section Close seule).