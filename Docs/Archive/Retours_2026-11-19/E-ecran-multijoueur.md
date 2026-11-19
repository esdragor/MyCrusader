# [RÉPONDU : OK, 2026-11-17 ~22:5x] E-ecran-multijoueur : l'écran Multijoueur (écrit en script) s'ouvre, montre le nom, l'erreur sans gestionnaire et ses 3 boutons

STATUT : VU À L'ÉCRAN   (REPRISE 6, 2026-11-17 ~20:30 : d751-root fusionnée avec main 4fd5394, build PASS 169 s, Get-StaleBinaryProblem vide, quick_check tests 1496/1498 (2 Nomad connus), preflight ecran_multijoueur PASS sur le -Root exact, PNG vu : voile sombre derrière la colonne, Créer un salon ACTIF ; le scénario n'a pas de .json, le geste interne n'a pas été joué à la main)
GESTE: interne   (le scénario ouvre l'écran par commande hol.UI.Open Multiplayer ; aucun vrai clic joué)
SON: non entendu   (-nosound)
branches: d751-root

Pré-vol : PASS (ARWEN-1117e, 2026-11-17 10:35, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, commit 80e743b ; tests script 3/3 verts, quick_check tests 1393/1395, les 2 rouges sont NomadFirstForum et NomadReplicated, connus).
PNG vu (Saved_Tests\Screenshots\WindowsEditor\HOL\ecran_multijoueur_liste.png) : titre « Multijoueur » en jaune, ligne « Votre nom » avec le champ « Bob », ligne rouge d'erreur sous la zone de liste (« Le serveur de parties ne répond pas »), boutons Actualiser / Créer un salon / Retour.
Écarts vus (ARWEN-1117f, 10:38, preflight PASS rejoué, PNG identique) : (1) « Créer un salon » ACTIF = voulu : CanCreateLobbyAfter ne grise que bad_key (log : HTTP 0, code vide = injoignable), le .look disait grisé à tort (non corrigé : voir Q-ecran-multijoueur-fond.md Q1, défaut A) ; (2) pas de fond, comme l'ancien écran C++ (Q2, défaut A : rien changé, 0 C++ demandé, BUILD À RELANCER seulement si B).
NON VÉRIFIÉ : clics réels (Actualiser, Créer, Retour), liste de salons avec un gestionnaire en marche (boutons Rejoindre / Regarder / Reprendre / Observer), comportement du champ nom.

Commande complète : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 ecran_multijoueur -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
