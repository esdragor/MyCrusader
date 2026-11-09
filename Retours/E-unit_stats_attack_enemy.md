# [À ESSAYER] E-unit_stats_attack_enemy : clique des unités ENNEMIES (Bélier, Mangonneau, Lancier de l'IA) : leur bloc de stats porte aussi la ligne Attaque avec sa valeur (bug de l'essai siege_wave, 2026-11-08 12:02) ; un Lancier ennemi qui combat un des tiens montre la ligne « Attaque » d'activité.

STATUT : VU À L'ÉCRAN
GESTE: réel      (clic gauche sur le corps dessiné de chaque unité ennemie, hol.Select.ClickUnit -1/N enemy ; les unités sont posées par cheat et l'IA est figée, aucune touche n'est nécessaire)
SON: non entendu   (aucun son attendu ici ; lancé sans fenêtre, rien entendu)
branches: aucune      (seul main compte : binaire de Faramir = main 02ac688, D750 dedans ; les seuls commits de la branche fog sont des scénarios)

Pré-vol (2026-11-09 09:21, agent Faramir-17, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, main 02ac688) : `preflight.ps1 unit_stats_attack_enemy -Root <worktree>` PASS (46 s, 4 PNG). Seul WARN : pas d'audit `Sanity:`.
PNG OUVERTS et vus :
- use_ram : Bélier ROUGE (« IA Normal 2 »), « Attaque bâtiments 60 · Armure mêlée 0 · Armure distance 60 »
- use_mangonel : Mangonneau « IA Normal 2 », « Attaque 45 »
- use_spearman : Lancier « IA Normal 2 », « Attaque 8 », 80/80 PV
- use_fight : Lancier « IA Normal 2 » à 72/80 PV, au contact d'un des miens, « Attaque 8 » puis la ligne d'activité « Attaque » en dessous
Panneau du scénario : « en préparation… » sur les PNG (pré-vol sans fenêtre).
NON VÉRIFIÉ : jamais joué avec une vraie fenêtre ; le geste `ClickUnit -1 enemy` vise la dernière unité ennemie connue : il dépend de l'IA figée (ThinkSteps 100000) ; aucune partie naturelle d'IA ici.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 unit_stats_attack_enemy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
