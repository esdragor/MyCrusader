# PREVOL-1119 : pre-vol de l'ensemble integre (worktree Elrond, tip d8fcc64)

Passage du 2026-11-19, 10:17 a 10:40, un jeu a la fois, aucun build, aucun commit, aucun fichier edite dans les worktrees.

## Pre-vol par scenario (`Tools\Scenarios\preflight.ps1 <nom>`, worktree Elrond)

| Lot | Scenario | Ligne finale | Controles en echec / WARN |
|---|---|---|---|
| 1 | ecran_hud_fil | `preflight ecran_hud_fil : PASS` | aucun |
| 1 | ecran_hud_grille | `preflight ecran_hud_grille : PASS` | aucun |
| 1 | ecran_hud_triche | `preflight ecran_hud_triche : PASS` | aucun |
| 1 | ecran_hud_selection_batiment | `preflight ecran_hud_selection_batiment : PASS` | aucun |
| 1 | ecran_hud_selection_unite | `preflight ecran_hud_selection_unite : PASS` | aucun |
| 1 | ecran_hud_selection_puces | `preflight ecran_hud_selection_puces : PASS` | aucun |
| 2 | ecran_hud_hautdroite | `preflight ecran_hud_hautdroite : PASS` | WARN seul : no 'Sanity:' audit line |
| 2 | ecran_hud_joueurs | `preflight ecran_hud_joueurs : PASS` | aucun |
| 2 | ecran_hud_alerte | `preflight ecran_hud_alerte : PASS` | aucun |
| 2 | ecran_hud_groupes | `preflight ecran_hud_groupes : PASS` | aucun |
| 2 | ecran_hud_file | `preflight ecran_hud_file : PASS` | aucun |
| 2 | ecran_hud_ages | `preflight ecran_hud_ages : PASS` | aucun |
| 3 | ecran_hud_bannieres | `preflight ecran_hud_bannieres : PASS` (check PASS 42 s) | aucun |
| 3 | alert_population | `preflight alert_population : PASS` (check PASS 69 s) | aucun |
| 3 | ecran_statistiques | `preflight ecran_statistiques : FAIL (1)` ; `check ecran_statistiques : PASS (35 s)` | FAIL de forme : `map: ecran_statistiques.json has no mapSize, the trial plays on the grey test map (D820: add mapSize Small to the .json and replay; -TestMap if the test map is wanted)` ; WARN : no 'HUD overlap:' line, no 'Sanity:' audit line |
| 3 | ecran_nouvelle_partie | `preflight ecran_nouvelle_partie : PASS` (check PASS 28 s) | WARN seuls : no 'HUD overlap:' line, no 'Sanity:' audit line |
| 3 | unit_stats_attack | `preflight unit_stats_attack : FAIL (1)` ; `check unit_stats_attack : PASS (25 s)` | FAIL de forme : `map: unit_stats_attack.json has no mapSize, ...` (meme texte) ; WARN : no 'Sanity:' audit line |
| 3 | unit_stats_attack_enemy | `preflight unit_stats_attack_enemy : FAIL (1)` ; `check unit_stats_attack_enemy : PASS (25 s)` | FAIL de forme : `map: unit_stats_attack_enemy.json has no mapSize, ...` (meme texte) ; WARN : no 'Sanity:' audit line |

Les trois FAIL sont des FAIL de forme (anciens scenarios sans mapSize, test sur la carte grise), pas un echec de jeu : leur `check <nom>` est PASS. Aucun FAIL reel.

## hud_rects_game_test.ps1 (worktree Elrond)

Ligne finale : `hud_rects_game_test : PASS`. Tous les cas PASS ; `INFO ai_evolving real overlaps: none`, `INFO duel_challenge real overlaps: none`.

## Point 3 : panneau de triche sur ecran_hud_joueurs (premiere capture hud_joueurs_depart.png)

| | Elrond (script, ensemble integre) | Arwen (C++, main) |
|---|---|---|
| Panneau de triche, coin haut gauche | REPLIE : bouton « Triches + » seul (16,70 a 222,115) | DEPLOYE : « Triches - » + 9 lignes (+1000 x4, +1000 de tout, Population +165, Construction rapide : non, 2 curseurs), 16,70 a 224,440 |
| Ligne `HUD cheats:` | `HUD cheats: cheats folded` (x2, 09:23:49 et 09:24:01 UTC) | `HUD cheats: cheats open, fast build off, unit speed x1.0, gather speed x1.0` (x2, 09:34:47 et 09:34:58 UTC) |
| Ligne `HUD rects:` cheats= | `cheats=16.0,70.0,222.0,115.0` | `cheats=16.0,70.0,224.0,440.0` |

Le reste de la capture est identique (barre du haut, bandeau « Scenario ecran_hud_joueurs en preparation... » en haut au centre, joueurs et Diplomatie / Menu en haut a droite, mini-carte). Les deux fenetres font 1920x1080, donc la regle de fenetre basse (LowWindowHeight = 700) ne joue pas.

Les deux lignes `HUD cheats:` sont donc DIFFERENTES (mot pour mot) et l'etat differe : replie cote script, deploye cote C++.

Cause trouvee (lecture seule) : les deux versions demarrent depuis la preference gardee `[HOL.CheatPanel] bOpen` de `Saved\Config\WindowsEditor\GameUserSettings.ini` de LEUR worktree :
- Elrond : `bOpen=False` (fichier du 7 nov. 15:47) ;
- Arwen : `bOpen=True` (fichier du 19 nov. 02:12).
Le C++ (HOLCheatPanel.cpp:146-149, GConfig + CheatsStartOpen) et le script (Script\UI\HOLCheatPanelScript.as:298, `CheatsStartOpen(bSaved, GetWindowHeight())`) appliquent la meme regle de depart. L'ecart vient donc tres probablement de la preference de chaque worktree et non d'un comportement different du code. Non prouve de bout en bout : il faudrait rejouer ecran_hud_joueurs dans Elrond avec `bOpen=True` (ou dans Arwen avec `bOpen=False`) ; je n'ai rien modifie. A corriger ensuite : faire que le pre-vol/scenarios fixent explicitement l'etat de depart du panneau (sinon les captures dependent du worktree), ou lever l'ambiguite avec ce rejeu.

## Fin de passage

`stray_processes.ps1 -Stop` : Elrond « aucun processus detache », Arwen « aucun processus detache ».
