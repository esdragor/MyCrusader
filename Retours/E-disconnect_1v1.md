# [A ESSAYER] E-disconnect_1v1 : Deconnexion en 1 contre 1 : Bob (seul adversaire) part, la fenetre finit, Bob est vaincu et Alice gagne

STATUT : VU A L'ECRAN (objet de l'essai = l'AFFICHAGE ; l'evenement de depart est simule)
GESTE: interne      (commandes hol.Net.FakeAbsent End 1 : le depart reseau est simule sans second client ; Entree = temps suivant)
SON: sans objet
branches: net-playtest

Pre-vol (2026-11-09 16:3x, agent Glorfindel-22 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main e9a349a) : preflight.ps1 disconnect_1v1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice PASS (35 s, 2 PNG). WARN : pas de ligne HUD overlap ni Sanity.
PNG OUVERTS et vus :
- dc1v1_away : base de Bob (forum, tours, baies, arbres), pastille "Connexion perdue" a cote de Bob (2 vies) dans la liste des joueurs, bandeau "Bob : connexion perdue - 1:00" sous le panneau du scenario
- dc1v1_end : ecran "Victoire" (Continuer / Quitter vers le menu / Statistiques), Bob "Vaincu" dans la liste des joueurs, ligne de journal "Bob a quitte la partie"
NON VERIFIE : le depart reel d'un second jeu (lobby_match -Reconnect/-Team) ; la liste de Bob montre encore "2 vies" apres "Vaincu" (non juge : c'est un affichage de la defaite comptee, pas un point de l'essai) ; le texte "en preparation" du panneau du scenario reste affiche pendant tout le scenario (le panneau des notes recouvre le bandeau si on ajoute des notes en cours de route : les notes sont donc a la fin) ; le scenario porte un waitkey (Entree) voulu : deux temps.

Lancer UNE commande (-PlayerName Alice obligatoire) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_1v1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice
Duree : environ 1 min. Reponse attendue : OK, BUG: ... ou REGLAGE: ...

A regarder : temps 1 = Bob "Connexion perdue" avec le bandeau 1:00 ; Entree ; temps 2 = Bob Vaincu, "Bob a quitte la partie", ecran Victoire d'Alice.
