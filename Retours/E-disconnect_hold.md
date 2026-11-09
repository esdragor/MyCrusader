# [A ESSAYER] E-disconnect_hold : Deconnexion, mode tenir : Bob parti, l IA tient son siege pendant la fenetre

STATUT : VU A L'ECRAN (objet de l'essai = l'AFFICHAGE ; l'evenement de depart est simule)
GESTE: interne      (commandes hol.Net.FakeAbsent / hol.Match.Pause : le depart reseau est simule sans second client ; Entree = temps suivant)
SON: sans objet
branches: net-playtest

Pre-vol (2026-11-09 16:23, agent Glorfindel-21 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main e9a349a) : preflight.ps1 disconnect_hold -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice PASS. WARN : pas de ligne HUD overlap ni Sanity.
PNG OUVERTS et vus :
- hold_start : base de Bob, pastille Connexion perdue, bandeau 1:00 (vu a la premiere passe, avant le changement des attentes 15 s -> 20 s ; la prise de vue est identique)
- hold_20s : bandeau 0:40, un Paysan rouge hors de la base
- hold_40s : bandeau 0:20, deux Paysans rouges (baies et pres de la base), aucune troupe
NON VERIFIE : la verification que le mode tenir ne lance AUCUNE vague (3 captures sur 40 s seulement) ; le depart reel d un second jeu (lobby_match -Reconnect/-Team) ; l intervention reelle de l utilisateur est un affichage, pas un geste.

Lancer UNE commande (-PlayerName Alice obligatoire) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_hold -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice
Duree : environ 1 min. Reponse attendue : OK, BUG: ... ou REGLAGE: ...

A regarder : pastille Connexion perdue, bandeau Bob : connexion perdue avec le compte a rebours qui baisse (1:00, 0:40, 0:20), Paysans rouges qui sortent, aucune troupe.

Reponse (utilisateur, 2026-11-09 16:35, joue par lui) : OK.
