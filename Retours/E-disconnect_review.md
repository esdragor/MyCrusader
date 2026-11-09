# [À ESSAYER] E-disconnect_review : Déconnexion en 2 contre 2 (tenir, pause, fin de fenêtre, retour) : la pastille et le bandeau de Bob

STATUT : VU À L'ÉCRAN (objet de l'essai = l'AFFICHAGE ; l'événement de départ est simulé)
GESTE: interne      (commandes `hol.Net.FakeAbsent Leave/End/Back`, `hol.Match.Pause/Resume` : le départ réseau est simulé sans second client ; Entrée = temps suivant)
SON: sans objet
branches: net-playtest

Règle (L7-6) : le départ réseau réel d'un second jeu n'est pas joué ici (voir NON VÉRIFIÉ) ; seul l'affichage l'est.

Pré-vol (2026-11-09 10:53, agent Elrond-integration, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 60013e8, rejoué sur le build de main fusionné) : `preflight.ps1 disconnect_review -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -PlayerName Alice` PASS (31 s, 7 PNG). WARN : pas de ligne `HUD overlap:` ni `Sanity:`.
PNG OUVERTS et vus :
- review_hold : base de Bob, table des joueurs « Bob 2 vies Connexion perdue », bandeau « Bob : connexion perdue — 1:00 »
- review_pause_a et review_pause_b : « Partie en pause » + bouton Reprendre, bandeau « Bob : connexion perdue — 0:58 » IDENTIQUE sur les deux (compte à rebours figé)
- review_end : pastille « IA » à côté de Bob, plus de bandeau, 2 vies
- review_back : plus aucune pastille pour Bob
Non rouverts cette fois (identiques à la passe de 10:33 de Glorfindel) : review_pause_before, review_pause_after (bandeau 0:57, il repart).
NON VÉRIFIÉ : le départ réel d'un second jeu (couvert par `lobby_match.ps1 -Reconnect/-Team/-Rejoin -Window`) ; la pause de Bob levée d'office à son départ ; le détail de ce que construit l'IA qui tient le siège ; disconnect_1v1 (Bob perd en 1 contre 1) non rejoué par moi sur main : il reste « prêt non donné ».

Lancer UNE commande (`-PlayerName Alice` obligatoire) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 disconnect_review -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -PlayerName Alice`   (Entrée : pause, fin de fenêtre, retour)
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

À regarder : temps 1 = pastille « Connexion perdue » et bandeau à compte à rebours ; temps 2 = Pause (le compte à rebours reste le même puis repart) ; temps 3 = fin de fenêtre, pastille « IA », Bob n'est pas battu ; temps 4 = Bob revient, plus de pastille.

Reponse (utilisateur, 2026-11-09, joue par lui) : OK.
