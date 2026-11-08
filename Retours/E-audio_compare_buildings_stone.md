# [À ESSAYER] E-audio_compare_buildings_stone : écoute le son de SELECTION des batiments en PIERRE (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Pierre (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) [Building.Stone] = _

Lettres (le panneau affiche le nom du cue a cote de la lettre) :
Building.Stone : A s_stone2_mine_deep, B s_stone2_mine_slow, C s_stone2_thud_low, D s_stone2_punch_deep, E s_stone2_timpani, F s_stone2_bass, G s_stone2_wood_heavy, H s_stone2_door_slow, I s_stone2_plank_low, J s_stone2_gong_dull

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_stone -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
