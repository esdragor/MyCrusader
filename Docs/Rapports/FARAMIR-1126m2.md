# FARAMIR-1126m2 (2026-11-26, worktree HeroesOfLiberty-Faramir, branche gate-refund-1126m2 depuis main 9d0a3f5)

APPELS_DECLARES: ~33 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 12:38 / 12:51 (arrêt dur 13:07 ; 2e tâche non prise : voir RESTE)
PREMIER_BUILD: sans C++   BUILD: sans C++   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 0987677 (Test_GateRefundsAShareOfTheRetiredPalisades + bouchon GateRefundOf / GateRefundPercent), vu à 12:45 : tests 2328/2329 ; assertit l'effet : oui (le nombre de pièces vient de GateRetireIndices, le remboursement de GateRefundOf)   TEST_ROUGE_SEPARE: oui
COMMITS: 3 (0987677 rouge, fcacede correctif, 69302f4 journal), dernier à 12:50, merge_main_fait: non (branche depuis main 9d0a3f5, rien à fusionner)
TESTS: quick_check -Parts tests (quick_check.ps1 du worktree) rouge 2328/2329 à 12:45, vert 2329/2329 à 12:49 ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (règle seule, pas de scénario)
QUESTIONS: aucune
FAIT / RESTE:
- Tâche 1 : D921 règle du remboursement de porte (Script\Buildings\HOLWallLineRules.as : HOLWallLine::GateRefundOf = pièces x prix de la case x pourcent / 100, arrondi bas ; cvar hol.Wall.GateRefundPercent défaut 25, 0 = ancien comportement ; HOLWallLine::GateRefundPercent()). Interprétation prise : pourcent appliqué au prix de la case de 4 m par pièce retirée (= la part de sous-case de D915, donc remboursement égal au paiement d'une pièce).
- RESTE D921 : le crédit au retrait (HOLRpcPlayer.as:1533-1555 pose seule, :1590-1617 ligne) est une LIAISON C++ : aucun crédit de stock appelable du script (OrderRemoveOwnBuilding détruit sans rembourser, la décision D921 veut le crédit hors de DestroyBuilding) ; à mettre au lot C++ ; puis scénario wall_gate_replace.
- Tâche 2 non prise : les autres lignes [ ] de la file Faramir sont périmées ou C++ : aperçu de visée Lumière sacrée (PreviewOf dans main, HOLFactionAim.as:70, dessin HOLGameScreenScript.as:2323), D900 (HOLFactionPowerServer.as:219-223, ApproachPointOf) et D904 / coût des murs (D915 dans main) sont déjà faits ; la seule ligne propre à la section Faramir est le C++ HUD heal. Lignes à cocher par la session principale : Auto_Journey_File.md l.58, l.59, l.62, l.107. Lignes 17 à 20 (launch_agent, after_commit) paraissent faites aussi (launch_agent 5ac564a, after_commit.ps1 existe), non vérifié.
NON VÉRIFIÉ: DIAGNOSTICS_AS non relus (serveur de langage non lancé ; compilation des scripts OK via quick_check tests 2329/2329) ; arrondi bas du remboursement choisi par moi (non tranché par l'utilisateur, une ligne à changer) ; appels de l'agent comptés de mémoire.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: attente du quick_check par boucle ping (Start-Process cmd ping) pour lire la sortie d'un travail déjà en arrière-plan : contourne la règle 3d « aucune attente par boucle » (aucun build ni Wait-Process) ; à ne pas refaire.
GIT_STATUS_PRINCIPALE: hors Docs\Rapports et .vscode seuls ArtistesInProgress.md, Auto_Journey_File.md, EtapeDefini.md modifiés (docs voulus, dont ma ligne de File et d'EtapeDefini) : oui
