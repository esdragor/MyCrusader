# SWEEP-1119 : balayage de hol.Units.MarchMemberDetourSteps (1, 2, 3, 4, 6), 2026-11-19

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`. Aucun build, aucun commit, aucun fichier suivi edite. Un scenario temporaire `neutrals_trace_mm2.txt/.look` a ete cree puis supprime (voir 3).
Commande par valeur : `powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1 -Parts formation -GameExec "hol.Units.MarchMemberDetourSteps N"`. Valeur verifiee dans `Saved_Tests\Logs\quick_game.log` apres chaque passe (`hol.Units.MarchMemberDetourSteps = "N"`, heures UTC du log).

## 1-2. Tableau (12 file gaps en cm, seuil du controle : 60)

| N | 12 file gaps (cm) | max | backsteps | autres cas de la partie formation |
|---|---|---|---|---|
| 0 (DIAG-1119b) | 44, 39, 39, 32, 35, 40, 38, 38, 39, 36, 39, 38 | 44 | PASS | aucun [KO] (PASS en 66 s) |
| 1 | 58, 39, 38, 32, 35, 40, 45, 38, 39, 36, 36, 38 | 58 | PASS | aucun [KO] (PASS en 67 s) |
| 2 | 58, 39, 38, 32, 35, 40, 45, 38, 39, 36, 36, 38 (identique a 1) | 58 | PASS | aucun [KO] (PASS en 65 s) |
| 3 | 78, 39, 52, 32, 35, 40, 63, 38, 39, 36, 37, 49 | 78 | FAIL | seul [KO] : `files: file gap 60 cm at most` (73 s) |
| 4 | 97, 39, 69, 32, 35, 40, 80, 38, 39, 36, 52, 55 | 97 | FAIL | seul [KO] : files (59 s) |
| 6 | 136, 39, 104, 32, 37, 40, 104, 41, 39, 36, 70, 61 | 136 | FAIL | seul [KO] : files (56 s) |
| 8 (DIAG-1119) | 174, 39, 138, 32, 37, 40, 128, 55, 39, 36, 82, 61 | 174 | FAIL | seul [KO] : files |

Lecture : le max monte regulierement avec N (44, 58, 58, 78, 97, 136, 174). Les marches 1, 3, 7 (indices 0, 2, 6) portent la derive ; les marches 4-6, 8-10 ne bougent pas. Les 5 autres cas de la partie formation ne produisent aucun [KO] a aucune valeur : le seul rouge est `backsteps` / `files`.
Valeurs 1 et 2 : resultats identiques a la derniere decimale (simulation deterministe, memes graines) ; le detour de membre tenu 1 ou 2 pas ne change rien l'un par rapport a l'autre dans cette partie. Marge faible : file gap 58 cm contre un seuil de 60 (marche 1 : 44 cm a 0, 58 cm a 1 et 2). Les files de 3 sont rouges (78 et 63).

PLUS GRANDE VALEUR QUI GARDE LES FILES : 2 (max 58 cm, backsteps PASS), avec 2 cm de marge sur la marche 1.

## 3. Le tremblement : comment il a ete mesure le 2026-11-15

Source : `Docs\Archive\Retours_2026-11-15\Q-march-member-detour.md` (section Mesure x16, CELEBORN-1115c), logs `HeroesOfLiberty-Celeborn\Saved\c1115c\x16.txt` et `gap.txt`.
- Scenario `neutrals_trace` (Tools\Scenarios\neutrals_trace.txt : camps neutres, marche de blocs, `hol.Sanity.DumpUnit -3`) et sa variante `neutrals_trace_mm0.txt` (meme chose + ligne `hol.Units.MarchMemberDetourSteps 0`). Une passe = un lancement du jeu : 47 a 94 s (le 2026-11-15 : 47-70 s en moyenne).
- Commande : `powershell -ExecutionPolicy Bypass -File Tools\Tests\repeat_scenario.ps1 <scenario> -Runs N -Root <worktree>` (copie des binaires, un jeu a la fois, une ligne `pass k/N` par passe, `repeat <name> : PASS k/N`). Les logs de passes sont gardes en `Saved_Tests\Logs\scenario_<name>_run<k>.log`.
- Ce qui compte comme un tremblement (« shakes ») : la ligne d'audit `Sanity: FAIL unit #X (slot 0) at (...) shakes over 8 s: path ..., turn ..., (net ..., to and fro ...), 0 reversals, ordered 100%` dans le resultat de la passe (regle `WALK_SHAKE = 8` : 8 inversions du pas dans une fenetre de 20 pas, chacun de plus de 5 cm ; `Tools\Tests\melee_stuck_gaps.py:71-132`). Les « spins » (`spins over 8 s`, `spins: turned ...`) sont une autre classe (boucles de seconde ligne en melee, D641) et ne comptent PAS.
- Comptes de 2026-11-15 : 0 -> 3 « shakes » sur 24 passes (2 passes), 8 -> 0 sur 24. Comptage pris dans les messages `... shakes over 8 s` des lignes `pass k/N` de `x16.txt` / `gap.txt`.

