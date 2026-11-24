# RECETTE-1124a (HALDIR-1124l, recette indépendante, Q5 / L5-4)

HEURE_DEBUT/HEURE_FIN: 12:02 / 12:10
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche integ-58, `git merge main` (main = 9d40517, main est ancêtre de HEAD : oui), merge commit 87038f8.
Build : `build_wait.ps1` absent de main, donc `Tools\build.ps1 -LiveCoding=0 -Wait` : PASS (HeroesOfLibertyEditor Development, 92 s ; LaunchCheck avant : DLL plus vieille que HOLMoraleOrderTests.cpp, d'où le build).
Pré-vol : `Tools\Scenarios\preflight.ps1 <nom> -Root <worktree>` (headless, pas de play.ps1 visible), un seul éditeur à la fois.

| Essai | Pré-vol | PNG vus | Verdict |
|---|---|---|---|
| wall_line_skip | PASS 34 s, 0 erreur au journal | during, after | PASS |
| faction_aim_free | PASS 56 s, 0 recouvrement de HUD | ready, aim, aim_far, after | PASS |

## wall_line_skip (D901)
- during : PASS. Objet vu : file de dalles vertes plates (une par case), DEUX dalles rouges sous le cadre filaire de la Maison, les autres restent vertes (ligne non refusée).
- after : PASS. Objet vu : 7 fondations de palissade (cadres filaires) de part et d'autre de la Maison, trou à la place de la Maison, plus de dalles vertes/rouges ; Bois 20150 -> 20115 (HUD) = 7 x 5.

## faction_aim_free
- ready : PASS. Objet vu : château, héros + 5 Hommes d'armes, anneau crème de 12 m, panneau Lumière sacrée 100 / 100 « prêt », PV héros 180 / 450, aucun cercle de visée.
- aim : PASS. Objet vu : cercle VERT de 6 m au sol, centre dans l'anneau de portée, pas de zone dorée, Honneur 100.
- aim_far : PASS. Objet vu : cercle ORANGE, centre hors de l'anneau de portée, pas de zone dorée.
- after : PASS. Objet vu : zone dorée au point cliqué, Honneur 85 / 100, case « recharge 18 s », PV héros 245 / 450.

## Fiches corrigées
`Retours\E-wall_line_skip.md` et `Retours\E-faction_aim_free.md` : commande play.ps1 complète avec `-Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, mention « dans main 9d40517 », bloc de recette HALDIR-1124l.

## Anomalies
- Les tips integ-41 (d8d2939) et integ-37 (7ea4e01) ne sont PAS ancêtres de main ni du HEAD du worktree (`merge-base --is-ancestor` faux) : le contenu est pourtant dans main (scénarios 7644229 / df47590, `Script\UI\HOLFactionAim.as`, `Script\Tests\HOLFactionAim_Test.as`). Le gate Essay `session_gate -Phase Essay` par ancêtre peut donc FAIL à tort : à examiner par la session principale.
- WARN préexistant : wall_line_skip n'appelle pas `hol.HUD.Report` (recouvrement de HUD non contrôlé).

## NON VÉRIFIÉ
Son ; vraie souris du bureau (curseur virtuel) ; `quick_check` / tests script (Test_FactionAimPreviewColoursOutOfReach etc.) non lancés ; cas « toutes les cases bloquées » ; client réseau.
