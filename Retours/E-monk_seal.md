# [À ESSAYER] E-monk_seal : le Moine selectionne, la touche du Sceau puis un clic sur le sol a 8 m pose une zone de lumiere (anneau bleu pale provisoire, rayon 6 m), le Sceau passe en recharge 40 s

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: monk-seal-1124s

Pré-vol (2026-11-24 20:02, agent ELROND-1124sc, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, branche monk-seal-1124s ce2c79e) : `preflight.ps1 monk_seal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -AllowUncommitted` PASS (32 s, 2 PNG). Journal : `Grid: monk seal, click on the ground`, `Scripted left click at (0.500, 0.299)`, `Zone effect: slot 0 puts a Seal of Light`, `RpcUnitAbility: Seal of Light put down by the Monk 13 of slot 0`.
PNG OUVERTS et vus :
- monk_seal_before : le Moine bleu seul au centre, selectionne (fiche « Moine », 80 PV) ; le panneau « Lumière sacrée : coût 25, ressource insuffisante » (barre 0 / 100) est affiche. La grille de commandes est VIDE a l'ecran (aucune cellule Sceau visible : icone / libelle absents).
- monk_seal_zone : apres le geste, un double anneau bleu pale (rayon 6 m) est pose sur le sol a 8 m devant le Moine ; le panneau dit « coût 25, recharge 40 s ». Le provisoire est BLEU, pas dore.
NON VÉRIFIÉ : la cellule du Sceau n'est pas visible dans la grille (touche testee par `hol.Select.Grid 3`, pas par un vrai appui clavier sur l'ecran) ; le ralentissement des ennemis dans la zone (aucun ennemi dans le scenario) ; la fin du Sceau a 10 s ; aucun son (-nosound) ; capture hors ecran (-RenderOffScreen), pas de fenetre jouee par un humain.

Commande complète :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 monk_seal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
