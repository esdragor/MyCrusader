# Auto_Journey : la procédure

Ce fichier est la **procédure** (elle change rarement). L'état du jour est dans Auto_Journey_Jour.md, la file de travail dans Auto_Journey_File.md, ce qui attend l'utilisateur dans Auto_Journey_Retours.md. EtapeDefini.md ne garde que la liste des chantiers ; le détail technique reste dans chaque *InProgress.md.

## Consigne de l'utilisateur (donnée le 2026-10-30)

- L'utilisateur est absent toute la journée du 2026-10-31. Il faut avancer, remplir **~8 h de travail**, et en faire plus si possible.
- Plusieurs chantiers **en parallèle**, par sous-agents, dans les **worktrees existants** (aucun nouveau worktree).
- **Commits autorisés partout, même sur `main`, sans validation, exceptionnellement ce jour-là. Jamais de push.**
- Quand un chantier est fini ou attend la validation de l'utilisateur : le ranger (scénario d'essai prêt) et passer à un autre.
- Réserver à la fin un ou plusieurs **essais** pour l'utilisateur (fichier `Auto_Journey_Retours.md`, section 1 « Essais à faire »).
- Répondre aujourd'hui (2026-10-30) à un maximum de questions, pour avoir un maximum de travail demain.

## Tests d'une heure

