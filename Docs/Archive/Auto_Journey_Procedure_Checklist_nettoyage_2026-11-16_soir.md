# Nettoyage 0a du 2026-11-16 soir (séance de migration) : lignes retirées de la procédure et de la check-list

Agent NETTOYAGE-0a-PROCEDURE. Règle : archiver sans jamais supprimer. Les règles restent en vigueur ; chaque ligne ci-dessous est devenue une règle de `Tools\AutoJourney\agent_rules.md` ou de `Tools\AutoJourney\brief_commun.md` (vérifié par recherche le 2026-11-16 soir).

## Auto_Journey_Checklist.md : lignes retirées (copie exacte) et preuve

| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L7-5 | Test rouge non observé quand un symbole manque : le test ne compile pas | l'énoncé d'un agent C++ impose un stub qui compile + test AVANT le premier build (rouge observé), budget de deux builds, et le champ TEST_ROUGE donne « observé » ou « supposé » | AGENT + RAPPORT |
| L12-4 | Des agents C++ ont ecrit du code avant de savoir si leur binaire demarrait | premier geste d'un agent C++ : `LaunchCheck` ; champ de rapport `LAUNCHCHECK_DEBUT` | AGENT + RAPPORT |
| L12-5 | Recette 12:26 : tests de regles pures en partie triviaux (30/45/60 s jamais assertis, identite) | au moins un test par regle qui passe par une vraie chaine ; champ de rapport `TESTS_TAUTOLOGIQUES : 0`, compte par la recette | AGENT + RAPPORT |
| L13-8 | 4 agents arretes a 45 appels avant d avoir ouvert les PNG et ecrit la fiche d essai | l enonce reserve les 8 derniers appels a PNG + fiche E + journal ; sinon l agent le dit dans NON VERIFIE | AGENT |
| L14-1 | Presque tous les agents depassent 45 appels (46 a 90 ; Glorfindel 48 sans build, Galadriel ~90) : attentes de build en arriere-plan, et 3 a 6 appels perdus par agent sur des Edit/Write refuses « fichier non lu » | l'enonce ordonne UN premier appel groupe qui lit (Read) tous les fichiers a editer ; les fiches `E-*.md`, EtapeDefini et la file s'editent par un outil de ligne (`Tools\AutoJourney\edit_line.ps1`, a ecrire) au lieu de PowerShell libre ; le rapport donne le compte VU, pas declare | AGENT + OUTIL |
| L15-5 | Le classifieur de permissions a refuse a au moins 5 agents (HALDIR-1115d/e, GALADRIEL-1115d, GLORFINDEL-1115e, ELROND-1115h) le commit du « test rouge seul » (la procedure dit « test rouge d'abord ») ; chacun a redecouvert « test + correctif dans UN commit, rouge VU a l'execution avant » et l'a dit dans ses anomalies | `agent_rules.md` : le rouge se prouve a l'EXECUTION (compte quick_check N-1/N, ligne du journal du test), jamais par un commit rouge ; un seul commit vert test + correctif ; le rapport donne `TEST_ROUGE: vu a <heure> : <compte ou ligne>` ; plus de champ « SHA du commit rouge » obligatoire | AGENT + RAPPORT |

Preuves : L7-5 et L15-5 = `agent_rules.md` règle 7 (« Rouge prouvé à l'EXÉCUTION (L15-5) : stub qui compile + test, rouge VU ... champ `TEST_ROUGE: vu a <HH:MM>` ») et `brief_commun.md` point 4 ; L12-4 = `agent_rules.md` « Binaire d'abord (L12-4) » et champ `LAUNCHCHECK_DEBUT` du rapport, `brief_commun.md` point 3 ; L12-5 = `agent_rules.md` règle 7 « Pas de test tautologique (L12-5) » et champ `TESTS_TAUTOLOGIQUES`, `brief_commun.md` point 4 ; L13-8 = `brief_commun.md` point 1 (« les 8 derniers sont réservés à : PNG + fiche E + journal + commit + rapport, L13-8 ») ; L14-1 = `brief_commun.md` point 2 (lecture en UN appel groupé), point 6 (`edit_line.ps1` pour les fichiers de suivi) et `agent_rules.md` règle 2 / champ `APPELS_DECLARES` (compte vu).

## Auto_Journey.md : lignes retirées (copie exacte)

Modèle d'énoncé d'un agent, redites d'`agent_rules.md` (« Binaire d'abord (L12-4) » et règle 7 « Pas de test tautologique (L12-5) »), remplacées par un renvoi :

```
Tâche C++ : premier geste après `git merge main`, AVANT tout code : `powershell -NoProfile -Command ". <worktree>\Tools\Scenarios\LaunchCheck.ps1; Get-StaleBinaryProblem <worktree>"` (vide = binaire à jour, sinon build d'abord) ; champ de rapport `LAUNCHCHECK_DEBUT` (L12-4).
Au moins un test par règle passe par une vraie chaîne (valeur calculée par le code testé, jamais recopiée) ; champ `TESTS_TAUTOLOGIQUES: 0` dans TESTS, recompté par la recette (L12-5).
```

Section « Séance de migration », texte remplacé (Q2 tranchée : A, confirmée le 2026-11-16 19:5x) :

Séance DÉDIÉE au passage du C++ en Angelscript, distincte des séances de fonctions. Le plan est `Docs\Rapports\ANALYSE-1116a.md` (ordre validé par D835 ; Q1 tests Mass : après le pilote ; Q2 rythme : à confirmer au lancement, voir D835).

2. Première question à l'utilisateur (interface de choix) : Q2 du plan (rythme), puis écrire la réponse dans D835.
