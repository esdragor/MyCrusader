# [À ESSAYER] E-unit_stats_attack_enemy : sélectionne par un vrai clic un Bélier / Mangonneau / Lancier ENNEMI et lis la ligne d'attaque de leur bloc de stats

STATUT : VU À L'ÉCRAN (4 PNG sur 4 ouverts et vus, rejoué par l'intégrateur Elrond sur le build de main)
GESTE: réel
SON: sans objet (aucun son dans cet essai)
branches: aucune

Ce qu'on voit : un Bélier ENNEMI (carré rouge « IA Normal 2 ») affiche « Attaque bâtiments 60 » ; Mangonneau ennemi 45 ; Lancier ennemi 8 ; au combat, le mot « Attaque » SEUL sur la dernière ligne du Lancier est la ligne d'ACTIVITÉ (« il frappe en ce moment »), pas une stat.

Pré-vol (2026-11-09 09:18, agent Elrond-19 intégrateur, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 02ac688, binaire du 08-11 12:33 >= dernier code du 12:27) : `preflight.ps1 unit_stats_attack_enemy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (32 s, 4 PNG).
PNG OUVERTS et vus :
- use_ram : Bélier ennemi, « IA Normal 2 », « Points de vie 400 / 400 · Attaque bâtiments 60 · Armure mêlée 0 · Armure distance 60 · Armure héros 0 · Vitesse 2,2 m/s ».
- use_mangonel : Mangonneau ennemi, « Attaque 45 · Armure distance 60 · Vitesse 1,8 m/s ».
- use_spearman : Lancier ennemi, « Attaque 8 · … · Vitesse 7,8 m/s ».
- use_fight : Lancier ennemi au contact d'un des tiens (bleu), mêmes stats puis « Attaque » seul en dernière ligne (activité).
NON VÉRIFIÉ : overlay « Shaders en préparation » sur la première capture (bruit de démarrage) ; le Bélier ennemi qui frappe un bâtiment n'est pas dans ce scénario ; geste = `hol.Select.ClickUnit -1 enemy` (clic réel simulé par le scénario), pas ma propre souris ; pas d'appel à `hol.Test.Sanity` (WARN du pré-vol) ; un bloc de lignes « LogAutomationTest: Error: Condition failed » apparaît AVANT le démarrage du scénario dans la sortie du pré-vol (hors scénario, le pré-vol conclut « no warning or error in the log » : non investigué).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 unit_stats_attack_enemy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
