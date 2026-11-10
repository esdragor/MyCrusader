# [À ESSAYER] E-audio_compare_alerts_attack : écoute l'alerte d'ATTAQUE subie : deux coups SOURDS et graves, reconnaissable mais pas stridente (pas usante) ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Attaque subie [Alert.UnderAttack] = _

Lettres (le panneau affiche le nom du cue a cote de la lettre) :
Alert.UnderAttack : A s_atk_drum_double, B s_atk_drum_deep, C s_atk_punch_double, D s_atk_wood_double, E s_atk_plate_double, F s_atk_mining_double, G s_atk_bass_double, H s_atk_gong_double, I s_atk_soft_double, J s_atk_metal_double

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_alerts_attack -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
