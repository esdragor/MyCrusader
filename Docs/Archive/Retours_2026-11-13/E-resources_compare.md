# [À ESSAYER] E-resources_compare : les dix styles A a J de ressources (arbre, baies, filon de pierre, filon d'or), Entree = style suivant ; rappel du choix deja fait (arbre C, baies et filons A), aujourd'hui fige dans le jeu

STATUT : VU À L'ÉCRAN
GESTE: interne      (hol.Resource.Review <lettre> ; seule la touche Entree passe au style suivant, aucun clic)
SON: sans objet
branches: visual

Pré-vol (2026-11-13 12:55, agent Sauron-1113b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, visual 1d2598a) : `preflight.ps1 resources_compare -Root <worktree>` PASS (64 s, 10 PNG, sanity PASS ; WARN sans hol.HUD.Report).
PNG OUVERTS et vus :
- res_A : pin de cones, dome vert a baie rouge, cubes gris inclines, cubes jaune et brun ; etiquette RESSOURCES A
- res_B : chene rond, baies bleues, rochers tachetes (blanc / jaune)
- res_C : peuplier mince, cone bas aux baies rouges, cristaux sur dalle grise et dalle brune
- res_D : parasol large, bloc de haie, couches empilees
- res_E : grand cone, anneau de boules, tas de boules
- res_F : trois boules, dome bas, gros cube et petits cubes de minerai
- res_G : cubes empiles, dome olive a baie rose, couronnes de pointes
- res_H : arbre orange et rouge, tambour, domes piquetes
- res_I : arbre nu a branches, dome deux tons, dalles penchees
- res_J : pin enneige, pointes aux bouts rouges, losange sur socle
En plus (scenario de decouverte resources_game, non commite, preflight FAIL (2) cause non lue, 3 PNG ouverts) : dans la partie, autour du Forum de Plain, la foret est faite de peupliers (style C), les baies de domes verts et les filons de pierre et d'or de cubes inclines (style A) : le choix est bien dessine par le jeu.
NON VÉRIFIÉ : baies rouges du buisson A non distinguees a la vue large de la partie ; tailles petit / moyen / gros des filons non comparees ; la revue montre toujours les dix styles (hol.Resource.Review garde pour memoire), pas les sources de la partie ; build de main fusionne non rejoue.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 resources_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
