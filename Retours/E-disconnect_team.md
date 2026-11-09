# [A ESSAYER] E-disconnect_team : Deconnexion en equipe : Bob et Carol partent, fin de fenetre, leurs IA gardent les sieges

STATUT : VU A L'ECRAN (objet de l'essai = l'AFFICHAGE ; l'evenement de depart est simule)
GESTE: interne      (commandes hol.Net.FakeAbsent / hol.Match.Pause : le depart reseau est simule sans second client ; Entree = temps suivant)
SON: sans objet
branches: net-playtest

Pre-vol (2026-11-09 16:23, agent Glorfindel-21 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main e9a349a) : preflight.ps1 disconnect_team -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice PASS. WARN : pas de ligne HUD overlap ni Sanity.
PNG OUVERTS et vus :
- team_hold : bandeaux Bob et Carol 1:00, deux pastilles Connexion perdue
- team_end : pastilles IA pour Bob et Carol, 2 vies, plus de bandeau
- team_later : identique, personne n est battu
NON VERIFIE : ce que fait l IA en cerveau complet apres la fin de fenetre (10 s sans activite visible : le cerveau complet n est observe que par le Report et la pastille) ; la fin de partie.

Lancer UNE commande (-PlayerName Alice obligatoire) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_team -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice
Duree : environ 1 min. Reponse attendue : OK, BUG: ... ou REGLAGE: ...

A regarder : team_hold : deux bandeaux 1:00 et pastilles ; team_end : pastilles IA, plus de bandeau, 2 vies chacun ; team_later identique (personne n est battu).

Reponse (utilisateur, 2026-11-09 16:35, joue par lui) : OK.
