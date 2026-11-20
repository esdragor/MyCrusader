# HALDIR-1120e : essais D861 / D860 (scenarios et .look seulement)

APPELS_DECLARES: ~22 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 09:36 / 09:41 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: sans C++   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenarios seulement) ; assertit l'effet : sans objet
COMMITS: 1 (a79e4e7, 09:39), merge_main_fait: non (git merge integ-26 fait en premier geste : OK)
TESTS: aucun lance (aucun preflight, aucun editeur, demande) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: deploy, signals_minimap_alt, signals_altclick : preflight NON lance, aucun PNG ouvert : LOGIQUE SEULEMENT. Commandes a lancer par la session principale : `Tools\Scenarios\preflight.ps1 deploy` (puis signals_minimap_alt, signals_altclick) puis `play.ps1 deploy -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` APRES build de integ-26 + les correctifs D861 (le scenario ne prouve D861 que sur une DLL qui les contient : branche Elrond d9d930b/78be07e/13ee758/d498a8d).
QUESTIONS: aucune
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun

## FAIT
- `Tools\Scenarios\deploy.txt` : etapes 6 a 10 avant `hol.Test.Sanity` : (b) engin remonte (D, `waitlog (delay over)`, `waitlog D=Pack`), clic droit au sol `hol.Select.OrderNear 3000 -1000` -> `waitlog Order: 1 units to (`, captures `deploy_move_start`, `deploy_move_unmounting`, `deploy_move_walked` (`waitlog (delay over)`, `waitlog D=Deploy`, `wait 3` de marche) ; (a) clic droit sur la Maison `hol.Select.OrderNear 5300 0` -> `waitlog units attack`, captures `deploy_attack_mounting`, `deploy_attack_mounted` (`waitlog D=Pack`), `deploy_attack_hit` (`waitlog Mangonel hits a building for 45`, clic Maison). mapSize Small deja dans `deploy.json`. Duree estimee : ~35 s de l'existant + ~45 s = < 3 min.
- `deploy.look` : 6 lignes (OBJET nomme). `signals_minimap_alt.look` cree (3 captures ajoutees au `.txt` : `_before`, `_alt`, `_after`). `signals_altclick` : `signals_altclick_pulse_a` / `_b` a 0,3 s (`wait 0.3`) + 2 lignes `.look`.
- Docs : Palier3InProgress.md, SignalsInProgress.md (lignes ajoutees), Auto_Journey_File.md (ligne ajoutee apres D859), EtapeDefini.md (ligne ajoutee).

## RESTE / NON VERIFIE
- `hol.Select.OrderNear` est le clic droit au sol : je n'ai trouve aucun scenario qui l'utilise sur un BATIMENT. Le point `5300 0` suppose la Maison (SpawnBuilding House 1 5300 0) dans le repere propre de `FocusNear` ; si le clic droit ne rend pas `Order: 1 units attack` (log `HOLPlayerController.cpp:1447`), essayer `5300 0 enemy` ou un autre point.
- `waitlog (delay over)` suppose que ce texte (HOLDeployProcessor.cpp:129) est journalise a chaque fin de delai de montage / demontage et que `waitlog` attend la PROCHAINE ligne.
- `wait 3` (marche) et `wait 0.3` (pulsation) sont des attentes d'horloge assumees (pas d'evenement a attendre) ; la pulsation n'a pas de ligne de journal connue de moi : si pulse_a et pulse_b ont la meme taille a la lecture des PNG, la marque ne pulse pas ou 0,3 s tombe sur une phase identique.
- Aucun PNG vu ; texte du `.look` non confronte a l'ecran. Fins de ligne : `signals_altclick.txt` est LF (git avertit LF -> CRLF).
- Le scenario deploy est AVANT correctif rouge (comportement D861 bug) tant que la DLL n'a pas les correctifs d'Elrond.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs modifies seulement (mes lignes + autres sessions) ; code inchange : oui
