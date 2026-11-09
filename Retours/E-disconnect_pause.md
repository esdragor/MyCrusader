# [A ESSAYER] E-disconnect_pause : Deconnexion et pause : la fenetre ne s ecoule pas pendant une pause

STATUT : VU A L'ECRAN (objet de l'essai = l'AFFICHAGE ; l'evenement de depart est simule)
GESTE: interne      (commandes hol.Net.FakeAbsent / hol.Match.Pause : le depart reseau est simule sans second client ; Entree = temps suivant)
SON: sans objet
branches: net-playtest

Pre-vol (2026-11-09 16:23, agent Glorfindel-21 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main e9a349a) : preflight.ps1 disconnect_pause -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice PASS. WARN : pas de ligne HUD overlap ni Sanity.
PNG OUVERTS et vus :
- pause_before : bandeau 1:00, pas de pause
- pause_a et pause_b : Partie en pause + Reprendre, bandeau 0:59 IDENTIQUE sur les deux (8 s d ecart)
- pause_after : 0:58, pause_later : 0:54 (le compte a rebours repart)
NON VERIFIE : la pause de Bob levee d office quand il part (couverte par lobby_match -Reconnect) ; pause en reseau a plusieurs clients.

Lancer UNE commande (-PlayerName Alice obligatoire) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_pause -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice
Duree : environ 1 min. Reponse attendue : OK, BUG: ... ou REGLAGE: ...

A regarder : pause_a et pause_b (8 s d ecart, en pause) montrent le meme temps 0:59 puis le compte a rebours repart (0:58, 0:54).

Reponse (utilisateur, 2026-11-09 16:35, joue par lui) : OK.
