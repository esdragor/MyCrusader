# [À ESSAYER] E-sacred_light_heal : la Lumière sacrée soigne des blessés ; le journal le prouve (Heal report 0.00 puis 765.00, PV du héros 180 puis 330 / 450), geste réel

STATUT : VU A L'ECRAN   (ARWEN-1124sc : vrai clic case Slate puis VRAI clic gauche au sol hol.Select.LeftClickGround -17400 100 ; total soigne 765.00, PV du heros 180 -> 330 / 450 sur 2 PNG ouverts et vus ; son non entendu)
GESTE: réel      (vrai clic case hol.Mouse.Click FactionBox0, puis vrai clic gauche sol par les gestionnaires d'entree hol.Select.LeftClickGround -17400 100, modele FARAMIR-1124sa tower_wood)
SON: non entendu   (-nosound)
branches: main
RECETTE 2026-11-24 22:06 sur main e503646 : PASS (preflight PASS ; 2 PNG vus : heros PV 180/450, jauge 100/100 prete, puis PV 330/450, jauge 90/100, recharge 14 s ; journal total soigne 765.00 ; reserve : barres de vie par soldat illisibles a cette taille)

Pré-vol 2026-11-24 11:17 (ARWEN-1124d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen) : `preflight.ps1 sacred_light_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (2 passages, 0.00 -> 705.00 identique, 2 PNG). waitlog sur les lignes exactes `Heal report: links 0, healed total 0.00` puis `... healed total 705.00`.
PNG OUVERTS et vus (1er passage, mêmes valeurs au 2e) :
- sacred_light_heal_before : héros PV 180 / 450, 5 Hommes d'armes barres entamées, jauge 100 / 100, case Lumière sacrée prête.
- sacred_light_heal_after : héros PV 330 / 450, jauge 89 / 100, recharge 14 s ; la zone de 5 s est déjà finie (6 s d'attente), pas de cercle doré.
NON VÉRIFIÉ : les PV par soldat (hol.Units.Report ne les imprime pas : seul le total soigné et le héros sont lus) ; rejeu sur un serveur dédié.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 sacred_light_heal -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`   (ARWEN-1124sc : preflight PASS 41 s, 2 PNG vus : total 0.00 -> 765.00, PV heros 180 -> 330 / 450, Honneur 100 -> 90 ; le premier essai au clic -16400 0 donnait 0.00 : zone de 600 cm trop loin des blesses)
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Recette HALDIR-1124m (2026-11-24, dans main 25e29ab, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, build PASS) : PASS (PV heros 330/450 sur after, Honneur 89/100, recharge 14 s ; zone doree finie)
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 sacred_light_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
