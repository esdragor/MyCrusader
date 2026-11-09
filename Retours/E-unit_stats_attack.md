# [À ESSAYER] E-unit_stats_attack : sélectionne par un vrai clic un Bélier / Mangonneau / Lancier / Homme d'armes / Archer / Moine à toi et lis la ligne d'attaque de leur bloc de stats

STATUT : VU À L'ÉCRAN (6 PNG sur 6 ouverts et vus, rejoué par l'intégrateur Elrond sur le build de main)
GESTE: réel
SON: sans objet (aucun son dans cet essai)
branches: aucune

Ce qu'on voit : un Bélier à toi affiche « Attaque bâtiments 60 » (il n'a pas d'attaque ordinaire : avant, AUCUNE ligne d'attaque) ; Mangonneau 45, Lancier 8, Homme d'armes 12, Archer 5 ; le Moine n'a pas de ligne d'attaque. Décision D750 (GDD) : ligne « Attaque bâtiments » pour une unité sans attaque ordinaire.

Pré-vol (2026-11-09 09:19, agent Elrond-19 intégrateur, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 02ac688, binaire du 08-11 12:33 >= dernier code du 12:27) : `preflight.ps1 unit_stats_attack -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (37 s, 6 PNG). Le correctif (7e16c6d) est DANS main : `branches: aucune` (galadriel-tab a des commits en plus, sans rapport avec cet essai : le gate refuse sinon).
PNG OUVERTS et vus :
- usa_ram : Bélier « Vous » (bleu), « Attaque bâtiments 60 · Armure mêlée 0 · Armure distance 60 · Armure héros 0 · Vitesse 2,2 m/s », grille Q Attaque / S Arrêt / V Tenir.
- usa_mangonel : Mangonneau, « Attaque 45 · Armure distance 60 · Vitesse 1,8 m/s ».
- usa_spearman : Lancier, « Attaque 8 · … · Vitesse 7,8 m/s ».
- usa_manatarms : Homme d'armes 155 PV, « Attaque 12 · Armure mêlée 4 · Armure distance 4 · Armure héros 2 · Vitesse 7 m/s ».
- usa_archer : Archer 70 PV, « Attaque 5 · … · Vitesse 7,8 m/s ».
- usa_monk : Moine 80 PV, aucune ligne d'attaque.
NON VÉRIFIÉ : l'overlay « Shaders en préparation » est visible sur la première capture (bruit de démarrage, pas le sujet) ; le Bélier en train de frapper un bâtiment (ligne d'activité côte à côte avec la stat) n'est pas dans ce scénario ; le geste est `hol.Select.ClickUnit` (clic réel simulé par le scénario, comme `neutrals_qground`), pas ma propre souris ; le scénario n'appelle pas `hol.Test.Sanity` (WARN du pré-vol).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 unit_stats_attack -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
