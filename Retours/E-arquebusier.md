# [À ESSAYER] E-arquebusier : un Arquebusier tire sur un Homme d'armes ; tu vois ses PV baisser (155 -> 117 au premier coup : la pénétration d'armure de 50 % joue, 119 sans)

STATUT : VU À L'ÉCRAN
GESTE: réel      (clic sur l'Homme d'armes, clic sur l'Arquebusier, clic droit sur l'Homme d'armes, par hol.Select.ClickUnit, la même voie que unit_stats_attack_enemy ; le Champ de tir et la case E ne sont pas dans ce scénario)
SON: non entendu   (lancé avec -nosound ; l'Arquebusier n'a pas de son propre, G15)
branches: tiers

Pré-vol (2026-11-09 17:00, agent Elrond-21, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main = base de tiers 4bd9dd2) : `preflight.ps1 arquebusier -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (28 s, 2 PNG).
PNG OUVERTS et vus :
- arq_before : l'Homme d'armes ennemi (rouge) sélectionné, bloc « Points de vie 155 / 155 », Arquebusier (bleu, mannequin de l'Arbalétrier) 14 m en dessous.
- arq_hit : le même Homme d'armes après le premier tir, bloc « Points de vie 117 / 155 », ligne « Attaque », barre de vie rouge plus courte au-dessus de lui ; le log donne `ManAtArms slot 1 hp 117/155` (waitlog du scénario).
NON VÉRIFIÉ : le chiffre « 119 sans pénétration » vient du test Combat.ArmorPenetration, pas du scénario (aucun réglage de jeu ne coupe la pénétration) ; un seul coup est contrôlé (les suivants 79, 41... ne le sont pas) ; l'Homme d'armes court sur l'Arquebusier et le corps à corps suit (non regardé) ; aucun son, aucune fumée de poudre (G15), corps provisoire (Placeholders.txt) ; le Champ de tir produit bien l'Arquebusier à la case E (tests, pas ce scénario).

Rejoué le 2026-11-10 10:12 (agent Elrond-22, binaire recompilé 10:11, main 644b196 fusionné) : `preflight.ps1 arquebusier` PASS (31 s), arq_before et arq_hit rouverts et vus (155/155 puis 117/155 ; au 2e PNG l'Arquebusier est déjà au contact de l'Homme d'armes). Nouvelle note 5 du scénario : explique le duel si on le laisse jouer (155 -> 117 -> 79 -> 41 en 3 tirs, l'Homme d'armes court à 7 m/s et frappe l'Arquebusier de 70 PV avant le 4e tir : équilibre de l'unité, pas un bug ; les chiffres restent à l'utilisateur, Q-arquebusier-valeurs). Aucune valeur de réglage modifiée.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 arquebusier -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
