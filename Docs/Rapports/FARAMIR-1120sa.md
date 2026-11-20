# FARAMIR-1120sa (zone-effect, sans C++, 2026-11-20 22:56 -> ~23:08)

APPELS_DECLARES: ~22 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 22:56:36 / voir fin de journal
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, outils PowerShell seuls)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: L20-1 = 5cfde92 (merge_union_test.ps1, rouge vu 22:57 : « FAIL avec le .gitattributes du worktree : aucun conflit », 2/3 PASS) ; L20-3 = e318343 (main_code_filter_test.ps1, rouge vu 22:58 : 5 FAIL sur 12 ; + 2 lignes gate_test) ; assertit l'effet : oui (fusion git reelle ; decompte de commits).
COMMITS: 4 sur zone-effect (5cfde92 test L20-1, b6863eb correctif L20-1, e318343 test L20-3, 4247f1e correctif L20-3), dernier 22:59, merge_main_fait: non necessaire (branche a jour avec main au depart)

## L20-1
`.gitattributes` : ligne `*InProgress.md merge=union` ajoutee apres `*.sh text eol=lf`. Test `Tools\AutoJourney\merge_union_test.ps1` : depot jetable dans TEMP, deux branches ajoutent chacune une ligne a la fin du meme `CheckInProgress.md`, fusion. Controle rouge (sans la regle : conflit) PASS ; cas vert (vrai .gitattributes : aucun conflit, deux lignes gardees) PASS apres correctif.
Attention (note du commit) : un journal en union peut garder deux lignes d'etat contradictoires : relire apres fusion.

## L20-3
`Tools\AutoJourney\MainCodeFilter.ps1` : fonction `Test-MainCodeCommit -Title -Files`, dot-sourcee par `session_gate.ps1` (bloc Close, controle L19-1) : un commit ne compte que s'il touche `Source/`, `Script/` ou `Tools/` (avant : `Tools/Tests/` seulement), sauf titre `Merge ...` / `Docs:` et sauf fichiers `.uasset` / `.umap` seuls. Les avances rapides et cherry-pick n'ont pas d'entree `commit:` du reflog : deja non comptes. Tests : `main_code_filter_test.ps1` (12 cas) VERT ; `gate_test.ps1` : 3 lignes ajoutees (commit `Docs:` + commit `.uasset` seul apres le cas des 5 commits : compte inchange).

TESTS: main_code_filter_test 12/12 ; merge_union_test 3/3 ; gate_test complet : voir ci-dessous (lance 22:59, long). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun. QUESTIONS: aucune. Placeholders: aucun ajout.
NON VERIFIE: le cas `gate_test` chaine complete (Docs: + .uasset) n'a pas ete vu rouge a l'execution (la fonction isolee, elle, l'a ete) ; le compte reel sur le reflog de la copie principale n'a pas ete relance.
ANOMALIES_PROCESSUS: aucune.

gate_test COMPLET : lance 22:59, toujours en cours (>10 min, sortie bufferisee) a l'arret dur 23:12, arrete par TaskStop : resultat inconnu, NON VERIFIE. Merge main fait (84dcad2, merge commit : main avait 1 commit de docs d'avance).
