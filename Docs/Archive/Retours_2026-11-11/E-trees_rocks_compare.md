# [A ESSAYER] E-trees_rocks_compare : les arbres et les rochers sous les cinq biomes, vus d'un coup ; dis aime / pas aime par lettre

STATUT : VU À L'ÉCRAN (rejoué par l'intégrateur sur main fusionné : 10 PNG ouverts en 3 planches de contact ; les arbres du bosquet sont identiques d'un biome à l'autre, sol, lumière et teinte des rochers changent de A à E ; aucun goût jugé)
GESTE: interne      (hol.Biome.Index lance par le scenario ; la touche Entree est le seul geste de l'utilisateur, passee en auto par preflight)
SON: sans objet
branches: visual

Ce que tu vois : sur la Plaine, pour chaque biome (A Herbe fraiche, B Steppe seche, C Automne, D Foret sombre, E Neige), deux vues : (1) le bosquet d'arbres de la carte, (2) la crete de trois rochers devant la base. Entree = biome suivant. Rien n'est change dans le jeu : c'est le rendu actuel (provisoire) des Placeholders.txt `Arbre biome ...` et `Rochers par biome`.
VU (agent, sans jugement de gout) : (a) L'ARBRE est le MEME dans les cinq biomes : tronc cylindre brun clair, couronne ovale en boule lisse (forme du moteur), c'est voulu (D655 : les ressources ne changent pas avec le biome) ; seuls la lumiere, la brume et le sol changent sa teinte (vert clair en Herbe et Steppe, vert-olive en Automne, vert franc fonce en Foret sombre, vert pale gris en Neige). Il n'y a PAS d'arbre d'automne orange ni d'arbre enneige dans le jeu. (b) LES ROCHERS de la crete sont trois ellipsoides aplatis soudes, teintes par le biome (D656) : beige creme (Herbe, Steppe), brun sable (Automne), gris-vert (Foret sombre), blanc-bleute (Neige). Meme forme partout.
Question pour toi (une seule, par lettre) : aime / pas aime pour l'arbre et pour le rocher tels quels ; et dis si tu veux, pour un biome (Automne ? Neige ? Foret sombre ?), un arbre PROPRE (forme ou couleur differente) : ce serait un lot du chantier Placeholders (dix echantillons), pas fait ici.

Pre-vol (2026-11-11 14:17, agent Sauron-R2, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branche visual) : `preflight.ps1 trees_rocks_compare -Root <worktree>` PASS (38 s, 10 PNG, 4 waitkey en auto, sanity OK, aucun warning).
PNG OUVERTS et vus : trees_A..E et rocks_A..E (planches de contact 2 colonnes, 640x360 par vue).
NON VERIFIE : vu en camera de jeu a zoom 2, pas en partie complete avec unites ; le bosquet est coupe a droite du cadre (camera centree sur l'est du bosquet) ; les rochers vus sont ceux de la crete de la Plaine (les autres cartes ont d'autres rochers : cols, murs de roche) ; aucune comparaison avec un autre biome sur une autre carte.

Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 trees_rocks_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir  (rejoue sur le build de main 521adcb fusionne, worktree Faramir ; sans -Hidden = fenetre, reservee a l'utilisateur ; un agent ajoute -Hidden)
Duree : environ 1 min 30. Reponse attendue : pour A a E `aime` / `pas aime` (arbre, rocher) ou `REGLAGE: ...`.

## Integrateur Faramir 2026-11-11 15:09-15:16, main 521adcb, binaire 15:03 (session_gate -Phase Essay PASS)
Constats factuels : A (herbe fraîche) montre un sol gris-beige sans vert, proche de E (neige) en plus clair ; le bosquet est coupé par le bord droit de l'image (à moitié hors cadre) ; la base annoncée devant la crête de rochers n'apparaît pas dans le cadre. Durée : 5 options, environ 2 min.

## Cadrage Faramir 2026-11-11 15:3x (branche fog, sans C++)
Zoom 2 remplace par Zoom -1 (camera plus haute). preflight PASS. trees_A_fresh_grass rouvert a plein format : le bosquet de 12 arbres est entier dans le cadre (plus coupe par le bord droit). Constat inchange, sans jugement : le sol de A reste gris-beige sans vert (c'est le rendu du sol du biome 0, pas le cadre). La crete de rochers n'est pas retrouvee dans rocks_A (FocusNear 3000 0 : a recadrer, rocks_* non rouverts apres ce changement). Le panneau est masque pendant l'essai (panel hide) : pas de bandeau sur ces PNG. Statut : trees_* VU A L'ECRAN pour le cadre ; rocks_* LOGIQUE SEULEMENT.

## Faramir-28 2026-11-11 15:5x (branche fog, commit 43a0063, sans C++)
Panneau de nouveau VISIBLE (avant : panel hide, donc ni bandeau ni ligne d'attente) ; une note courte par biome posee AVANT les captures de ce biome. preflight PASS. PNG ouverts a plein format : rocks_A_fresh_grass (bandeau = texte de A seul ; la crete de trois rochers est bien visible au centre du cadre, la base en bas) et rocks_E_snow (bandeau empile A a E, la ligne E est la derniere ; la crete est visible sous le bandeau, rochers blanc-bleute). FocusNear 3000 0 n'a pas ete change : le « crete non retrouvee » d'avant venait du zoom 2. Le bandeau s'allonge d'une ligne par biome (5 en E) car `note` n'efface jamais (C++). Statut : rocks_A et rocks_E VU A L'ECRAN pour le cadre et le bandeau ; trees_* et rocks_B a D non rouverts cette fois.

## Faramir-5 2026-11-11 16:2x (branche fog tip 3fd83e2 : `notereset` + etat « pret » des la 1re waitkey ; scenario converti)
preflight trees_rocks_compare PASS (1 WARN : pas de ligne « HUD overlap » ; le FAIL « no note line » d'avant = regex perimee de preflight.ps1, corrigee). PNG ouverts a plein format : trees_E_snow (bandeau = « E (5/5) Neige : bosquet d arbres (vue 1), crete de rochers (vue 2) » SEUL, etat « pret » ; bosquet de 12 arbres entier dans le cadre) et rocks_E_snow (meme bandeau seul, plus de pile A a E ; la crete de trois rochers blanc-bleute au centre sous le bandeau, la base en bas). Le bandeau ne s'allonge plus d'une ligne par biome. Statut : trees_E et rocks_E VU A L'ECRAN (bandeau juste, point net) ; A a D non rouverts cette fois. Aucun gout juge.
