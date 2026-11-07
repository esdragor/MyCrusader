# [À ESSAYER] E-audio_compare_alerts_volcano : écoute l'alerte de VOLCAN (le jeu la joue 3 fois a 1 s d'ecart) : grondements GRAVES et SOURDS, pas stridents ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Volcan [Alert.Volcano] = _

Lettres (le panneau affiche le nom du cue a cote de la lettre) :
Alert.Volcano : A s_volc_timpani_deep, B s_volc_gong_low, C s_volc_bass, D s_volc_heavy_metal, E s_volc_punch, F s_volc_wood_rumble, G s_volc_plate, H s_volc_soft_boom, I s_volc_creak, J s_volc_mining

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_alerts_volcano -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
