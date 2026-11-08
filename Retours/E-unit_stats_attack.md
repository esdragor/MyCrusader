# [À ESSAYER] E-unit_stats_attack : sélectionne par un vrai clic un Bélier / Mangonneau / Lancier ENNEMI (et les tiens) et lis la ligne d'attaque de leur bloc de stats

STATUT : VU À L'ÉCRAN (4 PNG ouverts sur 10 : use_ram, use_fight, usa_ram, usa_monk ; les chiffres des 6 autres sont dans le journal « HUD stats: »)
GESTE: réel
SON: sans objet
branches: galadriel-tab

Ce qu'on voit : un Bélier (ennemi ou à toi) affiche « Attaque bâtiments 60 » (il n'a pas d'attaque ordinaire : avant, AUCUNE ligne d'attaque) ; Mangonneau 45, Lancier 8, Homme d'armes 12, Archer 5 ; le Moine n'a pas de ligne d'attaque. Le mot « Attaque » SEUL sur la dernière ligne (use_fight, Lancier au combat) est la ligne d'ACTIVITÉ (« il frappe en ce moment »), pas une stat : voir Q-unit-stats-building (option « Autre »).

Pré-vol (2026-11-08 12:11, agent Galadriel-12, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel) : `preflight.ps1 unit_stats_attack_enemy -Root ...` PASS (28 s, 4 PNG) ; `unit_stats_attack` PASS (28 s, 6 PNG). Test rouge b357795 : timeout « HUD stats: building attack 60, » avec le build d'avant.
PNG OUVERTS et vus :
- use_ram : Bélier ENNEMI (carré rouge « IA Normal 2 »), bloc « Attaque bâtiments 60 · Armure mêlée 0 · Armure distance 60 · Armure héros 0 · Vitesse 2,2 m/s ».
- use_fight : Lancier ENNEMI au combat : « Attaque 8 · ... · Vitesse 7,8 m/s » puis, seul sur sa ligne, « Attaque » (activité).
- usa_ram : Bélier à toi, même ligne « Attaque bâtiments 60 ».
- usa_monk : Moine, pas de ligne d'attaque.
NON VÉRIFIÉ : usa_mangonel, usa_spearman, usa_manatarms, archer, use_mangonel, use_spearman (PNG non ouverts, chiffres 45 / 8 / 12 / 5 lus dans le journal) ; le Bélier ennemi en train de frapper un bâtiment (activité + ligne côte à côte) ; clic sur une unité ennemie dans l'essai siege_wave lui-même (le scénario neuf utilise le même clic réel `hol.Select.ClickUnit -1 enemy`).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 unit_stats_attack_enemy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
