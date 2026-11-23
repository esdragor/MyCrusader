branche integ-25, pas encore dans main

# [À ESSAYER] E-specialization_slot : à côté de la jauge de faction, le bouton « Specialisation » s'allume au palier 1, un vrai clic ouvre les voies, un second lance le déblocage (barre + « N s »), puis le bouton disparaît

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: integ-25

Pré-vol (2026-11-23 16:25, agent ELROND-1123ph, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main non fusionné) : `preflight.ps1 specialization_slot` PASS (60 s, 3 PNG), commit ad6aafd.
PNG OUVERTS et vus (image 1066 x 600) :
- specialization_slot_before : panneau de faction en bas à gauche : « 100 / 100 », « Lumière sacrée : coût 25, prêt », puis bouton gris « Specialisation » (environ 143 x 20 px, texte entier lisible) ; pas de panneau de voies, pas de barre.
- specialization_slot_during : sous le bouton, barre bleue très fine (environ 140 x 3 px) et « 28 s », puis « Voie A du palier 1 (annuler) » (bordure dorée) et « Voie B du palier 1 » ; texte entier.
- specialization_slot_after : il ne reste que « 100 / 100 » et « Lumière sacrée : coût 25, prêt » ; le bouton, la barre et les voies ont disparu ; aucune voie choisie n'est montrée.
NON VÉRIFIÉ : le son ; le clic sur la Voie B ; l'annulation par second clic ; la faction Templiers (3 voies) ; l'affichage de la voie choisie (absent du jeu) ; serveur dédié / client distant ; siège Nomade (Forum non achevé).

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 specialization_slot -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
- ELROND-1123pi (16:38, PNG after relu) : le PNG after montre maintenant, sous « Lumiere sacree : cout 25, pret », la ligne « Voie A du palier 1 » (texte provisoire, Retours\Q-sp3-libelles.md) ; la barre de progression du PNG during est a 12 de haut (6 avant) ; la note 3/3 du panneau de scenario a ete corrigee APRES ce preflight (txt seul, non rejoue).
