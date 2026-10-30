# Auto_Journey : journée autonome du 2026-10-31

Tout ce qui concerne cette journée est ici. **Ce qui attend un retour de l'utilisateur (essais, questions, fusions) est dans `Auto_Journey_Retours.md`.** `EtapeDefini.md` ne garde que la liste des chantiers (une ligne par avancée
de chantier, comme d'habitude) ; le détail de la journée, les consignes, le programme, les essais et le bilan sont dans ce
fichier. Le détail technique de chaque chantier reste dans son `*InProgress.md`.

## Consigne de l'utilisateur (donnée le 2026-10-30)

- L'utilisateur est absent toute la journée du 2026-10-31. Il faut avancer, remplir **~8 h de travail**, et en faire plus si possible.
- Plusieurs chantiers **en parallèle**, par sous-agents, dans les **worktrees existants** (aucun nouveau worktree).
- **Commits autorisés partout, même sur `main`, sans validation, exceptionnellement ce jour-là. Jamais de push.**
- Quand un chantier est fini ou attend la validation de l'utilisateur : le ranger (scénario d'essai prêt) et passer à un autre.
- Réserver à la fin un ou plusieurs **essais** pour l'utilisateur (fichier `Auto_Journey_Retours.md`, section 1 « Essais à faire »).
- Répondre aujourd'hui (2026-10-30) à un maximum de questions, pour avoir un maximum de travail demain.

## Règles de conduite (réécrites après le test d'1 h du 2026-10-30)

### La session principale est le chef d'orchestre
- **Économiser ses tokens** (demande de l'utilisateur) : les sous-agents font tout le travail (code, tests, compilation, journal, commit, `EtapeDefini.md`). La session principale lance, lit des rapports de 10 lignes, tient ce fichier et `Auto_Journey_Retours.md`. Pas de gros fichiers lus en direct, pas de recherche dans le code, pas de relecture de ce qu'un agent vient d'écrire.
- **L'étape 0 est un sous-agent en lecture seule** (test : 70k tokens, 72 s) : état réel de chaque worktree et de la copie principale (`IN_USE.txt`, `git status`, `git log`, fin du journal, lignes d'`EtapeDefini.md`), tableau de 25 lignes. Le tableau « Programme » ci-dessous est celui d'hier soir, pas une vérité.

### Qui travaille, et sur quoi
- **Un agent par worktree, tous les chantiers du tableau lancés dès le départ, en parallèle.** Au test, 4 chantiers sur 6 avaient été écartés à tort, et il a fallu que l'utilisateur le remarque.
- **« Pris » dans `IN_USE.txt` n'est pas un interdit.** Un worktree « Pris » avec du travail non commité se **reprend sans l'écraser** : `git status`, vérifier que ça compile et que les tests passent, commiter par lots cohérents, jamais `git checkout`/`reset` sur du travail non commité. Seul cas où on ne touche pas : le chantier n'a rien d'ouvert (ex. réseau avant l'essai E5).
- **Quand « À faire » d'`EtapeDefini.md` est vide ou ne convient pas** (il s'est vidé en 1 minute au test) : prendre un **chapitre du GDD non commencé** dont les décisions sont tranchées (Volcan/Catastrophe, Technologies, Arbre de talents...), ouvrir son `*InProgress.md` (plan en étapes de 15 min, premier test rouge), coder la première étape. « Après le prototype » exclut seulement ce que le GDD marque ainsi. Le relevé de 12 chapitres est dans la section « Chapitres du GDD non commencés ».
- **Le GDD (`GDD.md`) est éditable même s'il est modifié et non commité par d'autres sessions** : éditions ciblées après relecture, ne pas commiter le fichier entier (il mêle plusieurs sessions). Avant d'utiliser un numéro de décision, relire § 22 (le « prochain D564 » avait été pris par le rattrapage).
- Jamais de nouveau worktree. Les agents ne **tranchent jamais** une décision de design : ils la posent.

