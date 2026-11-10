# [INDEX] E-audio_compare_alerts : les six alertes sonores, une fiche PAR ALERTE (6 essais de 2 min), sons GRAVES, SOURDS, COURTS et DOUX

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore ecoute ; cet index n'est pas un essai, ce sont les 6 fiches qui le sont)
GESTE: interne      (voir chaque fiche : les sons sont joues par hol.Audio.Candidate, seule la touche Entree est la tienne)
SON: non entendu    (-nosound cote agent)
branches: heal-sleep

Une fiche par alerte, chacune : 10 sons (A a J), environ 2 min, le panneau affiche le nom du cue a cote de la lettre, Entree = suivant, une ligne `<Cue> = _` a remplir, la liste des lettres avec leur fichier, la commande `play.ps1` :

- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_alerts_attack.md`       : Attaque subie [Alert.UnderAttack] (deux coups sourds, reconnaissable mais pas stridente)
- (archivees le 2026-11-10 dans Docs\Archive\Retours_2026-11-10\ car ces trois alertes sont muettes par decision : BuildingDone, TechDone, Trade)
- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_alerts_volcano.md`      : Volcan [Alert.Volcano] (le jeu la joue 3 fois a 1 s d'ecart)
- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_alerts_diplomacy.md`    : Diplomatie [Alert.Diplomacy]

L'alerte d'entrainement fini a sa propre fiche : `E-audio_compare_training.md`.

Reponse attendue : dans chaque fiche, une lettre (ou plusieurs) pour son cue, `OK`, `BUG: ...` ou `REGLAGE: ...` ; tu peux repondre alerte par alerte, en plusieurs fois.

L'ancien scenario `audio_compare_alerts` (les 7 alertes a la suite, 70 sons, environ 14 min) est GARDE comme « tout » et n'est PAS compte comme un essai (plus de 3 min) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_alerts -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`. Il n'a pas de fiche : reponds dans les fiches ci-dessus.

Consigne appliquee (memoire "Sons : pas usants") : GRAVES, SOURDS, COURTS, DOUX ; mesure par `Tools\Audio\check_audio_soft.py` (duree max 0,75 s, centre spectral max environ 630 Hz, 0 % au-dessus de 3 kHz), ce n'est pas une ecoute. Les 6 anciens jeux (cloches, harpe, glockenspiel) sont remplaces. L'alerte d'attaque : deux coups graves (tambour sourd, bois, pierre, plaque etouffee, basse, gong etouffe), reconnaissable sans etre stridente. Rien n'est branche dans le jeu : le jeu entend toujours les sons actuels de HOLAudioTable.as.
