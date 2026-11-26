# [À ESSAYER] E-art_gallery : la galerie de l'Atelier pose un exemplaire de chaque unité, bâtiment et source des tables, étiqueté au sol (nom de type, nom affiché)

STATUT : VU À L'ÉCRAN
GESTE: interne      (commande `hol.Art.Gallery All` : c'est l'outil lui-même, pas un geste de joueur)
SON: sans objet
branches: art-gallery3-1125ar (AR1-d ; contient art-gallery-1125ar et art-gallery2-1125ar par main 4a72c03)

Pré-vol (2026-11-25 19:34, agent SAURON-1125ar, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main c386ed2) : `preflight.ps1 art_gallery -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -AllowUncommitted` PASS (22 s, 3 PNG) ; tout est commité depuis (9d2e9f0). Journal : `Art gallery: family=All places=51 units=21 buildings=19 sources=11 refused=0`.
PNG OUVERTS et vus :
- ag_units : unités bleues en grille, chacune avec son étiquette jaune à plat au sol (p. ex. « King / King », « Hero / Paladin Commander », « LightCavalry / Cavalier léger ») ; les grands modèles (Mangonel, Bélier) débordent sur leurs voisins.
- ag_buildings : bâtiments en grille (StoneWall, ArcheryRange, MiningCamp, Sanctuary, LumberCamp...), étiquettes jaunes lisibles « Sanctuary / Sanctuaire ... ».
- ag_sources : cylindres gris (formes d'attente à la taille Radius x Height de la table) ; leurs étiquettes sont en partie cachées par le cylindre.
NON VÉRIFIÉ : les 21 unités et 19 bâtiments ne tiennent pas dans un seul cadre (seule une partie de chaque famille est dans la capture) ; la grille des unités est près de la base du joueur (blocs bleus en haut de ag_units), pas « loin du Forum » au sens strict ; les sources ne sont PAS de vraies sources (AddSource non appelable en script : C++ à exposer) ; tests script HOLArtGallery_Test.as non lancés par l'agent.

Pre-vol AR1-c (2026-11-25 21:45, agent CELEBORN-1125ar7, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche art-gallery2-1125ar 9b0639d) : `preflight.ps1 art_gallery` PASS (32 s, 7 PNG) ; tests 2289/2289 PASS 21:44 (rouge a3c33bb vu 2286/2289 a 21:40).
PNG OUVERTS et vus (AR1-c) :
- ag_marks : chaque batiment de la moitie sud avec le contour cyan de son emprise, les lignes fines de la grille (cases de 4 m, sous-cases) sur l'emprise, un poteau orange (Height) au coin avant droit, une petite barre rouge de 1,80 m a sa droite ; les unites en bas ont aussi leur barre rouge.
- ag_light_steppe : meme cadre, sol jaune-olive et cailloux bruns du decor de la steppe (hol.Art.Light dry_steppe) : different de ag_marks.
- ag_light_snow : meme cadre, sol blanc, decor gris ; les etiquettes jaunes deviennent peu lisibles sur la neige.
- ag_units : unites en grille, etiquettes jaunes, barre rouge a cote de chacune ; ag_sources : vraies sources (filons, baies, arbre) etiquetees, barre rouge a cote ; le bas du cadre montre des batiments (Marche, Moulin).
NON VERIFIE (AR1-c) : ag_buildings (moitie nord) non rouvert ce run (vu par CELEBORN-1125ar6 avant les reperes) ; la barre de 1,80 m est petite a ce zoom (pas de vue rapprochee) ; tuile de 4 m ecrite dans le script (UHOLBuildingSettings::TileSize non lisible depuis un namespace) ; sous-cases = hol.Wall.SubDivision ou 4 quand il vaut 1.
Rejouer AR1-c : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 art_gallery -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` (Entree = famille suivante, puis reperes, Steppe seche, Neige).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 art_gallery -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
- AR1-d (CELEBORN-1125ar9, art-gallery3-1125ar 89d5c79) : VU A L ECRAN 22:51 : ag_light_snow (etiquettes jaunes cernees de noir, lisibles sur le sol blanc), ag_focus (hol.Art.Focus MainCentre 8 : centre principal entier, emprise cyan, lignes de 4 m, poteau orange Height, etiquette cernee ; barre rouge 1,80 m du centre principal non distinguee, cachee par la tour). hol.Art.Light accepte Steppe seche / Neige (noms affiches, FHOLBiome.Name). GESTE: interne. SON: non entendu.