### Comment écrire l'énoncé d'un agent
- Chemins **absolus** partout ; pointer les journaux à lire au lieu de recopier leur contenu ; une tâche précise, des sous-étapes de 15 min, la plus petite d'abord.
- **« Enchaîne sans t'arrêter au premier point qui n'exige pas de code »** (un agent s'était arrêté après avoir constaté qu'un point était déjà bon).
- Arrêt dur à une heure donnée, lue à l'horloge **PowerShell** (`Get-Date`). Plafond ~250k tokens (au test : 50 à 160k tokens, 10 à 30 min par agent, jamais près de 300k). Un agent près de 200k n'est pas relancé : on en prend un neuf qui repart du journal. **Le minuteur part dans le même message que l'appel Agent.**
- Rapport final de **10 lignes au plus** : fait, tests, commit, reste, anomalies du processus.
- Chaque agent : met à jour `EtapeDefini.md` (relire juste avant, ne toucher que ses lignes : aucun conflit observé) et le journal de son chantier à chaque avancée, `IN_USE.txt` à l'entrée et à la sortie (fichier non suivi par git), commite sur la branche de son worktree (anglais, sans `Co-Authored-By`, jamais de push).
- Test rouge d'abord sur la vraie chaîne ; un changement, une compilation, un passage du test ; `Tools\build.ps1` (file d'attente), `-LiveCoding=0` ; `quick_check -Parts` des seules parties touchées. Un test rouge peut venir du **script de test** et non du jeu (deux cas au test : IA du match 1v1 qui regroupait les unités, héros qui dérive d'1 m) : regarder le dernier `Test mark` du journal de jeu avant de toucher au code.

### Les retours de l'utilisateur (voir `Auto_Journey_Retours.md`)
- Tout ce qui attend l'utilisateur est dans **`Auto_Journey_Retours.md`** (1 essais, 2 questions, 3 commits et fusions). Les agents y écrivent, jamais ici.
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

## Décisions prises le 2026-10-30 (séance de questions)

| Question | Réponse | Où c'est noté |
|---|---|---|
| F-Q3 rendu du noir du brouillard | **A, plan dédié** (texture de cases du client, réutilisée par la mini-carte) | `FogInProgress.md` |
| M-Q2 effets de terrain du prototype | **A, seulement le visible** (marais, forêt, route, eau) ; couvert et hauteur après le prototype | `MapsInProgress.md` |
| M-Q9 petits tas variés | **A, types « petit filon »** d'or et de pierre (500) à budget égal, pas de gibier | `MapsInProgress.md` |
| Hommes d'armes inactifs autour du héros | **A, à corriger** : attaquants en trop d'une cible encerclée réattribués à un ennemi proche | `EtapeDefini.md`, `JourneyInProgress.md` |
| Lanciers ennemis -> Archer | déjà tranché (Archer gardé) ; reste le texte du GDD § 7.2 à harmoniser | `AIInProgress.md` |
| Paliers (chantier `tiers`, Elrond) | P-Q2 = A déjà tranchée (socle seulement : palier = f(niveau du héros), panneaux de bâtiments, vies du forum) ; P1 faite (a041984). **Les 3 valeurs par défaut confirmées (2026-10-30)** : le palier ne baisse jamais, même héros mort ; sans héros palier 0 ; un forum déjà tombé gagne la vie du palier dès qu'il est atteint | `VictoryInProgress.md` |
| M-Q3 le pont (Les Gués) | **A, chantier comme une ruine** (mécanique V3 réutilisée) | `MapsInProgress.md` |
| Mini-carte du prototype | **A, façon AoE4 complète** : camps en couleurs, sources explorées, cadre de caméra, clic gauche = caméra, clic droit = ordre de déplacement, alertes qui clignotent ; suit le brouillard | `InterfaceInProgress.md` |
| Fusion dans `main` demain | **A : oui, dès qu'un chantier fini a son `quick_check` des parties touchées vert** (libère le worktree ; rien n'est poussé) ; les essais restent à faire au retour | ici |

## Au démarrage : « commence ton Auto_Journey »

