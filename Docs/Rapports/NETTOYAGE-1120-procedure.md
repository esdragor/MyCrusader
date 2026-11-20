# NETTOYAGE-1120-procedure (étape 0a, séance du 2026-11-20)

Archive : `Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-20.md` (98098 octets, copie intégrale des 3 fichiers avant toute modification, un titre par fichier). Rien supprimé sans copie. Fins de ligne conservées (procédure et check-list CRLF, brief LF), pas de BOM avant ni après.

## Auto_Journey.md (43855 -> 43513 octets)
- Retiré : phrase « Ancienne règle : Fable 5.2 ... » (modèles remplacés par la consigne du 2026-11-16, ligne « Modèles et limites ») ; puce « Le premier build après modification d'un header prend ~200 s » (doublon de « Builds : ne pas saturer la file ») ; ligne `Tâche sans C++ : ... -Phase Launch -NoCpp` du modèle d'énoncé (doublon de « Builds » et de la check-list) ; première phrase de la puce « L'horloge de Git Bash diverge » (= `agent_rules.md` règle 3, renvoi gardé).
- Raccourci : parenthèse « 77 déclarés = 108 réels... » de l'ARRÊT DUR du modèle (= `agent_rules.md` règle 2, renvoi) ; phrase d'ouverture de « Fusions dans `main` » dernier paragraphe (doublon de « La fusion groupée est la règle par défaut »).
- Sommaire : renvoi à la nouvelle archive ajouté à la puce « Archives ».
- Titres lus par `session_gate.ps1` / `gate_test.ps1` intacts : « Modèle d'énoncé d'un agent » (le bloc garde `CPP: oui`, `LAUNCHCHECK_DEBUT`, `TESTS_TAUTOLOGIQUES`), `L12-6` présent, `## Agents du` non concerné (fichier Jour).

## Auto_Journey_Checklist.md (44405 -> 42206 octets)
- 19 lignes de tableau retirées (redites de règles d'`agent_rules.md` / champs de rapport) : L1-2, L1-3 (1re ligne), L3-2/L4-5, L3-5, L1-8, L1-9/L4-8, L1-7, T2-5, T2-6/L5-8, T2-7/L1-5, L4-12/T2, L4-13, Q1, Q2, Q3, Q4, Q6/Q7, Q9, L1-13. Chacune est listée dans le paragraphe « Nettoyage du 2026-11-20 » (avec la règle qui la porte) ; elles restent cochées au bilan comme les autres, selon le précédent des nettoyages du 11-13 à 11-16.
- L12-1b condensée en une ligne (le contrôle est fait : L12-1c, `build.ps1` colonne `load`, `Launch` WARN ; reste l'avis ProjectShiva).
- Titres de section et IDs restants intacts.

## Tools\AutoJourney\brief_commun.md (9209 -> 8967 octets)
- Retiré trois exemples datés : « Prouvé : la sonde RPC ... » (point P1d), « le 2026-11-17 des éditions perdues ... » (règle 0), « vu le 2026-11-19 18:55 : quick_check cassé ... » (règle 5, la consigne GARDE LE BOM reste).

## Session gate
`session_gate.ps1 -Phase Launch -NoCpp` : « => aucun FAIL ». WARN : 7 worktrees en retard de 2 commits sur main, fichiers suivis modifiés dans la copie principale (dont les miens, non commités au moment du lancement) ; aucun ne vient de mes éditions. `gate_test.ps1` lancé en entier : trop long (arrêté), les contrôles qui lisent `Auto_Journey.md` (jetons du modèle, `L12-6`) vérifiés à la main : tous présents.

## Doutes (rien retiré)
| Où | Section / ligne | Doute |
|---|---|---|
| brief_commun.md | point 4 (« UN commit vert test + correctif ») contre paragraphe 4 (« DEUX commits séparés, le test seul d'abord ») | contradiction réelle, à trancher par l'utilisateur ; `agent_rules.md` règle 7 dit un seul commit vert |
| brief_commun.md | règle 3 (« Tâche C++ ... `build.ps1 -Wait` ») et règle 2 (« UN test rouge + UN correctif ») | en tension avec « pas de build dans un agent » (déjà noté le 2026-11-19 soir) |
| Auto_Journey.md | modèle d'énoncé (45 appels, 80 appels) et `agent_rules.md` règle 2 | divergent des 25 appels / 15 min de `brief_commun.md` ; phrase d'en-tête du modèle le signale, bloc non réécrit (le test `gate_test.ps1` en lit les jetons) |
| Checklist | L15-3, L15-4, L15-6 | décrivent `replace_text.ps1`, `agents_table.ps1`, `build_wait.ps1` : aucun n'existe dans `Tools\AutoJourney` ; gardées (outils non faits, ou propositions abandonnées : à confirmer) |
| Checklist | L12-1 (a)-(d) | (a) fait (L12-2), (b) (c) (d) non vérifiés dans le gate ; gardée |
| Checklist | L1-1/L3-6/L4-7/L5-3 | dit 45 appels vus ; reste du texte utile (`tool_uses` > 100 = agent neuf) ; gardée |
| Checklist | L16-3, L16-8, L16-9 | contrôles « OUTIL » dont je n'ai pas vérifié la livraison (`preflight_merge`, gate Essay C++ seulement, `sync_worktrees`) ; gardées |
| Checklist | L15-10, L14-x | grosses lignes de constat daté (mesures de tokens) ; condensables mais mêlées à des règles encore actives ; gardées |
| Auto_Journey.md | « Mode interactif », « Le veilleur », « Plan de lancement » | non lus contre l'usage récent ; gardés |

## NON VÉRIFIÉ
- `gate_test.ps1` entier non terminé (durée > 3 min). Aucun test ne lit la check-list ou `brief_commun.md` (recherche dans `Tools\AutoJourney\*.ps1` : aucune référence) ; `Tools\Tests` non cherché.
- Le paragraphe de la check-list n'a pas été relu par un second lecteur.
