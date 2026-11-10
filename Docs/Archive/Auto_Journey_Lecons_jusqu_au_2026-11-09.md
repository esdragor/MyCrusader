# Archive Auto_Journey : leçons des séances du 2026-11-01, 2026-11-03 et 2026-11-04 (déplacées le 2026-11-10)

Texte repris tel quel de `Auto_Journey.md`. Chaque leçon a un équivalent dans `Auto_Journey_Checklist.md` (L1-n, L3-n, L4-n ; L1-4 = L3-7, L1-5 = T2-7, L3-10 et L4-2 = L5-1). Ce texte n'est conservé que pour l'historique.

## Leçons de la séance interactive du 2026-11-01 (test 4, 16:50-18:50, 8 worktrees, ~15 agents, 7 agents de fusion)

Résultat : 10 essais sur 10 validés (contre 5 sur 16 le 2026-10-31), 5 décisions de l'utilisateur en séance (D622-D625), boucle réponse -> correction en minutes. **Règles à appliquer dès la prochaine séance** (approuvées par l'utilisateur le 2026-11-01) :
1. **Durée et tokens des agents** : plusieurs ont dépassé 250k (343k, 312k, 312k) et travaillé plus d'une heure. Relancer un agent **neuf vers 200k**, repartant du journal.
2. **Commit par lot vert, au plus 30 min sans commit** (37 et 34 fichiers non commités pendant 60 min). Fichiers par nom, jamais `git add -A`.
3. **`git merge main` avant le rapport** : chaque agent fusionne `main` dans sa branche avant de rendre la main, pour que la fusion soit une avance rapide sans retest ; l'agent de fusion ne rejoue que `tests,aiserver,aiduel`.
4. **Veilleur silencieux** : un script unique qui ne réveille la session que si `Auto_Journey_Retours.md` change ou si un agent termine ; **ne rien écrire** quand rien n'a changé (12 passages « rien » en séance).
5. **Un fichier par item dans `Retours/`** (essai, question) au lieu d'une ligne géante par item (suppression = recopier toute la ligne dans Edit) ; l'en-tête est compté par script.
6. **Identifiants de réponse** : `E1`, `Q2` ; l'utilisateur répond `E1 OK`, `Q2 A`. Les réponses nues (« works », « fonctionne », « Cercle ») étaient ambiguës quand deux essais attendaient.
7. **Toute question porte une option « Autre » et les valeurs par défaut prises si l'utilisateur ne précise pas** (taille, durée, valeur d'armure) : la préférence réelle était hors des options 3 fois sur 4 (TB-Q1, MM-Q1, SQ3). Les hypothèses prises par la session sont listées « à confirmer » (cercle 24 -> 8 px sur 6 s ; armure distance 20).
8. **Un seul enregistrement de statut par agent**, consolidé par la session principale : 5 à 6 journaux mis à jour par agent (`*InProgress`, `EtapeDefini`, file, `Jour`, `IN_USE`, `Retours`) coûtent cher et `Jour.md` n'était pas à jour.
9. **Numéros de décision GDD réservés par plage et par agent**, ou écrits par la session principale (D622-D625 écrits par elle) ; vérifier « Prochain numéro » avant d'écrire.
10. **Docs de la copie principale commités à chaque boucle** (par nom de fichier, jamais `GDD.md` sans accord) : ils auraient pu bloquer chaque avance rapide.
11. **Machine** : l'utilisateur joue depuis **un instantané figé** (pas depuis le worktree d'un agent qui compile : un build a attendu sa partie, puis 517 s de file) ; `.gitattributes` pour normaliser CRLF/LF/BOM (l'outil Edit échouait sur des fins de ligne mélangées) ; **PowerShell seulement pour l'heure** (Git Bash diverge de 40 min).
12. **Outils manquants à construire** : (a) capture en séquence au pas de simulation pour prouver une animation (un cercle qui se rétrécit n'est pas prouvé par un instantané ; sinon l'essai est LOGIQUE SEULEMENT) ; (b) détection par le pré-vol d'un panneau qui en cache un autre (ligne HUD d'`ai_evolving` sous « Triches », fenêtre de duel sous le panneau du scénario) ; (c) `check.ps1` doit échouer quand l'audit Sanity échoue (seul `preflight` le voit).
13. **Test rouge d'abord vérifiable par l'ordre des commits** (test avant implémentation) : sauté pour R2 des relais, `Test_TechnologyNotImplemented` et le tribut.
14. **Passage d'intégration de 10 min en fin de séance** sur `main` fusionné (règles 5 et 10 de « Qualité des essais », toujours non appliquées) : 3 essais qui croisent les chantiers (Marché + tribut + mini-carte + talents).

