# [À ESSAYER] E-unit_stats_attack_enemy : clique des unités ENNEMIES (Bélier, Mangonneau, Lancier de l'IA) : leur bloc de stats porte aussi la ligne Attaque avec sa valeur (bug de l'essai siege_wave, 2026-11-08 12:02) ; un Lancier ennemi qui combat un des tiens montre la ligne « Attaque » d'activité.

STATUT : VU À L'ÉCRAN (objet de l'essai = l'AFFICHAGE du bloc de stats ennemi, pas un geste)
GESTE: interne      (`hol.Select.ClickUnit -1/N enemy` : appel interne, pas un vrai clic ; à l'utilisateur de cliquer lui-même l'unité rouge)
SON: sans objet     (aucun son attendu)
branches: aucune    (seul main compte : D750 est dans main 60013e8)

Règle (L7-6) : le geste de clic n'est pas jugé ici (interne), seul le contenu du bloc l'est.

Pré-vol (2026-11-09 10:51, agent Elrond-integration, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 60013e8, rejoué sur le build de main fusionné) : `preflight.ps1 unit_stats_attack_enemy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (18 s, 4 PNG, aucun panneau de HUD ne se couvre). Seul WARN : pas d'audit `Sanity:`.
PNG OUVERTS et vus :
- use_ram : Bélier « IA Normal 2 » (rouge) 400/400, « Attaque bâtiments 60 · Armure mêlée 0 · Armure distance 60 · Armure héros 0 · Vitesse 2,2 m/s »
- use_mangonel : Mangonneau « IA Normal 2 » 200/200, « Attaque 45 · ... · Vitesse 1,8 m/s »
- use_spearman : Lancier « IA Normal 2 » 80/80, « Attaque 8 · armures 0 · Vitesse 7,8 m/s »
- use_fight : Lancier ennemi au contact d'un Lancier bleu, même bloc de stats puis la ligne d'activité « Attaque » en dessous (les PV restent à 80/80 sur cette capture, le contact vient de commencer)
NON VÉRIFIÉ : jamais joué avec une vraie fenêtre ni un vrai clic ; `ClickUnit -1 enemy` dépend de l'IA figée (ThinkSteps 100000) ; aucune partie naturelle d'IA (l'essai ne juge que l'affichage) ; panneau du scénario « en préparation… » sur les PNG alors que le journal dit « ready ».

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 unit_stats_attack_enemy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
