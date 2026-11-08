# Règles communes de tout agent Auto_Journey (à lire EN PREMIER ; l'énoncé du lancement dit « lis ce fichier et applique-le »)

Chaque règle est mécanique : un nombre à compter, une commande à lancer, un champ du rapport. La session principale vérifie ces champs à la réception ; un champ manquant ou hors seuil est noté « non appliqué » au bilan (`Auto_Journey_Checklist.md`).

## Fenêtres (AGENT, 2026-11-08 : « des tests scénario pop sur mon écran et prennent le focus de ma souris »)
0. **Aucun lancement fenêtré.** Tu ne lances JAMAIS un `UnrealEditor.exe` visible : `-game` / `-server` toujours avec `-nullrhi -nosound` (pas de capture) ou `-RenderOffScreen -nosound -unattended` (capture), et `Start-Process ... -WindowStyle Hidden`. Pour rejouer le geste de l'utilisateur : `play.ps1 <nom> -Hidden -Root <worktree>` (sans `-Hidden`, `play.ps1` ouvre une fenêtre : réservé à l'utilisateur). `quick_check -Visible` et `net_match -Visible` n'existent que pour l'utilisateur. Les outils partagés (`quick_check`, `net_match`, `check.ps1`, `*_paths`, `*_shots`...) sont masqués par défaut ; `Tools\AutoJourney\no_window_test.ps1` échoue si un script d'outil lance un éditeur sans masquage (et `session_gate -Phase Launch` le signale en WARN). Un lancement visible constaté = outil à corriger, pas à contourner.

## Heure et durée
1. **Premier geste : `Get-Date` PowerShell.** Si l'heure d'arrêt dur de l'énoncé est déjà passée, ne rien faire d'autre que le dire dans le rapport. Jamais repousser l'arrêt ; seule exception : un build C++ déjà lancé, qu'on attend.
2. **80 appels d'outils maximum, relevés par la session principale** (champ `tool_uses` de la notification de fin ; ton propre décompte est faux de 10 à 30 % : 77 déclarés = 108 réels). **Tu ne vois pas tes appels : arrête-toi à 60 appels déclarés** (marge de 30 %). À 60 : commit, journal à jour pour un agent neuf (fait / prochaine étape / test rouge en cours), rapport. Jamais continuer « parce que c'est presque fini ». Compte en tenant une liste dans ton journal (un trait par appel), pas de mémoire.
3. Toute heure annoncée vient de `Get-Date` (Git Bash diverge de 5 à 40 min).

## Écriture
4. **Chemins absolus** dans TON worktree ; jamais de chemin relatif. Dans la copie principale (`C:\Users\baudo\Documents\MyCrusader`) tu n'écris que : `EtapeDefini.md` (relire juste avant, TES lignes seulement), tes fichiers `Retours\`. Jamais `GDD.md`, `CLAUDE.md`, `Auto_Journey*.md` (sauf ta ligne de `Auto_Journey_File.md`). Aucun numéro de décision D : la session principale les attribue.
5. **Un seul enregistrement de statut** : ton `*InProgress.md` (état précis pour un agent neuf) + ta ligne d'`EtapeDefini.md`. Pas six journaux.
6. Un fichier avec fins de ligne mélangées refuse `Edit` multi-lignes : lance `powershell -ExecutionPolicy Bypass -File <ton worktree>\Tools\AutoJourney\eol_check.ps1 -Root <ton worktree> -Fix` (L8-6 : remet l'arbre de travail en CRLF, aucun diff git), ou remplace par lignes et relis le résultat. **Écris les fichiers par l'outil Write/Edit, jamais par heredoc Bash** (le hook casse les heredocs). `IN_USE.txt` sans accents.

## Tests
7. **Test rouge d'abord, sur la vraie chaîne** (pas de valeur à la main). Il assertit l'**EFFET** (PV qui baissent, ressource reçue, bouton actif, unité arrivée), jamais seulement un prédicat ou une ligne de journal. Commit du test AVANT l'implémentation (le SHA va au rapport). Un test rouge peut venir du script de test : lis le dernier `Test mark` du journal avant de toucher au code.
8. `quick_check` **ne compile pas** : `Tools\build.ps1` d'abord. `-Parts` des seules parties touchées ; `tests` SEUL en cas de doute ; ne jamais lancer deux `quick_check` / `log_lock_test` en parallèle (collision de journaux). Un `net` rouge sans changement de code : le relancer deux fois avant de le croire. **Rouge connu ou régression (L8-5)** : `quick_check` imprime `(rouge connu depuis <date> : <cause>)` pour une partie listée dans `Tools\Tests\known_reds.txt`, `(REGRESSION : pas dans known_reds.txt)` sinon, et une ligne `red summary` en fin de sortie ; seules les REGRESSION bloquent une fusion. Une partie connue qui passe imprime `retirer de known_reds.txt` : retire la ligne.
9. **Scripts et outils** : un script n'est « prêt » qu'après un passage de bout en bout (colle la sortie au rapport). Outil partagé fautif (`check.ps1`, `preflight.ps1`, `quick_check.ps1`) : corrige l'outil et dis-le, pas de copie privée.

## Build (C++)
10. **Au plus 3 agents C++** en même temps : si l'énoncé dit « sans C++ », tu n'en fais pas (docs, scénarios, Angelscript `*_Test.as`, données).
11. **Tout le C++ en UN lot, puis `Tools\build.ps1 -LiveCoding=0` en `run_in_background` dans les 15 premières minutes**, et tu restes vivant jusqu'à la fin du build (un build détaché meurt avec l'agent). Un build en attente par worktree : relancer remplace. Rendu sans build = « NON COMPILÉ » en TÊTE du rapport.
12. Après `git merge main`, un échec de déclaration dans des fichiers que tu n'as pas touchés = supprimer `Intermediate\...\Inc\HeroesOfLiberty\UHT` de CE worktree avant de chercher une cause. Un fichier binaire (`.uasset`) en conflit se **régénère** depuis les CSV fusionnés, jamais « theirs » à l'aveugle.
13. Régénération des cartes (éditeur) : seulement seul, file de build vide ; l'éditeur se lance avec `-Multiprocess`.

