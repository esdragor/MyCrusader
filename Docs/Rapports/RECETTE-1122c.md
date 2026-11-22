# RECETTE-1122c : recette independante, worktree HeroesOfLiberty-Sauron, integ-52 edc631a

HEURE_DEBUT/HEURE_FIN : 18:25:38 / 18:32 (Get-Date). Appels : environ 29 (depassement du plafond de 25 : stealth_zones non lance, voir RESTE).
Git status au depart : seuls des Docs\Integrate_* non suivis. Aucun code, scenario ni commit modifie. Processus detaches : aucun (stray_processes.ps1 -Stop).

## Verdicts

| essai | verdict | duree | FAIL/WARN | erreurs Angelscript | WorldContext |
|---|---|---|---|---|---|
| deploy (preflight) | PASS VU A L'ECRAN (geste reste interne : selection, camera) | 87 s mur (check 83 s) | 0 / 0 | 0 | 0 |
| order_lines_blocks (preflight) | PASS VU A L'ECRAN (geste reste interne : Select.Army, spawn) | 22 s (check) | 0 / 2 (HUD overlap et Sanity absents, preexistants) | 0 | 0 |
| follow_crowd (play -Hidden) | LOGIQUE SEULEMENT | 48,7 s (50,1 s play) | 0 / 0 ligne LogHOL (4 Warning Angelscript au chargement de HOLCheatPanelScript.as, hors scenario) | 0 | 0 |
| follow_short (play -Hidden) | LOGIQUE SEULEMENT | 62,5 s (64,9 s play) | 0 / 0 | 0 | 0 |
| stealth_zones | NON LANCE (plafond d'appels depasse) | - | - | - | - |

play.ps1 n'a pas de PASS/FAIL : les deux follow_* n'ont aucune assertion, ils vont jusqu'a `quit` ; le verdict vient du journal et des PNG.

## deploy

Preflight PASS, 19 PNG ouverts (5 planches de contact 2x2, lisibles). Vu : deploy_packed D = Monter ; deploying D = Montage... [#------] (1 diese) ; deployed D = Demonter ; fire Maison 555/600 ; fire_hit 510/600 ; packing D = Demontage... [##----] ; packed_again D = Monter ; move_start D = Demonter ; move_unmounting barre Demontage vide, engin pas encore parti ; move_walked engin plus bas, D = Monter ; arrive_mounting D = Montage... ; arrive_mounted D = Demonter ; attack_mounting D = Monter (pas encore monte) ; attack_mounted D = Demonter ; attack_hit Maison 465/600 ; far_start/walking/midway/end : marche vers la 2e Maison (gauche), puis monte a portee, ligne rouge vers la Maison. far_end : PV non lisibles (Maison non selectionnee), seul le journal les prouve. Engin dessine en mannequin couche (provisoire connu).

Cible touchee prouvee par les PV : 1re Maison 555 -> 510 (deploy_fire/fire_hit) puis 465 apres le clic droit d'attaque (PNG attack_hit lit 465/600) ; 2e Maison lointaine 555 -> 510 (journal Building damaged ; step 1180 et 1280, coups de 45 Siege).

Clics droits (journal) :
- sol (0.207, 0.500) : `Scripted right click at ... armed attack-move: no, placing: no` puis `Order: 1 units to (-13400, -1008)` puis `Deploy phase 2 -> 3 (order 2, walk 2)`, demontage puis marche, puis a l'arrivee `phase 0 -> 1 (order 3)` montage seul. Rien d'autre.
- Maison proche (0.500, 0.337) : `Order: 1 units attack House of slot 1 (other)`, `phase 0 -> 1 (order 1, distance 1849 cm)`, coup a 51.17.
- Maison lointaine (0.172, 0.500) : `Order: 1 units attack House of slot 1 (other)`, `phase 0 -> 1 (order 1, distance 3175 cm)` apres marche, coups.
Aucun clic au sol n'a declenche autre chose qu'un deplacement ; aucun Suivre ; aucun soin.

## order_lines_blocks

Preflight PASS. PNG pendant ouvert : 12 unites en deux blocs (6+6, gauche et droite de l'ecran), DEUX lignes jaunes, chacune de son bloc vers le point d'ordre au centre. Journal : `Scripted right click at (0.500, 0.513)` puis `Order: 12 units to (-13000, 0)`. Remarque : la note du scenario dit « nord et sud », a l'ecran ce sont gauche / droite (cosmetique, la camera).

## follow_crowd (play -Hidden)

Journal : les 3 clics (corps Paysan, corps Homme d'armes selectionne, sol) donnent `Order: 10 units to (-14709, 2495)`, `(-14795, 2497)`, `(-15497, 2499)` ; aucune ligne Follow ; toutes les unites Idle apres chaque ; hol.Follow.OwnUnits = 0 : un Suivre non voulu n'apparait pas. PNG (planche 2x3) : start douteux (masse trop petite pour distinguer le damier ni les cercles ; 6 spheres vertes visibles au lieu de 10 Paysans, a identifier) ; body_peasant / body_selected / ground : masse deplacee vers la gauche monde, vu.
Soin : points de depart a surveiller.
- `hol.Cheat.Wound 50` blesse TOUTES les unites (« 22 units wounded », hp 78/155 sur les Hommes d'armes), pas seulement #22 comme le dit le commentaire du scenario. Consequence : le Moine soigne #22 tout seul AVANT l'ordre (step 240, 77,5/155) et #19 (Paysan 25/50) APRES (step 550) : soins automatiques sur d'autres unites, pas un soin « parti par clic sur la mauvaise unite ».
- 1er clic : `Order: 1 units heal #23` (Homme d'armes sain, cas 4) -> le Moine s'arrete a 600 cm `at range`, 2e clic `heal #22` -> `too close` a 300 cm, `#22 is healed to 155 by #21` au step 549.
- DOUTEUX : ce soin se termine (17:29:27) 5 s APRES la capture follow_crowd_heal_done (17:29:22) : le PNG ne montre pas le blesse plus plein (la selection est le Moine, 40/80) ; heal_start et heal_done sont quasi identiques. Preuve par le journal seulement.
- Ces clics passent par hol.Select.OrderNear (aucune ligne `Scripted right click at`) : geste interne.

## follow_short (play -Hidden)

Journal : `FollowBridge: follow started, 4 units follow #5`, `Order: 4 units follow #5 Peasant`, premier trajet : les 4 Hommes d'armes Moving puis Idle autour du Paysan (positions a 100-300 cm) ; retour : `Order: 1 units to (-13292, 5496)`, Hommes d'armes repartent avec le Paysan ; 2e trajet idem ; ordre manuel : `FollowBridge: manual order ended the follow of 4 units` (UNE ligne) puis `Order: 4 units to (-13292, 3497)` ; Hommes d'armes arrivent (-13174..-13410, ~3500), Paysan reste (-11791, 3496), ils ne reviennent pas. Aucun Suivre non voulu.
PNG (planche 2x2) : start vu (4 Hommes d'armes groupes + Paysan seul) ; walk DOUTEUX (Paysan selectionne, groupe d'Hommes d'armes colle au bord haut droit, Paysan non distinguable) ; second_leg vu (groupe de 5 au contact) ; cancelled vu (4 Hommes d'armes en bas avec cercles, Paysan seul plus haut).
DEFAUT D'OUTIL : `follow_short.look` declare `follow_short_walk2` ; le scenario prend `follow_short_second_leg` (et cancelled, walk, start). Un preflight follow_short echouerait sur le PNG walk2 absent : .look a corriger (je n'y ai pas touche, consigne).

## stealth_zones

Non lance : budget d'appels (25) depasse. A relancer : `play.ps1 stealth_zones -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, attendre les lignes `StealthOwn opacity unit 11 = 0.5` puis `= 1`.

## Lignes `recette:` ecrites
E-deploy.md ligne 9 (titre inchange), E-order_lines_blocks.md ligne 10 (via edit_line.ps1 -Append, relues).

## Champs
TESTS_TAUTOLOGIQUES : sans objet. NON VERIFIE : clic physique (aucun essai n'a de vrai evenement souris OS) ; son (-nosound) ; stealth_zones ; couleur du soin sur le PNG heal_done ; les 6 spheres vertes de follow_*.
ANOMALIES_PROCESSUS : aucune. GIT_STATUS_PRINCIPALE : non relu en fin (seuls Retours\E-deploy.md, E-order_lines_blocks.md et ce rapport ecrits dans la copie principale).
Captures et journaux copies dans le scratchpad de la session (olb.log, fc.log, fs.log, scenario_deploy.log).