L'utilisateur lance la journée par cette seule phrase. Les chantiers auront avancé depuis hier soir. Ce qu'on fait alors, sans poser de question déjà tranchée :
1. Lire ce fichier en entier (la session principale), puis **lancer l'étape 0** (sous-agent en lecture seule, voir « Règles de conduite ») et réécrire le tableau « Programme » avec l'état réel : étape déjà faite, worktree à reprendre, question nouvelle. Ne jamais refaire ce qui est fait.
2. **Lire les `Reponse:` de l'utilisateur dans `Auto_Journey_Retours.md`** : c'est la relance qui autorise les corrections et suites qu'elles demandent (« arbres trop espacés », etc.). Les « A trancher » restent ouvertes. Enregistrer ce qui est tranché (table des décisions ci-dessous, journaux, `EtapeDefini.md`).
3. **Lancer d'un coup un sous-agent par chantier** du tableau (tous, y compris les worktrees « Pris » à reprendre sans écraser), minuteur de 15 min dans le même message ; les mesures au repos (créneau 1) **avant** ce développement en parallèle.
4. À chaque agent fini : commit (hors push), ligne dans « Journal », ligne dans `EtapeDefini.md`, essai testable ou question ajouté à `Auto_Journey_Retours.md`, puis relancer un agent neuf sur la suite du même chantier si le plan en a une, sinon sur la ligne suivante.
5. **Si tous les chantiers sont finis ou en attente de l'utilisateur et qu'il reste du temps (objectif ~8 h)** : prendre dans `EtapeDefini.md` (« À faire ») la ligne suivante qui ne demande ni essai ni décision (autre chantier, reste d'un chantier, « Retours d'essai », « Revue de code », « Aléas et petits restes », fusion et nettoyage ; pas « après le prototype », « avec la cavalerie », « reporté »). Si « À faire » est vide : chapitre du GDD non commencé (voir « Règles de conduite »). Un worktree libre suffit.
6. À la fin : paquet et serveur (créneau 3), bilan, et `Auto_Journey_Retours.md` à jour (essais testables avec commandes en chemin absolu, questions, fusions).

## Programme de la journée (ordre proposé)

Créneau 1, machine calme (matin) : **mesures au repos** de la clôture de J8 (C2, `Tools/Tests/rest_measures.ps1`), seules.
Créneau 2, en parallèle (un sous-agent par chantier, compilations en file) :

| Worktree | Branche | Chantier | Travail prévu |
|---|---|---|---|
| Celeborn | `hero-j8-close` | J8 clôture | commit de C1 et C3 (déjà faits) ; correctif des Hommes d'armes inactifs autour du héros (test rouge d'abord) |
| Faramir | `fog` | Brouillard | F7 : plan dédié (F-Q3 = A), unités cachées non dessinées, fantômes teintés, ligne « Fog » de `hol.HUD.Report` ; puis texture et lecteur de cases pour la mini-carte |
| Arwen | `ai-ia1` | IA3 | IA3.3 (attaque de la ruine), IA3.4 (Régicide), IA3.5 (scénario `ai_forum`), IA3.6 (clôture) |
| Haldir | `maps` | Cartes | types « petit filon » (M-Q9), reprise de C2/M-Q8 selon l'essai, puis C3 (El Dorado) avec M-Q2 = A ; Plaine remise au budget |
| Elrond | `tiers` | Paliers | selon les réponses aux questions des paliers (à poser) |
| Galadriel | libre | Divers | texte du GDD § 7.2 (Lanciers), recalage de la numérotation des décisions, reste d'`EtapeDefini` |
| Glorfindel | `net-playtest` | Réseau | rien d'ouvert avant l'essai E5 avec un ami ; seulement si un reste apparaît |

Créneau 3, machine calme (fin de journée) : **paquet et serveur à recuire** (C4 de J8), seul ; puis `quick_check` des parties touchées.

## Essais et questions pour l'utilisateur

Déplacés dans `C:\Users\baudo\Documents\MyCrusader\Auto_Journey_Retours.md` (essais à faire, questions sans réponse, commits et fusions à valider). **Les sous-agents y ajoutent leurs lignes, pas ici.**

