# [À ESSAYER] E-biome_decor_tour : les cinq biomes avec sol + lumière + décor générique (touffes, buissons...), Entrée = biome suivant

branches: visual
STATUT : VU À L'ÉCRAN
GESTE: interne
SON: sans objet

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`. (Ici l'essai est un jugement de look : le geste est la touche Entrée du scénario, pas un clic de jeu.)

Pré-vol (2026-11-07 10:53, agent Sauron-4, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branche visual a50ab54) : `preflight.ps1 biome_decor_tour -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (36 s, 5 PNG).
PNG OUVERTS et vus (en une planche de 5 vignettes 800x450, pas un par un en pleine taille) :
- decor_0_fresh_grass : sol vert clair ; décor = petits points blancs (fleurs) et touffes, très petits et épars ; soldats lisibles.
- decor_1_dry_steppe : sol vert-jaune ; décor presque invisible à cette distance de caméra.
- decor_2_autumn : sol orange-brun ; quelques petits points sombres (feuilles/buissons), peu visibles.
- decor_3_dark_forest : sol vert très sombre, vue sombre ; décor quasi invisible.
- decor_4_snow : sol blanc-gris ; quelques points sombres (buissons nus) et de petites taches, épars.
CONSTAT HONNÊTE : le décor générique EST posé (1044 pièces en neige, 2128 en herbe sur la Plaine, lues sur le monde) mais, à la hauteur de caméra du jeu, il est minuscule et clairsemé : le jugement « sol + lumière + décor » porte surtout sur sol + lumière. Densités et tailles du décor = valeurs provisoires à balayer.
NON VÉRIFIÉ : décor jugé sur une seule carte (Plaine) ; densités par rang provisoires (pas celles des cartes) ; coût S1/S2 non mesuré ; client réseau non essayé ; paquet (moteur `/Engine/BasicShapes` chargé par LoadObject) non essayé.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 biome_decor_tour -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...` (par biome : aimé / pas aimé ; décor trop petit ?).
