# [À ESSAYER] E-glade-sweep : la même forêt de 300 arbres huit fois (A à H) avec d'autres clairières ; choisis la lettre

STATUT : VU À L'ÉCRAN   (les 8 variantes vues en PNG ; A, B, D, E regardées sur la passe précédente du scénario, mêmes réglages et même graine, C, F, G, H regardées sur la passe finale)
GESTE: interne      (Entrée est simulée par le pré-vol ; la caméra saute d'une variante à l'autre par `hol.Camera.Focus`, pas de clic)
SON: sans objet

Ce que tu vois : la carte d'essai (pas une carte de partie) porte huit forêts de 300 arbres, même graine, mêmes contours ; seul le nombre et la taille des clairières change. Entrée = variante suivante.
- A : les cartes d'aujourd'hui (une clairière OU un sentier de 3 m, un bosquet sur deux n'en a pas : ici un sentier qui coupe la forêt en deux)
- B : 1 clairière, rayon x1,5 (trou rond près du bord)
- C : 1 grande clairière, rayon x2,5, au centre (anneau d'arbres)
- D : 2 clairières x1,5
- E : 3 clairières x1,5
- F : 3 clairières x1,5, gardées à l'intérieur de la forêt
- G : 5 petites clairières (x1)
- H : 2 grandes clairières (x2,5), gardées à l'intérieur
Réponse attendue : une lettre (puis « une seule régénération des cartes avec ce réglage » est une tâche à part), ou « plus de clairières / plus grandes / plus petites ».
Vu : un grand rayon loin du centre ouvre le contour (croissant) ; au centre ou à l'intérieur on garde un anneau (C, F, H). Les forêts peintes à la main par l'utilisateur (Cols, Couronne, El Dorado, Arrière-pays, Plaine Normale, Forêt Moyenne) ne sont PAS touchées par ce réglage : seules les forêts tirées par le générateur le sont (Gués, Steppe, zones non peintes).

Pré-vol (2026-11-07 ~09:40, agent HALDIR-1, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, main 584f434) : `preflight.ps1 glade_sweep -Root <worktree>` PASS (~40 s, 8 PNG, HUD sans recouvrement, audit de sanité passé).
PNG OUVERTS et vus : glade_A (forêt coupée en deux par un sentier), glade_B (trou rond), glade_C (anneau autour d'une grande clairière), glade_D, glade_E, glade_F, glade_G, glade_H (croissants ou anneaux ouverts).
NON VÉRIFIÉ : le rendu vu sur la caméra par défaut du scénario seulement ; l'effet de ces clairières sur le passage des unités et la navmesh n'est pas mesuré ; aucune carte de partie n'est régénérée avec un de ces réglages ; le choix de la taille de forêt (300 arbres) est celui du balayage, pas celui d'une carte.

Lancer UNE commande (la carte d'essai est un fichier non commité : `Content\Maps\Crown.umap`, fait par le premier script ; le second la rejoue) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\make_glade_sweep.ps1` (une minute, éditeur seul) puis
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 glade_sweep -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 2 min. Réglages (une ligne à changer) : `GLADE_FORCE` / `GLADE_SCALE` / `GLADE_REACH` de `Tools\Editor\maps\common.py` (variables d'environnement `HOL_GLADE_FORCE`, `HOL_GLADE_SCALE`, `HOL_GLADE_REACH`) ; défaut codé = aucun changement (`GLADE_FORCE` 0, les cartes restent les mêmes).

Reponse (utilisateur, 2026-11-09, apres play.ps1 glade_sweep) : OK, « tout est bien » : aucun defaut vu dans les 8 variantes. Aucune lettre choisie : le reglage reste GLADE_FORCE 0 (cartes inchangees) tant qu'une lettre (ou « plus/moins de clairieres ») n'est pas donnee ; la regeneration des cartes reste une tache a part.

Reponse (utilisateur, 2026-11-09) : « j'aime tout » : les 8 variantes (A a H) plaisent, pas de lettre unique. Defaut qui serait code : varier le style de clairieres d'une carte a l'autre (ex. B/C pour la Foret, D/E pour la Plaine, G pour la Steppe...) -> a confirmer ; GLADE_FORCE reste 0 (cartes inchangees) d'ici la.