## Journal de la journée

- 2026-10-30 soir : fichier créé ; autorisation de commit partout (hors push) notée en mémoire ; réponses F-Q3, M-Q2, M-Q9 et Hommes d'armes enregistrées dans les journaux et `EtapeDefini.md` ; questions des paliers à poser.

- 2026-10-30 16:01-17:05 **TEST d'1 h** (mesures au repos sautées, commits autorisés sur branches, pas de fusion dans main) : étape 0 faite par un sous-agent (70k tokens, 72 s, rapport de 25 lignes). Constat : le tableau « Programme » est périmé (IA3 close, paliers ouverts) ; Celeborn (C1 non commité), Haldir (58 fichiers non commités), Faramir, Elrond, Glorfindel = pris/en veille, non touchés ; GDD.md de main modifié non commité donc pas de travail GDD. 16:05 lancés : Arwen = bug `victory`/Régicide (quick_check rouge), Galadriel = ordres après Tab (InterfaceInProgress), minuteurs à 16:17 et 16:33.

- 16:17 Arwen/victory CLOSE en ~10 min (86k tokens) : cause = le cerveau de l'IA du match 1v1 de `quick_check` regroupait les Lanciers sur la ruine (pas le Roi, pas l'IA3) ; correctif = `"aiBrain": false` pour l'emplacement 1 dans `Tools/Tests/quick_check.ps1`, `-Parts victory` vert (53 s), commit c806fd8 sur `ai-ia1` (pas dans main). 16:20 Arwen relancée sur un reste d'IA4 (agent neuf). Galadriel (Tab) toujours en cours. Minuteurs restants : 16:33, 16:48.

- 16:19 Arwen/IA4 (105k tokens, ~10 min) : un coéquipier n'est plus un ennemi pour l'IA (`HOLAISummary::Make` reçoit la table des camps ; 2 tests rouge puis vert, 484/484 + aiserver), commit 3f5f58a sur `ai-ia1`. Reste d'IA4 = décisions (aide aux coéquipiers, Paysans sans défense, vies D426) -> « Questions sans réponse ». Arwen relancée (agent neuf) sur la ligne suivante d'« À faire ».

