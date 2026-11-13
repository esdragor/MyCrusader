# Archive : Auto_Journey.md et Auto_Journey_Checklist.md avant le nettoyage du 2026-11-13 soir (copie integrale)

Copie integrale des deux fichiers au commit ecff29a, avant le nettoyage 0a du 2026-11-13 soir (redites, lignes devenues des controles de session_gate.ps1, exemples dates retires). Rien n'est supprime de l'historique : tout le texte retire est ici.

---

## Copie 1 : Auto_Journey.md

# Auto_Journey : la procédure

Ce fichier est la **procédure** (elle change rarement). L'état du jour est dans Auto_Journey_Jour.md, la file de travail dans Auto_Journey_File.md, ce qui attend l'utilisateur dans Auto_Journey_Retours.md. EtapeDefini.md ne garde que la liste des chantiers ; le détail technique reste dans chaque *InProgress.md.

## Sommaire
- **Au démarrage** : section « Au démarrage : « commence ton Auto_Journey » » (sync des worktrees, `session_gate.ps1 -Phase Launch`, étape 0, plan de lancement, clôture).
- **Nettoyage des docs** : étape 0a de « Au démarrage » (demande de l'utilisateur du 2026-11-10 : 4 agents en parallèle archivent dans `Docs\Archive\` sans jamais supprimer, avant tout agent de travail).
- **Règle de durée** : section « Règle de durée » (mieux vaut finir 15 min plus tard que 15 min plus tôt ; gate Launch refuse sous 5 min).
- **Qualité des essais** : section « Qualité des essais » (12 règles ; pré-vol, 3 min, recette indépendante) ; modèle d'énoncé d'un agent plus bas (45 appels vus par l'agent) ; modèles des agents (Fable 5.2 pour planifier et vérifier, Opus 5.5 pour le reste) dans « Comment écrire l'énoncé d'un agent ».
- **Fusions** : « Builds : ne pas saturer la file » (fusion groupée, plafond de 3 agents C++) et « Fusions dans `main` » (pré-vol de fusion).
- **Contrôles et leçons** : `Auto_Journey_Checklist.md` (un contrôle par leçon), `Tools\AutoJourney\session_gate.ps1`, `Tools\AutoJourney\agent_rules.md`.
- **État du jour** : `Auto_Journey_Jour.md` ; **file de travail** : `Auto_Journey_File.md`.
- **Ce qui attend l'utilisateur** : `Auto_Journey_Retours.md` et `Retours\`.
- **Archives** : `Docs/Archive/Auto_Journey_Lecons_jusqu_au_2026-11-09.md`, `Docs/Archive/Auto_Journey_Sections_historiques_2026-11-10.md`, `Docs/Archive/Auto_Journey_Test1_2026-10-30.md`, `Docs/Archive/Auto_Journey_Lecons_2026-11-05_et_corrections_2026-11-11.md`, `Docs/Archive/Auto_Journey_Nettoyage_2026-11-12.md` (mesures et procédure du test 2, rappel d'essais fait, lignes de la check-list devenues des contrôles du gate), `Docs/Archive/Auto_Journey_Checklist_nettoyage_2026-11-13.md` (lignes de la check-list devenues des contrôles du gate ou des règles d'`agent_rules.md`, redites, outils faits, lignes L12 condensées).

## Règles de conduite (réécrites après le test d'1 h du 2026-10-30)

### La session principale est le chef d'orchestre
- **Économiser ses tokens** (demande de l'utilisateur) : les sous-agents font tout le travail (code, tests, compilation, journal, commit, `EtapeDefini.md`). La session principale lance, lit des rapports de 10 lignes, tient ce fichier et `Auto_Journey_Retours.md`. Pas de gros fichiers lus en direct, pas de recherche dans le code, pas de relecture de ce qu'un agent vient d'écrire.
- **L'étape 0 est un sous-agent en lecture seule** (test : 70k tokens, 72 s) qui lance `Tools\AutoJourney\status.ps1` et rend le **plan de lancement** (voir ce titre). Le programme de `Auto_Journey_Jour.md` est celui d'hier soir, pas une vérité.
- **Avant de relayer une consigne entre deux agents, comparer ce que chacun doit produire** (test 3) pour éviter les doublons : si deux agents touchent au même fichier ou à la même question, répartir avant de lancer.
- **Recouper chaque affirmation de l'étape 0 avec le fichier concerné avant de la mettre dans un énoncé** (test 3) : le plan de lancement est un résumé ; un état « libre », « fusionné » ou « fait » se vérifie dans le fichier d'origine (`IN_USE.txt`, `git log`, journal du chantier).
- **Lancer un script une fois de bout en bout avant de dire qu'il est prêt** (test 3) : un script jamais exécuté jusqu'au bout n'est pas prêt.
- **Corriger l'outil partagé (`preflight.ps1`, `check.ps1`) au lieu d'en garder une copie privée, et le dire dans le rapport** (test 3) : une copie privée corrigée laisse le défaut au prochain agent.
- **Ne modifier `Auto_Journey_Retours.md` qu'avec l'outil Edit sur des lignes précises** (pas de script de remplacement, le fichier est en CRLF et édité en parallèle par plusieurs agents), et **recompter l'en-tête à chaque modification**.

### Qui travaille, et sur quoi
- **Un agent par worktree, tous les chantiers du tableau lancés dès le départ, en parallèle.** Au test, 4 chantiers sur 6 avaient été écartés à tort, et il a fallu que l'utilisateur le remarque.
- **« Pris » dans `IN_USE.txt` n'est pas un interdit.** Un worktree « Pris » avec du travail non commité se **reprend sans l'écraser** : `git status`, vérifier que ça compile et que les tests passent, commiter par lots cohérents, jamais `git checkout`/`reset` sur du travail non commité. Seul cas où on ne touche pas : le chantier n'a rien d'ouvert (ex. réseau avant l'essai E5).
- **Quand « À faire » d'`EtapeDefini.md` est vide ou ne convient pas** (il s'est vidé en 1 minute au test) : prendre un **chapitre du GDD non commencé** dont les décisions sont tranchées (Volcan/Catastrophe, Technologies, Arbre de talents...), ouvrir son `*InProgress.md` (plan en étapes de 15 min, premier test rouge), coder la première étape. « Après le prototype » exclut seulement ce que le GDD marque ainsi. Le relevé de 12 chapitres est dans la section « Chapitres du GDD non commencés ».
- **Le GDD (`GDD.md`) est éditable même s'il est modifié et non commité par d'autres sessions** : éditions ciblées après relecture, ne pas commiter le fichier entier (il mêle plusieurs sessions). Avant d'utiliser un numéro de décision, relire § 22 (le « prochain D564 » avait été pris par le rattrapage).
- Jamais de nouveau worktree. Les agents ne **tranchent jamais** une décision de design : ils la posent, **en début de session** (pas au bilan), et si les deux options sont peu coûteuses ils préparent les deux derrière un réglage (cvar/option) au lieu de s'arrêter.

### Comment écrire l'énoncé d'un agent
- Chemins **absolus** partout ; pointer les journaux à lire au lieu de recopier leur contenu ; une tâche précise, des sous-étapes de 15 min, la plus petite d'abord.
- **« Enchaîne sans t'arrêter au premier point qui n'exige pas de code »** (un agent s'était arrêté après avoir constaté qu'un point était déjà bon).
- Arrêt dur à une heure donnée, lue à l'horloge **PowerShell** (`Get-Date`). Plafond : 80 appels d'outils jugés sur `tool_uses`, arrêt à 45 vus par l'agent (L5-3, L9-2 ; voir « Plafond de tokens = plafond d'appels d'outils » ; mesure de tokens du test 2 archivée : `Docs/Archive/Auto_Journey_Nettoyage_2026-11-12.md`). Un agent près de 200k n'est pas relancé : on en prend un neuf qui repart du journal. **Pas de minuteur par agent** : un seul veilleur (voir « Le veilleur »).
- **Modèles (consigne de l'utilisateur du 2026-11-12)** : Fable 5.2 seulement pour la planification (étape 0) et la vérification (recette, relecture) ; Opus 5.5 pour tout le reste ; contexte de la session principale le plus petit possible ; contexte d'un sous-agent sous 300k.
- Rapport final de **10 lignes au plus** : fait / tests / commit / reste / **NON VÉRIFIÉ (obligatoire)** / anomalies du processus.
- Chaque agent : met à jour `EtapeDefini.md` (relire juste avant, ne toucher que ses lignes : aucun conflit observé) , le journal de son chantier, `Auto_Journey_File.md` (sa ligne seulement ; le tableau des agents de `Auto_Journey_Jour.md` est tenu par la session principale : `agent_rules.md` règle 4) à chaque avancée, `IN_USE.txt` à l'entrée et à la sortie (fichier non suivi par git), commite sur la branche de son worktree (anglais, sans `Co-Authored-By`, jamais de push).
- Test rouge d'abord sur la vraie chaîne ; un changement, une compilation, un passage du test ; `Tools\build.ps1` (file d'attente), `-LiveCoding=0` ; `quick_check -Parts` des seules parties touchées. Un test rouge peut venir du **script de test** et non du jeu (deux cas au test : IA du match 1v1 qui regroupait les unités, héros qui dérive d'1 m) : regarder le dernier `Test mark` du journal de jeu avant de toucher au code.

### Les retours de l'utilisateur (voir `Auto_Journey_Retours.md`)
- Tout ce qui attend l'utilisateur est dans **`Auto_Journey_Retours.md`** (1 essais, 2 questions, 3 commits et fusions). Les agents n'y écrivent pas : chacun crée ses fichiers `Retours\Q-<nom>.md` / `Retours\E-<nom>.md` (`agent_rules.md` règles 4, 17, 18), la session principale tient `Auto_Journey_Retours.md`.
- **`Auto_Journey_Retours.md` est léger et sans historique** : dès qu'une réponse de l'utilisateur est reportée dans son chantier (`*InProgress.md`, GDD, file de travail), l'item est **retiré** du fichier et **archivé dans `Docs\Archive\`** (jamais supprimé : consigne du 2026-11-10) ; idem un essai fait et reporté. Avant de supprimer, vérifier que la réponse est bien écrite dans le chantier (sinon l'y écrire d'abord). Le compteur en tête est recalculé à chaque modification.
- **Une question est une vraie question pour lui** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences visibles, recommandation, fichier. Jamais une remarque pour soi-même.
- **Les questions sont posées (décidé le 2026-11-01) : jamais l'agent qui applique la recommandation à la place de l'utilisateur.** Quand c'est rapide (un réglage, une valeur, une forme visible), l'agent construit en plus **un scénario de comparaison `<nom>_compare`** où toutes les options sont à l'écran ou sélectionnables d'une touche (`waitkey` : Entrée passe à l'option suivante, une note dit laquelle est affichée), et la question porte la commande `play.ps1`. L'utilisateur compare puis répond. Ce scénario passe le pré-vol comme un essai (3 min, un PNG par option).
- **Un essai est listé seulement s'il est testable** : scénario `Tools/Scenarios/<nom>.txt` + `<nom>.look`, **`preflight.ps1` PASS**, statut `VU À L'ÉCRAN` (voir « Qualité des essais »), commande `play.ps1` en chemin absolu, gestes précis, durée annoncée (≤ 3 min). Pas de scénario = on le crée d'abord (Tab et Hommes d'armes en surnombre en manquaient).
- L'utilisateur répond par `Reponse: ...` sous chaque item. **`Reponse : A trancher` = il veut en discuter en profondeur** : rien n'est noté comme décidé.
- **Une réponse n'enclenche pas de travail.** On enregistre la réponse dans les docs de chantier (`*InProgress.md`, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction ou la suite de la feature (ex. « arbres trop espacés ») attend dans la file du chantier. **Elle ne se fait qu'au prochain « commence ton Auto_Journey ».**

### Qualité des essais (retour de l'utilisateur du 2026-11-01 sur la journée du 2026-10-31)
Bilan : sur 16 essais, 5 propres ; les 11 autres cassés par le scénario ou le processus, rarement par la fonctionnalité (couleur des neutres, lancier qui tourne, Marché A/Q inversé, IA sans caserne, attentes de 10-15 min, grésillement du brouillard vu 2 fois). Règles, **toutes obligatoires** :
1. **Pré-vol** : `Tools\Scenarios\preflight.ps1 <nom>` doit passer avant qu'un essai soit listé comme prêt (scénario qui atteint « prêt », durée, `note`, aucune erreur au journal, audit `hol.Test.Sanity`, un PNG par point à regarder). Chaque point « à regarder » est déclaré dans `Tools/Scenarios/<nom>.look` (`<nom de capture> | ce que l'utilisateur doit voir`) et pris par un `capture`. **L'agent ouvre chaque PNG (Read) et voit le point** avant d'écrire l'essai.
2. **Deux statuts, jamais mélangés** : `VU À L'ÉCRAN` (listé comme prêt, compté dans l'en-tête de Retours) et `LOGIQUE SEULEMENT` (jamais compté prêt ; le dire en première ligne de l'essai, ou ne pas le lister). Toute mention « non vu à l'écran » dans un essai = `LOGIQUE SEULEMENT`.
3. **3 minutes maximum par essai.** Le scénario met l'utilisateur dans la situation (état préparé : caserne, âge, stade de l'IA, vie perdue) ; pas d'attente morte : un événement attendu est déclenché d'office, ou libéré par `waitkey` (Entrée = étape suivante, ex. `ai_evolving` : une entrée par stade Facile / Normal / Difficile). Interdit : « laisse tourner 10 à 25 min ».
4. **Un essai d'IA ou de système = au moins une partie naturelle** (pas seulement un état injecté, qui prouve la règle et pas l'intégration : c'est ainsi que l'IA sans caserne et les Paysans qui oscillent sont passés).
5. **Recette indépendante** : l'agent qui n'a pas écrit la fonctionnalité joue l'essai comme l'utilisateur (captures à l'appui), chargé de le faire échouer, avant la liste. À défaut, la session principale relit chaque essai contre le pré-vol.
6. **Réglage à l'œil = balayage, jamais une valeur à la fois.** Un scénario montre N valeurs étiquetées côte à côte (ex. espacement des arbres de 0,2 à 1,2 m en 10 échantillons) et l'utilisateur choisit sur l'écran.
7. **Un défaut visuel qui persiste après 2 réglages = remonter à la cause** (post-process, ordre de dessin, couche) avant de proposer une 3ᵉ valeur. Pas de question de réglage (épaisseur, halo) à un utilisateur dont l'œil est le seul juge : un balayage (règle 6).
8. **Défaut commun = ligne `[ ]` en tête de file, avant de relancer les essais concernés.** Un défaut vu dans 2 essais (couleur des neutres, brouillard) est corrigé une fois, à la source, puis les essais sont refaits ; la session principale regroupe ces défauts dès le premier retour négatif.
9. **Libellés, touches, noms viennent du GDD et des conventions du jeu (grille AoE4), jamais inventés** ; un même nom partout (une seule table de textes). Doute = question à l'utilisateur. Un effet de gameplay (point de talent dépensé, achat) doit être **visible** (texte, couleur, son) sinon l'essai est noté `LOGIQUE SEULEMENT`.
10. **Un essai par intégration** : les essais d'un jour sont rejoués sur un build de `main` fusionné (une seule fois, en fin de journée) avant d'être donnés ; sinon ils indiquent leur worktree et ne comptent pas comme intégrés (la rencontre cartes + neutres n'a été vue que par l'utilisateur).
11. **Plafond de volume** : au plus ~6 essais de 3 min par jour donnés à l'utilisateur, classés par priorité ; le reste attend dans la file « prêts mais non donnés », hors de Retours.
12. **Réponses balisées** : l'utilisateur écrit `OK`, `BUG: ...`, `REGLAGE: ...` ou `A trancher`. Une ligne `BUG:` ou `REGLAGE:` devient une ligne `[ ]` de la file (règle 8), faite au prochain « commence ton Auto_Journey ».

### Pièges techniques rencontrés
- **Test 2** : `quick_check` **ne compile pas** (faux vert 530/530 sans les nouveaux tests : `Tools\build.ps1` d'abord) ; les chaînes PowerShell entre guillemets doubles mangent les backticks (relire les lignes écrites par script) ; `IN_USE.txt` écrit **sans accents** (mélange UTF-8/ANSI) ; un scénario échoué laisse l'éditeur ouvert et verrouille `-abslog` (le tuer) ; `make_maps.ps1 -Sizes` ne marche qu'en appel direct `& ...` ; un `git commit`/`merge` qui affiche un avertissement LF/CRLF peut rendre un code non nul alors qu'il a réussi (vérifier `git log -1`) ; les heredocs Python imbriqués dans Bash échouent (Edit/Write ou fichier script). Certaines lignes de la file étaient périmées : voir « Plan de lancement ».
- L'horloge de Git Bash diverge de 5 à 40 minutes de PowerShell (`agent_rules.md` règle 3). Les chemins `\` sont mangés dans Bash (12 minutes perdues) : **PowerShell pour tout chemin Windows**.
- Ne pas mélanger `victory` et `build:*` dans un même passage de `quick_check` (faux échec en cascade). Le message « game launch ended by itself » cache souvent une étape expirée plus haut.
- Le premier build après modification d'un header prend ~200 s (autres éditeurs ouverts), les suivants 20 s.
- Mesures au repos (S1-S7) et paquet : machine calme, créneau à part, **seuls dans leur créneau** (le paquet bloque les autres sessions).
- La fusion dans `main` se fait selon les accords donnés au lancement : sans accord explicite, pas de fusion ni de push ; avec l'accord, dès que le `quick_check` des parties touchées est vert. `git checkout -- HeroesOfLiberty.uproject` si la fusion refuse.
- (Rappel du rejeu de `unit_stats_attack(_enemy)`, `E-audio_panel`, `E-disconnect_review` : fait, archivé dans `Docs/Archive/Auto_Journey_Nettoyage_2026-11-12.md`.)

## Pipeline en bref

```
Tools\Scenarios\preflight.ps1 = le pré-vol d'un essai (durée, journal, audit, un PNG par point du .look)
Auto_Journey.md          = la procédure (ce fichier, change rarement)
Auto_Journey_Jour.md     = l'état du jour : décisions, programme, tableau des agents, chapitres GDD, bilan
Auto_Journey_File.md     = la file de travail : prochaines étapes de chaque chantier, prêtes à lancer
Auto_Journey_Retours.md  = ce qui attend l'utilisateur : essais, questions, fusions
Tools\AutoJourney\status.ps1 = l'état de tous les worktrees en une commande (étape 0 et veilleur)
```
Historique : `Docs/Archive/Auto_Journey_Test1_2026-10-30.md` (journal et bilan complets du premier test). Sections historiques retirées de ce fichier le 2026-11-10 (consigne du 2026-10-30, tests d'une heure) : `Docs/Archive/Auto_Journey_Sections_historiques_2026-11-10.md`.

## Plan de lancement (sortie de l'étape 0)

L'étape 0 lance `Tools\AutoJourney\status.ps1`, lit la file (`Auto_Journey_File.md`) et rend **un plan de lancement : une ligne par worktree**, soit « lancer <tâche de la file> », soit « ne pas lancer parce que <raison précise : file vide, tout attend l'utilisateur, ou session réellement active> ». **« Pris » n'est pas une raison.** **Test 2 : « rien d'ouvert » non plus** : un worktree sans tâche prend un chapitre du GDD non commencé (section de `Auto_Journey_Jour.md`), l'étape 0 doit donc proposer ce chapitre. **L'étape 0 confronte aussi chaque ligne de la file au git** (commits, `EtapeDefini.md`) et marque les lignes périmées (V4 close, « non commité » déjà commité...) pour que les agents ne perdent pas leur temps à les vérifier. La session principale lance ce plan en le **relisant contre la procédure** (deux erreurs du test 2 sont passées faute de relecture) ; toute exception est écrite dans `Auto_Journey_Jour.md` avec sa raison.

## La file de travail (`Auto_Journey_File.md`)

Une liste ordonnée par worktree, de tâches **déjà préparées** (une phrase, le fichier journal à lire, le premier test rouge, 15-30 min). États : `[ ]` à faire, `[~]` en cours (agent X, heure), `[x]` faite (commit), `[?]` attend une réponse de l'utilisateur (renvoi vers `Auto_Journey_Retours.md`), `[B]` bloquée par une autre tâche. L'agent qui finit une tâche **ajoute la suivante** qu'il voit venir et coche la sienne. Une correction demandée par l'utilisateur (`Reponse:`) devient une ligne `[ ]` en tête de file. **Quand un agent finit, la session principale lance l'agent suivant du même worktree sans réfléchir : première ligne `[ ]` de sa file.**

## Le veilleur (un seul, pas de minuteur par agent)

**Cadence (décision de l'utilisateur du 2026-11-10) : 30 min en séance autonome, 5 min en séance interactive ; silencieux si rien n'a changé** (aucun message, aucun lancement). Test 2 : l'ancien veilleur de 20 min n'avait rien trouvé à faire en 2 passages (les notifications de fin d'agent suffisent et la session principale relance). Il reste utile **uniquement comme filet** : signaler un agent sans commit depuis 45 min ; il ne lance rien de lui-même.

Au démarrage la session principale crée **un seul rappel récurrent : toutes les 30 min en séance autonome, toutes les 5 min en séance interactive** (prompt : « veilleur Auto_Journey : lance `Tools\AutoJourney\status.ps1`, relance tout worktree libre qui a une tâche `[ ]` dans la file, signale tout agent sans commit depuis 45 min ; silencieux si rien n'a changé »). Les notifications de fin d'agent réveillent déjà la session : les minuteurs « point de 15 min » par agent sont **supprimés** (au test ils tombaient presque tous après le rapport). Le veilleur est la seule sécurité contre un agent bloqué ou une session qui dort. Un rappel disparaît à la fin de la session : le recréer à chaque « commence ton Auto_Journey ».

## Contrôles mécaniques (2026-11-05, à dérouler : `Auto_Journey_Checklist.md`)

Les leçons de toutes les séances sont maintenant des contrôles : `Tools\AutoJourney\session_gate.ps1 -Phase Launch|Merge|Essay|Close` (avant chaque lancement, avant la fusion, avant de donner une commande `play.ps1`, avant le bilan), `Tools\AutoJourney\agent_rules.md` (les règles que chaque agent lit en premier, rapport à 13 champs vérifié à la réception) et le tableau « Leçons appliquées » du bilan (`### Lecons appliquees <date>`, une ligne par règle de la check-list, preuve à l'appui). **Chaque énoncé d'agent commence par « Lis C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\agent_rules.md et applique-le » et donne l'heure d'arrêt calculée à partir d'un `Get-Date` lu juste avant.**

## Modèle d'énoncé d'un agent (à copier, remplir les <>)

```
Lis C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\agent_rules.md et applique-le (80 appels d'outils max jugés sur `tool_uses`, arrêt à 45 appels VUS PAR TOI (L9-2, 2026-11-07), rapport à 13 champs).
Worktree <chemin absolu> (branche <b>, <commits>). Lis <chemin>\CLAUDE.md, la fin de <chemin>\<Chantier>InProgress.md,
la ligne <n> de <chemin>\Auto_Journey_File.md et les `Reponse:` utiles de <chemin>\Auto_Journey_Retours.md.
TÂCHE : <la ligne de la file>. Test rouge d'abord sur la vraie chaîne ; `Tools\build.ps1 -LiveCoding=0 -Wait` en UN SEUL appel bloquant (il attend son tour dans la file puis compile, rien à sonder : jamais de boucle ni de relevé de la file, jamais de minuteur/cron/Monitor ; sans C++, aucun build) ; quick_check -Parts <parties touchées>.
Tâche sans C++ : la session principale vérifie le lancement par `Tools\AutoJourney\session_gate.ps1 -Phase Launch -NoCpp` (plafond de 3 agents C++ en WARN seulement, L11-7), et l'énoncé dit « sans C++ ».
ENCHAÎNE sans t'arrêter au premier point qui n'exige pas de code ; prends ensuite la ligne suivante de la file.
Ne tranche aucun design : pose-le dans C:\Users\baudo\Documents\MyCrusader\Retours\Q-<nom>.md titré [À RÉPONDRE] en VRAIE question (contexte 2 lignes, options A/B/C
avec leurs conséquences en jeu, recommandation). Si une question bloque la tâche et que les deux options sont peu coûteuses,
prépare les deux derrière un réglage (cvar/option) et passe à la suite.
Un essai n'est listé (C:\Users\baudo\Documents\MyCrusader\Retours\E-<nom>.md titré [À ESSAYER]) que si Tools\Scenarios\preflight.ps1 <nom> passe (scénario + <nom>.look, un PNG ouvert et vu par point, <= 3 min, waitkey pour toute attente) : sinon écris-le « LOGIQUE SEULEMENT » et ne le compte pas prêt. Question de réglage rapide : construis aussi <nom>_compare (toutes les options à l'écran, Entrée = option suivante) au lieu de choisir à sa place. Rejouer un geste : `play.ps1 <nom> -Hidden -Root <worktree> [-TimeoutSec <s>]` ATTEND la fin du jeu (au plus -TimeoutSec, 300 par défaut), le ferme avec ses fils et affiche `hidden game ended after N s` : un seul appel, pas de sondage ni de `stray_processes` à lancer derrière ; `-NoWait` = ancien comportement (rend la main, l'éditeur reste vivant).
Commits verts sur ta branche (anglais, sans Co-Authored-By, jamais de push). Fusion dans main : interdite (l'agent de fusion s'en charge ; dis dans ton rapport que la branche est prête). Au plus 2 questions, regroupées.
Tiens à jour EtapeDefini.md (relis avant, tes lignes seulement), ton *InProgress.md, Auto_Journey_File.md (ta ligne seulement),
IN_USE.txt à l'entrée et à la sortie. PowerShell pour tout chemin Windows.
ARRÊT DUR à <HH:MM> (Get-Date PowerShell) ou 45 APPELS VUS PAR TOI (L9-2 ; remplace les 60 déclarés) (un agent ne voit pas ses tokens et sous-compte ses appels de 10 à 30 % : 77 déclarés = 108 réels ; la session principale juge le seuil de 80 sur `tool_uses` de la notification de fin, au-delà de 100 la tâche suivante part sur un agent neuf). À 45 : commit, journal à jour pour un agent neuf (fait / prochaine étape / test rouge en cours), rapport. Dernier geste : `Tools\AutoJourney\stray_processes.ps1 -Root <worktree> -Stop` (champ PROCESSUS_DETACHES). Un essai porte les statuts `GESTE: réel / interne` et `SON: entendu / non entendu` (gabarit `Tools\AutoJourney\E_template.md`) ; `session_gate.ps1 -Phase Close -Exclude <worktree>` écarte un worktree hors périmètre. RAPPORT de 13 champs (liste dans `agent_rules.md`, dont APPELS_DECLARES et PROCESSUS_DETACHES), résumé en 10 lignes MAX, dans cet ordre :
fait / tests / commit / reste / NON VÉRIFIÉ (obligatoire : ce qui n'a pas de test ni de preuve) / anomalies du processus / PROCESSUS_DETACHES.
```

## Builds : ne pas saturer la file

Un premier build après un header prend ~200 s et la file de `Tools\build.ps1` est le goulot ; le plafond de 3 agents C++ ci-dessous fait la règle (mesure du test 2 archivée : `Docs/Archive/Auto_Journey_Nettoyage_2026-11-12.md`). Un agent qui finit tôt est **relancé immédiatement** (première ligne `[ ]` de sa file, sinon un chapitre du GDD) tant qu'il reste plus de 15 min avant l'arrêt dur ; en dessous, on le laisse libre. Dans l'énoncé : « c'est l'heure d'arrêt qui compte, pas ton sentiment d'avoir fini » (Elrond s'était arrêté 33 min trop tôt).

**Plafond de questions (test 2)** : le temps de lecture de l'utilisateur est le vrai goulot (le test a posé une quinzaine de questions en 1 h, dont 6 pour la seule ouverture de Technologies). Un agent pose **au plus 2 questions par chantier et par séance**, regroupées, classées par ce qu'elles débloquent ; le reste est préparé derrière un réglage ou noté dans le journal. Un chapitre ouvert sans question tranchée s'arrête au plan et aux règles pures (pas de bâtiment, pas d'écran).

**File de build : règles de la séance du 2026-11-03** (7 agents en parallèle : attentes de 10 à 35 min, builds orphelins, file FIFO sans priorité). `Tools\build.ps1` tient maintenant la file lui-même (tickets dans `Docs\BuildQueue.state\`, statistiques dans `Docs\BuildQueue.csv`, résumé par `Tools\build_queue_report.ps1`) :
- **Un seul build en attente par worktree** : un nouveau build du même `-Root` remplace celui qui attend (l'ancien sort en code 3, « superseded by a newer build of the same copy ») ; un build en cours de compilation n'est jamais tué. Un build en attente dont le processus lanceur a disparu (agent terminé) quitte la file (code 4). Il suffit donc de relancer `build.ps1` ; ne jamais empiler des builds « au cas où ». **`build.ps1 -Wait` (L11-2, 2026-11-11)** : UN appel bloquant qui attend son tour puis compile (l'agent n'a plus rien à sonder) ; un build `-Wait` n'est jamais remplacé (plus de code 3, le build plus récent du même worktree se met derrière lui), la règle des orphelins (code 4) reste. Modèle d'énoncé : `Tools\build.ps1 -LiveCoding=0 -Wait`. **`session_gate.ps1 -Phase Launch -NoCpp`** (L11-7) pour une tâche sans C++ : le plafond de 3 agents C++ ne compte alors pas cette tâche (WARN au lieu de FAIL) ; sans `-NoCpp`, il se calcule depuis les tickets vivants de `Docs\BuildQueue.state` et `IN_USE.txt`.
- **Priorité** : `-Priority merge` (ou `HOL_BUILD_PRIORITY=merge`) passe avant tous les builds normaux, jamais avant le build en cours. Ordre de la file : merge, puis les worktrees qui n'ont rien compilé depuis 30 min, puis premier arrivé premier servi.
- **La fusion groupée est la règle par défaut** : toutes les branches prêtes dans UN seul worktree, un seul build (`-Priority merge`), un seul `quick_check`, puis avance rapide de `main` (le 2026-11-03 au matin, 7 branches = 1 build de 74 s). Une fusion branche par branche n'est qu'une exception à justifier.
- **Plafond d'agents C++ : valeur par défaut à régler, pas une règle ferme** : au plus 3 agents qui compilent en même temps. À ajuster d'après `Docs\BuildQueue.csv` (relever le plafond si l'attente médiane de `build_queue_report.ps1` ne s'allonge pas avec plus d'agents). Les agents de docs/GDD/script/scénarios ne comptent pas. On peut lancer les 7 worktrees d'emblée : ceux qui dépassent le plafond commencent par un travail sans compilation (docs, tests Angelscript, lecture, scénarios) et ne passent au C++ qu'à la libération d'une place.
- **Économiser les builds** : `git merge main` dans un worktree seulement avant le rapport, pas en cours de lot ; un build par lot de changements cohérents, pas par fichier, SAUF le premier test rouge ; préférer les règles pures en Angelscript (pas de build UBT).
- **Modèle d'énoncé d'une fusion** (à copier) : « Worktree <libre> : fusionne d'un coup <b1, b2, ...> (ordre de `Auto_Journey_Retours.md` section 3), résous les conflits, UN build `Tools\build.ps1 -Priority merge`, UN `quick_check -Parts <union des parties touchées>`, puis `git -C C:\Users\baudo\Documents\MyCrusader merge --ff-only <branche>`. Si un fichier non commité de la copie principale bloque, ne l'écrase pas, rapporte. »

## Fusions dans `main` (agent dédié)

**Test 2, leçon : les fusions sont le goulot de fin de séance.** `main` bouge pendant les tests (3 refusions + rebuilds chez plusieurs agents) et le `GDD.md` non commité de la copie principale bloque tout fast-forward d'une branche qui le touche (3 branches sur 7 non fusionnées). Donc : (1) **avant la journée, commiter `GDD.md` avec l'accord de l'utilisateur** (ou lui demander de le mettre à l'abri) ; (2) les agents **ne fusionnent pas eux-mêmes** : ils commitent sur leur branche et le disent ; (3) **un seul agent de fusion** en fin de créneau (ou toutes les heures), dans l'ordre fixe, qui refusionne `main` en dernier juste avant chaque fast-forward ; (4) un chantier qui change des chemins/cartes ne se fusionne qu'après ses vérifications (chemins, `crossings.ps1`, `aiduel` pour les cartes).

La fusion groupée de « Builds : ne pas saturer la file » est la règle par défaut (procédure branche par branche du test 2 archivée : `Docs/Archive/Auto_Journey_Nettoyage_2026-11-12.md`). Restent en vigueur : fusion seulement si l'énoncé l'autorise (accord donné au lancement), jamais dans la copie principale, une correction du **script de test** qui fait échouer les autres branches passe en premier, rien n'est poussé.

**Pré-vol de fusion (2026-11-04, séance PM : fast-forward bloqué 20 min par 10 scripts d'une autre session et une Cavalerie fusionnée sans compilation)** : avant de lancer l'agent de fusion, la session principale lance `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\preflight_merge.ps1 -Branch <branche>` (lecture seule : la branche contient-elle main, quels fichiers non commités de la copie principale bloquent le fast-forward et à quelle heure ils ont été modifiés, dernier commit de main). Un blocage se règle AVANT : demander à l'autre session de commiter, ou accord de l'utilisateur. Règles : (1) aucune session ne laisse de fichier suivi modifié non commité dans la copie principale (un fichier commun se commite tout de suite) ; (2) rien n'entre dans `main` sans `build.ps1` PASS et `quick_check -Parts tests` entièrement vert sur le résultat de la fusion : une branche « non compilée » (assets à régénérer) ne fusionne pas.

## Les docs de la copie principale

`GDD.md`, `EtapeDefini.md` et les journaux mêlent plusieurs sessions. La session principale commite en fin de boucle **seulement les fichiers qu'elle a elle-même touchés** (`git add <fichier>` précis, jamais `-A`) ; `GDD.md` ne se commite qu'avec l'accord de l'utilisateur (il contient le travail d'autres sessions).

## Mode interactif (demandé par l'utilisateur le 2026-11-01, essayé depuis ; jamais par défaut)

Lancé seulement par « commence ton Auto_Journey interactif » (ou à la demande pour un test d'1 h). **But : fermer une boucle (question ou essai -> réponse -> correction relancée) en minutes au lieu d'un jour.** Ce mode **suspend, pour sa durée seulement**, la règle « une réponse n'enclenche pas de travail » : l'utilisateur est présent et répond pendant la séance.
- **Marqueurs dans `Auto_Journey_Retours.md`** : un agent qui pose une question la titre `[À RÉPONDRE]` ; un essai qui passe le pré-vol est titré `[À ESSAYER]` (statut `VU À L'ÉCRAN` obligatoire). L'en-tête compte les deux marqueurs (« N à répondre, M à essayer »). Un agent **n'attend jamais** une réponse : il prépare le provisoire derrière un réglage et passe à la ligne suivante de sa file.
- **Où l'utilisateur répond** : en ligne dans le fichier (`Reponse: OK` / `BUG: ...` / `REGLAGE: ...` / `A trancher`) **ou dans la conversation** (la session principale l'écrit dans le fichier). Une réponse = une ligne, sans réécrire un bloc, pour ne pas entrer en conflit avec les agents qui éditent le fichier (outil Edit, lignes précises).
- **Le veilleur passe toutes les 5 minutes** (contre 30 min en séance autonome ; décision de l'utilisateur du 2026-11-10) : il cherche les `[À RÉPONDRE]` / `[À ESSAYER]` dont la `Reponse:` est devenue non vide, puis pour chacune : (1) enregistre la réponse dans le chantier (`*InProgress.md`, GDD, `EtapeDefini.md`) et retire le marqueur ; (2) `OK` : l'item est rangé et supprimé de Retours ; (3) `BUG:` / `REGLAGE:` / une décision qui demande du travail : ligne `[ ]` en tête de la file **et relance immédiate de l'agent du worktree** avec cette ligne ; (4) `A trancher` : l'item reste, rien n'est décidé.
- **Prévenir l'utilisateur** : à chaque `[À RÉPONDRE]` ou `[À ESSAYER]` nouveau, la session principale l'écrit dans la conversation (une ligne : quoi, la commande `play.ps1` pour un essai), et groupe les messages (au plus un toutes les 10 minutes).
- **Limites inchangées** : au plus 2 questions par chantier ; essais de 3 min ; ~6 essais par jour ; pas de fusion ni de push sans accord explicite.
- **Mesures du test** (à écrire au bilan) : délai entre `[À RÉPONDRE]`/`[À ESSAYER]` et la réponse ; délai entre la réponse et la relance de l'agent ; nombre de boucles fermées par heure ; nombre de fois où l'utilisateur a été dérangé ; conflits d'édition sur Retours.

Leçons de toutes les séances : `Auto_Journey_Checklist.md` (archives listées au Sommaire).

## Au démarrage : « commence ton Auto_Journey »

L'utilisateur lance la journée (ou un test d'une heure) par cette seule phrase. Les chantiers auront avancé depuis la dernière fois. Sans poser de question déjà tranchée :
0a. **Nettoyage des docs (demande de l'utilisateur du 2026-11-10, à chaque début de séance)** : 4 agents en parallèle sur des blocs de fichiers disjoints (`Auto_Journey_Jour.md` + `Auto_Journey_Retours.md` ; `Auto_Journey_File.md` ; dossier `Retours\` ; `Auto_Journey.md` + `Auto_Journey_Checklist.md`) qui ARCHIVENT dans `Docs\Archive\` sans jamais supprimer, une section douteuse reste et le doute est posé à l'utilisateur en un tableau ; commits par nom de fichier ; aucun agent de travail n'est lancé avant que ce nettoyage soit commité. Mémoire : auto-journey-start-cleanup.
0. **Mise à jour des worktrees AU DÉBUT (L8-9, demande de l'utilisateur du 2026-11-06)** : AVANT le premier lancement, `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\sync_worktrees.ps1` (aucun agent ne tourne) pour que tous les worktrees partent à jour avec `main` et le restent le plus longtemps possible ; les sauts (travail non commité) et conflits sont notés dans `Auto_Journey_Jour.md`. **Déroulé obligatoire (2026-11-05)** : lire `Auto_Journey_Checklist.md` ; lancer `session_gate.ps1 -Phase Launch -End <heure de fin>` avant CHAQUE lancement d'agent (un FAIL = pas de lancement) ; écrire le tableau `## Agents du <date>` dans `Auto_Journey_Jour.md` ; poser l'alarme « fin - 15 min » ; avant chaque commande `play.ps1` donnée à l'utilisateur : `-Phase Essay -Root <worktree>` ; avant l'agent de fusion : `-Phase Merge -Branches ...` ; avant le bilan : `-Phase Close` puis le tableau `### Lecons appliquees <date>`.
1. Lire ce fichier, `Auto_Journey_Jour.md` et `Auto_Journey_Retours.md` (la session principale). **Créer le veilleur** (voir « Le veilleur » : toutes les 30 min en autonome, toutes les 5 min en interactif, silencieux si rien n'a changé).
2. **Lancer l'étape 0** : sous-agent en lecture seule qui lance `Tools\AutoJourney\status.ps1`, compare avec la file et rend le **plan de lancement** (une ligne par worktree). Il réécrit aussi le programme de `Auto_Journey_Jour.md` avec l'état réel.
3. **Lire les `Reponse:` de l'utilisateur** dans `Auto_Journey_Retours.md` : c'est la relance qui autorise les corrections et suites qu'elles demandent ; elles deviennent des lignes `[ ]` en tête de file. Les « A trancher » restent ouvertes. Enregistrer ce qui est tranché (décisions du jour, journaux, `EtapeDefini.md`).
4. **Lancer d'un coup le plan de lancement** (modèle d'énoncé), dans la limite de **3 agents C++** en même temps (« Builds : ne pas saturer la file », L3-1 du 2026-11-03 : les autres démarrent sans compiler) ; les notifications de fin et le veilleur lancent les suivants. Mesures au repos (créneau 1) **avant**, seules, s'il y en a.
5. À chaque agent fini (notification) : lire son rapport de 10 lignes, mettre sa ligne du tableau des agents à jour (`Auto_Journey_Jour.md`), commiter les docs qu'il a touchées (voir « Les docs de la copie principale »), lancer l'agent suivant du worktree (première `[ ]` de la file). Les essais testables et les questions arrivent dans `Auto_Journey_Retours.md` sans que la session principale ait à les recopier.
6. **File vide ou tout en attente de l'utilisateur et du temps de reste (objectif ~8 h)** : prendre dans `EtapeDefini.md` (« À faire ») la ligne suivante qui ne demande ni essai ni décision ; sinon un chapitre du GDD non commencé (section dans `Auto_Journey_Jour.md`). Pas « après le prototype », « avec la cavalerie », « reporté ». Jamais de nouveau worktree.
7. **Avant de donner les essais** (« Qualité des essais ») : fusionner, rejouer le pré-vol de chaque essai sur le build de `main`, faire passer la recette indépendante, trier les ~6 essais prioritaires de 3 min ; les autres restent « prêts mais non donnés ».
8. **À la fin : mise à jour des worktrees (L8-9)** après la dernière fusion et le dernier commit de docs, quand aucun agent ne tourne : `sync_worktrees.ps1` (`-NoBuild` si le temps manque), puis `session_gate.ps1 -Phase Close -Exclude <hors périmètre>` qui échoue (FAIL) pour tout worktree encore en retard sur `main`. Puis : paquet et serveur (créneau 3), bilan dans `Auto_Journey_Jour.md` (généré depuis le tableau des agents), `Auto_Journey_Retours.md` à jour et son compteur en tête.


## Règle de durée (2026-11-05 après-midi, demandée par l'utilisateur, « le plus important »)
**Mieux vaut une séance qui se termine 15 min plus tard que 15 min plus tôt, sauf si le prompt de lancement dit le contraire** (ex. « arrêt dur strict »). Elle remplace la lecture stricte de « l'arrêt dur est un contrat » (leçon du 2026-11-04, 1 h 45 de dépassement) : le dépassement toléré est **de 15 min au plus** et jamais décidé après coup sans le dire ; au-delà, on demande. Conséquences : (1) l'arrêt dur des agents = l'heure de fin annoncée (plus « fin − 15 min ») ; (2) `session_gate.ps1 -Phase Launch` refuse seulement s'il reste moins de 5 min (avant : 20, la séance du 2026-11-05 est restée vide 30 min) ; (3) l'alarme de clôture reste à « fin − 15 min » pour préparer la fusion et le bilan, pas pour arrêter les agents ; (4) un agent qui rend la main tôt est relancé tant qu'il reste du temps, avec une tâche `sans C++` de la file si les 3 places C++ sont prises.


---

## Copie 2 : Auto_Journey_Checklist.md

# Auto_Journey : check-list des leçons (chaque leçon = un contrôle mécanique)

Créée le 2026-11-05 à la demande de l'utilisateur : des axes d'amélioration revenaient de séance en séance sans être appliqués (plafond de tokens écrit trois fois, dépassé par 4 agents). **Règle de méthode : une leçon n'existe que si elle a un contrôle qu'on peut exécuter ou compter.** Quatre mécanismes :

- **GATE** = `Tools\AutoJourney\session_gate.ps1 -Phase Launch|Merge|Essay|Close` (script, sortie PASS/WARN/FAIL, code 1 s'il y a un FAIL). La session principale le lance : `Launch` avant CHAQUE lancement d'agent, `Merge` avant l'agent de fusion, `Essay` avant de donner une commande `play.ps1`, `Close` avant le bilan.
- **AGENT** = règle numérotée de `Tools\AutoJourney\agent_rules.md`, que chaque énoncé fait lire en premier.
- **RAPPORT** = champ du rapport final à 13 champs (voir `agent_rules.md`) que la session principale vérifie à la réception.
- **SESSION** = ligne que la session principale coche elle-même dans le tableau « Leçons appliquées » du bilan.

**Bilan de fin de séance (obligatoire, vérifié par `-Phase Close`)** : dans `Auto_Journey_Jour.md`, un titre `### Lecons appliquees <AAAA-MM-JJ>` suivi d'une ligne par règle ci-dessous : `<ID> : appliquee / non appliquee : <preuve (nombre, heure, SHA, sortie)>`. Une règle « non appliquée » devient une ligne `[ ]` de `Auto_Journey_File.md` (ou un durcissement du contrôle).

Origines : T2 = test 2 du 2026-10-30 ; Q = « Qualité des essais » du 2026-11-01 ; L1 = leçons du 2026-11-01 ; L3 = 2026-11-03 ; L4 = 2026-11-04 ; L5 = 2026-11-05 ; texte intégral des leçons L1, L3 et L4 archivé dans `Docs\Archive\Auto_Journey_Lecons_jusqu_au_2026-11-09.md`.

**Devenues des contrôles automatiques de `session_gate.ps1`** (lignes archivées le 2026-11-12 dans `Docs\Archive\Auto_Journey_Nettoyage_2026-11-12.md`, règles toujours en vigueur, cochées au bilan comme les autres avec la sortie du gate pour preuve) : L6-1 et L8-7 (`Launch` FAIL sous 3 lignes `[ ]` `sans C++`), L6-3 (`-Phase Report` à la réception d'un rapport), L7-8 (`Close -Exclude`), L8-3 (heure de fin lue dans `Docs\SessionEnd.txt`), L9-3 (`Essay` : seule la ligne `branches:` compte, nom de scénario accepté), L11-1 (`Launch -Scenarios` : binaire périmé = FAIL), L11-7 (`Launch -NoCpp`).

**Devenues des contrôles de `session_gate.ps1` (2026-11-13)** (lignes archivées dans `Docs\Archive\Auto_Journey_Checklist_nettoyage_2026-11-13.md`, règles toujours en vigueur, cochées au bilan comme les autres) : L6-5 (après `git merge main`, `build.ps1` avant tout `quick_check` : `-Phase Report` WARN binaire périmé), L7-4 (aucun processus détaché à la fin d'un agent : AGENT 20, champ `PROCESSUS_DETACHES`, gate WARN « processus detaches restants » ; la session les arrête avant l'agent suivant du même worktree), L7-6 (`GESTE: réel / interne` et `SON: entendu / non entendu` dans chaque `E-*.md` : AGENT 18, `Close` WARN sinon), L8-1 (essai joué depuis un worktree qui porte le tip de chaque branche citée : `Essay` ; la commande `play.ps1` pointe le worktree qui porte `main` compilé), L9-5 (`commit_docs.ps1` avant chaque `Launch` et `Merge` ; `Merge` distingue docs (WARN) et code (FAIL)), L10-1 (aucun `play.ps1` sans `-Hidden` dans un énoncé : AGENT 0, `Launch` WARN par `no_window_test.ps1`).

**Devenues des règles d'`agent_rules.md` (2026-11-13, même archive)** : L8-5 (AGENT 8 : `known_reds.txt` lu par `quick_check`, « rouge connu » distinct de « REGRESSION »), L8-6 (AGENT 6 : Write/Edit, jamais de heredoc ; `.gitattributes` fait, voir L9-4 ; reste à appliquer par l'énoncé : variables PowerShell de noms distincts, `$r`/`$R`).

**Redondantes ou outils faits (2026-11-13, même archive)** : L7-1 et L9-2 (= ligne « L1-1, L3-6, L4-7, L5-3 » ci-dessous et AGENT 2), L7-3 (fait : `Tools\Scenarios\EditorSlot.ps1`, au plus 2 éditeurs, pris par `Scenarios\check.ps1`), L9-7 et L10-5 (fait : `ab_matrix.ps1`, PASS k/N ; leur règle est condensée dans la ligne L11-10), L11-5 (fait : `commit_docs.ps1` couvre Placeholders*, *InProgress.md suivis, `Docs\SessionEnd.txt`), L11-6 (fait : `play.ps1 -Hidden` ferme ses éditeurs, `play_hidden_test.ps1` ; règle AGENT 0).

## Avant le lancement
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| T2-1 | Étape 0 par sous-agent en lecture seule ; plan relu contre la procédure ; « rien d'ouvert » interdit | `Auto_Journey_Jour.md` contient le plan de lancement : une ligne par worktree (lancer / raison précise) | SESSION |
| T2-2 | Un agent par worktree, tous lancés d'emblée ; « Pris » n'est pas un interdit | tableau des agents : une ligne par worktree libre, ou la raison écrite | SESSION |
| L3-12 | Étape 0 sautée seulement si la file porte déjà les tâches du jour | la file contient des `[ ]` datés du jour | SESSION |
| L5-1, L3-10, L4-2 | Heure lue à `Get-Date` avant CHAQUE lancement ; aucun lancement s'il reste moins de 5 min (règle de durée du 2026-11-05 PM ; ancien seuil : 20 min) | `session_gate.ps1 -Phase Launch -End HH:MM` (FAIL si < 5 min) | GATE |
| L5-2 | Tableau agent -> id -> worktree -> branche -> tâche, écrit AU LANCEMENT, relu avant chaque `SendMessage` | titre `## Agents du <date>` dans `Auto_Journey_Jour.md` | GATE |
| L3-1, L4-4 | Plafond de 3 agents C++ vérifié avant chaque lancement et écrit DANS l'énoncé | `Launch` : tickets de `Docs\BuildQueue.state` < 3 ; énoncé « sans C++ » sinon | GATE + AGENT 10 |
| L1-10 | Docs de la copie principale commités ; rien de suivi modifié ni de `*InProgress.md` non suivi | `Launch` : FAIL sur `*InProgress.md` non suivi, WARN sur fichier suivi modifié | GATE |
| L5-7 | Accord de commiter les dossiers non suivis demandé AU LANCEMENT | première question de la séance si le gate le signale | SESSION |
| L4-1, L4-3 | L'arrêt dur est un contrat ; alarme de fin posée au lancement ; prolongation demandée 15 min avant, max 2 cycles | `CronList` montre l'alarme « fin - 15 min » ; heure de fin écrite dans le titre du tableau ; **assoupli par la règle de durée du 2026-11-05 PM** : arrêt dur des agents = l'heure de fin, dépassement toléré 15 min au plus, au-delà on demande | SESSION |
| L3-8, L4-9, Q5, L1-14 | Agent d'intégration NOMMÉ au lancement ; il rejoue les essais après la fusion | ligne `intégrateur` du tableau des agents | SESSION |
| L3-9 | La session principale se relance vers 200k : compter ses propres appels d'outils | à 150 appels de la session principale : écrire le point de reprise dans `Auto_Journey_Jour.md` | SESSION |
| T2-3 | Énoncé : chemins absolus, « enchaîne sans t'arrêter », arrêt dur, rapport fixe | l'énoncé commence par « lis Tools\AutoJourney\agent_rules.md » + l'heure d'arrêt calculée par `Launch` | AGENT |

## Pendant les agents
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L1-1, L3-6, L4-7, L5-3 | Un agent est remplacé à 200k tokens, jamais plus tard ; bug dur = agent neuf | **80 appels d'outils** max (jugés sur `tool_uses`), arrêt à **45 appels VUS PAR L'AGENT** dans l'énoncé (L9-2 du 2026-11-07 ; avant : 60 déclarés) ; `tool_uses` de la notification > 100 = hors règle, tâche suivante sur un agent NEUF (jamais `SendMessage` pour reprendre) | AGENT 2 + RAPPORT `APPELS_DECLARES` |
| L1-2 | Commit par lot vert, au plus 30 min sans commit, fichiers par nom | `RAPPORT COMMITS` (dernier commit à HH:MM) ; `status.ps1` à chaque tour signale 45 min sans commit | AGENT 14 |
| L1-3 | `git merge main` avant le rapport | `RAPPORT merge_main_fait` | AGENT 15 |
| L3-2, L4-5 | Build C++ lancé dans les 15 premières minutes, en `run_in_background`, agent vivant jusqu'à sa fin ; jamais de code non compilé rendu comme fini | `RAPPORT PREMIER_BUILD`, `BUILD` ; « NON COMPILÉ » en tête sinon | AGENT 11 |
| L3-5 | Un seul lot de headers par agent avant le premier build | `RAPPORT PREMIER_BUILD` | AGENT 11 |
| L1-8 | Un seul enregistrement de statut par agent | `RAPPORT` : `*InProgress.md` + ligne d'`EtapeDefini.md` | AGENT 5 |
| L1-9, L4-8 | Numéros D réservés ; question sans réponse = valeur par défaut déjà codée | agents n'écrivent pas `GDD.md` ; fichier `Q-*.md` porte « valeur par défaut codée » | AGENT 4, 17 |
| L1-7 | Toute question : option « Autre », recommandation, valeurs par défaut, au plus 2 par chantier | `RAPPORT QUESTIONS` | AGENT 17 |
| L5-6 | Questions groupées dans UN tableau (question, recommandée, effet), raccourci « recommandées partout » | message de la session principale | SESSION |
| T2-5 | `quick_check` ne compile pas : `build.ps1` d'abord | `RAPPORT TESTS` après `BUILD: PASS` | AGENT 8 |
| T2-6, L5-8 | Un script n'est prêt qu'après un passage de bout en bout ; `net` rouge sans changement = relancer deux fois | sortie collée au rapport | AGENT 8, 9 |
| T2-7, L1-5 | `Auto_Journey_Retours.md` seulement par `Edit` de lignes précises (un fichier par item dans `Retours\`) | agents écrivent `Retours\Q-*.md` / `E-*.md` | AGENT 17 |
| L1-11 | Machine : l'utilisateur joue depuis un build figé ; heure à PowerShell | commande `play.ps1` pointe un worktree validé par `Essay` | GATE |
| L3-4 | Écriture au bon endroit ; `git status` de la copie principale inchangé | `RAPPORT GIT_STATUS_PRINCIPALE` + `Close` | AGENT 4 + GATE |
| L4-12, T2 | Outil partagé corrigé au lieu d'un contournement privé | `RAPPORT ANOMALIES_PROCESSUS` cite l'outil corrigé | AGENT 9 |
| L4-13 | `Intermediate` périmé : supprimer UHT de CE worktree | AGENT 12 | AGENT |
| L3-11, L5-5 | Régénération des cartes seule, éditeur seul, `-Multiprocess`, mesure de perf, jamais mêlée à un réglage de budget | tâche dédiée dans le tableau des agents ; AGENT 13 | AGENT + SESSION |
| L3-7, L1-4 | Veilleur : état déjà vu persisté, silencieux si rien n'a changé | ignorer le premier tour | SESSION |
| T2-4 | Au plus 2 questions par chantier et par séance | compte des `Q-*.md` par chantier | SESSION |

## Essais
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| Q1 | Pré-vol `preflight.ps1` PASS, `.look` présent, chaque PNG ouvert et vu | `RAPPORT ESSAIS` : « preflight PASS, PNG ouverts » | AGENT 18 + RAPPORT |
| Q2 | Deux statuts jamais mélangés : `VU À L'ÉCRAN` / `LOGIQUE SEULEMENT` | `RAPPORT ESSAIS` | AGENT 18 |
| Q3 | 3 minutes max, pas d'attente morte (`waitkey`) | durée mesurée par le pré-vol | AGENT 18 |
| Q4 | Un essai d'IA/système = au moins une partie naturelle | `RAPPORT ESSAIS` | AGENT 18 |
| Q5, L5-4 | **Recette indépendante, geste RÉEL, effet mesuré** pour tout essai ; REGLAGE/BUG rejoué par un autre agent avant la commande | ligne `intégrateur` + `RAPPORT TEST_ROUGE assertit l'effet : oui` ; `Essay` avant la commande | GATE + AGENT 7, 19 |
| Q6, Q7 | Réglage à l'œil = balayage ; défaut persistant après 2 réglages = remonter à la cause | `RAPPORT ESSAIS` (nombre de réglages tentés) | AGENT 18 |
| Q8 | Défaut commun = ligne `[ ]` en tête de file avant de relancer les essais concernés | `Auto_Journey_File.md` | SESSION |
| Q9 | Libellés et touches du GDD / grille AoE4 ; effet de gameplay visible sinon `LOGIQUE SEULEMENT` | AGENT 18 | AGENT |
| Q10, L4-10 | Un essai n'est donné qu'après fusion et build vérifié de `main` ; la commande pointe ce worktree | `session_gate.ps1 -Phase Essay -Root <worktree>` PASS (aucun écart de code avec main, propre, binaire plus récent que le dernier commit de code) | GATE |
| Q11 | ~6 essais de 3 min par jour au plus ; le reste « prêt mais non donné » | compte des `[À ESSAYER]` | SESSION |
| Q12, L1-6 | Réponses balisées et identifiées : `E1 OK`, `Q2 A`, `BUG:`, `REGLAGE:`, `A trancher` | la session principale enregistre ; `Close` signale une réponse non traitée dans `Retours\` | GATE |
| L4-11 | Plusieurs cartes/variantes = un scénario de revue (Entrée = suivante) | AGENT 18 | AGENT |

## Fusion et fin de séance
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L3 / T2 | Fusion groupée : toutes les branches prêtes, UN build `-Priority merge`, UN `quick_check`, avance rapide de `main` | énoncé de fusion de `Auto_Journey.md` | SESSION |
| L4-6 | La fusion groupée démarre à « fin - 30 min » (assoupli par L7-7 et L8-8 : dès que 3 branches sont vertes, branche C++ d'abord) | alarme + heure | SESSION |
| Pré-vol | Pré-vol de fusion de chaque branche ; rien n'entre dans `main` sans build PASS et `tests` tout vert sur le résultat | `session_gate.ps1 -Phase Merge -Branches a,b,c` | GATE |
| L1-3 | Les agents ont fusionné `main` avant le rapport : l'agent de fusion ne rejoue que `tests,aiserver,aiduel` | `RAPPORT merge_main_fait` | RAPPORT |
| L4-14 | Fin de séance : tous les worktrees remis à jour (`sync_worktrees.ps1`) ; travail non commité d'abord commité | `session_gate.ps1 -Phase Close` : FAIL si fichier suivi non commité, WARN si retard sur `main` | GATE |
| L1-13 | Test rouge vérifiable par l'ordre des commits | `RAPPORT TEST_ROUGE` : SHA du test antérieur au correctif | RAPPORT |
| L5-bilan | Chaque leçon « appliquée / non appliquée » avec preuve | `Close` FAIL sans le titre `### Lecons appliquees <date>` | GATE |

## Leçons du 2026-11-06 (séance d'1 h du 2026-11-05 après-midi) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L6-2 | La session principale ne lit jamais la file en entier (42k tokens) : l'étape 0 rend un extrait | consigne de l'étape 0 ; ligne de nettoyage de la file | SESSION |
| L6-4 | Un bug rapporté se rejoue par le scénario et le geste exacts de l'utilisateur ; test « sur le geste réel » = scénario `click` | ligne `[ ]` « Scénario click » ; question de repro posée AVANT de lancer l'agent | SESSION + AGENT |
| L6-6 | Mesures répétées sur un binaire copié, jamais sur celui que `build.ps1` doit écraser | ligne `[ ]` des mesures | AGENT |

## Outils encore à construire (lignes `[ ]` de `Auto_Journey_File.md`)
- L1-12 (a) capture en séquence au pas de simulation (prouver une animation) ; (b) détection par le pré-vol d'un panneau qui en cache un autre ; (c) `check.ps1` qui échoue quand l'audit `Sanity` échoue.
- L3-3 : journaux nommés de `quick_check` (`net`, `aiserver`...) sans `Get-FreshLog` partout.
- Un script `launch_agent` qui lit `Launch`, écrit la ligne du tableau des agents et génère l'énoncé (heure d'arrêt, rapport fixe) : supprimerait le reste de dépendance à ma mémoire.

## Leçons du 2026-11-05 soir (séance autonome 19:12-21:38) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L7-2 | Un lancement ne vérifie pas l'état réel du worktree (DLL périmée, SHA périmés donnés à la fusion) | session_gate.ps1 -Phase Launch -Root <worktree> : FAIL si la DLL du worktree est plus ancienne que le dernier commit de code de main ET que la tâche joue des scénarios ; les énoncés de fusion contiennent les tips relevés par git rev-parse <branche> au moment du lancement, jamais des SHA recopiés d'un rapport | GATE + SESSION |
| L7-5 | Test rouge non observé quand un symbole manque : le test ne compile pas | l'énoncé d'un agent C++ impose un stub qui compile + test AVANT le premier build (rouge observé), budget de deux builds, et le champ TEST_ROUGE donne « observé » ou « supposé » | AGENT + RAPPORT |
| L7-7 | Fusion anticipée (20:00 au lieu de 21:35) : a laissé du temps à deux intégrateurs qui ont trouvé 4 défauts invisibles aux auteurs | première fusion groupée dès que 3 branches sont vertes, pas à « fin - 30 min » ; intégrateur nommé AU LANCEMENT | SESSION |


## Leçons du 2026-11-06 (séance interactive 10:53-13:00, cartes exclues) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L8-2 | Un blocage en mêlée (soldat en idle ou début d'animation d'attaque en boucle devant sa cible) n'est détecté par aucun test : l'audit Sanity ne voit que les rotations | audit « cible à portée, aucune attaque depuis N s » dans `hol.Test.Sanity` ; devient le test rouge de `BUG-melee-stuck-attack` | AGENT C++ |
| L8-4 | Une réponse d'essai sonore ambiguë (« UI.Open F, UI.Open A » = en fait Open puis Close) a coûté une question de plus | `E-audio_compare.md` porte d'avance une ligne `Ui.Open = _` par cue ; le panneau d'écoute affiche le nom du cue à côté de la lettre | AGENT 18 |
| L8-8 | Le build de fusion (11 à 18 min) est le chemin critique ; la fusion 3 est partie à 12:30 pour finir à 12:47 | lancer la fusion C++ dès que la branche C++ est prête (avant les autres) et fusionner en second tour les branches script-only sans rebuild (comme `heal-sleep` en fusion 3) | SESSION |
| L8-9 | **Demande de l'utilisateur (2026-11-06) : au DÉBUT et à la FIN de chaque Auto_Journey, tous les worktrees sont remis à jour avec `main`**, pour qu'ils restent à jour le plus longtemps possible | début : `Tools\AutoJourney\sync_worktrees.ps1` AVANT le premier lancement (`session_gate -Phase Launch` : WARN par worktree en retard) ; fin : `sync_worktrees.ps1` après la dernière fusion et le dernier commit de docs, aucun agent ne tourne (`session_gate -Phase Close` : FAIL par worktree en retard, `-Exclude` pour les worktrees hors périmètre) ; un saut (travail non commité) ou un conflit entre dans le bilan | GATE + SESSION |

## Leçons du 2026-11-07 après-midi (séance 12:10 -> 16:10, 29 agents, 2 fusions) : contrôles à ajouter
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L9-1 | 3 agents (Glorfindel-15/16, Celeborn-18) ont re-notifié leur rapport en boucle (minuteur de réveil / Monitor propre à l'agent) et ont consommé des appels | l'énoncé interdit « minuteur, cron, Monitor » ; la session principale fait TaskStop à la 2e notification identique | AGENT + SESSION |
| L9-4 | Fins de ligne mêlées : conflit TOTAL à chaque fusion (3 fois), 2 lignes perdues | CRLF partout (arbre de travail en CRLF par `.gitattributes` `* text=auto eol=crlf`, index en LF, AUCUN BOM) ; `session_gate.ps1 -Phase Close` : une ligne WARN par worktree et pour la copie principale qui nomme les fichiers mêlés et la commande `eol_check.ps1 -Fix` (**FAIT 2026-11-12**) ; ligne de statut par agent dans un fichier à part (comme Retours\) plutôt qu'une ligne du fichier partagé | OUTIL + AGENT |
| L9-6 | La fusion 1 a pris 55 min et 99 appels : les tips de 3 branches ont bougé pendant la fusion (net-playtest en 3 temps) | figer les branches à fusionner (aucun agent actif dessus) AVANT de lancer l'agent de fusion ; énoncé avec tips relevés à la minute | SESSION |
| L9-8 | Haldir (3 agents), Sauron, Arwen, Elrond sans tâche utile de 14:20 à 16:10 : la file « sans C++ » était vide et aucun chapitre du GDD n'est libre (tous ont un *InProgress.md) | l'étape 0 liste les chapitres GDD libres (sans *InProgress.md) ; en fin de séance précédente préparer 3 lignes sans C++ par worktree ; réserver un créneau MACHINE CALME en début de séance pour les mesures S1/S2 (jamais possible en cours de séance) | SESSION + ÉTAPE 0 |
| L9-9 | Mode autonome choisi alors que l'utilisateur était présent : ses réponses (Q-melee-stall A, sites, panneau B) n'ont pas été appliquées en séance | si l'utilisateur répond pendant une séance, proposer de passer en mode interactif (réponse = ligne de tête + relance de l'agent du worktree) | SESSION |
| L9-10 | Essais sonores : aucun agent n'entend rien ; 12 écoutes réparties en 12 scénarios | un seul panneau d'écoute (touche = cue suivant, lettre = vote, résumé écrit à la fin) ; statut LOGIQUE SEULEMENT à vie pour les agents | AGENT 18 |

## Lecons du 2026-11-08 (seance interactive 10:25-13:40) : controles ajoutes
| ID | Regle | Controle | Ou |
|---|---|---|---|
| L10-2 | net rouge 15/15 sur 2 branches puis sur main : le premier reflexe etait de suspecter les branches | au premier net rouge apres un net vert du jour : relancer net sur main SEUL (worktree propre) avant de toucher aux branches | SESSION |
| L10-3 | 'sans C++' ne dit pas que le binaire est a jour : la fusion 5 a du rebuild (BuildingSelected inexistant) | l'agent de fusion lit session_gate -Phase Report (binaire plus recent que le dernier commit de code) avant son quick_check | AGENT + GATE |
| L10-4 | Edit echoue sur le C++ en CRLF : 5 agents ont perdu 3 a 5 appels | l'enonce d'un agent C++ donne la methode (remplacement PowerShell sur chaine exacte) | AGENT |

## Lecons du 2026-11-10 (seance autonome 09:57-10:55, 1 h, 15 agents, 3 fusions) : controles a ajouter
| ID | Regle | Controle | Ou |
|---|---|---|---|
| L11-2 | Des agents ont brule 10 a 20 appels a sonder la file de build (Galadriel 50, Sauron 55, Arwen 85 > 80) | build.ps1 -Wait : un seul appel bloquant (un build -Wait n'est jamais remplace, le build plus recent du meme worktree se met derriere) ; l'enonce modele impose `build.ps1 -LiveCoding=0 -Wait` et interdit de sonder la file | OUTIL + AGENT **[x] OUTIL ET ENONCE MODELE FAITS 2026-11-11** |
| L11-3 | Un agent C++ de 30 min sur un bug deja corrige (infobulles : l'essai du 09-11 tournait sur un binaire d'avant d813698) | un bug rapporte commence par un verdict de 5 min sur main a jour, sans build ; l'agent C++ part seulement si le verdict est rouge | SESSION + AGENT |
| L11-4 | ~20 lignes sur 58 de la file etaient perimees ; les lignes faites n'etaient pas cochees (les agents n'editent pas le fichier) | la session principale coche en un passage a la fin, `session_gate -Phase Close` echoue si un rapport « fait » n'a pas de ligne cochee (fait a la main le 2026-11-10) | SESSION + GATE |
| L11-8 | Fusion en 3 tours : un message a l'integrateur en cours est arrive apres son build | liste figee des branches ecrite dans le tableau AVANT de lancer l'integrateur, attente jusqu'a une heure fixe, pas de message en cours | SESSION |
| L11-9 | Aucun essai donne ni integrateur de recette nomme ; ESSAIS-A-JOUER.md date de la veille | integrateur de recette nomme au lancement, ou « aucun essai donne » ecrit dans le bilan ; ESSAIS-A-JOUER.md rejoue sur main fusionne avant d'etre donne | SESSION |
| L11-10, L10-5, L9-7 | Mesures de melee a N=8 bruitees (base 3/8 le 09-11, 5/8 le 10-11) ; un format FAIL k/N lu a l'envers | N=16 minimum via ab_matrix.ps1 (PASS k/N) avant de changer un defaut ; lire un tableau de mesure par un agent neuf avant toute conclusion | AGENT |
| L11-11 | Une fiche supprimee apres un OK de l'utilisateur a ete recreee (E-fog_trees.md) | l'agent lit Retours\REPONSES-*.md avant de creer une fiche d'essai | AGENT |
| L11-12 | 15 fiches de Retours\ portent une reponse a reporter, signalees par Close mais jamais purgees | ligne de file « purger Retours » en debut de seance suivante | SESSION |
| L11-13 | Heures annoncees de memoire (decalage de 2 min) | Get-Date avant toute annonce d'heure (deja L5-1, appliquee aux agents, pas a mes messages) | SESSION |

## Lecons du 2026-11-12 (seance a partir de 10:39) : controles a ajouter
| ID | Regle | Controle | Ou |
|---|---|---|---|
| L12-1 | La file de build est le goulot (2026-11-12, `build_queue_report.ps1 -Since "2026-11-12 10:39"`, releve de HALDIR-1112c a 11:08) : 1 seul build fini en 30 min (Glorfindel, 17 min, FAIL), 7 tickets en attente jusqu'a 25 min (dont 2 tickets Elrond), debit ~1 build / 17 min, le dernier de la file attend ~2 h. Cause : DLL des 8 worktrees du 2026-11-11 : chaque PREMIER build du jour recompile la fusion de main ; des agents « sans C++ » compilent aussi (scenarios a jouer apres `git merge main`) : le plafond de 3 agents C++ ne borne pas la file (releve detaille archive le 2026-11-13) | PROPOSE, `build.ps1` inchange : (a) au debut de seance, `sync_worktrees.ps1` SANS `-NoBuild`, en serie, machine calme, AVANT le premier lancement ; `session_gate -Phase Launch` FAIL (et non WARN) au premier lancement si un worktree a plus de N = 5 commits `Source` de retard ou une DLL plus vieille que le dernier commit `Source` de main (le calcul existe deja : `Say-StaleBySource`) ; (b) plafond EFFECTIF : `session_gate -Phase Launch` compte les tickets en attente + compilant (pas seulement les agents declares C++) et dit FAIL a plus de 3 tickets pour tout agent qui va compiler ou jouer un scenario ; (c) `build.ps1 -Wait` : un 2e ticket du meme worktree derriere un ticket en attente = WARN (Elrond 10:43 + 11:00) ; (d) l'enonce de chaque agent qui compile donne l'attente estimee (tickets devant x duree mediane du jour) pour qu'il prepare le travail sans C++ pendant l'attente au lieu de sonder | OUTIL + SESSION |
| L12-1b | Complement (diagnostic HALDIR-1112d, 11:28) : la lenteur ne vient PAS du volume a compiler mais de la machine (Glorfindel : 18 actions comme Haldir la veille, 1006 s au lieu de 99 a 134 s ; UBA tue des processus « Low on memory » ; memoire virtuelle libre 549 Mo ; ProjectShiva, autre moteur invisible de `build.ps1`, compile 5 worktrees a la fois avec 2 editeurs et deux jeux ouverts). FAIL de Glorfindel = code, pas machine (fusion incomplete du revenu : `GetIncomePerMinute`, `GetGoldIncome`) (preuves detaillees archivees le 2026-11-13) | (1) seance : demander a l'utilisateur de mettre ProjectShiva en pause (ou de fermer les jeux) tant que la file HoL tourne ; sinon compter 10 a 20 min par build, soit 1 h 30 a 2 h pour les 6 en attente. (2) `build.ps1`, juste avant `$StartedAt = Get-Date` (ligne 288) : relever `Win32_OperatingSystem.FreeVirtualMemory` et les `cl.exe`/UBT HORS `$EngineRoot` ; ecrire `build : machine chargee (N compilations d'un autre moteur, M Go virtuels libres)` et ajouter une colonne `load` a `BuildQueue.csv` ; attendre (poll) tant que memoire virtuelle libre < 8 Go. (3) controle : `session_gate -Phase Launch` WARN si un cl.exe d'un autre moteur tourne ou si la memoire virtuelle libre < 8 Go ; `build_queue_report.ps1` signale toute duree > 3 x la mediane de la veille pour le meme nombre d'actions | OUTIL + UTILISATEUR |
| L12-1c | Bilan du 2026-11-12 : 5 agents C++ sur 5 n'ont rien compile apres 10:56 (1,2 Go de RAM libre a 11:52, 6 editeurs ProjectShiva = 33 Go, 2 jeux = 15 Go) | `session_gate -Phase Launch` WARN si un cl.exe d'un autre moteur tourne ou si la memoire virtuelle libre < 8 Go ; `build.ps1` attend tant que < 8 Go et l'ecrit (journal + `BuildQueue.csv`) | OUTIL + GATE |
| L12-2 | sync_worktrees lance avec -NoBuild au debut : DLL du 2026-11-11 partout, chaque premier build du jour recompile la fusion de main | `sync_worktrees.ps1` SANS `-NoBuild` avant le premier lancement ; sinon `session_gate -Phase Launch` FAIL si un worktree a plus de 5 commits `Source` de retard sur sa dll | GATE + SESSION |
| L12-3 | `build.ps1 -Wait` passe en arriere-plan apres 180 s : 5 agents bloques, arret dur 12:30 depasse (messages d'arret a 12:38) | l'enonce donne du travail sans binaire pendant l'attente et un delai maximal d'attente (`build.ps1 -MaxWaitMin`) apres lequel l'agent commite en WIP et rend la main | OUTIL + AGENT |
| L12-4 | Des agents C++ ont ecrit du code avant de savoir si leur binaire demarrait | premier geste d'un agent C++ : `LaunchCheck` ; champ de rapport `LAUNCHCHECK_DEBUT` | AGENT + RAPPORT |
| L12-5 | Recette 12:26 : tests de regles pures en partie triviaux (30/45/60 s jamais assertis, identite) | au moins un test par regle qui passe par une vraie chaine ; champ de rapport `TESTS_TAUTOLOGIQUES : 0`, compte par la recette | AGENT + RAPPORT |
| L12-6 | Recette 12:26 : index de Retours perimes, EtapeDefini disait encore « pas dans main » apres la fusion | l'agent de docs met a jour les index (Retours, LISEZ-MOI, EtapeDefini « dans main ») dans le meme commit que la fusion ; gate Close WARN si EtapeDefini dit « pas dans main » pour une branche ancetre de main | AGENT + GATE |
| L12-7 | 1er build de 17 min a 10:56, diagnostic lance a 11:28, utilisateur prevenu a 11:33 : 37 min perdues alors que la cause (machine saturee par un autre projet) dependait de lui | un build qui depasse 3 fois sa duree habituelle (mediane de BuildQueue.csv pour le meme nombre d actions) fait ecrire a build.ps1 « BUILD LENT » et la session principale lance le diagnostic de charge et previent l utilisateur dans les 5 min ; le veilleur le verifie a chaque passage | OUTIL + PRINCIPALE |
| L13-1 | 2026-11-13 11:12 : 6 agents sur 8 coupes par des 403 Cloudflare ; la reprise avec 3 puis 5 agents simultanes n a plus rien donne | un lancement de reprise commence par 2-3 agents et monte par paliers ; le bilan note le nombre d agents simultanes au moment d une erreur d API | PRINCIPALE |
| L13-2 | `LaunchCheck` compare la dll d un module aux sources de tous les modules (faux « perime » chez Galadriel) ; `session_gate -Phase Launch` compte 5 agents C++ d apres le texte des `IN_USE.txt` (faux FAIL a 13:00) | un test par module dans `launch_check_test.ps1` (rouge d abord) ; le plafond C++ lit les tickets vivants de `Docs\BuildQueue.state`, pas du texte libre | OUTIL |
| L13-3 | Fast-forward de main refuse a 13:39 : `Auto_Journey_File.md` non commite dans la copie principale et touche par 3 branches ; deblocage manuel par la session principale | avant l agent de fusion, la session principale commite les docs de seance (par nom de fichier) ; `preflight_merge.ps1` signale tout fichier modifie des deux cotes | PRINCIPALE + OUTIL |
| L13-4 | `quick_check -Parts net` rouge 3/3 chez Elrond et 15/15 chez Arwen sous charge (Maj relache avant « & »), vert 17/17 a la fusion | un net rouge « valeurs vides » est refait une fois machine calme avant d etre note REGRESSION ; le maintien de Maj (27d91a4) est verifie par un rejeu net a la fusion | OUTIL + AGENT |
| L13-5 | Preflight en FAIL pour 294 fichiers `Content/Audio/Candidates/` non suivis (Arwen) : 3 essais bloques, personne ne decide | le gate Close liste les dossiers non suivis de plus de 50 fichiers et la question « commiter ou exclure » part dans Retours des qu elle bloque un preflight | GATE + PRINCIPALE |
| L13-6 | Scenarios ecrits sans verifier que l objet est visible : `spear_arm` (camera min 15 m, Lancier ~70 px), `neutrals_minimap` (carre minuscule), `deploy` (`waitlog D=Pack` jamais emis) | le `.look` declare la taille en pixels attendue et le texte exact du journal attendu, verifies contre le code avant le premier preflight | AGENT |
| L13-7 | 38 variantes balayees sans verifier la chaine : un dict partage entre deux cles (deepcopy) faussait toutes les mesures | apres 2 balayages sans vert, verifier les donnees d entree (valeurs resolues par cle) avant un 3e (complete la regle 18) | AGENT |
| L13-8 | 4 agents arretes a 45 appels avant d avoir ouvert les PNG et ecrit la fiche d essai | l enonce reserve les 8 derniers appels a PNG + fiche E + journal ; sinon l agent le dit dans NON VERIFIE | AGENT |
