# [À ESSAYER] E-disconnect_review : Déconnexion en 2 contre 2 (tenir, pause, fin de fenêtre, retour) puis en 1 contre 1 (Bob perd)

STATUT : VU À L'ÉCRAN (captures ouvertes et vues ; l'événement de départ est simulé, voir GESTE)
GESTE: interne (commandes `hol.Net.FakeAbsent Leave/End/Back`, `hol.Match.Pause/Resume` : le départ réseau est simulé sans second client ; Entrée = temps suivant)
SON: sans objet
branches: net-playtest

Pré-vol (2026-11-08 11:08, agent Glorfindel-19, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main c158640 + branche net-playtest) :
- `preflight.ps1 disconnect_review -Root ... -PlayerName Alice` PASS (41 s, 7 PNG, 3 waitkey, 4 notes).
- `preflight.ps1 disconnect_1v1 -Root ... -PlayerName Alice` PASS (29 s, 2 PNG, 1 waitkey, 2 notes).
PNG OUVERTS et vus :
- review_hold : base de Bob, table des joueurs « Bob 2 vies Connexion perdue », bandeau « Bob : connexion perdue — 0:59 ».
- review_pause_a et review_pause_b : « Partie en pause » + bouton Reprendre, bandeau « Bob : connexion perdue — 0:58 » IDENTIQUE sur les deux (4 s d'écart : le compte à rebours est figé).
- review_pause_after : plus de pause, bandeau « 0:57 » (il repart).
- review_end (vu en première version, le panneau du scénario couvrait le bandeau) : pastille « IA » pour Bob, plus de bandeau, 2 vies. review_back (idem) : aucune pastille. Le journal confirme : « the AI keeps the seat (full brain) », « Disconnect report: ... AI mode full brain », « he is back, the AI lets the seat go ».
- dc1v1_away : « Bob : connexion perdue — 1:00 », pastille « Connexion perdue ». dc1v1_end : écran « Victoire », Bob « Vaincu », ligne « Bob a quitté la partie ».
NON VÉRIFIÉ : le départ réel d'un second jeu (couvert par `lobby_match.ps1 -Reconnect/-Team/-Rejoin -Window`, pas par ces scénarios) ; la pause de Bob levée d'office à son départ (réseau seulement, ici la pause est celle d'Alice en solo) ; ce que l'IA tenant le siège construit en détail (on voit seulement la base, une caméra sur le forum de Bob) ; `session_gate.ps1 -Phase Essay` non lancé (branche pas fusionnée dans main : jouer depuis le worktree Glorfindel, qui contient le tip, après un `Tools\build.ps1` si le binaire est plus ancien que le dernier commit de code).

Lancer UNE commande par scénario (jouer depuis le worktree Glorfindel, `-PlayerName Alice` obligatoire) :
- `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_review -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice`   (Entrée : pause, fin de fenêtre, retour)
- `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_1v1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice`   (Entrée : fin de fenêtre, Bob perd)
Durée : environ 2 min pour le premier, 1 min pour le second. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

À regarder :
- 2 contre 2 : temps 1 = base de Bob avec la pastille « Connexion perdue » et le bandeau à compte à rebours ; temps 2 = Pause (le compte à rebours reste le même puis repart) ; temps 3 = fin de fenêtre, la pastille devient « IA » et Bob n'est pas battu ; temps 4 = Bob revient, plus de pastille.
- 1 contre 1 : même départ, puis à la fin de fenêtre Bob (seul dans son camp) est vaincu, « Bob a quitté la partie », écran « Victoire ».
Note : le panneau « en préparation… » du scénario reste affiché et grandit avec les notes (les notes sont à la fin pour ne pas cacher le bandeau). Les scénarios disconnect_hold, disconnect_team et disconnect_pause demandés sont réunis ici (hold = temps 1, pause = temps 2, team = temps 3 et 4) ; disconnect_1v1 est à part (autre configuration de camps).
