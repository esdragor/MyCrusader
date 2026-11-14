# [À ESSAYER] E-rocks_compare : dix styles A-J des rochers et du décor des cartes (rocher de crête, gros rocher, petites pierres, souche, fourré), Entrée = style suivant ; dis aime / pas aime par lettre

STATUT : VU À L'ÉCRAN
GESTE: interne      (les styles s'affichent par la commande `hol.Rocks.Review <lettre>` ; ton seul geste est Entrée pour passer au suivant)
SON: sans objet
branches: visual

Lot « ressources et décor » (PQ-1 = A), tour 1 des rochers et du décor, même méthode que `resources_compare` (dix échantillons, aime / pas aime, puis dix nouveaux). **A = ce que les cartes dessinent aujourd'hui** (sphères aplaties, souche cylindre), pour comparer ; B à J = neuf autres styles. Formes de base du moteur à la taille réelle du décor des cartes (gros rocher 2,6 m, fourré 3,6 m), rien n'est sauvegardé ni changé dans le jeu. Question : `Retours\Q-rocks_compare.md`.

Pré-vol (2026-11-14 11:1x, agent SAURON-1114b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, visual e3c4225) : `preflight.ps1 rocks_compare -Root` PASS (37 s, 10 PNG, Sanity PASS, 10 lignes `Rocks review: shown=1`).
PNG OUVERTS et vus (de gauche à droite : rocher de crête, gros rocher, pierres, souche, fourré ; étiquette jaune ROCHERS <lettre>) :
- rocks_A : sphères aplaties blanc-gris, souche brune minuscule, fourré ovale vert
- rocks_B : blocs à facettes clairs à flancs sombres, petits cubes, fourré de trois cubes verts
- rocks_C : dalles empilées beige, petites dalles, fourré de cinq cônes verts
- rocks_D : grappes de boules claires, souche à racines, fourré de trois boules
- rocks_E : aiguilles (cônes) claires, tronc cassé, fourré de pointes vertes
- rocks_F : mesas cylindriques saumon-rouille à sommet plat, souche moussue, fourré brun sec
- rocks_G : blocs blancs à fissure sombre, fourré vert vif en deux boules
- rocks_H : orgues (cylindres) gris, souche noire, fourré vert très sombre
- rocks_I : rochers à chapeau de mousse verte, fourré à dessus clair
- rocks_J : rochers à chapeau de neige, souche et fourré enneigés
NON VÉRIFIÉ : les couleurs sortent plus claires à l'écran que les valeurs données (l'ardoise de E et le basalte de H se lisent gris clair, pas sombres) ; le sol de la Plaine d'essai est un damier gris, pas un sol de biome (la lecture sur herbe ou neige n'est pas vue) ; pas d'ombre (D258) ; la souche (50 cm) est minuscule à ce zoom (taille réelle).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 rocks_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...` (et la réponse à `Q-rocks_compare.md`).
