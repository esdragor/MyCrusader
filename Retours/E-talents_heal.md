# [À ESSAYER] E-talents_heal : prendre des talents avec le Paladin-Commandant et voir la jauge de faction (SacredLight) rester AU-DESSUS du bloc héros, sans le recouvrir, puis le trait de soin du Moine doré (« ++ ») après le talent « Mains du guérisseur »

STATUT : LOGIQUE SEULEMENT   (captures vues ; le trait « doré » du talent n'est pas net sur talents_heal_boosted : le trait y paraît vert)
recette: RECETTE-1121sb FAIL sur main a7833ea (Sauron) : jauge SacredLight au-dessus du bloc heros OK (base, cell, boosted), aucun ++ sur l'Arbre de talents OK, MAIS talents_heal_boosted : trait de soin VERT (pixels or 255,215,60 = 0, verts 414 contre 431 en base), pas de ++ : le talent Mains du guerisseur (pris, 2 talents) ne dore pas le trait. Statut reste LOGIQUE SEULEMENT (geste interne).
GESTE: interne
SON: non entendu
branches: zone-effect

Pré-vol (2026-11-21 19:36, agent FARAMIR-1121se, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, tip 64a212a) : `preflight.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (40 s, 3 PNG, plus de FAIL « HUD overlap HeroPanel x FactionPanel »).
PNG OUVERTS et vus :
- talents_heal_base : bloc héros avec 1 talent pris (rangée d'icônes + « Talents pris (1) »), la jauge « 0 / 100 SacredLight » est au-dessus du bloc, sans recouvrement ; trait de soin vert + « + » du Moine vers un Homme d'armes visible.
- talents_heal_cell : fenêtre « Arbre de talents », cases Boucliers levés et Mains du guérisseur dorées (prises), bloc héros plus haut (2 talents pris), la jauge remonte avec lui et ne le recouvre pas.
- talents_heal_boosted : mêmes blessés, 2 talents pris, jauge au-dessus du bloc ; le trait de soin du Moine est vert sur la capture (doré attendu : à regarder en jeu, hors de ce correctif).
NON VÉRIFIÉ : le doré du trait après le talent ; le suivi de la hauteur au tick exact où la rangée change (relue au rafraîchissement suivant) ; jeu au clavier/souris réel ; son.

Commande complète (règle 13, 2026-11-14) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Correctif FARAMIR-1121sg (2026-11-21 20:21) : cause (c) scenario, pas le code : le Moine etait a ~1741 du heros (-17535 300), hors aura (rayon 1200), donc bBoosted=false (HOLHealProcessor.cpp:314) et trait vert. Scenario deplace en -17000 450 (~550 du heros) ; preflight PASS, talents_heal_boosted.png ouvert : '++' dore visible (76 px or, 0 vert), base : vert. branche zone-effect, commit du scenario seulement. Statut reste LOGIQUE SEULEMENT (GESTE interne).
recette: RECETTE-1121sd PASS sur main a4f3531 (Sauron) : preflight PASS, 3 PNG ouverts : jauge SacredLight au-dessus du bloc heros (base, cell, boosted), aucun ++ sur l Arbre de talents, boosted : ++ dore visible a cote du trait de soin (Mains du guerisseur pris) ; base : + vert. Statut reste LOGIQUE SEULEMENT (GESTE interne).
Commande play.ps1 (RECETTE-1121sd) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
NETTOYAGE-1123g 2026-11-23 : la mention branches zone-effect est perimee, les commits du talent (64a212a, 4e58051, 2a11ec2, 4146ec1) et 0e328e9 sont dans main ; la commande Sauron est jouable (Sauron contient main a ce niveau) ; manque pour devenir pret : le trait +/++ sort du DrawDebugString (D886, ligne [ ] de l Auto_Journey_File.md l. 118, C++), un geste reel (ouvrir l arbre, cliquer le talent), le son.
