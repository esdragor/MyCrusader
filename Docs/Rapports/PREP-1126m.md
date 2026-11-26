# PREP-1126m : rapport (préparateur, docs seulement)

APPELS_DECLARES: 38 (arrêt à 45 ; premier Edit fait au 20e appel, pas avant le 6e : écart à la consigne, la vérification dans main passait d'abord ; archive copiée au 6e appel)
HEURE_DEBUT/HEURE_FIN: 11:15:57 / 11:27
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build, docs seulement)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (docs seulement) + assertit l'effet : sans objet   TEST_ROUGE_SEPARE: non (pas de code ; le « rouge puis correctif en deux commits » de l'énoncé ne s'applique pas à des docs)
COMMITS: 1 sur main (copie principale), 99a1306 à 11:26 (docs par `commit_docs.ps1 -Only`, `Agent: PREP-1126m`) ; RIEN commité dans la branche de Sauron ; merge_main_fait: sans objet
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune créée ; deux sont à poser par les agents suivants, défauts dits dans leurs tâches (`Q-porte-remboursement-base-1126m` si la réponse B est ambiguë, Celeborn ; `Q-sync-branche-prete-1126m`, Sauron : défaut A = sauter les branches « PRÊTES »)

FAIT :
- Relevé de `Docs\Priorite_Socle_Factions_2026-11-16.md` refait sur main 5dc2318 (shas par `git merge-base --is-ancestor`, grep datés) ; archive avant : `Docs\Archive\Priorite_Socle_Factions_2026-11-16_avant_1126m.md`.
- `Auto_Journey_File.md` : 15 lignes corrigées par une note `|| PREP-1126m` (états [x] ou RESTE vérifié) : l. 31 (D915 fait 2b402fd), 33 (G-1 à G-5a, G-6 dans main, RESTE G-5b/c), 34 (waitlog 01ee4bf, RESTE le pré-vol), 35 (tower_wood fait), 36-38 (D908 5b7ddb6, D911 0da0a6f, D909 985d6db en SCRIPT sans C++), 56 (D918 d7b14d5), 64 (S1-d e5d68be, a4d6a48), 65 (note), 67-70 et 72 (S2 : 05f4465, 9c515ca, a371040, 21facdc, 4f688be, e61a729, a3de24e, fad4af7 dans main ; l. 69 RageMs TOUJOURS lu : HOLCombatProcessor.cpp:1160-1168, HOLSelectionMarkers.cpp:791, HOLInterpolatedInstancesProcessor.cpp:583). Copie avant : `Docs\Archive\Auto_Journey_File_avant_PREP-1126m.md`. Les `[x]` sont à archiver au prochain nettoyage.
- Tâches écrites : `Docs\Rapports\TACHE-CELEBORN-1126m.md` (D921 règle + cvar + test, G10 restes), `TACHE-SAURON-1126m.md` (L29-6, L29-8), `TACHE-CPP3-1126m.md` (G-5b/c, D923 bouton T, liaison de crédit D921).
- Ligne `EtapeDefini.md` ajoutée (ligne 70).

DÉCOUVERTES QUI CHANGENT LE PLAN (à lire par l'orchestrateur) :
1. **D921 n'est pas « sans C++ » (File l. 14, L29-2)** : `OrderRemoveOwnBuilding` (HOLScriptRpcPlayerActor.cpp:496) détruit sans créditer, aucune API script ne crédite un stock (seul `CheatResources`). Règle + cvar + test = script (Celeborn) ; le crédit = liaison C++ neuve, reportée au lot C++ n°3. Celeborn ne doit pas appeler une fonction C++ inexistante.
2. **G-5 de la priorité est déjà largement fait** : G-5a (a493984) a retiré 11 cas du switch ; restent Produce / Research / Trade / Build (HOLPlayerController.cpp:2755-2872) car le pont n'a pas `FHOLGridCostFacts`.
3. **G10 portes : déjà branché** dans `ServerPlaceWallLine` ET `ServerPlaceBuilding` (HOLRpcPlayer.as:1533-1555, :1613) ; ne reste que l'empreinte orientée (C++) et le rejeu de `wall_gate_replace`.
4. **Prérequis du lot C++ n°3** : `market-quote-1125s` / `tests-bindings-1125s` modifient HOLPlayerController.cpp et ne sont PAS dans main : passe d'intégration d'abord (Haldir, `integ-100`+).
5. **L29-6 ambigu** : l'IN_USE de Faramir disait « libre : ... DEUX BRANCHES PRÊTES » (pas OCCUPE) ; sauter seulement OCCUPE n'aurait rien changé. Défaut proposé dans la tâche de Sauron (sauter aussi « PRÊTE », switch, question à poser).
6. Test de la sync : `Tools\Tests\sync_worktrees_test.ps1` (pas `Tools\AutoJourney\`) ; ligne File l. 24 à corriger au prochain nettoyage.
7. La touche T « non injectable » n'a aucune cause au journal : `hol.UI.Key` accepte `FKey("T")` ; instrumenter d'abord (L29-3), dit dans TACHE-CPP3.
8. Chemins du `ForeignSession` : `Get-ForeignSessionNote` ne lit pas IN_USE.txt.

RESTE : lignes 34 (pré-vol de duel_challenge_accept), 65 (a) / (b), 67 (RageMs) non tranchées faute de vérification fine ; Placeholders.txt relu, rien à ajouter (aucun provisoire créé ni remplacé) ; Auto_Journey_Retours.md « tête = 577d066 » à recompter (agent de docs).
NON VÉRIFIÉ : aucun essai rejoué ; shas vérifiés présents dans main, pas leur effet à l'écran ; états « fait » des lignes 65 et 67-70 d'après les shas et le grep, pas d'après un test lancé ; le contenu de `Q-porte-remplacement-remboursement.md` n'a pas été relu (absent de `Retours\`, texte repris de `FortificationsInProgress.md:303` et de la File l. 14) ; `ToolsInProgress.md` n'existe pas (journal de Sauron = `JourneyInProgress.md`, non lu).
PROCESSUS_DETACHES: aucun (aucun processus lancé ; `stray_processes.ps1 -Stop` NON lancé sur le worktree Sauron car la séance art peut y tourner)
ANOMALIES_PROCESSUS: premier Edit après le 6e appel ; `rg` absent du PowerShell (Grep utilisé) ; une 1re passe de `PREP-1126m_file.ps1` a refusé les 15 lignes (jokers `[ ]` du `-like`), corrigée en `StartsWith`, aucune écriture partielle (le script réécrit le fichier une seule fois) ; EtapeDefini.md non commité par moi (contient les lignes d'autres sessions, L29-8).
GIT_STATUS_PRINCIPALE: `M ArtistesInProgress.md`, `M Auto_Journey_Jour.md`, `M Docs/SessionEnd.txt`, `M EtapeDefini.md` (ma ligne + celles d'autres sessions), `Docs/Rapports/*` non suivis ; mes 4 fichiers commités en 99a1306 : oui, inchangé hors docs voulus : oui
