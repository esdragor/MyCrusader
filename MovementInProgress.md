# Déplacement — avancement

Chantier des défauts de la marche des unités, hors parcours du joueur. Il regroupe quatre lignes d'`EtapeDefini.md`
(deux « questions techniques ouvertes », deux « retours d'essai »), qui touchent toutes le même domaine : l'évitement,
la séparation des corps (D292), le regard en avant (D293), les chemins et les formations. Chaque étape est notée ici
dès qu'elle avance.

**Quand** : préparé le 2026-10-22 (journal et scénarios, sans code) ; le code commence **après la clôture de J7**
(J7c.5), comme prévu au nettoyage d'`EtapeDefini.md` du 2026-10-21. Les mesures de fin de J7 (S1 à S7) servent de
référence avant tout changement.

## Les quatre cas

Un scénario par cas (`Tools/Scenarios/<nom>.txt`, lancé par `play.ps1 <nom>`). Les ordres y sont donnés par le
scénario, un cas après l'autre, la caméra suit ; le bouton « Relancer » du panneau rejoue tout.

| Cas | Ligne d'`EtapeDefini.md` | Scénario | État |
|---|---|---|---|
| 1. Croisement de face | Croisement de deux unités de face (avant M5) | `crossing` | essayé : évitement **OK** ; **tremblement** droite-gauche pendant l'évitement |
| 2. Immobiles superposées | Unités immobiles qui restent superposées (vitesse d'arrêt nulle) | `standing_overlap` | essayé : **accepté** par l'utilisateur (pas grave à l'arrêt) |
| 3. Marcheurs bloqués par des corps | Marcheurs bloqués par des corps (passage étroit, mur d'unités) | `blocked_bodies` | essayé : passage étroit **KO**, mur à nous **KO** (trop lent), mur ennemi OK |
| 4. Formation gênée | Formation gênée entre les bâtiments et contre les ressources | `formation_obstacles` | essayé : maisons OK ; veine de pierre et arbres **KO** |

### 1. Croisement de face

- **Origine** : question ouverte de M1 (`ImplementationInProgress.md`, « Questions ouvertes » et journal du
  2026-10-14). Mesure du scénario C du stress test (2 unités, somme des rayons 80 cm) : de face, l'écart minimal
  tombe à 3,5 cm ; la trace montre un **pur freinage**, sans composante latérale, dans une géométrie parfaitement
  symétrique. Hors face-à-face, l'échec était une **limite cinématique** (détection à 250 cm, accélération plafonnée
  à 250 cm/s²) ; à 600 cm de détection, tout passe sauf le face-à-face exact.
- **Depuis** : D292 (séparation dure : aucun corps n'en traverse un autre) et **D293** (regard en avant à chaque pas,
  détour du plus petit angle ; « de face, les deux gardent la même main et se croisent »), tous deux du 2026-10-14,
  **après** la question. Le face-à-face est donc peut-être déjà réglé dans le jeu ; la question n'a pas été remesurée
  depuis.
- **Scénario `crossing`** : deux Paysans exactement de face (16 m), deux Hommes d'armes pareil, deux Paysans à angle
  droit qui arrivent en même temps au milieu, deux groupes de 5 Paysans de face.
- **À faire** : regarder le scénario ; si le face-à-face passe, remesurer le scénario C (`hol.StressTest C`) pour
  clore la question avec un chiffre, sinon instruire (pistes de M1 : dissymétrie « passer à droite », détection plus
  longue par type ou selon la vitesse de rapprochement, plafond d'accélération latérale plus haut).

### 2. Unités immobiles superposées

- **Origine** : question ouverte de M1. `MassStandingAvoidanceProcessor` pousse un **fantôme** ; la conduite d'arrêt
  déplace le corps en appliquant `MoveTarget.DesiredSpeed`, que la naissance met à 0 : le fantôme s'écarte, le corps
  ne bouge pas (~215 paires recouvertes à la naissance dans S1, inchangées ensuite).
- **Depuis** : D292 ne défait un recouvrement que si **une unité en marche** y prend part ; « les unités à l'arrêt
  laissées seules ne coûtent rien ». Deux unités à l'arrêt qui se recouvrent le restent. Mesure de S3 (J7c.4) : 5 188
  paires recouvertes au début, 3 218 à la fin ; et deux ennemis ne se poussent jamais (superpositions qui restent).
- **Scénario `standing_overlap`** : 12 Paysans et 12 Hommes d'armes nés en tas (rayon 1 m), un groupe de 9 pour
  l'essai à la main (même point donné un par un).
- **Pistes (M1)** : vitesse d'arrêt non nulle ; résorption explicite des recouvrements (à la naissance, ou en continu
  pour les unités à l'arrêt, à petit budget). **Coût** à mesurer sur S1 / S3 : S1 est le cas « 2 000 unités à l'arrêt ».

### 3. Marcheurs bloqués par des corps

- **Origine** : retour d'essai 5 de J7b (`JourneyInProgress.md`, 2026-10-20, enquête de « build gap two » et
  « orders push » en `-Full`). Deux Paysans arrivés **de front** devant le passage entre deux maisons s'y coincent
  (4 cm en 5 s) ; 3 Hommes d'armes restent bloqués contre un **mur de 12 Paysans** ennemis serrés, sans le contourner.
  **Faiblesse ancienne** : un marcheur bloqué par des corps redemande le même chemin toutes les 3 s
  (`HOLUnitOrderProcessor.cpp`, `PathStuckSteps` 60), les corps n'étant pas des obstacles du navmesh.
- **Attendu (retour 5)** : l'un cède le passage ou contourne (détour latéral) au lieu de redemander le même chemin.
- **Scénario `blocked_bodies`** : (a) deux maisons qui se touchent (place d'un Paysan, pas de deux, D324) et deux
  Paysans de front ; (b) 3 Hommes d'armes face à un mur de 12 Paysans à nous (qui peuvent être poussés, D292) ;
  (c) 3 Paysans face à un mur de 12 Paysans ennemis (qui ne se poussent pas). Les Paysans ont un rayon d'agression
  de 0 : le mur ennemi reste planté, sans combat.
- **Dans `quick_check`** : « build gap two » (graine 113) et « orders push » (graine 128) gardent ces situations
  stables, sans les juger sur ce point.

### 4. Formation gênée entre les bâtiments et contre les ressources

- **Origine** : retour d'essai 6 de J7b (`JourneyInProgress.md`, 2026-10-20) : « la formation a du mal contre les
  ressources et entre les bâtiments » ; test ultime : entre deux maisons, les unités se gênent pour passer, au mieux
  4 sur 6 passent. À voir avec les retours 1 et 5 (D292, D293, D324).
- **Scénario `formation_obstacles`** : 6 Paysans entre deux maisons qui se touchent, 6 Hommes d'armes entre deux
  maisons à une case d'écart, 9 soldats mêlés envoyés à ~4 m d'une veine de pierre, 9 Hommes d'armes entre le centre
  et la lisière des arbres.

## Étapes

| Étape | État |
|---|---|
| MV.0 Journal, un scénario par cas | **fait** (2026-10-22, `main`, rien de commité) : 4 scénarios lancés sans fenêtre jusqu'au bout |
| MV.1 Essai de l'utilisateur sur les 4 scénarios : ce qui est déjà bon, ce qui ne l'est pas | **fait** (2026-10-22) : les 4 scénarios essayés |
| MV.2 Croisement de face : clos par une mesure, ou instruit | après MV.1 |
| MV.3 Immobiles superposées | **sans objet** : accepté par l'utilisateur (2026-10-22) |
| MV.4 Marcheurs bloqués par des corps | après MV.1 |
| MV.5 Formation gênée | après MV.4 (même cause probable : passages étroits) |
| MV.6 Mesures (S1 à S5), décisions au GDD, clôture | à la fin |

## Journal

### 2026-10-22

- Demande de l'utilisateur (« que peux-tu faire sur EtapeDefini ? », point 1 choisi) : préparer le chantier sans code,
  J7c.4 étant mené en parallèle dans `HeroesOfLiberty-Elrond` (mycrusader-43).
- Sources relues : questions ouvertes et journal du 2026-10-14 d'`ImplementationInProgress.md` (croisement, vitesse
  d'arrêt) ; D292, D293, D324 au GDD ; retours d'essai 5 et 6 et analyse de S3 dans `JourneyInProgress.md`.
- Constat : la question du croisement de face date d'**avant** D292 / D293 ; à revoir dans le jeu avant de l'instruire.
- Scénarios écrits : `crossing`, `standing_overlap`, `blocked_bodies` (+ `.json`, deux joueurs), `formation_obstacles`.
- Carte Plain, repère du centre du slot 0 (−15000, 0), sans rotation (sonde `hol.Sources.Nearest`) : arbres à gauche
  du centre (le plus proche 115, −1781), buissons de baies vers (0, 2300), **veine de pierre (3913, −4592)**, **veine
  d'or (3913, 4592)**, rochers devant le centre. À l'écran, X monte, Y négatif à gauche ; zoom par défaut ≈ 80 m de
  large. `hol.Select.ClickUnit N` prend la N-ième unité à nous par identifiant (0 = la première née).
- Mise au point : une sélection par rectangle qui ne change rien n'écrit rien au journal → `hol.Select.Report` puis
  `waitlog Selection report: selected …` ; le journal écrit `ManAtArms`, pas `Man-at-arms`.
- **Ce que montrent les scénarios** (lancés sans fenêtre, captures regardées, une seule passe chacun ; pas de mesure) :
  1. `crossing` : les deux Paysans et les deux Hommes d'armes exactement de face se dépassent côte à côte ; angle
     droit passé ; les deux groupes de 5 se croisent et arrivent en rang. Le face-à-face de M1 semble réglé par D293.
  2. `standing_overlap` : les deux tas restent en tas (bug reproduit).
  3. `blocked_bodies` : les deux Paysans de front restent **coincés à l'entrée du passage** entre les maisons
     (toujours « en marche » 13 s après ; bug reproduit). Les 3 Hommes d'armes traversent le mur de Paysans à nous en
     les écartant ; les 3 Paysans contournent le mur ennemi et arrivent.
  4. `formation_obstacles` : les 6 Paysans passent le passage étroit (ensemble, en formation) et se reforment ; les 6
     Hommes d'armes passent le passage large ; les 9 se rangent entre le centre et les arbres. Contre la **veine de
     pierre**, la formation se tasse en désordre au-dessus de la veine (retour 6 reproduit).
- Chaque lancement a attendu son tour (mesures de mycrusader-80, S5, et de mycrusader-43, S2, prévenus).
- **Case de 4 m (D311 révisée, mycrusader-4b, branche `tile-4m` dans `HeroesOfLiberty-J5c`, pas encore sur `main`)** :
  maison 2×2 = 8 m au lieu de 6. À la fusion, dans `blocked_bodies` et `formation_obstacles`, les deux maisons qui
  se touchent (centres à 6 m : −3300 / −2700) passent à **8 m** (−3400 / −2600) ; les deux maisons à ±600 (centres à
  12 m) laissent alors **4 m**, soit encore une case d'écart. Les scénarios tels qu'écrits valent pour la case de 3 m.
- **Essai de l'utilisateur : `blocked_bodies`** (fenêtre, `play.ps1`) :
  1. Passage étroit : **KO**. « Ils ne bougent pas au début, puis après se poussent mutuellement et personne ne
     passe. » Pire que l'attendu (l'un cède, l'autre passe) : aucun des deux ne passe.
  2. Mur de Paysans à nous : **KO**, nouveau constat : les Hommes d'armes écartent bien les Paysans, mais **cela les
     ralentit énormément**. Attendu à préciser avec l'utilisateur (contourner, ou traverser sans perdre de vitesse).
  3. Mur ennemi : **OK** (« ça fonctionne »).
- **Choix de l'utilisateur (C, la recommandation)** pour un mur d'unités à nous immobiles : **contourner si le détour
  est court, sinon traverser sans ralentir** (la règle de D293, « contourner avant de bousculer », sans la perte de
  vitesse en écartant). Écartés : A (toujours traverser sans ralentir) ; B (toujours contourner). Au GDD à la clôture
  du chantier (MV.6).
- **Essai de l'utilisateur : `formation_obstacles`** : le scénario est resté « en préparation » (journal : le
  rectangle `hol.Select.Box` en fractions d'écran n'a rien pris dans sa fenêtre, `Selection: none`, attente expirée à
  l'étape 58) ; cas joués **à la main** :
  1. Passage étroit (6 Paysans) : **OK**.
  2. Passage large (6 Hommes d'armes) : **OK**.
  3. Veine de pierre : **KO**. « Ils heurtent la mine, s'écrasent les uns contre les autres puis passent à côté, au
     lieu de la définir comme un obstacle depuis le début. » La formation (places et marche) doit tenir compte de la
     ressource dès l'ordre, pas la découvrir au contact.
  4. Lisière des arbres : **KO**, même chose que pour la mine.
- Correctif des scénarios : sélections par `hol.Select.BoxGround` (coordonnées au sol, centre du slot 0 à −15000, 0 sur
  Plain) au lieu de `hol.Select.Box` (fractions d'écran).
- Vérifié sans fenêtre après le correctif : `crossing`, `blocked_bodies`, `formation_obstacles` vont jusqu'au bout,
  chaque sélection prend le bon groupe.
- **Retour d'essai de l'utilisateur transmis par mycrusader-43** (J7c.4, scénario `melee_reinforce` de
  `HeroesOfLiberty-Elrond`, branche `j7c-shot`, `hol.Simulation.FrontNeedsCombat 1` ; règles de mêlée e7e0151 :
  `FrontHoldsFriends`, `EnemySideGuard`, `RearSlide`) : 50 Hommes d'armes envoyés par simple clic au sol dans le dos de
  leur propre ligne en mêlée. « Les renforts ont buté plus d'une fois contre le mur, ce qui les a empêchés de bien se
  déployer » ; « certains sont restés en place » (une partie n'a pas rejoint la ligne). Classé par l'utilisateur hors
  scénario, pour ce chantier : même famille que le cas 3 « mur à nous » (choix C : contourner si le détour est court,
  sinon traverser sans ralentir), avec en plus une ligne amie qui **tient** (`FrontHoldsFriends`) et ne se laisse
  peut-être pas écarter. À reprendre avec ce scénario quand `j7c-shot` sera sur `main`.
- **Essai de l'utilisateur : `crossing`** (scénario allé jusqu'au bout dans sa fenêtre) : « c'est un évitement parfait,
  aucun problème », mais **à partir de la paire 2** (Hommes d'armes de face, puis angle droit et groupes), **les unités
  qui s'évitent tremblent (droite-gauche)** pendant l'évitement. Le croisement de face de M1 est réglé (D293) ; nouveau
  défaut : oscillation du cap pendant le détour. Pistes à lire : D293 (détour par pas de 10°, « le même côté à chaque
  pas pendant qu'il le dépasse ») contre l'évitement de Mass (D220 / D230, un pas sur six, impulsion retenue 5 pas),
  qui peuvent tirer en sens contraires ; ou l'orientation affichée qui suit la vitesse pas à pas. Pas vu sur la paire
  1 (Paysans) : différence de vitesse (523 contre 600 cm/s) ou de hasard.
- **Retour d'essai de l'utilisateur transmis par mycrusader-43** (J7c.4, scénario `crowds_cross` de
  `HeroesOfLiberty-Elrond`, `j7c-shot`) : 100 contre 100 Hommes d'armes **ennemis** qui se croisent par simple ordre
  de marche, avec la garde côté ennemi (`EnemySideGuard`) partout : « ils ont énormément de mal, mais ils traversent ;
  je pense qu'on ne veut pas qu'ils se bousculent, l'ennemi est un mur ». Variante B (garde assouplie) rejetée : les
  foules tremblaient, s'interpénétraient puis se bloquaient. **Rangé au cas 3** (l'ennemi est un mur qu'on ne bouscule
  pas : le contourner), à grande échelle et avec deux murs qui marchent ; lien avec le cas 1 (évitement en
  croisement, tremblement). Attendu : deux foules ennemies se croisent en se contournant, sans se pousser ni se
  bloquer. À reprendre avec ce scénario quand `j7c-shot` sera sur `main`.
- **Essai de l'utilisateur : `standing_overlap`** :
  1. Tas : « ils ne se sont pas écartés d'eux-mêmes, mais ce n'est pas grave à l'arrêt ; quand je leur ai dit de bouger,
     ils se sont bien mis. » **Accepté tel quel** : le cas 2 est clos sans changement (MV.3 sans objet). Les
     superpositions entre ennemis en mêlée (S3) relèvent du cas 3, pas de celui-ci.
  2. Même point donné un par un à 9 Paysans : **OK**, côte à côte à l'arrivée.
- **Bilan MV.1** : à traiter, par ordre du journal : tremblement en s'évitant (cas 1) ; passage étroit où personne ne
  passe, mur à nous qui ralentit (choix C), renforts contre leur ligne en mêlée, foules ennemies qui se croisent (cas
  3) ; formation qui découvre mine et arbres au contact (cas 4). Clos : croisement de face (D293), immobiles
  superposées (accepté), mur ennemi, passages entre maisons, même point donné un par un.
