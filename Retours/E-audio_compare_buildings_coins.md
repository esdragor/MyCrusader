# [À ESSAYER] E-audio_compare_buildings_coins : écoute le son de SELECTION du batiment de PIECES (Marche) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Pieces (Marche) [Building.Coins] = _

Lettres (le panneau affiche le nom du cue a cote de la lettre) :
Building.Coins : A s_coins_purse, B s_coins_leather_b, C s_coins_leather, D s_coins_drop, E s_coins_tin_b, F s_coins_counter, G s_coins_tin, H s_coins_cloth, I s_coins_soft, J s_coins_slap

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_coins -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
Pre-vol (2026-11-07 12:48, ARWEN-16, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche heal-sleep) : preflight.ps1 audio_compare_buildings_coins -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen PASS (environ 35 s, sanity audit passed, 10 lignes 'candidate played Building.*' sur 10, 0 erreur) ; PNG audio_compare_buildings_coins_start OUVERT et vu (panneau du scenario lisible, nom du cue dans les notes ; un avertissement 'Shaders en preparation' passager sur la capture metal). Duree : 10 sons, environ 2 min (< 3 min). NON VERIFIE : aucune oreille ; Entree reelle en jeu ; rien n'est branche (Select.Building reste muet).
