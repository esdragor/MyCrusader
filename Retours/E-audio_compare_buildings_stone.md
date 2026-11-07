# [À ESSAYER] E-audio_compare_buildings_stone : écoute le son de SELECTION des batiments en PIERRE (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Pierre (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) [Building.Stone] = rien de bien : aucune lettre, dix autres candidats a refaire  [REPONSE DE L'UTILISATEUR 2026-11-07, ecoute reelle]

Lettres (le panneau affiche le nom du cue a cote de la lettre) :
Building.Stone : A s_stone_mine_a, B s_stone_mine_b, C s_stone_mine_c, D s_stone_mine_d, E s_stone_punch_heavy, F s_stone_punch_soft, G s_stone_door, H s_stone_thud, I s_stone_drum, J s_stone_mallet

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_stone -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
Pre-vol (2026-11-07 12:48, ARWEN-16, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche heal-sleep) : preflight.ps1 audio_compare_buildings_stone -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen PASS (environ 35 s, sanity audit passed, 10 lignes 'candidate played Building.*' sur 10, 0 erreur) ; PNG audio_compare_buildings_stone_start OUVERT et vu (panneau du scenario lisible, nom du cue dans les notes ; un avertissement 'Shaders en preparation' passager sur la capture metal). Duree : 10 sons, environ 2 min (< 3 min). NON VERIFIE : aucune oreille ; Entree reelle en jeu ; rien n'est branche (Select.Building reste muet).