### Rejeu court (3 passes) a N = 2
Scenario temporaire `neutrals_trace_mm2` (neutrals_trace + `hol.Units.MarchMemberDetourSteps 2` en tete), `repeat_scenario.ps1 neutrals_trace_mm2 -Runs 3`. Variable verifiee a 2 dans les 3 logs. Resultat : pass 1 PASS (75 s), pass 2 PASS (92 s), pass 3 FAIL (81 s) par un SPIN de #8 en (236, 3550), pas un shake. Donc 0 tremblement sur 3 passes avec N = 2.
ATTENTION : 3 passes ne prouvent rien de sur (le defaut touchait 3 passes sur 24 avec 0, soit environ 1 sur 10 : on a une chance sur 4 environ de ne rien voir en 3 passes meme sans correctif). Cette valeur-ci ne dit pas que 2 supprime le tremblement.

### Commande a lancer pour 24 passes (a faire, ~30 min machine calme)
1. Recreer le scenario : copier `Tools\Scenarios\neutrals_trace.txt` (+ `.look`) vers `neutrals_trace_mm2.txt` (+ `.look`) en ajoutant en premiere ligne `hol.Units.MarchMemberDetourSteps 2`.
2. `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Tests\repeat_scenario.ps1 neutrals_trace_mm2 -Runs 24 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` (en arriere-plan, timeout de la session > 30 min), puis compter les lignes `pass k/24 ... shakes over 8 s`. Comparer a la reference de 2026-11-15 : 0 -> 3 shakes / 24 ; 8 -> 0 / 24. Un test deterministe plus court : `MarchPairNoTremble` (C++ `HOLAvoidTests.cpp:124`, miroir script `HOLAvoidRules_Test.as`) avec la variable a 1, 2 ; non rejoue ici (a lancer par son lot de test habituel avec `hol.Units.MarchMemberDetourSteps 2`).

## Anomalies
- 1 et 2 donnent exactement les memes chiffres.
- Marge de 2 cm seulement a N = 2 (58 contre 60).
- La passe 3 du rejeu (N = 2) est rouge par un spin (famille D641, connue, identique avec 0 et 8 le 2026-11-15).
- Les deux commandes longues sont passees en arriere-plan automatiquement (180 s) ; sans consequence.

## Etat du worktree
Binaire inchange. Scenario temporaire `neutrals_trace_mm2.*` supprime. Reste des fichiers non suivis d'un autre chantier (`c28_*`, `c31_*`), non touches. Logs de passes ajoutes dans `Saved_Tests\Logs\` (scenario_neutrals_trace_mm2_run1-3.log, quick_game.log) ; `stray_processes.ps1 -Stop` : aucun processus detache.