Test 1 : 2026-10-30 16:01-17:05 (archivé dans `Docs/Archive/Auto_Journey_Test1_2026-10-30.md`). Test 2 : **préparé le 2026-10-30, à lancer seulement quand l'utilisateur le demande** ; mêmes règles que ci-dessous, mesures au repos et paquet sautés. **Autorisé (confirmé par l'utilisateur le 2026-10-30) : commits sur les branches des worktrees ET fusions dans `main` (agent de fusion, `quick_check` des parties touchées vert avant) ; jamais de push.** Les agents du 17:50 ont tous fini et les six branches sont fusionnées dans main (e547b2f, 2026-10-30 19:30) : tous les worktrees sont libres et à jour avec main pour le départ du test 2 (lancé par l'utilisateur dans un autre agent).

## Règles de conduite (réécrites après le test d'1 h du 2026-10-30)

### La session principale est le chef d'orchestre
- **Économiser ses tokens** (demande de l'utilisateur) : les sous-agents font tout le travail (code, tests, compilation, journal, commit, `EtapeDefini.md`). La session principale lance, lit des rapports de 10 lignes, tient ce fichier et `Auto_Journey_Retours.md`. Pas de gros fichiers lus en direct, pas de recherche dans le code, pas de relecture de ce qu'un agent vient d'écrire.
- **L'étape 0 est un sous-agent en lecture seule** (test : 70k tokens, 72 s) qui lance `Tools\AutoJourney\status.ps1` et rend le **plan de lancement** (voir ce titre). Le programme de `Auto_Journey_Jour.md` est celui d'hier soir, pas une vérité.

### Qui travaille, et sur quoi
- **Un agent par worktree, tous les chantiers du tableau lancés dès le départ, en parallèle.** Au test, 4 chantiers sur 6 avaient été écartés à tort, et il a fallu que l'utilisateur le remarque.
- **« Pris » dans `IN_USE.txt` n'est pas un interdit.** Un worktree « Pris » avec du travail non commité se **reprend sans l'écraser** : `git status`, vérifier que ça compile et que les tests passent, commiter par lots cohérents, jamais `git checkout`/`reset` sur du travail non commité. Seul cas où on ne touche pas : le chantier n'a rien d'ouvert (ex. réseau avant l'essai E5).
- **Quand « À faire » d'`EtapeDefini.md` est vide ou ne convient pas** (il s'est vidé en 1 minute au test) : prendre un **chapitre du GDD non commencé** dont les décisions sont tranchées (Volcan/Catastrophe, Technologies, Arbre de talents...), ouvrir son `*InProgress.md` (plan en étapes de 15 min, premier test rouge), coder la première étape. « Après le prototype » exclut seulement ce que le GDD marque ainsi. Le relevé de 12 chapitres est dans la section « Chapitres du GDD non commencés ».
- **Le GDD (`GDD.md`) est éditable même s'il est modifié et non commité par d'autres sessions** : éditions ciblées après relecture, ne pas commiter le fichier entier (il mêle plusieurs sessions). Avant d'utiliser un numéro de décision, relire § 22 (le « prochain D564 » avait été pris par le rattrapage).
- Jamais de nouveau worktree. Les agents ne **tranchent jamais** une décision de design : ils la posent, **en début de session** (pas au bilan), et si les deux options sont peu coûteuses ils préparent les deux derrière un réglage (cvar/option) au lieu de s'arrêter.

### Comment écrire l'énoncé d'un agent
- Chemins **absolus** partout ; pointer les journaux à lire au lieu de recopier leur contenu ; une tâche précise, des sous-étapes de 15 min, la plus petite d'abord.
- **« Enchaîne sans t'arrêter au premier point qui n'exige pas de code »** (un agent s'était arrêté après avoir constaté qu'un point était déjà bon).
- Arrêt dur à une heure donnée, lue à l'horloge **PowerShell** (`Get-Date`). Plafond ~250k tokens (au test : 50 à 160k tokens, 10 à 30 min par agent, jamais près de 300k). Un agent près de 200k n'est pas relancé : on en prend un neuf qui repart du journal. **Pas de minuteur par agent** : un seul veilleur (voir « Le veilleur »).
- Rapport final de **10 lignes au plus** : fait / tests / commit / reste / **NON VÉRIFIÉ (obligatoire)** / anomalies du processus.
- Chaque agent : met à jour `EtapeDefini.md` (relire juste avant, ne toucher que ses lignes : aucun conflit observé) , le journal de son chantier, `Auto_Journey_File.md` (sa ligne) et sa ligne du tableau des agents (`Auto_Journey_Jour.md`) à chaque avancée, `IN_USE.txt` à l'entrée et à la sortie (fichier non suivi par git), commite sur la branche de son worktree (anglais, sans `Co-Authored-By`, jamais de push).
- Test rouge d'abord sur la vraie chaîne ; un changement, une compilation, un passage du test ; `Tools\build.ps1` (file d'attente), `-LiveCoding=0` ; `quick_check -Parts` des seules parties touchées. Un test rouge peut venir du **script de test** et non du jeu (deux cas au test : IA du match 1v1 qui regroupait les unités, héros qui dérive d'1 m) : regarder le dernier `Test mark` du journal de jeu avant de toucher au code.

### Les retours de l'utilisateur (voir `Auto_Journey_Retours.md`)
- Tout ce qui attend l'utilisateur est dans **`Auto_Journey_Retours.md`** (1 essais, 2 questions, 3 commits et fusions). Les agents y écrivent, jamais ici.
- **`Auto_Journey_Retours.md` est léger et sans historique** : dès qu'une réponse de l'utilisateur est reportée dans son chantier (`*InProgress.md`, GDD, file de travail), l'item est **supprimé** du fichier ; idem un essai fait et reporté. Avant de supprimer, vérifier que la réponse est bien écrite dans le chantier (sinon l'y écrire d'abord). Le compteur en tête est recalculé à chaque modification.
- **Une question est une vraie question pour lui** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences visibles, recommandation, fichier. Jamais une remarque pour soi-même.
- **Un essai est listé seulement s'il est testable** : scénario `Tools/Scenarios/<nom>.txt` créé et démarré hors écran sans erreur, commande `play.ps1` en chemin absolu, gestes précis. Pas de scénario = on le crée d'abord (Tab et Hommes d'armes en surnombre en manquaient).
- L'utilisateur répond par `Reponse: ...` sous chaque item. **`Reponse : A trancher` = il veut en discuter en profondeur** : rien n'est noté comme décidé.
- **Une réponse n'enclenche pas de travail.** On enregistre la réponse dans les docs de chantier (`*InProgress.md`, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction ou la suite de la feature (ex. « arbres trop espacés ») attend dans la file du chantier. **Elle ne se fait qu'au prochain « commence ton Auto_Journey ».**

### Pièges techniques rencontrés
- L'horloge de Git Bash retarde de 5 minutes sur PowerShell. Les chemins `\` sont mangés dans Bash (12 minutes perdues) : **PowerShell pour tout chemin Windows**.
- Ne pas mélanger `victory` et `build:*` dans un même passage de `quick_check` (faux échec en cascade). Le message « game launch ended by itself » cache souvent une étape expirée plus haut.
- Le premier build après modification d'un header prend ~200 s (autres éditeurs ouverts), les suivants 20 s.
- Mesures au repos (S1-S7) et paquet : machine calme, **seuls dans leur créneau** (le paquet bloque les autres sessions). Sautés pendant le test.
- Les fusions dans `main` : pas pendant un test ; le jour d'absence, oui dès que le `quick_check` des parties touchées est vert (décision A), `ai-ia1` en premier. `git checkout -- HeroesOfLiberty.uproject` si la fusion refuse.

## Pipeline en bref

```
Auto_Journey.md          = la procédure (ce fichier, change rarement)
Auto_Journey_Jour.md     = l'état du jour : décisions, programme, tableau des agents, chapitres GDD, bilan
Auto_Journey_File.md     = la file de travail : prochaines étapes de chaque chantier, prêtes à lancer
Auto_Journey_Retours.md  = ce qui attend l'utilisateur : essais, questions, fusions
Tools\AutoJourney\status.ps1 = l'état de tous les worktrees en une commande (étape 0 et veilleur)
```
Historique : `Docs/Archive/Auto_Journey_Test1_2026-10-30.md` (journal et bilan complets du premier test).

## Plan de lancement (sortie de l'étape 0)

L'étape 0 lance `Tools\AutoJourney\status.ps1`, lit la file (`Auto_Journey_File.md`) et rend **un plan de lancement : une ligne par worktree**, soit « lancer <tâche de la file> », soit « ne pas lancer parce que <raison précise : file vide, tout attend l'utilisateur, ou session réellement active> ». **« Pris » n'est pas une raison.** La session principale lance ce plan sans le réinterpréter ; toute exception est écrite dans `Auto_Journey_Jour.md` avec sa raison.

## La file de travail (`Auto_Journey_File.md`)

Une liste ordonnée par worktree, de tâches **déjà préparées** (une phrase, le fichier journal à lire, le premier test rouge, 15-30 min). États : `[ ]` à faire, `[~]` en cours (agent X, heure), `[x]` faite (commit), `[?]` attend une réponse de l'utilisateur (renvoi vers `Auto_Journey_Retours.md`), `[B]` bloquée par une autre tâche. L'agent qui finit une tâche **ajoute la suivante** qu'il voit venir et coche la sienne. Une correction demandée par l'utilisateur (`Reponse:`) devient une ligne `[ ]` en tête de file. **Quand un agent finit, la session principale lance l'agent suivant du même worktree sans réfléchir : première ligne `[ ]` de sa file.**

## Le veilleur (un seul, pas de minuteur par agent)

Au démarrage la session principale crée **un seul rappel récurrent toutes les 20 min** (prompt : « veilleur Auto_Journey : lance `Tools\AutoJourney\status.ps1`, relance tout worktree libre qui a une tâche `[ ]` dans la file, signale tout agent sans commit depuis 45 min »). Les notifications de fin d'agent réveillent déjà la session : les minuteurs « point de 15 min » par agent sont **supprimés** (au test ils tombaient presque tous après le rapport). Le veilleur est la seule sécurité contre un agent bloqué ou une session qui dort. Un rappel disparaît à la fin de la session : le recréer à chaque « commence ton Auto_Journey ».

## Modèle d'énoncé d'un agent (à copier, remplir les <>)

```
Worktree <chemin absolu> (branche <b>, <commits>). Lis <chemin>\CLAUDE.md, la fin de <chemin>\<Chantier>InProgress.md,
la ligne <n> de <chemin>\Auto_Journey_File.md et les `Reponse:` utiles de <chemin>\Auto_Journey_Retours.md.
TÂCHE : <la ligne de la file>. Test rouge d'abord sur la vraie chaîne ; Tools\build.ps1 (-LiveCoding=0) ; quick_check -Parts <parties touchées>.
ENCHAÎNE sans t'arrêter au premier point qui n'exige pas de code ; prends ensuite la ligne suivante de la file.
Ne tranche aucun design : pose-le dans Auto_Journey_Retours.md section 2 en VRAIE question (contexte 2 lignes, options A/B/C
avec leurs conséquences en jeu, recommandation). Si une question bloque la tâche et que les deux options sont peu coûteuses,
prépare les deux derrière un réglage (cvar/option) et passe à la suite.
Un essai n'est listé dans Auto_Journey_Retours.md section 1 que si son scénario Tools/Scenarios/<nom>.txt existe et démarre hors écran.
Commits verts sur ta branche (anglais, sans Co-Authored-By, jamais de push). Fusion dans main : <interdite | autorisée si quick_check vert>.
Tiens à jour EtapeDefini.md (relis avant, tes lignes seulement), ton *InProgress.md, Auto_Journey_File.md (ta ligne), ta ligne du tableau
des agents de Auto_Journey_Jour.md, IN_USE.txt à l'entrée et à la sortie. PowerShell pour tout chemin Windows.
ARRÊT DUR à <HH:MM> (Get-Date PowerShell) ou ~250k tokens. RAPPORT de 10 lignes MAX, dans cet ordre :
fait / tests / commit / reste / NON VÉRIFIÉ (obligatoire : ce qui n'a pas de test ni de preuve) / anomalies du processus.
```

## Builds : ne pas saturer la file

Un premier build après un header prend ~200 s et la file de `Tools\build.ps1` est le goulot. **Au plus 4 agents lancés en même temps ; les suivants sont lancés par le veilleur à mesure que les premiers finissent**, de préférence en alternant les agents qui compilent et ceux qui font des docs, scénarios ou du GDD.

## Fusions dans `main` (agent dédié)

Autorisé le jour d'absence (décision A du 2026-10-30) et pour le test 2 (confirmé le 2026-10-30) ; sinon seulement si l'énoncé les autorise. Un **agent de fusion**, seul, après chaque fin d'agent ou toutes les heures : dans l'ordre fixe de `Auto_Journey_Retours.md` section 3 (`ai-ia1` d'abord), pour chaque branche dont le `quick_check` des parties touchées est vert : `git merge` dans le worktree de la branche (jamais dans la copie principale, qui porte des docs non commités), `quick_check -Parts` des parties touchées, puis fast-forward de `main`. Une correction du **script de test** qui fait échouer les autres branches (ex. `victory`) passe en premier. Rien n'est poussé.

## Les docs de la copie principale

`GDD.md`, `EtapeDefini.md` et les journaux mêlent plusieurs sessions. La session principale commite en fin de boucle **seulement les fichiers qu'elle a elle-même touchés** (`git add <fichier>` précis, jamais `-A`) ; `GDD.md` ne se commite qu'avec l'accord de l'utilisateur (il contient le travail d'autres sessions).

## Au démarrage : « commence ton Auto_Journey »

L'utilisateur lance la journée (ou un test d'une heure) par cette seule phrase. Les chantiers auront avancé depuis la dernière fois. Sans poser de question déjà tranchée :
1. Lire ce fichier, `Auto_Journey_Jour.md` et `Auto_Journey_Retours.md` (la session principale). **Créer le veilleur** (voir « Le veilleur »).
2. **Lancer l'étape 0** : sous-agent en lecture seule qui lance `Tools\AutoJourney\status.ps1`, compare avec la file et rend le **plan de lancement** (une ligne par worktree). Il réécrit aussi le programme de `Auto_Journey_Jour.md` avec l'état réel.
3. **Lire les `Reponse:` de l'utilisateur** dans `Auto_Journey_Retours.md` : c'est la relance qui autorise les corrections et suites qu'elles demandent ; elles deviennent des lignes `[ ]` en tête de file. Les « A trancher » restent ouvertes. Enregistrer ce qui est tranché (décisions du jour, journaux, `EtapeDefini.md`).
4. **Lancer d'un coup le plan de lancement** (modèle d'énoncé), dans la limite de 4 agents en même temps ; le veilleur lance les suivants. Mesures au repos (créneau 1) **avant**, seules, s'il y en a.
5. À chaque agent fini (notification) : lire son rapport de 10 lignes, mettre sa ligne du tableau des agents à jour (`Auto_Journey_Jour.md`), commiter les docs qu'il a touchées (voir « Les docs de la copie principale »), lancer l'agent suivant du worktree (première `[ ]` de la file). Les essais testables et les questions arrivent dans `Auto_Journey_Retours.md` sans que la session principale ait à les recopier.
6. **File vide ou tout en attente de l'utilisateur et du temps de reste (objectif ~8 h)** : prendre dans `EtapeDefini.md` (« À faire ») la ligne suivante qui ne demande ni essai ni décision ; sinon un chapitre du GDD non commencé (section dans `Auto_Journey_Jour.md`). Pas « après le prototype », « avec la cavalerie », « reporté ». Jamais de nouveau worktree.
7. À la fin : paquet et serveur (créneau 3), bilan dans `Auto_Journey_Jour.md` (généré depuis le tableau des agents), `Auto_Journey_Retours.md` à jour et son compteur en tête.

