# [À RÉPONDRE] Accord de fusion : main avance-t-il en local à chaque passe verte ?

Contexte : integ-53 (5a920a1, la passe à fusionner : elle remplace integ-52 et contient integ-49 à 52, D880 aura de l'Aube, zones de furtivité de Forest, pont de Suivre, deploy) est verte du premier coup (build PASS, tests 2097/2097, net et build PASS) mais `main` n'a pas avancé (4b6b573 = docs seulement) : aucun accord de fusion n'a été donné au lancement de la séance du 2026-11-22.
Les textes se contredisent : D838 (`GDD.md` l. 3634) n'autorise l'avance locale que pour la séance de migration ; `Auto_Journey.md` l. 75 exige un accord explicite donné au lancement (« sans accord explicite, pas de fusion ni de push ») ; l. 188 cite « fusion locale sans push » parmi les règles qui restent.

Question : que fait la séance autonome du 2026-11-22 (et les suivantes) quand une branche d'intégration est verte ?
- A. Avancer `main` en local à chaque passe verte, pour CETTE séance seulement (jamais de push). Effet : les worktrees peuvent se resynchroniser sur `main` et recevoir le travail des autres ; vos essais se jouent sur `main`. Il faudra redonner l'accord à chaque séance.
- B. Pareil, et le noter comme accord PERMANENT des séances Auto_Journey autonomes (jamais de push) ; la l. 75 d'`Auto_Journey.md` est alors précisée (et la contradiction avec la l. 188 levée). Effet : comme A, sans redemander.
- C. Ne pas avancer `main` : les branches `integ-*` restent prêtes pour vous. Effet : les essais se jouent depuis le worktree de la branche d'intégration, et les worktrees ne reçoivent pas le travail des autres (chaque agent reste sur le `main` du lancement, les correctifs se cumulent seulement sur la branche `integ-*`).
- Autre : (à préciser).

Recommandation : B (supprime la contradiction et évite de bloquer chaque séance sur la même question ; le push reste interdit).

Valeur par défaut : C (`main` n'avance pas ; integ-53 5a920a1 reste prête).

## Relecture indépendante d'integ-52 (REVUE-1122) : aucun bloquant, 5 gênants
Lecture seule de `git diff main...integ-52` (35 fichiers), rien compilé ni lancé (Docs\Rapports\REVUE-1122.md). État dans integ-53 (5a920a1) : gênants 1 et 3 corrigés (1 testé, 3 NON PROUVÉ seul) ; gênants 2, 4 et 5 NON corrigés.
1. CORRIGÉ dans integ-53, testé (3efc630, tests Arwen 2097/2097 ; rouge b0c7f81 jamais vu à l'exécution) : opacité de furtivité jamais remise à 1.0 d'une partie à l'autre (HOLUnitOpacity.cpp:9-13).
2. Suivi : un Move par image et par suiveur, un Gather complet par suiveur (HOLFollowBridge.as, HOLUnitScriptLibrary.cpp:34-62), coût non mesuré.
3. CORRIGÉ dans integ-53 mais NON PROUVÉ seul (2e2e0d5 : 8 appels NativeManualOrder, AUCUN test rouge ; RECETTE-1122e : la récolte arrête le suivi, mais ce chemin passait déjà par OrderAt) : Suivi : les ordres hors des cinq `Request*` accrochés ne mettent pas fin au suivi (HOLPlayerController.cpp:3202, 3479, 3403, 3497, 3514, 4815).
4. Suivi sur un client réseau sans RPC : SUPPOSÉ, à vérifier en réseau (HOLFollowBridge.as `Step`).
5. Soin : le clic droit sur une unité de la sélection avec un Moine devient un ordre de soin (HOLPlayerController.cpp:~1497-1508).

- D. Fusionner `main` après correction des gênants restants (2, 4 et 5 ; 1 et 3 sont déjà dans integ-53) (séance suivante). Effet : `main` n'avance pas dans cette séance (comme C) ; la fusion se fait en début de séance suivante, avec ces correctifs d'abord.
La recommandation reste B (les gênants ne sont pas des bloquants : le C++ est neutre sans classe script) ; D est à préférer si vous voulez que main ne reçoive aucun de ces défauts.

Reponse: B, accord permanent, jamais de push (2026-11-22 ~22:05, dans la conversation ; GDD D888)
