# [À ESSAYER] E-audio_compare_training : écoute l'alerte d'entrainement fini, troisieme tour : sons GRAVES, SOURDS, COURTS et DOUX (bois, cuir, tambour sourd, pierre, metal etouffe), sans cor ni trompette ni clochette ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Alert.TrainingDone = _

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_training -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 3 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.

Consigne de l'utilisateur appliquee (2026-11-07, memoire "Sons : pas usants") : sons entendus tres souvent et longtemps, donc GRAVES, SOURDS, COURTS, DOUX ; ni trompette, ni cor, ni clochette, ni aigus. Les sons "medievaux" (cor, trompette, cloches, harpe aigue) ont ete rejetes : ici tout vient de bois, cuir, pierre, tambour sourd, metal etouffe, cordes graves, tous passes au filtre passe-bas (Tools\Audio\make_soft_candidates.py, 3 filtres, hauteur baissee, 0,16 a 0,75 s).
MESURE (Tools\Audio\check_audio_soft.py, sur les fichiers WAV eux-memes, PASS) : duree max 0,75 s, centre spectral max environ 730 Hz (la plupart sous 500 Hz), 0,0 % de l'energie au-dessus de 3 kHz ; le meme outil refuse bien une trompette et un glockenspiel (autotest). Ce sont des mesures, PAS une ecoute : SON: non entendu.
Pre-vol (2026-11-07 12:33, ARWEN-15, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche heal-sleep) : preflight.ps1 audio_compare_training -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen PASS (37 s, sanity audit passed, 10 lignes 'candidate played' sur 10, 0 missing) ; PNG audio_compare_training_start OUVERT et vu (panneau du scenario lisible, notes dites). Troisieme tour de Alert.TrainingDone : A cuir, B bois, C planche, D tambour sourd, E basse pincee, F maillet, G pierre, H bouclier etouffe, I tissu, J livre (le fichier est dit par le panneau a chaque pas). NON VERIFIE : aucune oreille ; Entree reelle en jeu ; le jeu entend toujours bookPlace2 (table non changee).
