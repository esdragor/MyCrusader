# TACHE-SAURON-1126m : L29-6 (sync_worktrees saute les worktrees pris) puis L29-8 (commit_docs -Only limité à ses lignes)

Écrit par PREP-1126m le 2026-11-26 d'après main 5dc2318. Worktree : `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (branche neuve partant de main ; art-sound2-1126ar est finie) ; séance art possible : `IN_USE.txt` `CPP: non`, céder au premier commit art. Outils PowerShell seulement, sans C++. Lis `brief_commun.md` et `agent_rules.md`.

## Joueur (une ligne)
Aucun effet en jeu : la fin de séance ne fusionne plus main dans le worktree d'une autre session, et un commit de docs n'emporte plus les lignes des autres sessions.

## L29-6 : `Tools\AutoJourney\sync_worktrees.ps1` saute tout worktree « pris »
Cas vu : la sync de fin a fusionné main dans `tests-bindings-1125s` (Faramir) alors qu'elle l'avait sautée au début. Relevé :
- `sync_worktrees.ps1` (114 lignes) saute déjà : fichiers SUIVIS modifiés (:71), session étrangère (`Get-ForeignSessionNote`, `Tools\AutoJourney\ForeignSession.ps1:21`, commit de moins de 2 h hors main absent de `## Agents du`). Il ne lit PAS `IN_USE.txt`.
- Format de `IN_USE.txt` (règle 9 de `brief_commun.md`) : 1re ligne `OCCUPE par <AGENT> (<date heure>) : <tâche>` ou `libre : ...`, 2e ligne `CPP: oui|non`. L'IN_USE de Faramir disait « libre : CLAUDE-1125s fini ... DEUX BRANCHES PRETES pour integrate.ps1 » : donc OCCUPE seul ne suffit pas.
- Test existant : `Tools\Tests\sync_worktrees_test.ps1` (fonctions `Check`, `RunGit`, `Sync`, `Commit-OnMain`, worktree jetable `$Wt`).

1. **Premier Edit** : `Tools\Tests\sync_worktrees_test.ps1`, nouveau cas en fin de fichier (après le dernier `Check`) : trois worktrees jetables en retard sur main avec (a) `IN_USE.txt` « OCCUPE par TEST-1 (...) », (b) « libre : X fini ... BRANCHE PRETE pour integrate.ps1 », (c) « libre : ... » sans mention. Attendu : (a) et (b) SAUTES avec le mot `pris` dans la ligne et HEAD inchangé ; (c) fusionné. Assertion sur l'EFFET : `git rev-parse HEAD` avant / après, pas seulement sur la ligne imprimée.
2. Bouchon : ne rien changer au script, lancer le test : rouge VU (le cas (a) et (b) fusionnent à tort), commit 1 = test seul (aucun bouchon nécessaire, le script compile) `Agent: SAURON-1126m`.
3. Correctif : dans `sync_worktrees.ps1`, après le contrôle de `$dirty` (:71) et avant `Get-ForeignSessionNote` (:73), lire la 1re ligne de `<worktree>\IN_USE.txt` ; fonction `Get-InUseTakenNote` (dans `ForeignSession.ps1`, une seule source) qui renvoie une phrase si la ligne commence par `OCCUPE` ou, avec le switch `-SkipReadyBranches` (défaut ON, une ligne à changer), contient `PRETE` / `PRETES` (insensible à la casse) ; sortie `<nom> | <branche> | SAUTE (pris : <ligne>) : ni fusion ni build`. Le défaut ON pour « BRANCHE PRETE » est un choix de comportement à confirmer : pose `Retours\Q-sync-branche-prete-1126m.md` `[À RÉPONDRE]` (A : sauter les branches prêtes, recommandé car elles attendent `integrate.ps1` ; B : seul OCCUPE saute ; Autre), défaut A derrière le switch.
4. Test vert, `commit 2`. Mettre à jour l'en-tête du script (règles) et la ligne L29-6 de `Auto_Journey_Checklist.md` n'est PAS à toi (la session principale).

## L29-8 : `Tools\AutoJourney\commit_docs.ps1 -Only` limité à ses lignes
Cas vu : des commits de docs emportant des lignes d'autres sessions (`EtapeDefini.md`, `Auto_Journey_Jour.md`) et `EtapeDefini.md` passé en LF.
- Relevé : `commit_docs.ps1` (42 lignes) : `-Only` (:21-26) accepte une liste de chemins et commite le fichier ENTIER (`git commit -- <chemins>`, :36-40) ; aucune notion de lignes ; aucun appel à `eol_check.ps1`. Test existant : `Tools\AutoJourney\commit_docs_test.ps1` (dépôt jetable, `Put`, `Git`, `Check`).
- `eol_check.ps1` (36 lignes, `-Root`, `-Fix`) remet l'arbre en CRLF sans diff git.

5. Premier Edit de la 2e tâche : `Tools\AutoJourney\commit_docs_test.ps1`, fin du fichier : cas A (`EtapeDefini.md` avec deux lignes modifiées par deux sessions, `-Only EtapeDefini.md -Lines "<texte de ma ligne>"` ne commite que la ligne choisie : `git show HEAD:EtapeDefini.md` contient ma ligne et PAS l'autre) ; cas B (`EtapeDefini.md` en LF dans l'arbre : le commit passe par `eol_check -Fix` d'abord, `git diff` vide sur les fins de ligne après). Rouge VU, commit 1 = test seul.
6. Correctif : paramètre `-Lines` (liste de textes littéraux, un par ligne à emporter) valable avec `-Only` et un seul fichier ; construction de l'index avec `git update-index --cacheinfo` d'un blob dont seules ces lignes sont prises (patch limité : `git apply --cached` d'un diff filtré) ; avant l'`add` d'un `EtapeDefini.md` lancer `eol_check.ps1 -Root <Root> -Fix`. Sans `-Lines`, comportement inchangé (tous les tests existants restent verts). Le design du mécanisme (index partiel) est à ta main, pas de question tant que le comportement sans `-Lines` est inchangé.

## Limites
- Aucun C++, aucun build. Deux tâches seulement si 10 min et 15 appels restent après L29-6.
- Ne lance jamais un script que tu n'as pas écrit (`SAURON-1126m_*.ps1` pour les temporaires).
- Journal : ton `*InProgress.md` (`JourneyInProgress.md`, le journal des outils Auto_Journey ; relis sa fin) ; ta ligne `EtapeDefini.md` ; File l. 24 et l. 25.
- Rapport : `Docs\Rapports\SAURON-1126m.md` (13 champs) ; tests : `sync_worktrees_test.ps1` et `commit_docs_test.ps1` (sorties collées).
