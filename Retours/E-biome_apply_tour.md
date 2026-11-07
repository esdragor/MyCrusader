# [À ESSAYER] E-biome_apply_tour : voir les cinq biomes (sol, lumière) posés sur la Plaine, Entrée = biome suivant, et dire aimé / pas aimé par biome

branches: visual
STATUT : VU À L'ÉCRAN   (sol et niveau de lumière vus sur les 5 PNG ; le ciel et la brume ne se distinguent pas sur cette vue de dessus, voir NON VÉRIFIÉ)
GESTE: interne      (le biome est posé par la commande hol.Biome.Index du scénario ; seule la touche Entrée est réelle, elle fait passer au biome suivant)
SON: sans objet   (essai purement visuel)

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`. Cet essai juge un LOOK, pas un geste ni un son.

Pré-vol (2026-11-07 10:24, agent Sauron-3, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main 80e3c07) : `preflight.ps1 biome_apply_tour -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (30 s de partie, 5 PNG, aucune erreur de journal, audit Sanity OK ; WARN seul : pas de `hol.HUD.Report`, sans objet ici).
PNG OUVERTS et vus (dans C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- biome_0_fresh_grass.png : herbe vert franc, clair, la grille de la carte lisible ; arbres, camp et soldats bleus / rouges nets ; pas d'artefact.
- biome_1_dry_steppe.png : sol vert jaunâtre, plus sec et un peu plus chaud que l'herbe ; même lumière ; soldats lisibles ; proche de l'herbe fraîche mais distinct.
- biome_2_autumn.png : sol orange-brun, net ; les arbres restent verts (le décor ne change pas encore, c'est B3) ; soldats rouges un peu moins contrastés sur l'orange que sur le vert, mais lisibles.
- biome_3_dark_forest.png : sol vert très sombre, vue nettement plus sombre ; arbres et soldats lisibles ; aucun noir plat, les ombres propres des arbres ne sont pas dessinées (voulu, D258).
- biome_4_snow.png : sol blanc-gris très clair, grille grise discrète ; les soldats bleus ressortent bien, les rouges aussi ; pas de pâté de blanc brûlé ; aucune texture de neige (le détail est à 0 en attendant le choix d'une texture CC0).
Aucun sol noir, aucune brume opaque, aucun bandeau d'interface masqué sur les cinq vues.

Ce que tu dois juger (les valeurs de la table Look de `Script/Biomes/HOLBiomeRules.as` sont PROVISOIRES : tu choisis à l'œil, je règle ensuite par balayage de variantes étiquetées) :
- pour chacun des cinq biomes : aimé / pas aimé (sol et lumière) ; dis surtout lequel est trop proche d'un autre (herbe fraîche et steppe sèche sont les plus proches) ;
- la neige est-elle lisible pour jouer (la grille grise, les soldats) ?
- la forêt sombre est-elle trop sombre pour jouer ?

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 biome_apply_tour -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 2 min (5 vues, Entrée entre chacune). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

NON VÉRIFIÉ : les couleurs du soleil, du ciel et de la brume ne sont pas relues sur le monde (`hol.Biome.World` ne lit que des intensités, RESTE b, il faudrait du C++) ; à l'écran la vue de dessus ne montre ni le ciel ni la brume, seuls le sol et l'intensité de la lumière se distinguent ; l'effet est vu sur la Plaine seule ; la couleur de lumière rendue au retour sur la carte (biome `default` après un biome) est rendue par le même chemin mais non relue ; le client réseau (configuration répliquée en retard) non essayé ; le coût (S1/S2) inconnu ; le décor (arbres, touffes) ne change pas avec le biome (B3).
