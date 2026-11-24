# [À ESSAYER] E-sacred_light_heal : la Lumière sacrée soigne des blessés ; le journal le prouve (Heal report 0.00 puis 705.00, PV du héros 180 puis 330 / 450)

STATUT : LOGIQUE SEULEMENT   (les chiffres du journal et les PV du héros sont lus et vus sur 2 PNG ; c'est une PREUVE de règle, pas un réglage à l'oeil)
GESTE: réel      (vrai clic case hol.Mouse.Click FactionBox0, vraie souris hol.Mouse.Hover, vrai clic sol hol.Mouse.Click 600 380 ; curseur virtuel de Slate)
SON: non entendu   (-nosound)
branches: main

Pré-vol 2026-11-24 11:17 (ARWEN-1124d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen) : `preflight.ps1 sacred_light_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (2 passages, 0.00 -> 705.00 identique, 2 PNG). waitlog sur les lignes exactes `Heal report: links 0, healed total 0.00` puis `... healed total 705.00`.
PNG OUVERTS et vus (1er passage, mêmes valeurs au 2e) :
- sacred_light_heal_before : héros PV 180 / 450, 5 Hommes d'armes barres entamées, jauge 100 / 100, case Lumière sacrée prête.
- sacred_light_heal_after : héros PV 330 / 450, jauge 89 / 100, recharge 14 s ; la zone de 5 s est déjà finie (6 s d'attente), pas de cercle doré.
NON VÉRIFIÉ : les PV par soldat (hol.Units.Report ne les imprime pas : seul le total soigné et le héros sont lus) ; rejeu sur un serveur dédié.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 sacred_light_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Recette HALDIR-1124m (2026-11-24, dans main 25e29ab, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, build PASS) : PASS (PV heros 330/450 sur after, Honneur 89/100, recharge 14 s ; zone doree finie)
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 sacred_light_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
