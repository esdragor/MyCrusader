# [À ESSAYER] E-monk_seal : le Moine selectionne, la touche du Sceau puis un clic sur le sol a 8 m pose une zone de lumiere (anneau bleu pale provisoire, rayon 6 m), le Sceau passe en recharge 40 s

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: main
RECETTE 2026-11-24 22:06 sur main e503646 : PASS (preflight PASS ; 2 PNG vus : case R Sceau non vide, panneau cout 25 ressource insuffisante, puis double anneau bleu + recharge 40 s ; reserve : l'infobulle Sceau de lumiere du .look n'est pas visible sur le PNG, survol non capture)

Pré-vol (2026-11-24 20:02, agent ELROND-1124sc, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, branche monk-seal-1124s ce2c79e) : `preflight.ps1 monk_seal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -AllowUncommitted` PASS (32 s, 2 PNG). Journal : `Grid: monk seal, click on the ground`, `Scripted left click at (0.500, 0.299)`, `Zone effect: slot 0 puts a Seal of Light`, `RpcUnitAbility: Seal of Light put down by the Monk 13 of slot 0`.
PNG OUVERTS et vus :
- monk_seal_before : le Moine bleu seul au centre, selectionne (fiche « Moine », 80 PV) ; le panneau « Lumière sacrée : coût 25, ressource insuffisante » (barre 0 / 100) est affiche. MIS A JOUR (ELROND-1124sg, 2026-11-24 ~20:45, build e5d68be PASS, preflight PASS, PNG rouverts) : la case R (derniere de la rangee A Z E R) n'est PLUS vide : libelle « Sceau » visible, infobulle « Sceau de lumiere » (rayon 6 m, 10 s, ralentis 40 %, recharge 40 s) ; pas d'icone (provisoire texte seul). Ancienne mention « grille VIDE » (ELROND-1124sc) perimee.
- monk_seal_zone : apres le geste, un double anneau bleu pale (rayon 6 m) est pose sur le sol a 8 m devant le Moine ; le panneau dit « coût 25, recharge 40 s ». Le provisoire est BLEU, pas dore.
NON VÉRIFIÉ : la cellule du Sceau n'est pas visible dans la grille (touche testee par `hol.Select.Grid 3`, pas par un vrai appui clavier sur l'ecran) ; le ralentissement des ennemis dans la zone (aucun ennemi dans le scenario) ; la fin du Sceau a 10 s ; aucun son (-nosound) ; capture hors ecran (-RenderOffScreen), pas de fenetre jouee par un humain.

Commande complète :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 monk_seal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Recette RECETTE-1125mb sur main 7922539 : PASS avec reserve (preflight PASS 25 s, 2 PNG ouverts : case R Sceau non vide, panneau cout 25 ressource insuffisante, puis double anneau bleu pale rayon 6 m a 8 m du Moine et recharge 40 s, journal Grid: armed mode 102 (LightSeal) + Zone effect Seal of Light ; reserve : le .look promet l infobulle Sceau de lumiere (rayon 6 m, 10 s, 40 %) or elle n est PAS sur le PNG (survol non capture) - .look a corriger ou survol a capturer ; texte Shaders en preparation visible sur before ; WARN preflight sans HUD overlap ni Sanity)
Reponse: BUG: le Sceau marche mais la recharge ne s affiche pas (2026-11-25 14:0x, joue par l utilisateur sur main 7922539, dans la conversation ; correctif en cours : HALDIR-1125me)
Recette RECETTE-1125me sur Haldir a6cb051 (branche monk-seal-cooldown-1125m, preflight PASS 32 s, 3 PNG ouverts) : PASS. monk_seal_before : case R Sceau claire (active, sans decompte) ; monk_seal_zone : double anneau bleu, case R deja grisee avec Sceau 40 s ; monk_seal_cooldown : anneau bleu, case R grisee (texte attenue, fond sombre) avec Sceau 38 s (lu 30 s a la loupe, 38 coherent avec le panneau recharge 38 s), le decompte descend 40 -> 38. Reserve : geste interne pour le scenario, aucun son, case non revue apres les 40 s (retour a l etat actif non capture).
