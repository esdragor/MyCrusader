# [À ESSAYER après correction du build] E-fog_trees : un bosquet coupé pendant que tu ne le vois pas reste dessiné dans le gris de l'exploré, puis disparaît quand tes soldats le revoient

STATUT : VU À L'ÉCRAN (les trois PNG ouverts et vus par l'agent ; binaire Editor du 2026-11-11 14:26, voir NON VÉRIFIÉ)
GESTE: interne   (le scénario utilise `hol.Select.Grid` puis `hol.Select.LeftClickGround` : clic scripté, pas la souris du système)
SON: sans objet   (lancé sans son ; l'essai ne porte pas sur un son)
branches: aucune   (visual = main 521adcb ; le correctif du scénario 115dd98 est déjà dans main)

Pré-vol (2026-11-11 15:18, agent Sauron-F, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main 521adcb) : `preflight.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (53 s de vérification, 25,7 s de jeu, 3 PNG, aucun WARN ; audit Sanity passé, aucun panneau ne en couvre un autre).
PNG OUVERTS et vus (dossier C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- fogtrees_a.png : 4 Hommes d'armes en bas, au centre de la zone claire ; le bosquet de 10 arbres (cônes verts) au nord, à la limite de la zone claire, entouré de noir sur les côtés et au-dessus (le noir entoure le bosquet : il est à l'extrémité de la vue, pas au milieu).
- fogtrees_b.png : les soldats ne sont plus dans l'image ; le sol est dans le gris de l'exploré (plus sombre qu'en a) et les 10 arbres sont toujours dessinés, assombris, au même endroit.
- fogtrees_c.png : les 4 Hommes d'armes sont revenus en bas, zone claire ; plus aucun arbre à l'endroit du bosquet (sol nu).
Écart à signaler (pas jugé) : en a, la caméra montre le bosquet à la limite de la vue, pas « en pleine vue » comme le dit la ligne du fichier `.look` ; en b la zone grise se voit mal en dehors de l'avant-plan (le sol près des arbres est déjà presque noir). Le panneau du scénario affiche encore « en préparation » en tête des trois captures.
NON VÉRIFIÉ : binaire PÉRIMÉ : le build de main 521adcb échoue (HOLCombatRules.h(403) C4459 « Margin », clash unity avec HOLSourceNavTests.cpp) ; les captures viennent du binaire du 14:26, construit avant les fusions de 14:33 à 15:01 (Guerrier maudit, déploiement : sans lien avec le brouillard). À rejouer après correction du build. Le crépitement (clignotement) évoqué dans les essais du 2026-10-31 n'a pas été cherché (captures statiques). Le goût du gris n'est pas jugé. Le son n'est pas concerné.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Hidden`
Durée : environ 1 min (25 s de jeu). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