## Leçons de la séance interactive du 2026-11-03 (15:41-17:30, 7 agents C++, 15 agents au total ; approuvées par l'utilisateur le 2026-11-03)

Mesure (`Tools\build_queue_report.ps1`) : 24 builds en 3 h 40, 5 orphelins, 4 abandonnés ; attente médiane 38 s mais jusqu'à 44 min (Glorfindel), 32 min (Elrond) ; 17 builds demandés en 2 h par 7 agents ; 3 agents ont rendu du code NON compilé (deux relances de 25 min pour compiler/tester). Règles **toutes obligatoires** :
1. **Plafond de 3 agents C++ écrit DANS l'énoncé.** Les autres démarrent par des docs, des règles en Angelscript, des scénarios ou des tests de script, et ne passent au C++ qu'à la libération d'une place. La session principale regarde `Docs\BuildQueue.state\` avant chaque lancement.
2. **Un agent qui touche au C++ lance son build TOUT DE SUITE en `run_in_background` et reste vivant jusqu'à sa fin** (un build détaché meurt avec l'agent : exit 4). Son arrêt dur est repoussé jusqu'à la fin de son build, et il écrit « NON COMPILÉ » en tête du rapport s'il rend la main avant. Un agent qui rend du code non compilé = un agent neuf dédié « compiler et tester », jamais une fusion.
3. **Outils de test** : `check.ps1` (PASS périmé quand le journal précédent est tenu) et `quick_check -Parts tests` avec d'autres parties (faux FAIL 0/0, journal `quick_tests_2.log`) sont corrigés (7e808ae, dans `main` 58657f2, test `Tools\Tests\log_lock_test.ps1`) pour les journaux `tests` et `game` ; les autres journaux nommés de `quick_check` (net, aiserver...) ont encore l'ancien défaut (ligne `[ ]` de la file) ; en cas de doute, lancer `tests` SEUL.
4. **Écriture au bon endroit** : chaque agent n'écrit que par chemin absolu dans son worktree (jamais de chemin relatif), et son rapport se termine par `git status` de la copie principale inchangé (hors docs voulus). Un fichier modifié par erreur dans la copie principale bloque l'avance rapide.
5. **Builds plus rapides plutôt que plus nombreux** : un seul lot de modifications de headers par agent avant le premier build (un premier build après un header prend 200 à 750 s) ; `git merge main` seulement avant le rapport.
6. **Bugs de comportement durs (mouvement, mêlée) : un agent neuf à 200k tokens**, repartant du journal avec la mesure acquise (l'agent de D641 a consommé 250k+ sans corriger). La session principale ne relance pas le même agent.
7. **Veilleur** : au réarmement, il liste à nouveau les fichiers existants comme « nouveaux » ; ignorer le premier tour, ou persister l'état vu dans un fichier.
8. **Agent d'intégration nommé en début de séance** : aucune validation à l'écran pendant la séance du 2026-11-03 (5 branches dans `main` sans essai joué) ; les règles 5, 10 et 14 (recette indépendante, essais rejoués sur `main` fusionné, passage d'intégration de 10 min) n'ont pas été appliquées pour la troisième séance de suite. La session principale désigne dès le lancement UN agent chargé de rejouer, après la fusion, les 2 ou 3 essais prêts sur le build de `main` (pré-vol + PNG ouverts) et de les écrire dans `Retours\`.
9. **La session principale se relance vers 200k tokens**, comme un agent : repartir d'une session neuve qui lit `Auto_Journey_Jour.md`, `Auto_Journey_File.md` et `Retours\`. (Séance du 2026-11-03 : 290k en fin de séance.)
10. **L'heure se lit à `Get-Date` PowerShell avant toute annonce de temps** (arrêt dur, « il est X h ») : plusieurs heures annoncées de mémoire étaient fausses de 5 à 10 minutes et ont brouillé les arrêts durs.
11. **Régénération des cartes** : elle demande l'éditeur, dont le démarrage (Turnkey / `ValidatePlatforms`) attend les builds des autres worktrees (éditeur de Haldir bloqué 14 min sans rien produire). Ne pas la planifier en même temps que des builds ; la lancer seule ou hors file, avec le harnais Python à module `unreal` factice pour les réglages.
12. **Étape 0 sautée** : acceptable seulement quand la file de travail porte déjà les tâches du jour (réponses de l'utilisateur en tête, comme D644-D649) ; sinon l'étape 0 reste obligatoire.

## Leçons de la séance interactive du 2026-11-04 (11:40 -> prévue 14:00, réelle ~15:50 ; approuvées par l'utilisateur le 2026-11-04, toutes obligatoires)

Bilan : fusion groupée de 4 branches (921/921) et 7 décisions, mais 1 h 45 de dépassement reproché par l'utilisateur, 5 agents C++ en même temps, builds de 20 à 40 min, 3 agents ayant rendu du code non compilé, un agent à 424k tokens.
**Heure et durée**
1. **L'arrêt dur est un contrat.** Aucun agent n'est lancé ni relancé après l'heure de fin ; un retour tardif devient une ligne `[ ]` de la file pour la séance suivante. Une prolongation se demande **15 minutes avant l'heure**, et ne dépasse pas **2 cycles (30 min)**.
2. **L'heure vient de `Get-Date` PowerShell à chaque annonce** (mes estimations de mémoire étaient fausses de plusieurs minutes) ; les arrêts durs sont écrits en heure réelle dans chaque énoncé.
3. **Alarme de fin** : au lancement, un rappel programmé à « fin - 15 min » déclenche la clôture : plus de lancement, bilan, fusion groupée.
**File de build (le vrai goulot)**
4. **Plafond de 3 agents C++ vérifié avant CHAQUE lancement** (`Docs\BuildQueue.state\` + agents actifs) ; au-delà, l'agent démarre sans compiler (docs, scénarios, tests de script).
5. **Un build par agent, lancé dans les 15 premières minutes** : tout le C++ en un seul lot, puis `build.ps1` en `run_in_background`. Un agent qui n'a pas compilé à mi-parcours passe la main ; jamais de code non compilé rendu comme fini (Faramir, Celeborn, Sauron, Galadriel l'ont fait).
6. **La fusion groupée démarre à « fin - 30 min »** avec ce qui est prêt (elle a pris 70 min dont 40 d'attente de file) ; ce qui n'est pas prêt attend la séance suivante.
**Tokens et agents**
7. **Un agent est remplacé à 200k tokens, jamais plus tard** (Haldir 424k) ; le journal du chantier doit permettre de repartir d'un agent neuf.
8. **Question sans réponse = valeur par défaut déjà codée** (a bien marché : aucun agent n'a attendu) ; l'option « Autre » et la recommandation restent.
**Essais**
9. **L'agent d'intégration est nommé au lancement** et rejoue les essais après la première fusion (il a trouvé le bug du tableau des joueurs que personne n'avait vu) ; c'est le seul à écrire les essais d'intégration dans `Retours\`.
10. **Un essai n'est donné qu'après fusion et build vérifié** : un essai d'une branche non fusionnée (`E-maps-review`) attend, sauf demande explicite de l'utilisateur.
11. **Plusieurs cartes/variantes = un scénario de revue** (une commande, Entrée charge la suivante, nom et taille à l'écran), pas une commande par élément.
**Outils**
12. **Corriger l'outil partagé au lieu du contournement privé** : `bEdgeScroll` à couper dans les scénarios (défilement de caméra pendant un `check`), parties `netfogview`/`netspec` inexistantes dans `quick_check` (vraies : `netfogtrees`, `netspecteams`), démarrage de l'éditeur bloqué par les builds des autres copies (`-Multiprocess` le contourne, à mettre dans `crossings.ps1`, `hinterland_paths.ps1`, `quick_check.ps1`).
13. **Détecter un `Intermediate` périmé** : après `git merge main`, un build qui échoue avec des erreurs de déclaration dans des fichiers que l'agent n'a pas touchés (UHT/.generated.h) = supprimer `Intermediate\...\Inc\HeroesOfLiberty\UHT` de CE worktree avant de chercher une cause dans le code.
14. **Fin de séance : tous les worktrees se remettent à jour avec main** (demandé par l'utilisateur le 2026-11-04) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\sync_worktrees.ps1` (`-DryRun` pour voir). Il fait `git merge main` dans chaque branche ; un worktree avec du travail suivi non commité est SAUTÉ et signalé, un conflit annule la fusion et est signalé, jamais de push. À lancer après la fusion groupée et avant le bilan, **quand aucun agent ne tourne** ; les sauts et conflits entrent dans le bilan (ligne `[ ]` de la file). Le travail non commité d'un agent est d'abord commité sur sa branche (jamais laissé en l'air) : le 2026-11-04 le C++ de Biomes B2 (Faramir) n'était PAS dans `fog` bien que la fusion l'ait laissé croire.

