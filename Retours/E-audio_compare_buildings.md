# [INDEX] E-audio_compare_buildings : le son de SELECTION des batiments, une fiche PAR FAMILLE (5 essais de 2 min au lieu d'un de 11 min, regle 18)

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore ecoute ; cet index n'est pas un essai, ce sont les 5 fiches qui le sont)
GESTE: interne      (voir chaque fiche : les sons sont joues par hol.Audio.Candidate, seule la touche Entree est la tienne)
SON: non entendu    (-nosound cote agent)
branches: heal-sleep

Une fiche par famille, chacune : 10 sons (A a J), environ 2 min, le panneau affiche le nom du cue a cote de la lettre, Entree = suivant, une ligne `<Cue> = _` a remplir, la liste des lettres avec leur fichier, la commande `play.ps1` :

- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_buildings_wood.md`   : Bois (Maison, camps, Moulin, Ferme) [Building.Wood]
- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_buildings_stone.md`  : Pierre (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) [Building.Stone]
- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_buildings_sacred.md` : Sacre (Chapelle, Sanctuaire) [Building.Sacred]
- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_buildings_metal.md`  : Metal (Forge) [Building.Metal]
- `C:\Users\baudo\Documents\MyCrusader\Retours\E-audio_compare_buildings_coins.md`  : Pieces (Marche) [Building.Coins]

Reponse attendue : dans chaque fiche, une lettre (ou plusieurs) pour son cue, `OK`, `BUG: ...` ou `REGLAGE: ...` ; tu peux repondre famille par famille, en plusieurs fois.

L'ancien scenario `audio_compare_buildings` (les 5 familles a la suite, 50 sons, environ 11 min) est GARDE comme « tout » et n'est PAS compte comme un essai (plus de 3 min) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`. Il n'a pas de fiche : reponds dans les 5 fiches.

Consigne appliquee (memoire "Sons : pas usants") : GRAVES, SOURDS, COURTS, DOUX ; mesure par `Tools\Audio\check_audio_soft.py` (duree max 0,75 s, centre spectral max environ 730 Hz, 0 % au-dessus de 3 kHz), ce n'est pas une ecoute. Rien n'est branche dans le jeu (Select.Building reste muet).
