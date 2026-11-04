# [À ESSAYER] E-diplomacy_locked : taxe écrite une fois, tribut de toutes les ressources

Intégration (2026-11-04 17:3x, agent de fusion) : sur le build de la fusion groupée (worktree Elrond), preflight diplomacy_locked PASS (21 s, 3 PNG). VU À L'ÉCRAN : dipl_locked_food (bouton « Ressource : nourriture », boutons « Tribut 100 nourriture » sur chaque ligne, taxe de 30 % écrite une seule fois en bas du panneau). NON OUVERTS : dipl_locked_panel et dipl_locked_wide. Statut : VU SUR MAIN FUSIONNÉ (panneau, une fois la taxe et le bouton Ressource) ; boutons Tribut grisés (pas de Marché) ; main = pas encore avancée.

Statut : pré-vol PASS (preflight.ps1 diplomacy_locked, 24 s, audit de sanité passé) ; PNG ouverts et vus : dipl_locked_panel, dipl_locked_wide, dipl_locked_food.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 diplomacy_locked -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`

Ce qui a changé : (1) « Tribut : taxe de 30 % gardée par le Marché » est écrit UNE fois en bas du panneau, plus sur les boutons ; (2) un bouton « Ressource : or » en bas du panneau fait tourner or, nourriture, bois, pierre ; tous les boutons « Tribut 100 <ressource> » suivent, mêmes règles de taxe (la commande serveur prenait déjà les 4 ressources). Le scénario clique une fois sur « Ressource » (vu : « Ressource : nourriture », « Tribut 100 nourriture »).
Limite : le scénario n'a pas de Marché (boutons Tribut grisés) ; le don réel d'une ressource autre que l'or n'est pas vu à l'écran (le don d'or est vérifié au test réseau, la logique serveur est inchangée pour les 4).