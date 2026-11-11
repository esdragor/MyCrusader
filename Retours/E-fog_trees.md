# [À ESSAYER] E-fog_trees : un bosquet coupé pendant que tu ne le vois pas reste dessiné dans le gris de l'exploré, puis disparaît quand tes soldats le revoient

STATUT : VU À L'ÉCRAN (les trois PNG ouverts et vus par l'agent sur le binaire de main 39ba5ec, build PASS 461 s, 2026-11-11)
GESTE: interne   (le scénario utilise `hol.Select.Grid` puis `hol.Select.LeftClickGround` : clic scripté, pas la souris du système)
SON: sans objet   (lancé sans son ; l'essai ne porte pas sur un son)
branches: aucune   (visual contient main 39ba5ec ; le correctif du scénario 115dd98 est dans main)

Pré-vol (2026-11-11 15:54, agent Sauron-R3, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, visual = main 39ba5ec + 5 fichiers de scénario) : `preflight.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (16 s d'attente, 3 PNG, aucun WARN ; audit Sanity passé, aucun panneau n'en couvre un autre).
Correction de cadre (écart de la fiche précédente, « bosquet à la limite de la vue ») : les soldats vont maintenant à -6800 (10 m du bosquet) au lieu de -7800 (20 m, la limite de leur vue), en a et en c.
PNG OUVERTS et vus (dossier C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- fogtrees_a.png : 4 Hommes d'armes au centre ; le bosquet de 10 arbres (cônes verts) juste au nord d'eux, entouré de sol clair à l'ouest, à l'est et au sud ; un coin noir derrière (au nord) le bosquet, le reste de la zone claire forme deux lobes de chaque côté. Le bosquet est donc bien dans la zone claire.
- fogtrees_b.png : les soldats ne sont plus dans l'image ; le sol est dans le gris de l'exploré (plus sombre qu'en a) et les 10 arbres sont toujours dessinés, assombris, au même endroit.
- fogtrees_c.png : les 4 Hommes d'armes sont revenus ; plus aucun arbre à l'endroit du bosquet (sol nu).
Constat (pas jugé) : en b le contraste entre sol gris et noir est faible ; le panneau du scénario affiche « en préparation » en tête des trois captures (même chose dans les deux autres essais : le scénario est en cours de déroulement à cet instant).
NON VÉRIFIÉ : le crépitement (clignotement) évoqué dans les essais du 2026-10-31 n'est pas cherché (captures statiques) ; la raison du coin noir derrière le bosquet n'est pas établie (ligne de vue ou forme de la zone) ; le goût du gris n'est pas jugé ; la liste « Brouillard » de l'écran Nouvelle partie (note du scénario) n'est pas rejouée.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Hidden`
Durée : environ 1 min (25 s de jeu). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.