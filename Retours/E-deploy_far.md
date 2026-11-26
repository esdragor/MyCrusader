# [À ESSAYER] E-deploy_far : un Mangonneau démonté, envoyé par un clic droit sur une Maison ennemie à plus de 2 x sa portée, marche, se monte tout seul à portée et tire (D877 moitié A)

STATUT : VU À L'ÉCRAN (PNG ouverts ; le coup de siege est prouvé par le journal, pas par les PV : la Maison n'est pas selectionnée, aucun projectile dessiné pour un coup de siège)
GESTE: réel (vrai clic gauche hol.Mouse.Click 533 300 pour sélectionner, vraie touche D hol.UI.Key D deux fois pour monter puis démonter, vrai clic droit hol.Select.RightClickGround sur la Maison lointaine ; aucune sélection interne, contrairement à deploy.txt étape 11)
SON: non entendu
branches: main

Pré-vol (2026-11-24 11:38, ELROND-1124e, C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, integ-25 7c790ca) : `preflight.ps1 deploy_far -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (check 56 s, 4 PNG, 0 warning, sanity OK).
Journal (Saved_Tests\Logs\scenario_deploy_far.log) : D monte (Deploy: #13 phase 0 -> 1 order 3), D démonte (phase 2 -> 3 -> 0), clic droit sur la Maison (Order: 1 units attack House of slot 1), PUIS `Deploy: #13 phase 0 -> 1 (order 1, ... distance 3173 cm, walk 2)` 16 s plus tard, `phase 1 -> 2 (delay over)`, `Mangonel hits a building for 45` (2 coups en 5 s).
PNG OUVERTS et vus :
- deploy_far_start : Mangonneau démonté (case D = Monter) à droite, Maison ennemie rouge à gauche, loin ; aucun ordre.
- deploy_far_walking : 3 s après le clic droit, Mangonneau démonté (case D = Monter) déjà plus à gauche, en marche, la Maison encore loin.
- deploy_far_arrived : Mangonneau monté (case D = Démonter, activité Attaque), ligne d'attaque rouge vers la Maison, nettement plus près d'elle.
- deploy_far_end : 5 s plus tard même position, monté, Attaque ; PV de la Maison non lisibles (non sélectionnée).
Défaut D877 moitié A : NON REPRODUIT (vert) sur integ-25 f426bd4 + scénario ; même résultat que ELROND-1122a (main 1447448).
NON VÉRIFIÉ : le texte « Scénario deploy_far en préparation » et la note 1 couvrent le haut de l'écran (note posée après le notereset) ; la Maison hors cadre de capture à l'arrivée n'est pas lisible ; `session_gate.ps1 -Phase Essay` non lancé ; son non entendu ; le point d'arrêt de la marche (3173 cm) est celui du journal, pas mesuré à l'écran.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 deploy_far -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Recette HALDIR-1124m (2026-11-24, dans main 25e29ab, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, build PASS) : PASS (Mangonneau demonte marche, case D=Monter, puis monte case D=Demonter, activite Attaque, plus pres de la Maison)
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 deploy_far -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
recette 8d79aaa : PASS 17:22 (GLORFINDEL-1125d, Haldir integ-85 = main 8d79aaa) : preflight deploy_far PASS (sanity OK, HUD sans recouvrement), 5 PNG ouverts : start D=Monter Maison loin a gauche, walking engin demonte en marche D=Monter, mounting D=Montage... + barre PENDANT le montage a l'arret, arrived et end D=Demonter + activite Attaque, ligne vers la Maison. Pas de PV lus (Maison non selectionnee).
Reponse: BUG: « le scénario ne s'est jamais lancé » (2026-11-25 18:3x, DOCS-1125d) ; commande donnée : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 deploy_far -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir ; ligne [ ] en tête de Auto_Journey_File.md.