- 16:25 Arwen (agent n°3, 47k tokens, 1 min) : « rien à prendre » dans « À faire » (Retours d'essai et Revue de code vides, Aléas = déjà corrigé / intermittent sans cause, le reste exclu). **Constat de processus** : sans chantier ouvert la boucle de l'étape 5 se vide vite ; il faut donc lancer tous les chantiers du tableau en parallèle dès le début. Erreur de la session principale : Fog, Maps, Celeborn, Elrond avaient d'abord été écartés à tort (« Pris » dans IN_USE lu comme session vivante) ; relancés à 16:25 sur remarque de l'utilisateur. Règle à retenir : un worktree « Pris » avec travail non commité n'est PAS un interdit, le sous-agent reprend sans écraser. Arwen libre, Galadriel (Tab) en cours.

- 16:33 Arwen/GDD (70k tokens, ~4 min, aucune compilation) : § 7.2 aligné (Lanciers -> Archer), numérotation vérifiée (aucun doublon), D564-D570 écrites depuis les journaux (budget de ressources, positions naturelles, aucune carte vide, effets de terrain visibles, Arrière-pays, paliers = f(niveau du héros), coéquipier != ennemi) ; **prochain numéro = D571**. `GDD.md` modifié et NON commité (contient aussi D558-D563 d'autres sessions) ; les sessions qui comptaient sur D564 (Elrond P5, Haldir) doivent relire § 22 (avertissement dans `VictoryInProgress.md`).

- 16:36 Galadriel/Tab CLOSE : bug réel (clic droit, ligne d'ordre, pose de bâtiment partaient à toute la sélection après Tab ; seuls stop/garde/attaque-déplacement lisaient l'onglet actif), test rouge puis vert (`move` cas « tab », `-Parts tests,move,formation`, 482/482), commit 5e4da9f sur `galadriel-tab` (pas dans main). Essai ajouté ci-dessous. Anomalie : `quick_check -Parts build` entier rouge sur main e6fbaae (cas « hero aura » étape 1390, puis cascade) -> Galadriel relancée dessus (agent neuf, arrêt 17:00).

- 16:41 point des commits (aucun rapport final encore) : Faramir a94304b (F7.3 boucle 1, ligne « HUD fog »), Haldir 533f480 + c3579ba + 3594b4f (C2 Arrière-pays, six tailles de filon M-Q9, arbre plus fin), Celeborn b297add (C1 stress J8), Elrond 48e3244 (P4 : contrôle en partie `build:tiers` + scénario d'essai), Galadriel 5e4da9f (Tab), Arwen c806fd8 + 3f5f58a. Arwen relancée sur un chapitre du GDD (étape 5 d'Auto_Journey corrigée).

## Chapitres du GDD non commencés (relevé Arwen, 2026-10-30 16:45, par recherche dans `Source/` et `Script/`)

Relevé rapide, pas un audit : « aucun code » = aucun fichier ne porte le nom ou la règle ; les lignes marquées (?) sont à vérifier avant d'ouvrir le chantier.

- **§ 12 Volcan + option Catastrophe** (D205, D208, D377-D380, D398-D400, D427) : tout est tranché (coulées qui avancent, site à gisements riches, refroidissement puis basalte, autel 60 s, réarmement, IA opportuniste). Code : seulement le drapeau `bCatastrophe` (config, menu, salon). Manque : tout le moteur d'événement (état du site, lave, dégâts, autel, alertes, IA). **Choisi : `VolcanoInProgress.md`.**
- **§ 16.2 World State Manager** (D29) : états `Warning` etc. prévus, aucun code. Se construit avec le volcan (V1), pas à part.
- **§ 11 Technologies** (D27, D180, grilles 11.1 et 11.2) : trois conditions (niveau du héros/palier, bâtiment, coût), 70-80 % communes ; grilles de palier 0-3 écrites. Code : types de bâtiments et grille de commandes seulement (?), pas de recherche. Manque : table CSV des technologies, file de recherche, effets sur les statistiques.
- **§ 9.6 Arbre de talents** (D08, D103, D194 ; familles des six factions) : rangées 2-9, 1 talent sur 3, sceau à 3, talent clé à 5. Code : aucun. Manque : le contenu des talents de la faction du prototype (Ordre de l'Aube : Serments), points de talent, écran ; dépend du palier/niveau du héros (J8).
- **§ 10 Duel de héros** (D14, D17) : déclenchement, arène, kit semi-automatique, durée, issue largement tranchés. Code : quelques tests de héros/victoire et une mention dans l'IA. Manque : déclenchement, arène, kit de duel, écran (?).
- **§ 5.5 Marché** (D70) : échange de ressources décidé. Code : aucun (?). Manque : bâtiment, taux, interface.
- **§ 5.2 Ressources spéciales, § 6.1 Siège (D23), § 6.2 Bonus de hauteur** : décidés ; la hauteur est exclue du prototype pour les effets de terrain (M-Q2), le siège est partiel (?) ; à relire avant d'ouvrir.
- **§ 8.4 Unités neutres et monstres** (D32, D401 « Neutre ») : camps neutres décidés. Code : trace de « Neutral/Camp » dans la simulation (?) ; manque : camps, récompenses (gisement ou passage, D102), IA des neutres.
- **D384/D388 Centres secondaires et relais neutres** (XP de territoire) : règle de la ruine (D348) réutilisable. Code : une mention dans `HOLHeroXPRules.h` seulement (?) ; manque : lieux posés par la carte et prise par présence.
- **§ 13 Cinq autres factions** (Légions Noires, Enfants du Dragon, Cercle de l'Ombre, Héritiers du Feu, Templiers) : contenu écrit (D87, D106-D173) ; le prototype ne joue que l'Ordre de l'Aube, donc **après le prototype**, ne se prend pas.
- **§ 12.3 Événements futurs** (D209 : inondation, invasion de monstres, faille, tempête) : décidés, **après le prototype** ; dépendent du volcan (V1).
- **§ 14.3 Modes de jeu : Battlefield, Reliques** (D416, D102) : **après le prototype**, ne se prend pas.
- **§ 9.9 Progression hors partie** : décidée dans les grandes lignes ; **après le prototype** (comptes, serveur persistant).

- 16:42 Elrond/Paliers P4 PRÉPARÉ (20 min) : nouveau cas `quick_check -Parts build:tiers` (héros niveau 1, 3, 6), rouge d'abord, **2 vrais défauts d'écran trouvés** (panneaux d'âge verrouillés aux niveaux 3 et 6 ; bloc du forum debout avec vies périmées) corrigés dans `HOLGameScreen.cpp/.h` ; `build:tiers` PASS, `tests` 485/485 ; commit 48e3244 sur `tiers` (main e6fbaae fusionné dedans, pas dans main). Scénario `tiers` écrit, non lancé. Reste : essai, `net` et `victory` à rejouer quand `ai-ia1` sera dans main, P5. Note de processus : ne pas mélanger `victory` et `build:*` dans un même passage de `quick_check`.

- 16:47 Faramir/Fog F7.3 en partie (20 min) : ligne `HUD fog` de `hol.HUD.Report`, unités cachées non dessinées, anneaux/barres/lignes filtrés, scénario `fog` ; commits a94304b, 648da21 sur `fog` ; 383/383. Pas de test pour le dessin (preuve par captures hors écran). Reste : double clic « même type », fantômes teintés, arbres coupés -> agent neuf lancé (12 min, a d'abord). Anomalie : l'horloge de Git Bash retarde de 5 min sur PowerShell (l'arrêt dur se lit à PowerShell).

- 16:50 Haldir/Cartes (arrêt 16:50) : le travail non commité (58 fichiers) vérifié et commité en 3 lots (c3579ba, 3594b4f, 533f480 : tests 329/329, rocks 17/17, sizes OK) + 89ce28f (arbre un tiers plus bas, M-Q10). **M-Q10 (décor retiré, sol uniforme, cartes régénérées) NON commité et rouge** : `Map.Table` contrôle 7 (cercle vide <= 28 m) échoue sans décor ; arbres x3 impossible avec le solveur actuel (budget remis à 150/225). Question M-Q10b (A refaire le solveur, B affleurements bloquants, C relâcher la règle) -> « Questions sans réponse ». Attention : `git checkout` des .umap de Haldir ramènerait les cartes avec décor.

- 16:51 Galadriel/`build` rouge CLOSE (commit 97bbe4b sur `galadriel-tab`, seul `quick_check.ps1` change) : pas de régression du jeu, deux défauts du script de test (héros qui dérive de 1 m : bannière demandée à 17 m plantée, à 16 m refusée ; `TestCursor off` laissait le vrai curseur faire défiler la caméra). `-Parts build` entier PASS en 138 s.

- 16:52 Arwen/Volcan V1 (chapitre GDD, ~10 min) : 12 chapitres non commencés listés dans la section « Chapitres du GDD non commencés » (recherche rapide, les (?) sont à vérifier ; 4 sont « après le prototype ») ; chantier Volcan/Catastrophe ouvert (`VolcanoInProgress.md`, plan V0-V12, C++/AS selon D483, ligne dans `EtapeDefini.md` vérifiée) ; V1 = cycle pur d'un site (Dormant sans Catastrophe, alerte, éruption, refroidissement, repos), test rouge puis vert, `tests` 486/486, commit b8bfb9e sur `ai-ia1`. Questions V-Q1 à V-Q4 (fenêtres de Catastrophe, tirage équilibré, tracés sous le brouillard, vitesse de la lave) -> « Questions sans réponse ». V2 bloquée par V-Q4, V3 par V-Q2.

- 16:53 Faramir (agent n°3, F7.3 b/c) : rien codé (10 min restantes, trop peu pour test rouge + compilation + quick_check + commit) ; constat et plan écrits dans `FogInProgress.md` : `AHOLBuilding::RebuildPieces` ne lit pas la vue, `GetGhosts(Slot)` n'est lu que par le HUD ; règle pure `HOLVisionRules::BuildingDrawsFromMemory` (test rouge), `ELook::Memory`, puis (c) arbres via fantômes de sources. Worktree Faramir libre.

- 16:57 Celeborn/J8 : C1 commité (b297add) ; correctif des Hommes d'armes en surnombre (dd16203, `hol.Combat.SurplusReassign`, test `Hero.SurplusAttackers`, 359/359, `-Parts tests,combat` vert, `-Parts hero` non lancé) ; Paysan sans agression ne se rabat pas ; pas d'essai joueur. Tous les agents ont rendu leur rapport avant 17:00.

## Bilan de fin de journée

**Test d'1 h du 2026-10-30, 16:01-16:57. Tout est commité sur des branches de worktree ; rien dans `main`, rien poussé.** Le test a surtout servi à trouver des défauts du processus (ci-dessous).

### Fait (commits par worktree)

| Worktree / branche | Commits | Contenu |
|---|---|---|
| Arwen / `ai-ia1` | c806fd8, 3f5f58a, b8bfb9e | bug `victory` (script de test : IA du 1v1 sans cerveau) ; un coéquipier n'est plus un ennemi pour l'IA (D570) ; Volcan V1 (cycle d'un site, chantier `VolcanoInProgress.md`) |
| Galadriel / `galadriel-tab` | 5e4da9f, 97bbe4b | ordres après Tab suivent l'onglet actif (vrai bug) ; `quick_check -Parts build` de nouveau vert (2 défauts du script) |
| Elrond / `tiers` | 48e3244 | P4 : `build:tiers`, 2 défauts d'écran corrigés (panneaux d'âge, vies du forum), scénario `tiers` |
| Faramir / `fog` | a94304b, 648da21 | F7.3 boucles 1-2 : ligne `HUD fog`, unités cachées non dessinées, anneaux/barres filtrés, scénario `fog` ; (b) fantômes et (c) arbres : plan écrit, pas codé |
| Haldir / `maps` | c3579ba, 3594b4f, 533f480, 89ce28f | 58 fichiers non commités vérifiés et commités (C2 Arrière-pays, filons M-Q9, arbre fin) ; arbre plus bas ; **M-Q10 non commité, rouge** (voir questions) |
| Celeborn / `hero-j8-close` | b297add, dd16203 | C1 (un héros par camp en S3/S4) ; Hommes d'armes en surnombre : l'attaquant à ordre de clic droit saturé se rabat sur l'ennemi proche (cvar `hol.Combat.SurplusReassign`), test sur la vraie chaîne, 359/359 |
| Copie principale | non commité | `GDD.md` (§ 7.2 aligné, D564-D570, prochain D571), `EtapeDefini.md`, journaux, `Auto_Journey.md` : **à commiter** (contiennent aussi le travail d'autres sessions) |

### Bloqué ou en attente de l'utilisateur
- Essais : Tab, `tiers`, `fog`, Arrière-pays (`Auto_Journey_Retours.md`, section 1), plus le correctif des Hommes d'armes (Celeborn, pas de scénario encore : à créer).
- Questions : IA4 (aide aux coéquipiers, Paysans), M-Q10b (Haldir), V-Q1 à V-Q4 (Volcan), voir `Auto_Journey_Retours.md` (section 2).
- Fusions dans `main` : `ai-ia1` (corrige le `victory` rouge de `main`), `galadriel-tab`, `tiers`, `fog`, `maps` (sans M-Q10), `hero-j8-close`. Non faites (test) ; demain la décision « fusion dès `quick_check` vert » les autorise.

### Ce que le test a appris sur le processus

Réécrit et intégré dans « Règles de conduite » et « Au démarrage » ci-dessus (c'est la version à suivre).
