# DIAG-1121sd : quick_check -Parts build, "build hud" REGRESSION (integ-43)

APPELS_DECLARES: 17 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 2026-11-21 20:01 / 20:06 (arret dur 20:16)
PREMIER_BUILD: sans C++   BUILD: sans objet (binaire a jour)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (correctif d'une attente de quick_check.ps1 ; rouge vu a 20:03 : `build hud : FAIL (REGRESSION)`, 2 KO ; vert a 20:05 : `quick_check : PASS in 83 s`)
COMMITS: 1 (a7833ea, 20:04), merge_main_fait: non (hors perimetre, integ-43 est la branche d'integration)
TESTS: quick_check -Parts build : PASS complet (build hud + 34 autres etapes) ; TESTS_TAUTOLOGIQUES: 0 (aucun test neuf)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   EDITS_REFUSES: 0   DIAGNOSTICS_AS: sans objet (aucun .as touche)

## Cause exacte
Attente perimee, le jeu est conforme. Deux controles de l'etape "build hud" etaient KO (journal `Saved_Tests\Logs\quick_game.log`, `HUD report` / `HUD tooltips` du mark `build hud`) :
1. "Peasants without wood: the age panels..." : la page II de la grille porte maintenant `Z=SecondaryCentre greyed` (entre `A=Market` et `Q=SiegeWorkshop`), absent du regex (~L4268).
2. "tooltips: name, cost missing, effect" : la page II contient `II Z: Centre secondaire | W 150 missing | S 400 missing | Palier non atteint`, absent du regex (~L4273).
Le controle des cellules (`HUD cells`, ~L4271) passait par hasard (le `March.+` gourmand avalait `; II Z: Centre secondaire`).

## Pourquoi c'est l'attente qu'on corrige
D879 (GDD) : Centre secondaire = case 1 de l'onglet II (cle Z, apres le Marche en A), Paysan seulement ; couts du CSV `BuildingTypes.csv` (4599ab8 : SC2, 150 bois + 400 pierre, age II). Le jeu affiche exactement cela. FactionSlot (Faramir 64a212a) et l'infobulle de revenu (Galadriel 116bf29) ne sont pas en cause : les autres lignes du rapport HUD (page I, tooltips I, messages) sont inchangees. Comme prevenu par DIAG-1121sa, c'est l'extension attendue des regex de la page II.

## Correctif (a7833ea, Tools\Tests\quick_check.ps1, 3 lignes)
- grille : `II=locked [A=Market greyed Z=SecondaryCentre greyed Q=SiegeWorkshop ...`
- cellules : `II A: March.+; II Z: Centre secondaire; II Q: ...` (rend le controle explicite)
- infobulles : `II Z: Centre secondaire \| W 150 missing \| S 400 missing \| Palier non atteint;` apres le Marche.
BOM conserve (premier octet 0xEF verifie avant/apres), analyse `Parser::ParseFile` : 0 erreur, diff git 3 lignes (fins de ligne gardees).

## Resultat
`quick_check.ps1 -Parts build` : build hud PASS, toutes les etapes build PASS, `quick_check : PASS in 83 s`.

NON VERIFIE: les autres parties de quick_check (tests, net) relancees apres ce commit ; le panneau de page II a l'ecran (seulement le journal HUD).
PROCESSUS_DETACHES: voir sortie stray_processes ci-dessous (lance en dernier geste)
ANOMALIES_PROCESSUS: aucune ; le journal de quick_check est dans `Saved_Tests\Logs\quick_game.log` (pas `Saved\Logs`), et `Docs\` du worktree n'en contient pas.
GIT_STATUS_PRINCIPALE: copie principale non touchee par moi (docs de suivi deja modifies avant la seance : oui, inchange).
