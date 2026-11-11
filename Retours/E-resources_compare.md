# [A ESSAYER] E-resources_compare : dix STYLES (A a J) de l'arbre, du buisson de baies, du filon de pierre et du filon d'or, vus d'un coup dans le jeu ; dis aime / pas aime par lettre

STATUT : VU A L'ECRAN   (les 10 PNG ouverts et vus, tour 2 ; c'est un choix de gout pour toi, pas pour l'agent)
GESTE: interne      (commande hol.Resource.Review lancee par le scenario ; la touche Entree est le seul geste de l'utilisateur, passee en auto par preflight)
SON: sans objet
branches: visual

Ce que tu vois : sur la carte Plain, a l'ecart de ta base, une rangee de quatre ressources, de gauche a droite : arbre, buisson de baies, filon de pierre, filon d'or, en formes de base du moteur (cube, cylindre, cone, sphere) en couleurs franches. Une lettre jaune (RESSOURCES A) est ecrite sur le sol. Entree = style suivant (10 styles). Ce ne sont PAS des modeles finals : c'est pour dire ce que tu aimes (silhouette, couleur) avant qu'on les fasse en vrai. Rien n'est change dans le jeu (aucune table, aucun mesh).
Styles : A pin 3 cones / baies rouges / 3 cubes ; B chene rond / baies bleues / rocher a taches ; C peuplier mince / cone bas / cristaux ; D parasol / haie / couches ; E grand cone / anneau de 5 boules / tas de boules ; F 3 boules decalees / dome orange / gros cube ; G cubes empiles / dome olive rose / pointes ; H automne / tambour / dome pique ; I arbre nu / deux tons / dalles ; J pin enneige / pointes bouts rouges / losange.

Pre-vol : tour 1 (2026-11-11 14:00, agent Sauron-R) puis tour 2 (14:17, agent Sauron-R2, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branche visual) : `preflight.ps1 resources_compare -Root <worktree>` PASS (41 s, 10 PNG, 9 waitkey en auto, sanity OK, aucun warning).
PNG OUVERTS et vus (tour 2) : res_A a res_J (les dix, en planches de contact) : etiquette jaune lisible, les quatre objets entiers dans le cadre sur chaque.
TOUR 2 (retouche des defauts du tour 1, meme liste de styles A a J) : camera plus pres (zoom 3), rangee resserree (2800 cm), etiquette plus petite, baies 1,7 fois plus grosses ; l'arbre ne penche plus vers le bord, les baies se lisent. Reste : pas d'ombre portee (formes un peu plates).
NON VERIFIE : aucun style juge pour son gout (c'est ton role) ; pas vu en jeu normal (carte pleine, camera de jeu, foule d'unites) ; les couleurs sont en espace lineaire du materiau M_MatteColor, a l'ecran elles peuvent sembler plus claires que prevu ; filon petit/moyen/gros non montres (seul le filon normal) ; arbres par biome (neige, sable...) vus a part dans E-trees_rocks_compare ; Mangonneau et autres lignes de Placeholders.txt inchangees.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 resources_compare -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (sans -Hidden : fenetre, reservee a toi).
Duree : environ 2 min. Reponse attendue : pour chaque lettre `aime` / `pas aime` (ou `REGLAGE: ...`) ; ensuite dix nouveaux styles.