## Git
14. **Commit par lot vert, au plus 30 minutes sans commit**, fichiers par NOM (jamais `git add -A`), message en anglais, sans `Co-Authored-By`, jamais de push, **jamais de fusion dans `main`** (l'agent de fusion s'en charge).
15. **`git merge main` avant le rapport** (pour que la fusion soit une avance rapide sans retest).
16. Ne jamais `checkout`/`reset` du travail non commité d'un autre.

## Questions, décisions, essais
17. **Ne jamais trancher un design.** Question = fichier `C:\Users\baudo\Documents\MyCrusader\Retours\Q-<nom>.md` titré `[À RÉPONDRE]` : contexte en 2 lignes, options A/B/C **+ « Autre »**, conséquences visibles, **recommandation**, valeur par défaut codée derrière un réglage (cvar/CSV, une ligne à changer). **Au plus 2 questions par chantier**, regroupées. Tu n'attends jamais une réponse : tu appliques le défaut et continues.
18. **Un essai n'est listé (`[À ESSAYER]`) que si** `Tools\Scenarios\preflight.ps1 <nom>` PASS, `<nom>.look` présent, **chaque PNG ouvert avec Read et le point vu**, ≤ 3 min, aucune attente morte (`waitkey`), geste RÉEL (touche puis clic, pas un appel interne), statut `VU À L'ÉCRAN`. Sinon `LOGIQUE SEULEMENT`, jamais compté prêt. Un essai d'IA ou de système comprend au moins une partie naturelle. Un réglage à l'œil = **balayage** (N valeurs étiquetées côte à côte), pas une valeur à la fois. Un défaut visuel qui persiste après 2 réglages : remonte à la cause avant une 3e valeur. Libellés, touches, noms viennent du GDD et de la grille AoE4, jamais inventés. **Chaque `Retours\E-*.md` porte deux lignes de statut** (gabarit : `Tools\AutoJourney\E_template.md`) : `GESTE: réel` (vraie touche puis vrai clic, modèle `neutrals_qground`) ou `GESTE: interne` (commande `hol.*`) et `SON: entendu` ou `SON: non entendu` (`-nosound`, rien entendu par l'agent) ; un essai de geste ou de son reste LOGIQUE SEULEMENT tant que la ligne dit `interne` / `non entendu` ; `session_gate.ps1 -Phase Close` signale (WARN) les E-*.md `[À ESSAYER]` sans ces deux lignes. **Chaque `E-*.md` porte aussi une ligne `branches: a, b`** (les branches dont le correctif est joué) : `session_gate.ps1 -Phase Essay -Root <worktree de la commande play.ps1> -Essay <nom>` doit afficher PASS pour chacune (le worktree contient son tip, `git merge-base --is-ancestor`), sinon FAIL : fusionner la branche dans ce worktree et recompiler, ou jouer depuis un worktree qui la contient (2026-11-06 : qground joué depuis Elrond sans les correctifs de mêlée de Celeborn).
19. Un **REGLAGE ou BUG** de l'utilisateur est rejoué par un AUTRE agent (l'intégrateur) sur le build de `main` fusionné avant toute commande donnée à l'utilisateur.

20. **Aucun processus détaché à ta fin.** Dernier geste : `powershell -ExecutionPolicy Bypass -File <ton worktree>\Tools\AutoJourney\stray_processes.ps1 -Root <ton worktree> -Stop` (liste puis arrête les `chain`/`loop`/`UnrealEditor` de ton worktree ; un build C++ en cours attendu est listé mais jamais arrêté) ; colle la sortie dans `PROCESSUS_DETACHES`. La session principale relance le script avant l'agent suivant du même worktree.

## Rapport final : 13 champs, dans cet ordre (la session principale les vérifie)
```
APPELS_DECLARES: <n> (arrêt à 60 ; le seuil de 80 se juge sur `tool_uses` relevé par la session principale, pas sur ce champ)
HEURE_DEBUT/HEURE_FIN: <Get-Date> / <Get-Date>
PREMIER_BUILD: <HH:MM ou « sans C++ »>   BUILD: PASS / FAIL / NON COMPILÉ
TEST_ROUGE: <SHA du commit du test, antérieur au correctif> + « assertit l'effet : oui/non »
COMMITS: <n>, dernier à <HH:MM> (seuil 30 min), merge_main_fait: oui/non
TESTS: <parties lancées + chiffres>
ESSAIS: <nom : preflight PASS, PNG ouverts, VU À L'ÉCRAN / LOGIQUE SEULEMENT, commande play.ps1 avec -Root>
QUESTIONS: <fichiers Retours\Q-*.md créés, valeur par défaut codée>
FAIT / RESTE: <10 lignes au plus>
NON VÉRIFIÉ: <obligatoire>
PROCESSUS_DETACHES: <sortie de `Tools\AutoJourney\stray_processes.ps1 -Root <ton worktree>` lancé en dernier geste, ou « aucun »>
ANOMALIES_PROCESSUS: <obligatoire>
GIT_STATUS_PRINCIPALE: <sortie de `git -C C:\Users\baudo\Documents\MyCrusader status --short`, inchangée hors docs voulus : oui/non>
```
