branche integ-25 : DANS main depuis integ-62 (3b6d29c) ; pré-vol rejoué sur main 2026-11-24 (HALDIR-1124i)

# [À ESSAYER] E-specialization_slot : à côté de la jauge de faction, le bouton « Specialisation » s'allume au palier 1, un vrai clic ouvre les voies, un second lance le déblocage (barre + « N s »), puis le bouton disparaît

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: main

Pré-vol HALDIR-1124i (2026-11-24 10:53-10:54, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir = main 173d31b + integ-58, via preflight.ps1) : preflight.ps1 specialization_slot PASS (check PASS, binaire à jour, aucun warning ; scénario à vrais clics hol.Mouse.Click SpecButton puis SpecOption0). PNG OUVERTS et vus (1066 x 600) : before (panneau de faction bas gauche : « 100 / 100 », « Lumière sacrée : coût 25, prêt », bouton gris « Specialisation » ~143 x 20 px, texte entier ; aucun panneau de voies ni barre), during (sous le bouton : barre bleue fine, « 28 s », puis « Voie A du palier 1 (annuler) » avec bordure dorée et « Voie B du palier 1 » ; texte entier), after (le bouton, la barre et les voies ont disparu ; la voie choisie « Voie A du palier 1 » reste affichée sous la ligne « Lumière sacrée » : conforme au .look, correctif bebce63 présent dans main). Remarque : la ligne « Lumière sacrée : coût 25, prêt » porte une bordure dorée dans after (non décrite dans le .look). GESTE: réel et SON: non entendu inchangés ; libellés provisoires (Retours\Q-sp3-libelles.md). Commande complète (main, copie à jour) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 specialization_slot -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir

Pré-vol (2026-11-23 16:25, agent ELROND-1123ph, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main non fusionné) : `preflight.ps1 specialization_slot` PASS (60 s, 3 PNG), commit ad6aafd.
PNG OUVERTS et vus (image 1066 x 600) :
- specialization_slot_before : panneau de faction en bas à gauche : « 100 / 100 », « Lumière sacrée : coût 25, prêt », puis bouton gris « Specialisation » (environ 143 x 20 px, texte entier lisible) ; pas de panneau de voies, pas de barre.
- specialization_slot_during : sous le bouton, barre bleue très fine (environ 140 x 3 px) et « 28 s », puis « Voie A du palier 1 (annuler) » (bordure dorée) et « Voie B du palier 1 » ; texte entier.
- specialization_slot_after : il ne reste que « 100 / 100 » et « Lumière sacrée : coût 25, prêt » ; le bouton, la barre et les voies ont disparu ; aucune voie choisie n'est montrée.
NON VÉRIFIÉ : le son ; le clic sur la Voie B ; l'annulation par second clic ; la faction Templiers (3 voies) ; l'affichage de la voie choisie (absent du jeu) ; serveur dédié / client distant ; siège Nomade (Forum non achevé).

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 specialization_slot -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
- ELROND-1123pi (16:38, PNG after relu) : le PNG after montre maintenant, sous « Lumiere sacree : cout 25, pret », la ligne « Voie A du palier 1 » (texte provisoire, Retours\Q-sp3-libelles.md) ; la barre de progression du PNG during est a 12 de haut (6 avant) ; la note 3/3 du panneau de scenario a ete corrigee APRES ce preflight (txt seul, non rejoue).

Recette HALDIR-1124m (2026-11-24, dans main 25e29ab, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, build PASS) : PASS culture ja (bouton de specialisation, barre et voies visibles, voie A affichee apres choix)
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 specialization_slot -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Reponse: OK (2026-11-25 18:3x, DOCS-1125d) : essai joué par l'utilisateur ; reporté dans SpecializationInProgress.md, fiche archivée.
