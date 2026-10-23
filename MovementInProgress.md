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
| 1. Croisement de face | Croisement de deux unités de face (avant M5) | `crossing` | **réglé** (MV.2) : évitement OK, plus de tremblement, croisement de biais réglé |
| 2. Immobiles superposées | Unités immobiles qui restent superposées (vitesse d'arrêt nulle) | `standing_overlap` | essayé : **accepté** par l'utilisateur (pas grave à l'arrêt) |
| 3. Marcheurs bloqués par des corps | Marcheurs bloqués par des corps (passage étroit, mur d'unités) | `blocked_bodies` | essayé : passage étroit **KO**, mur à nous **KO** (trop lent), mur ennemi OK |
| 4. Formation gênée | Formation gênée entre les bâtiments et contre les ressources | `formation_obstacles`, `formation_resources` | **réglé** (MV.5) : maisons OK ; veine, filon et arbres OK |

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
| MV.2 Croisement de face : clos par une mesure, ou instruit | **fait** (2026-10-22) : tremblement réglé (détour mémorisé), règle de croisement (le second passe derrière) ; essai « super » ; `main` 35a8be9 (fusion deb9b93) |
| MV.3 Immobiles superposées | **sans objet** : accepté par l'utilisateur (2026-10-22) |
| MV.4 Marcheurs bloqués par des corps | après MV.1 |
| MV.5 Formation gênée | **fait** (2026-10-22, avancée avant MV.2 / MV.4 à la demande de l'utilisateur) : arrivée et marche, essai de l'utilisateur « 0 problème détecté », `main` e123afc (fusion 5bd8b98) |
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
- **MV.5 avancée** (demande de l'utilisateur, « oui, go ») : le cas 4 touche les places et le chemin du bloc, pas la
  mêlée ; mené avant MV.2 / MV.4, sans attendre la clôture de J7.
- **Cause** (lecture du code, `HOLUnitOrderProcessor.cpp`) : les sources sont bien interdites sur le navmesh (D297,
  `HOLSourceNavComponent`), mais :
  1. **Arrivée** : `ProjectFormationPlace` ramène une place hors du navmesh vers le point cliqué (3/4, 1/2, 1/4 du
     chemin, sinon le point lui-même) : plusieurs places tombent au même endroit, les unités s'écrasent en tas.
  2. **Marche** : le bloc suit le chemin de son centre ; une file dont la case sort du navmesh n'est resserrée vers le
     chemin qu'à ce pas-là (rayon du chemin vers la case) : découverte au contact. Dans `formation_obstacles`, le chemin
     des 9 soldats passe à ~4 m du centre de la veine (rayon 3 m) : les files de droite la heurtent.
- **Choix de l'utilisateur pour l'arrivée (A, la recommandation)** : chaque place bloquée va à la **place libre la plus
  proche**, à au moins un écart des autres places, du côté libre ; la formation reste au point cliqué et contourne la
  ressource. Écartés : B (toute la formation décalée hors de la ressource, forme intacte) ; C (places bloquées retirées,
  rangs rallongés du côté libre).
- **Choix de l'utilisateur pour la marche** : « B si ce n'est pas trop long de faire le tour, sinon A » : le chemin du
  bloc s'écarte de la ressource pour garder sa demi-largeur libre de chaque côté quand le détour reste court ; sinon
  les files se **resserrent d'avance** (chacune regarde quelques mètres devant elle sur le chemin, glisse de biais
  avant la ressource, reprend sa place après). Personne ne touche la ressource.
- **MV.5.1 Arrivée faite** (sous-agent, worktree `HeroesOfLiberty-J5c`, branche `mv5-formation` depuis `main` 0886717,
  rien de commité) : règle pure `HOLFormation::SettleBlockedPlaces` : chaque place bloquée cherche sur des anneaux
  autour d'elle (pas d'un demi-écart, jusqu'à 8 écarts) le point libre du premier anneau qui convient le plus proche de
  la cible, à 0,9 écart au moins des autres places ; sans point libre, l'ancien repli (3/4, 1/2, 1/4, la cible).
  Surcharge de `PlaceGroup` avec `IsFree` ; côté serveur `FindFreeFormationPoint` / `IsFormationPlaceFree` (même test
  qu'avant : navmesh à 0,4 écart, ligne droite depuis la cible). Places d'arrivée et places de mise en rang du bloc ;
  les écarts du bloc en marche restent ceux de la grille d'origine (sinon il marcherait déformé). Test
  `Formation.Settle` (veine de 3 m à 1 m de la cible ; cas sans point libre). Compilation PASS ; `Formation` 11/11 ;
  `quick_check -Parts tests,formation` PASS (180 s, tests 91/91). Coût au pire ~850 sondes navmesh par place bloquée
  sans point libre à 8 écarts (une fois par ordre). Pas encore vu en jeu.
- **MV.5.2 Marche faite** (sous-agent, même branche, rien de commité) : règle pure `HOLFormation::MakeCorridor`
  (échantillons tous les 100 cm du chemin du bloc ; par échantillon le resserrement le plus grand pour lequel un
  décalage d'au plus `MaxShift` suffit, puis resserrement et décalage lissés à la pente tan 20° avant et après
  l'obstacle) et `GetCorridorAt`. `FBlockMarch` garde le couloir, construit au premier pas de chaque bloc
  (`BuildCorridor` : 2 rayons navmesh en travers par mètre de chemin, portée la demi-étendue + `MaxShift` + 20 cm,
  1000 cm au plus ; `MaxShift` = max(demi-largeur du bloc, 300 cm)), refait quand un bloc absorbé ou un retardataire
  élargit l'étendue. Chaque unité lit le couloir à sa propre place le long du chemin (`Progress` + sa profondeur) : sa
  case = point + (profondeur, décalage + resserrement × écart). Le rayon qui ramène la case vers le chemin reste en
  filet. Test `Formation.Corridor` (champ libre, obstacle d'un côté, passage étroit, décalage plafonné) ; `Formation`
  12/12, compilation PASS. Limites connues : décalage et resserrement bornés chacun en pente, pas leur somme ; les
  points de départ de l'attribution d'arrivée ignorent le couloir.
- Scénario d'essai **`formation_resources`** (J5c) : 16 Hommes d'armes qui longent le filon d'or, 9 soldats mêlés
  contre la veine de pierre, 9 Hommes d'armes à la lisière des arbres.
- **MV.5.3 Vérification** (sous-agent, hors écran) : `quick_check -Parts tests,formation,move` **PASS** (228 s, tests
  92/92, marches de bloc toutes PASS). `formation_resources` : le repère du centre est à **X −14800** dans le monde
  (pas −15000, `hol.Sources.Nearest` : filon d'or (−11087, 4592), veine de pierre (−11087, −4592), rangée d'arbres à
  y −1781 / −1816) ; les ordres des cas veine et arbres ne mettaient aucune place sur la ressource : corrigés
  (`OrderNear 3950 −4250` à 4,1 m du centre de la veine ; `OrderNear 115 −1650`, rang du fond sur les arbres). À noter :
  `formation_obstacles` (MV.1) a le même décalage.
  1. Filon d'or (bloc de 16) : **OK** : couloir « shift −300 cm at most, squeeze 1.00 » ; le bloc s'écarte ~12 m avant
     le filon, au plus près 323 cm de son centre (rayon 300), paire la plus proche en marche 120 cm, revient sur sa
     ligne après.
  2. Veine de pierre : **OK** : places déplacées, au plus près 369 cm du centre, paire la plus proche 131 cm, au repos.
  3. Arbres : **OK à l'image** (en colonne le long de la lisière, à 124 cm d'un arbre au plus près), mais une paire à
     **103 cm** : le contrôle d'écart portait sur la place avant sa projection (jusqu'à 0,4 écart). Correctif :
     `IsFormationPlaceFree` exige le point du navmesh à 5 cm de la place (`FreePlaceMaxPull`) ; compilé, à revérifier.
- **Revérifié après `FreePlaceMaxPull`** : veine : paire la plus proche au repos **140 cm**, 389 cm du centre ; arbres :
  **139 cm**, 145 cm d'un arbre ; les deux « settled after 3 steps, 0 crossings, 0 pushes » ; `Formation` 12/12.
  Cas 1 du scénario : `wait 15` (le bloc arrive en 13,3 s). Reste : essai de l'utilisateur sur `formation_resources`
  (`play.ps1 formation_resources -Root ..\HeroesOfLiberty-J5c`), puis commit et fusion dans `main` après validation.
- **Essai de l'utilisateur sur `formation_resources`** : « 0 problème détecté ». **MV.5 close.**
- Commit `e123afc` (code, tests, scénario) sur `mv5-formation`, `main` 7f0e206 fusionné dedans (5bd8b98) :
  compilation PASS, `quick_check -Parts tests,formation,move` PASS (210 s, tests 92/92) ; `main` avancé à 5bd8b98
  (accord de mycrusader-c1, seule session de la copie principale). Ligne d'`EtapeDefini.md` retirée.
- Décisions à reporter au GDD à la clôture du chantier (MV.6) : arrivée A (place libre la plus proche), marche « B si
  le détour est court, sinon A ».
- **MV.2 commencée** (demande de l'utilisateur, « tu peux travailler sur MV.2 ? ») : le tremblement droite-gauche en
  s'évitant (paires 2 à 4 de `crossing`). Worktree `HeroesOfLiberty-J5c`, branche `mv2-tremble` depuis `main` d368a3c.
  Attention : `j7c-shot` (mycrusader-43, pas encore sur `main`) change beaucoup `HOLSimulationProcessors.cpp` et
  `HOLParallelMassProcessors.cpp` ; question posée à mycrusader-43 (touche-t-elle l'évitement hors combat, quand
  arrive-t-elle). Diagnostic d'abord (sous-agent, mesures par pas de la paire 2), sans correctif.
- **Diagnostic MV.2** (sous-agent, hors écran, paires 1 et 2 de `crossing`, trace temporaire `hol.Units.LookAheadTrace`
  laissée non commitée dans `HOLSimulationProcessors.cpp`) : **cause = le regard en avant (D293, `AvoidBodies`) n'a pas
  de mémoire**. Chaque pas repart de la direction « tout droit vers la place » (phases `FinalLeg` et `Path` ; la force
  de Mass n'y joue pas), et l'autre est prévu à sa vitesse du pas d'avant. Au pas N, chacun voit l'autre en route de
  collision et tourne de 10° ; au pas N+1, l'autre s'écarte déjà, tout droit est libre, chacun repart tout droit ; au
  pas N+2, collision prévue de nouveau. **Période 2 pas (10 Hz)** : Hommes d'armes, détour 0 / −10° un pas sur deux,
  11 inversions en 0,6 s, saut de cap jusqu'à 31° par pas, déport 2 à 7 cm par pas. L'évitement de Mass (un pas sur
  six) n'y est pour rien. **Les Paysans (paire 1) tremblent aussi**, plus fort : de face exacte, les deux tournent du
  même côté du monde, restent face à face et repassent la ligne à chaque pas (cap ±20 à ±48°, sauts jusqu'à 92°) ;
  les positions zigzaguent, pas seulement l'affichage. L'orientation affichée suit `atan2(vitesse)` à chaque pas
  (`UHOLInstantOrientationProcessor`) : chaque bascule se voit telle quelle.
- Pistes : (1) garder le détour en mémoire (côté et angle tant que le même gêneur est devant, relâché de 10° par pas
  quand la voie est libre : ce que dit D293) ; (2) évitement réciproque (chacun la moitié de l'écart, l'autre prévu à
  sa vitesse voulue) ; départage fixe de face (sa propre droite) dans les deux cas ; (3) rotation affichée bornée, en
  complément seulement.
- **Choix de l'utilisateur (A, la recommandation)** : **détour mémorisé** : chaque unité garde son côté et son angle de
  détour tant que le même gêneur est devant elle, ne revient vers sa place que de 10° par pas et seulement quand la voie
  est libre ; de face exacte, chacune passe sur **sa droite**. Écartés : B (évitement partagé à la RVO, chacun la
  moitié de l'écart) ; C (A + rotation affichée bornée). Au GDD à la clôture (MV.6), comme précision de D293.
- `j7c-shot` touche `BuildLookAhead` et ajoute du code après `AvoidBodies`, pas son corps : correctif tenu dans
  `AvoidBodies` et un petit fragment de mémoire, pour limiter les conflits à la fusion.
- **Réponse de mycrusader-43** : `j7c-shot` arrive sur `main` maintenant (fusion de MV.5 dedans, puis avance de
  `main`) ; ne pas avancer `main` d'ici là. Hors combat, `j7c-shot` change la séparation : `EnemySideGuard` (une paire
  ennemie écartée le long de la normale du début du pas, jamais au-delà du centre) et `SeparationMaxStep` (poussée
  bornée à un rayon) valent pour **toute paire ennemie**, combat ou non ; `FrontHoldsFriends` seulement où il y a
  combat ; `RearSlide` seulement pour les unités qui vont se battre. **Le détour D293 lui-même n'est pas touché.**
  `mv2-tremble` sera remise sur ce `main` et revérifiée avant l'essai.
- **Boucle 1 du correctif (sous-agent, 15 min, rien de commité)** : règles pures `HOLAvoid::ChooseDetourSide` (côté
  opposé au corps ; de face, à moins de 0,15 contact de travers, sa propre droite) et `NextDetourTurn` (du cran retenu :
  revenir d'un cran si libre, sinon le garder, sinon élargir ; jamais 0) ; fragment `FHOLDetourFragment` (gêneur,
  ordre, phase, côté, cran) ; tests `Movement.DetourSide` / `DetourTurn`. Mesures (trace) : **paire 1 réglée** (sauts
  92° → 10°, une inversion) ; **paire 2 encore** : le cran alterne 10° ↔ 20° du même côté un pas sur deux (sauts 31° →
  20°) ; **paire 3** (angle droit) pas mieux (14 inversions, sauts jusqu'à 82°) ; groupes 0 à 9 inversions. Personne
  bloqué. Reste : le retour d'un cran paraît libre parce que l'autre est prévu à sa vitesse du pas d'avant.
- Bilan de la boucle : la mise en place de la mesure a pris la moitié du temps ; elle est prête maintenant (banc de
  40 s). `main` 48ffa00 (`j7c-shot`) fusionné dans `mv2-tremble`, conflits d'ajouts résolus à la main (`AvoidBodies`
  prend le joueur et la mémoire), compilation PASS.
- **Boucle 2 (sous-agent)** : **hystérésis** : le retour d'un cran n'a lieu qu'après 4 pas d'affilée où ce cap plus
  étroit est libre (`ClearSteps` dans `FHOLDetourFragment`, `HOLAvoid::EaseBackSteps = 4`) ; élargir reste immédiat.
  Paire 3 : un détour élargi jusqu'à 60° (les deux couraient côte à côte) était oublié au bout et le côté re-choisi à
  l'opposé (saut de 82°) : maintenant, corps passé ou aucun cran libre jusqu'à 60° → il garde son côté et revient d'un
  cran par pas (puis pousse tout droit, D293). Tests `Movement` + `Formation` 18/18.
  | Cas | Avant : inversions / saut max / virage max | Après |
  |---|---|---|
  | Paire 1 (Paysans de face) | 1 / 10° / 10° (boucle 1) | 1 / 10° / 10° |
  | Paire 2 (Hommes d'armes de face) | 11-12 / 20-21° / 20° | 1 / 20° / 20° |
  | Paire 3 (Paysans à angle droit) | 14 / 82° / 60° | 1-3 / 12-20° / 60° |
  | Groupes de 5 | 0-9 / 31° / 50° | 1-5 / 11-50° / 30-60° |
  Sauts restants > 12° : seulement le premier virage d'un détour (20°, jusqu'à 50° pour un nouveau détour en route) et
  l'élargissement de sécurité (20° par pas). Reste : à angle droit, les deux s'écartent du même sens et courent côte à
  côte jusqu'à 60° (choisir le côté selon la vitesse de l'autre, passer derrière celui qui croise : nouvelle règle, à
  faire décider) ; détours un peu plus longs (paire 3 : 30-34 pas au lieu de 20). Trace `hol.Units.LookAheadTrace` à
  retirer avant le commit.
- `quick_check -Parts tests,move,formation` (boucle 2) : tout PASS (tests 94/94) **sauf la partie « backsteps »** : une
  marche en bloc à « file gap 99 cm » (60 au plus) ; aucun pas en arrière. Cause probable : la mémoire garde plus
  longtemps hors de sa file un membre qui évite un voisin du bloc. Boucle 3 (sous-agent) : cause et correctif.
- **Boucle 3 arrêtée à 15 min** (sous-agent) : cause trouvée (avec la mémoire, des membres d'un bloc dérivaient jusqu'à
  2,4 m de leur file en marche) ; correctif : **pas de mémoire autour d'une unité du même ordre** (membre de son bloc,
  unité qui se met en rang : sa file la ramène au pas suivant), dans `AvoidBodies`. A/B sur le croisement : identique
  (le tremblement reste réglé). Arrêté pendant la recompilation finale, avant le `quick_check` : relancé par la session
  principale. Bilan : la boucle a fait cause + correctif + A/B, trop pour 15 min avec un `quick_check` de 3,5 min ;
  à l'avenir, la session principale lance elle-même la vérification finale.
- Recompilé ; `quick_check -Parts tests,move,formation` **PASS** (196 s, tests 94/94 ; dérive de file en marche 32 à
  40 cm, aucun pas ralenti, aucun pas en arrière). Essai de l'utilisateur lancé : `play.ps1 crossing` depuis J5c.
- **Essai de l'utilisateur sur `crossing`** : « tout fonctionne », mais l'angle droit **gêne** (les deux Paysans
  s'écartent dans le même sens et courent côte à côte, détour jusqu'à 60°) : règle de croisement à décider.
- **Choix de l'utilisateur pour le croisement de biais (A, la recommandation)** : celle qui arriverait **la seconde**
  au point de croisement **passe derrière** l'autre (tourne vers le côté d'où l'autre vient) ; la première **garde sa
  route** ; arrivée quasi simultanée : **priorité à droite** (celle qui a l'autre sur sa droite passe derrière).
  Écartés : B (toujours la priorité à droite) ; C (ralentir au lieu de tourner). Boucle 4 (sous-agent).
- **Boucle 4 (sous-agent)** : règle pure `HOLAvoid::ChooseCrossingRole` (croisement si l'écart des caps est entre 30° et
  150° ; on prolonge les deux trajectoires, la première au point garde son cap, la seconde passe derrière, du côté d'où
  vient l'autre ; à 0,2 s près, priorité à droite) ; rôle décidé une fois au premier contact prévu, gardé tant que
  l'autre est proche (`Crosser`, `CrossRole`, `CrossSide` dans `FHOLDetourFragment`), jamais entre unités du même
  ordre ; « garder le cap » ignore l'autre sauf contact prévu à moins de 0,2 s (détour de sécurité). Test
  `Movement.CrossingRole`. Banc : paire 3 : une unité garde son cap (0°), l'autre passe derrière (virage 40° au lieu de
  60°, 20 pas de détour au lieu de 35) ; plus de course côte à côte ; paire 1 10° ; paire 2 3-4 inversions / 35° avec
  ou sans la règle (état du code après la fusion de `main`, pas cette règle) ; groupes à peu près pareils. Reste : le
  rôle est décidé tard (horizon 0,5 s), d'où un virage de 40° ; pistes : horizon de croisement plus long, ou ralentir.
- `quick_check -Parts tests,move,formation` **PASS** (196 s, tests 95/95). 2e essai de l'utilisateur lancé (`crossing`).
- **2e essai de l'utilisateur sur `crossing`** : « super ». **MV.2 close.**
- Trace temporaire `hol.Units.LookAheadTrace` et scénario `_mv2_check` retirés. Commit `35a8be9` sur `mv2-tremble`,
  `main` d7fa072 fusionné dedans (c0df8c4) : compilation PASS, `quick_check -Parts tests,move,formation` PASS (199 s,
  tests 96/96) ; fusion dans `main` (deb9b93 ; `main` n'avait reçu que deux commits de journal). Ligne
  d'`EtapeDefini.md` retirée.
- Décisions à reporter au GDD à la clôture (MV.6), comme précisions de D293 : détour mémorisé (côté et angle gardés
  tant que le même corps est devant, retour d'un cran de 10° après 4 pas libres d'affilée ; de face, sa propre droite ;
  pas de mémoire entre unités du même ordre) ; croisement de biais : la seconde au point passe derrière, la première
  garde sa route, à 0,2 s près priorité à droite. Reste connu : le rôle de croisement se décide à 0,5 s du contact
  (virage de 40° pour celle qui passe derrière) ; accepté à l'essai.
- **Reprise (prochaine session)** : **MV.4** (marcheurs bloqués par des corps : passage étroit où personne ne passe ;
  mur d'unités à nous qui ralentit, choix C « contourner si le détour est court, sinon traverser sans ralentir » ;
  renforts contre leur ligne en mêlée, `melee_reinforce` ; foules ennemies qui se croisent, `crowds_cross`, ces deux
  scénarios étant maintenant sur `main` avec `j7c-shot`), puis **MV.6** (décisions de MV.2 et MV.5 au GDD, clôture).
  Travailler dans un worktree libre (`HeroesOfLiberty-J5c`, `IN_USE.txt`), compiler par `Toolsuild.ps1`.
- **MV.4 commencée** (« continuons le chantier déplacement ») : worktree `HeroesOfLiberty-J5c`, branche `mv4-bodies`
  depuis `main` 0ba89e9 ; `HeroesOfLiberty-Elrond`, branche `mv4-diag` (traces temporaires seulement). Attention :
  `HeroesOfLiberty-Haldir` (D345, en cours) touche `HOLUnitOrderProcessor.cpp` et `HOLCombatProcessor.cpp`.
  Diagnostic d'abord, sans correctif, deux sous-agents en parallèle : (a) passage étroit + (b) mur à nous dans J5c
  (`blocked_bodies`) ; (c) renforts contre leur ligne + (d) foules ennemies dans Elrond (`melee_reinforce`,
  `crowds_cross`).
- **Diagnostic (a) + (b)** (sous-agent, J5c, 11 min, trace temporaire `hol.Units.BodiesTrace` laissée non commitée dans
  `HOLSimulationProcessors.cpp`, `UHOLFullSpeedProcessor::Execute` autour d'`AvoidBodies`) : **aucun des deux défauts
  ne se reproduit sur `main` 0ba89e9** (une passe hors écran ; `-nullrhi` impossible, le scénario attend l'interface :
  `-RenderOffScreen`). Probablement réglés depuis MV.1 par MV.2 (détour mémorisé, chacun son côté) et D344 / D345.
  1. Passage étroit : les deux Paysans partent au premier pas (40 cm/pas), aucune redemande de chemin, chacun garde son
     côté ; ils passent **de front**, à 108 cm l'un de l'autre (chevauchement ~12 cm, ~9 cm dans le corps d'une maison :
     les corps des maisons laissent 2 × `BodyInset` = 210 cm pour un Paysan de 120 cm). Plus de blocage, mais le
     passage n'est plus « un seul Paysan » (D324). « Ils ne bougent pas au début » pas reproduit hors écran.
  2. Mur de Paysans à nous : les 3 Hommes d'armes **contournent à pleine vitesse** (34-35 cm/pas = 697 cm/s, détour
     jusqu'à 50°, aucun pas ralenti) : c'est déjà le choix C pour un mur court.
  Lu dans le code, non mesuré : un mur trop large pour le détour (aucun cap libre jusqu'à `AvoidMaxAngle`) → tout
  droit, et `HOLCombat::GetOverlapShare` (`HOLCombatRules.cpp:68`) donne 0,5 entre amis même marcheur contre immobile :
  le marcheur rend la moitié du chevauchement, d'où le ralentissement. Piste : 0 pour le marcheur contre un ami
  immobile (hors unités engagées), à mesurer sur un mur plus large (~30 Paysans sur 600).
  Suite proposée : nouvel essai de l'utilisateur sur `blocked_bodies` avant tout correctif.
- **Essai de l'utilisateur relancé** : `play.ps1 blocked_bodies -Root ..\HeroesOfLiberty-J5c` (cas 1 et 2 à revoir).
- **Diagnostic (c) + (d)** (sous-agent, Elrond, 11 min, trace temporaire `MV4DIAG` non commitée dans
  `HOLSimulationProcessors.cpp`, une passe `-RenderOffScreen` par cas ; journaux dans le scratchpad de la session) :
  - (c) `melee_reinforce` : le simple clic sur 50 fait **une marche en bloc** ; le bloc se **fige** (X moyen immobile
    ≥ 17 s, à ~8 m du point d'ordre), ses 49 membres restent « en marche » (31 `Block`, 18 encore `FormUp`), arrivée
    jamais déclarée. Causes : `AvoidBodies` ne trouve aucun cap libre à 60° contre la ligne (≈ 700 « tout droit » par
    0,5 s contre 0-30 détours) → pousse ; `FrontHoldsFriends` lui rend tout le recouvrement (300-560 fois / 0,5 s) →
    bute ; le bloc marche au pas de son membre le plus lent (`SlowestOfOrder`, « held ») → tout le bloc gèle ; aucun
    filet en `Block` / `FormUp` (`PathStuckSteps` seulement en `Path`, `FinalLegStuckSteps` près des cases) ; places
    d'arrivée dans la ligne qui se bat. Pistes : A places d'arrivée hors de la ligne (`SettleBlockedPlaces` avec les
    unités du front comme obstacles) ; B le bloc cesse d'attendre une file bloquée par un mur (ou se défait en jambes
    finales) ; C contournement plus large contre le front ami. Recommandation du sous-agent : A + B, C ensuite.
  - (d) `crowds_cross` : au contact, aucun cap libre (le front d'en face est un mur de la largeur du bloc) → 1 000 à
    1 300 « tout droit » par 0,5 s, jusqu'à 2 444 paires ennemies recouvertes (834 m cumulés) ; ~4 s de bousculade
    puis traversée de biais ; arrivée désordonnée (joueur 0 : 33 pas en arrière ; joueur 1 : 17 s, 183 pas en arrière,
    ~9 s presque immobile à 6,6 m du but ; dissymétrie non expliquée). Le chemin du bloc ignore l'autre bloc ; la règle
    de croisement de MV.2 ne joue pas de face exacte. Pistes : A **le bloc évite le bloc** (chemin / couloir du bloc
    écarté de l'enveloppe du groupe d'en face, chacun sa droite de face, hors attaque-déplacement et mêlée) ; B
    évitement par unité plus loin et plus large contre les ennemis ; C pas de poussée contre un ennemi (risque : les
    deux foules se figent). Recommandation du sous-agent : A.
- **Essai de l'utilisateur sur `blocked_bodies`** : « les deux Paysans peuvent désormais passer côte à côte, donc ça
  fausse le résultat » ; « l'évitement fonctionne » (mur à nous : **OK**, contourné). Le passage étroit ne respecte
  plus D324 (corps rentré de 115 cm, `Config/DefaultGame.ini` `BodyInset=115` : 230 cm pour deux Paysans de 120 cm
  qui passent à 108 cm l'un de l'autre et mordent ~9 cm dans le corps) : **régression**, à retrouver avant de juger
  le « céder le passage ». Boucle MV.4.1 (sous-agent, J5c). Reste du choix C non essayé : mur trop large pour être
  contourné (traverser sans ralentir), scénario à ajouter.
- **MV.4.1 (sous-agent, J5c, 5 min, pas de correctif)** : **pas une régression** : rien n'a jamais mis deux marcheurs
  à la file ; avant MV.2 ils se bloquaient (c'est ce qui empêchait le passage de front), depuis MV.2 chacun garde son
  côté et ils passent de front. Par pas dans le passage : séparation (`UHOLUnitSeparationProcessor`, poussée en travers
  du passage, jamais l'un derrière l'autre) puis garde des emprises (`UHOLFootprintKeepOutProcessor`, rayon +
  `KeepOutSlack` = 61 cm du mur, a le dernier mot) → centres à 230 − 2 × 61 = **108 cm, 12 cm de chevauchement**,
  17 pas sans ralentir, arrivés en ~6 s. Les « 9 cm dans le corps » n'existent qu'à l'intérieur du pas. Chemins
  parallèles (navmesh `AgentRadius=60` : bande de 110 cm), `role=0` (la règle de croisement ne joue pas entre
  marcheurs parallèles). « build gap two » (graine 113) passait sans doute parce que les Paysans naissent décalés.
  Options : A céder dans la garde des emprises (chevauchement restant réglé le long du passage : celui de derrière
  cède ; ex æquo : plus petit index, D231) ; B les murs dans `AvoidBodies` (plus coûteux, départage quand même
  nécessaire) ; C ligne du milieu dans un passage étroit + règle de croisement élargie aux marcheurs qui convergent.
  Recommandation du sous-agent : A. Question à poser après celle des renforts (c).
- **Demande de l'utilisateur** : le passage étroit se teste avec **5 Paysans**, pas 2. `blocked_bodies` (J5c) : cas 1
  à 5 Paysans (naissance rayon 150, sélection élargie, note D324 « à la file, un seul de front ») ; vérifié hors écran
  jusqu'au bout. Choix de la règle de passage (A / B / C de MV.4.1) après l'essai à 5.
- **Renforts (c), réponse de l'utilisateur** : « ce qui pose problème, c'est que dès qu'un élément d'un bloc est au
  combat, il n'y a plus de bloc et formation qui tienne » ; A / B / C proposés (places hors de la ligne, etc.) écartés
  tels quels. Question de précision posée.
- **Précision de l'utilisateur (« les deux »)** : (1) la **ligne qui se bat perd sa formation dès le premier combat** :
  un bloc doit garder sa forme au combat (les renforts trouveraient une ligne nette) ; (2) les **renforts** doivent se
  défaire (chacun finit seul) en arrivant à la mêlée, au lieu que tout le bloc gèle. Diagnostic du (1) d'abord
  (sous-agent, Elrond : que devient un bloc quand un membre entre au combat), avant de proposer les règles.
- Essai de l'utilisateur lancé : `blocked_bodies` à 5 Paysans (J5c).
- **Essai de l'utilisateur sur `blocked_bodies` à 5 Paysans** : passage étroit **très bien** (cas a clos sans règle
  nouvelle ; options A / B / C de MV.4.1 sans objet). **Mur ennemi : KO, régression** depuis MV.1 (« OK » alors) :
  relancé plusieurs fois, **un seul Paysan sur 3 passe**, « ils foncent dans le pack d'ennemis ». Suspects : MV.2
  (détour mémorisé, règle de croisement), `j7c-shot` (`EnemySideGuard`, `SeparationMaxStep`). Boucle MV.4.2
  (sous-agent, J5c) : cause et correctif.
- **Demande de l'utilisateur** : « il devrait y avoir des tests qui vérifient ça pour éviter les régressions ». Constat :
  `quick_check` n'a que « build gap two » (2 Paysans, passait parce qu'ils naissent décalés) et « orders push » ; rien
  pour le croisement (MV.2), le mur à nous, le mur ennemi ; MV.2 / MV.5 n'ont ajouté que des tests de règles pures.
  **MV.4.3 ajoutée** : nouvelle partie `quick_check` **« bodies »** (cible ~1 min), chaque cas jugé par un chiffre :
  croisement (paires de `crossing` : arrivées, inversions de cap bornées) ; passage étroit à 5 (tous passent, un seul
  de front) ; mur à nous (contourné sans pas ralenti) ; mur ennemi (3 sur 3 arrivés, recouvrements ennemis ~0). Écrite
  après le correctif du mur ennemi, et vérifiée **en échec sans le correctif** (preuve qu'elle attrape la régression).
  Lancée à chaque commit qui touche le déplacement (`-Parts bodies`).
- **Diagnostic « bloc au combat »** (sous-agent, Elrond, 14 min ; compteurs ajoutés à la trace `MV4DIAG`, non
  compilés : deux jeux `-nullrhi` du premier diagnostic, bloqués depuis 17 h 32, tiennent les DLL d'Elrond ; chiffres
  tirés de la passe précédente) :
  - **La ligne qui charge n'est jamais un bloc** : charge d'~28 m, trop courte pour une marche en bloc
    (`BlockMarchMinDistanceInDepths`, `HOLFormationRules.h:242-251`) : chacun marche seul vers sa case (`FinalLeg`),
    cases d'attaque-déplacement derrière la ligne ennemie.
  - **Au contact, tout est soldat par soldat** (`HOLCombatProcessor.cpp`) : en attaque-déplacement chacun cherche un
    ennemi (16 m) pendant sa marche, hors de portée `IssueCombatWalk` (marche à lui seul), à portée `StopWalk` (planté) ;
    face à une ligne presque tous ont un ennemi à 16 m et foncent chacun vers le plus proche. `FrontLines`,
    `FrontHoldsFriends`, `RearSlide`, `EnemySideGuard`, `RearFlank` ne tiennent **aucune forme** (ni rangs, ni largeur,
    ni retour à la case). G0 : marcheurs 81 → 65 (+2 s) → 38 (+5 s) → ~30 immobiles derrière la ligne.
  - Renforts (simple clic) : personne ne cherche d'ennemi en marche, aucun n'entre au combat, le bloc ne se défait
    jamais (fige à ~6 s).
  - Pistes, souhait 1 (la ligne garde sa forme) : A **tenir le rang** (seuls les soldats du front / à portée frappent,
    les rangs de derrière gardent leur case relative qui avance avec le front et comblent les morts) ; B recaler sur sa
    case après chaque mise à mort ; C vraie marche en bloc même courte, cases posées au contact. Reco : A. Souhait 2
    (renforts) : A le bloc se défait près de l'ennemi / quand le front ami bloque une file (tous en `FinalLeg`) ; B
    membre par membre (les bloqués quittent le bloc) ; C arrivée en garde (change le simple clic). Reco : A + filet B.
- Jeux bloqués d'Elrond (41888, 21568) arrêtés avec l'accord de l'utilisateur. À l'avenir, un lancement sans fenêtre
  se fait en `-RenderOffScreen` (jamais `-nullrhi` : le scénario attend l'interface) et avec un délai qui tue le jeu.
- **Choix de l'utilisateur pour la ligne au combat (A, la recommandation) : tenir le rang** : seul le premier rang (ou
  ceux déjà à portée) frappe ; les rangs de derrière restent à leur case, un pas derrière, et avancent pour combler quand
  un soldat devant tombe ; la ligne reste droite et nette. Écartés : B revenir à sa case après chaque combat ; C charge
  en vraie marche en bloc. Au GDD à la clôture (MV.6), avec J7-Q5 / D327 / D328 à relire.
- **Choix de l'utilisateur pour les renforts (A, la recommandation)** : le bloc marche formé ; dès que l'un d'eux arrive
  à quelques mètres de la mêlée (ennemi proche, ou ligne amie qui se bat devant lui), **tout le bloc se défait** et
  chacun finit seul jusqu'à sa case derrière la ligne ; filet : un membre bloqué plus de 2 s ne retient plus les
  autres. Écartés : B membre par membre ; C en garde à l'arrivée (changerait le simple clic).
- Suite : **MV.4.4** renforts (Elrond, sous-agent : correctif) ; **MV.4.5** tenir le rang (d'abord un plan de règles
  précises, sous-agent en lecture seule, questions de jeu à poser ensuite). Attention : `HeroesOfLiberty-Haldir` (D345)
  a des changements non commités dans `HOLUnitOrderProcessor.cpp` et `HOLCombatProcessor.cpp`.
- **Règle permanente de l'utilisateur (2026-10-22) : test rouge d'abord.** Chaque développement commence par un test
  qui échoue sur le code actuel et doit passer avec le dev ; on boucle jusqu'au vert ; le test reste comme
  anti-régression (sauf l'esthétique). Transmise aux deux sous-agents en cours (mur ennemi, renforts). MV.4.3 (partie
  `quick_check` « bodies ») en devient le socle : chaque cas réglé du chantier y a son chiffre.
- **Plan MV.4.5 « tenir le rang »** (sous-agent en lecture seule) : rang et file connus à l'ordre (`PlaceGroup`) puis
  perdus (seule la case est gardée) ; un retour à la case existe déjà (`PlaceHold` / `WatchPlace`, ligne droite sans
  chemin). Règles proposées (recommandations) : R1 seulement en attaque-déplacement d'un groupe en formation ; R2 premier
  rang = par file, le premier vivant (+ quiconque a un ennemi à portée frappe) ; R3 rang 2+ : frappe seulement à portée,
  ne marche jamais vers l'ennemi (zone de 16 m fermée) ; R4 tireurs : tirent depuis leur case ; R5 la case avance par
  file derrière le soldat de devant, front aligné sur la médiane des premiers de file, jamais au-delà de la case
  d'origine ; R6 comblement par la file, file vide → la voisine la plus fournie glisse (une par seconde au plus) ; R7
  celui qui tue reste en ligne, pas de poursuite ; R8 flanc / arrière : tout soldat frappe ce qui est à portée, files
  extrêmes et dernier rang cherchent jusqu'à ~2 m ; R9 fin : plus d'ennemi à 16 m pendant ~2 s, ou nouvel ordre, ou
  moins d'une file × 2 rangs ; R10 suit l'ordre, pas le camp (l'IA aussi) ; R11 `RearFlank` coupé pour une ligne tenue
  (contredit la boucle 21 de J7c.4, à noter au GDD). Code : `LineFile` / `LineRank` / `LineSerial` / `bHoldsLine` dans
  `FHOLMoveOrderFragment` (0 bit répliqué), règles pures `HOLFormation::GetRankAndFile` / `MakeHeldLine`,
  `HOLCombat::MayWalkToFight`, passe par ordre dans `UHOLCombatProcessor` (~0,2 ms à 2000), variante `StandInLine` de
  `StopWalk`. Tests rouges d'abord : `Formation.HeldLine.*`, `Combat.MayWalkToFight`, partie `quick_check`
  « holdline » (rapport `hol.Combat.LineReport`, en échec avec `hol.Combat.HoldLine 0`). 8 sous-étapes de ≤ 15 min.
  Risques : `melee_press` (nuées en formation), marche en bloc longue, conflit avec D345 (Haldir) dans
  `HOLCombatProcessor.cpp`. Code après MV.4.4 (même worktree, mêmes fichiers).
- **MV.4.2 mur ennemi (sous-agent, J5c, 11 min, rien de commité)** : reproduit (1/3 : e=34 pousse tout droit, e=35
  figé > 10 s dans un creux du paquet). Deux causes dans `HOLSimulationProcessors.cpp` : (1) le détour mémorisé de
  MV.2 revient d'un cran par pas quand rien n'est libre jusqu'à 60° → retombe à 0 → pousse tout droit dans l'ennemi ;
  (2) `TimeToContact` ignore un corps déjà au contact (`Gap <= 0` → `Max`) : foncer dans un ennemi touché est jugé
  libre. Pas `EnemySideGuard` (même blocage sans lui). Correctif : `TimeToContact` = 0 pour un corps au contact dont
  on se rapproche (amis compris) ; `AvoidBodies(bAroundEnemies)` (marche hors combat seulement : pas en frappe, ni
  approche de cible, ni attaque-déplacement) : contre un ennemi, élargir jusqu'à 60° et y rester au lieu de revenir,
  et sans voie libre prendre le virage le plus large de son côté au lieu de pousser. Résultats : mur ennemi 1/3 → 2/3
  (correctif 1 seul) → **3/3** (~3,8 s) ; mur à nous 3/3 ; passage à 5 : 5/5 mais deux Paysans en ~11 s au lieu de
  ≤ 4,5 s (à voir) ; `crossing` non comparé à `main`. **Pas encore de test rouge** (temps) : propositions : cas chiffré
  « bodies » (3/3 en 12 s : rouge 1/3 sur `main`) et test pur `TimeToContact` déplacé dans `HOLAvoid`.
  Suite : MV.4.3 (sous-agent, J5c) : tests rouges puis verts, partie `quick_check` « bodies », comparaison `crossing`
  et passage à 5 avec `main`.
- **MV.4.4 renforts (sous-agent, Elrond, 14 min, rien de commité)** : test `Formation.BreakAtMelee` (rouge sur la règle
  vide, vert avec : `Formation` + `Movement` 21/21). Règle pure dans `HOLFormationRules.h` : `BlockBreakDistance` 400 cm
  (ennemi ou ami au combat d'un autre ordre), `BlockMemberStuckSteps` 40 (2 s), `ShouldBreakBlock`, `IsMemberStuck` ;
  dans `AdvanceBlocks` (`HOLUnitOrderProcessor.cpp`) : relevé de la mêlée par pas (trié par id), tout le bloc en jambe
  finale (« broke up at the melee »), filet cumulé (« held the block 40 steps, left it »). Mesure `melee_reinforce` :
  plus aucun membre en bloc à 10 s (avant : gel ≥ 17 s), mais à 26 s 35 renforts encore en `FinalLeg` à ~3-4 m de leur
  case (médiane 290 cm). **Constat** : ce qui arrête les renforts n'est pas la mêlée (à 15-25 m) mais les **amis
  immobiles derrière la ligne** et les cases posées sur eux ; c'est le filet de 2 s qui défait le bloc. **Effet de
  bord** : la règle défait dès le 10e pas les deux blocs de la mise en ligne du début (ami d'un autre ordre « au
  combat » à 1,5 m). Reste : effet de bord ; cases des renforts sur des amis ; cas chiffré `quick_check` ; parties
  `move` / `formation`.
- **Réponses de l'utilisateur sur « tenir le rang »** : R1, R2, R4, R5, R6, R9, R10, R11 : **oui** (recommandations).
  **R3 précisée** : rang 2+ frappe ce qui est à portée et ne marche pas vers l'ennemi, **mais peut aller de côté pour
  se battre, avec une distance maximale** (réglage ; proposé : une file, ~2 m, sans avancer au-delà de sa case).
  **R8 précisée** : attaqué par derrière (ou le flanc), un soldat **qui n'a pas de cible se retourne pour combattre
  celui qui le frappe**, même hors des files extrêmes. R7 sans réponse (« jamais vu » à éclaircir).
- **R3 confirmée** (exemple dessiné) : un soldat de rang 2+ peut se **décaler de côté** vers un trou du premier rang ou
  un ennemi libre sur le côté pour frapper, à **une file (~2 m) au plus** de sa case (réglage), jamais en avant de sa
  case. **R7 : oui.** Les 11 règles de « tenir le rang » sont validées.
- **Choix de l'utilisateur pour les places des renforts (A, la recommandation)** : dès l'ordre, une place qui tombe sur
  un soldat à nous immobile va à la **place libre la plus proche**, de notre côté (`SettleBlockedPlaces` de MV.5 avec
  les amis immobiles comme obstacles) : les renforts se rangent derrière la ligne sans la toucher. Écartés : B pousser
  les nôtres ; C s'arrêter où ils sont. À faire après la boucle en cours (test rouge d'abord).
- **MV.4.3 boucle 1 (sous-agent, J5c, 15 min)** : règles déplacées dans `HOLAvoid` (`TimeToContact`,
  `TurnWithoutClearWay`) ; tests `Movement.TimeToContact` (7 cas) et `Movement.NoClearWay` (3 cas) : **rouge** avec les
  anciennes règles (4 vérifications), **vert** avec le correctif (`Movement` 9/9). Mesures : passage à 5 : `main`
  2,0-2,7 s, correctif 2,1-2,5 puis **5,7 / 5,9 s** ; `crossing` : sauts de cap > 25° dans les groupes de 5 : `main`
  6 et 2, correctif **12** (tous arrivent). Cause probable : « au contact → 0 » s'applique aussi aux amis. Piège : un
  fichier restauré par `Copy-Item` garde sa date, UBT ne recompile pas (toucher la date). Partie « bodies » pas faite.
  Boucle 2 : partie « bodies » d'abord (passage à 5 ≤ ~4 s : rouge sur le correctif actuel ; mur ennemi 3/3 ≤ 12 s :
  rouge sur `main`), puis correctif limité aux ennemis, jusqu'au vert.
- **MV.4.4 boucle 2 (sous-agent, Elrond, 9 min)** : `ShouldBreakBlock(bPlainMove, …)` (simple marche seulement ; un
  bloc est en attaque-déplacement si un membre a `EHOLCombatMode::AttackMove`) et `IsFightingFriend` (seulement
  `bAttacking`) ; `Formation.BreakAtMelee` : **rouge** sur 4 cas nouveaux, **vert** (`Formation` + `Movement` 21/21).
  Les deux blocs de la charge (attaque-déplacement, donc bien des marches en bloc) ne se défont plus. `quick_check
  -Parts tests,move,formation` **PASS** (202 s, tests 97/97, « held » 0 et « left » 0 sur les 12 marches en bloc).
  `melee_reinforce` : le bloc des renforts arrive (« arrived after 217 steps of march, 28 held »), la règle « près de
  la mêlée » ne joue pas ici (mêlée trop loin) ; 26-30 renforts restent en `FinalLeg` à ~3 m de leur case (cases sur
  les amis immobiles). Suite : **MV.4.6 places des renforts** (choix A), test rouge d'abord.
- **MV.4.3 boucle 2 (sous-agent, J5c, 13 min)** : partie `quick_check` **« bodies »** (cas `2p`, après `move`, ~40 s) :
  maisons bâties d'un coup en (-8800, -3200) / (-8800, -2400) ; passage étroit 5 sur 5 en ≤ 4 s ; mur ennemi 3 sur 3 à
  < 4 m en ≤ 12 s ; mur à nous 3 sur 3 en ≤ 28 m / vitesse × 1,5. **Rouge prouvé** : passage étroit sur le correctif
  complet d'avant (5,5 s). **Pas rouge** : mur ennemi sur `main` (dispersion 300 trop lâche, ils traversent en ligne
  droite) : à durcir. Correctif : `HOLAvoid::TimeToContact(..., bPressedCounts)`, « au contact → 0 » seulement contre
  un ennemi en marche hors combat (`bAroundEnemies && Owners[Other] != Owner`) ; `Movement.TimeToContact` + 2 cas
  amis. **Vert** : `quick_check -Parts tests,bodies` PASS (53 s, tests 98/98) : passage 3,0 s, mur ennemi 4,0 s, mur à
  nous 4,5 s (plafond ~6 s). Incident : une ligne écrite par erreur dans `quick_check.ps1` de la copie principale,
  remise par `git checkout` (vérifié propre). Boucle 3 : mur ennemi durci (rouge sur `main`), cas de croisement.
- **MV.4.6 places des renforts (sous-agent, Elrond, 14 min, rien de commité)** : `HOLFormation::FStillBody`,
  `IsClearOfBodies` (place prise à moins de rayon ami + rayon unité d'un ami immobile ; avec des amis, pas de place
  devant le premier rang), relevé une fois par ordre des amis immobiles (`ObstacleGrid`, trié par `UnitId`) avant
  `PlaceGroup`. Test `Formation.StillFriends` : **rouge** (4 échecs) puis **vert** (`Formation` 15/15). Mais sur
  `melee_reinforce`, **aucun gain** (19-21 posés, 29-33 en `FinalLeg`, médiane ~286 cm) : à l'ordre aucun ami n'est
  immobile (51 `Moving`, 49 `Attacking`), et les cases des bloqués ne sont pas leurs cases de formation (7-14 m devant
  la cible, parfois partagées). **Hypothèse** : une fois posés, les renforts prennent un ennemi dans leur zone
  d'agression ; leur approche de combat est un nouveau `FinalLeg` bloqué derrière la ligne qui se bat → relève du
  combat / « tenir le rang ». Cas `quick_check` « formation reinforce » écrit (6 Hommes d'armes immobiles en travers de
  la cible) mais pas lancé. Boucle suivante : vérifier l'hypothèse, rouge / vert du cas, `quick_check`.
- **MV.4.3 boucle 3 (sous-agent, J5c, 11 min)** : mur ennemi durci (12 ennemis posés un par un, deux rangs de 6
  décalés à 1,25 m, corps qui se touchent) : **rouge sur `main`** (« 3 of 3 behind it in 99 s », jamais, deux passes),
  **vert** avec le correctif (4 s). Cas croisement ajouté (deux groupes de 5 Paysans de face, 16 m) : tous arrivés en
  2,5 s ; sauts de cap > 25° affichés mais **non jugés** (0,5 s d'échantillonnage trop grossier : `main` 13 / 11,
  correctif 12). `quick_check -Parts tests,move,formation,bodies` **PASS** (238 s ; « bodies » ~41 s). Fragile : le
  mur à nous passe pile au seuil (4 s pour 4 s). Boucle 4 : compteur de volte-face côté jeu (chaque pas) pour juger le
  croisement (rouge prouvé sans le détour mémorisé de MV.2), seuil du mur à nous revu.
- **MV.4.6 diagnostic (sous-agent, Elrond, 15 min)** : **hypothèse confirmée** : dès +12 s, 100 % des renforts bloqués
  en `FinalLeg` à plus de 1,5 m de leur case (30-37) sont en **approche de combat** (case = position de leur cible),
  sans jamais frapper, à 2,3-4,6 m de la cible, la ligne amie entre les deux ; l'approche est relancée toutes les ~2 s
  (`HOLCombatProcessor.cpp` ~1165-1177, cible hors de portée → `IssueCombatWalk` vers l'ennemi), parfois vers une autre
  cible. MV.4.6 (places hors des amis immobiles) n'y peut rien : à garder seulement si le cas « formation reinforce »
  est rouge sans elle. Options de jeu pour un renfort arrivé derrière une ligne amie qui se bat : **A** il se range
  et attend son tour (R3 / R8 appliquées aux renforts : frappe à portée, décalage d'une file vers un trou, prend la
  place d'un mort, se retourne si frappé) ; B il cherche une prise libre sur les ailes ; C il ne s'engage pas sans ordre
  d'attaque. Reco du sous-agent : A. **Question à poser à l'utilisateur** à son retour.
- **MV.4.3 boucle 4 (sous-agent, J5c, 11 min)** : compteur de volte-face permanent (`HOLSimulationProcessors.cpp` :
  cap qui tourne de plus de 25° entre deux pas d'au moins 3 cm ; commande `hol.Units.TurnReport`, sans allocation) et
  variable d'essai `hol.Units.DetourMemory` (0 = détour et rôle de croisement oubliés à chaque pas, comme avant MV.2).
  Croisement jugé à part dans « bodies » : **rouge** sans le détour mémorisé (72 volte-face / 403 pas), **vert** avec
  (6 / 431) ; seuil 20 ; arrivée ≤ 8 s. Seuil du mur à nous corrigé (la vitesse était lue ~3 fois trop haute) : 4-4,5 s
  pour une limite de 5,6-5,8 s. `quick_check -Parts tests,move,formation,bodies` PASS (235 s).
- Trace temporaire `MV4 TEMP TRACE` retirée de J5c par la session principale ; compilé PASS ; `quick_check -Parts
  tests,bodies` **PASS** (54 s, tests 98/98 ; passage 3 s, mur ennemi 4 s, mur à nous 4 s / 5,6, croisement 2,5 s et
  10 volte-face / 432 pas). J5c prêt pour l'essai de l'utilisateur (`blocked_bodies`, `crossing`), puis commit.
- **MV.4.6 tranchée par le test rouge (session principale)** : cas « formation reinforce » rendu probant (6 amis posés
  exactement sur les places de la formation) : **vert avec la règle ET sans elle** (les amis au repos s'écartent, D292).
  La règle n'a aucun test rouge et ne règle pas `melee_reinforce` : **retirée** (`FStillBody`, `IsClearOfBodies`, relevé
  des amis immobiles, test `Formation.StillFriends`) ; le cas `quick_check` est gardé comme anti-régression (renforts
  posés sur des places occupées par des amis au repos). Elrond recompilé ; `quick_check -Parts tests,move,formation`
  **PASS** (212 s, tests 97/97). Elrond = MV.4.4 seule (+ traces `MV4DIAG` à retirer avant commit).
- **Pause** (demande de mycrusader-f3 : mesures au repos de J7c.4, ~20-30 min) : aucun sous-agent, build ni jeu de cette
  session. **Reprise exacte** :
  1. Essai de l'utilisateur sur J5c (`play.ps1 blocked_bodies -Root ..\HeroesOfLiberty-J5c`, puis `crossing`) → commit
     de `mv4-bodies` (correctif mur ennemi, `TimeToContact` / `TurnWithoutClearWay`, compteur de volte-face,
     `hol.Units.DetourMemory`, partie « bodies », `blocked_bodies` à 5 Paysans ; trace déjà retirée) et fusion dans
     `main` (`quick_check -Parts tests,move,formation,bodies`).
  2. Question à l'utilisateur : renfort arrivé derrière notre ligne qui se bat (A se range et attend son tour, R3 / R8 ;
     B prise libre sur les ailes ; C pas d'engagement sans ordre). Reco A.
  3. Elrond : retirer les traces `MV4DIAG`, commit de MV.4.4 après essai `melee_reinforce` (une fois la question 2
     tranchée et codée, test rouge d'abord).
  4. MV.4.5 « tenir le rang » (R1-R11 validées) : sous-étapes du plan, tests rouges d'abord.
- Mesures au repos de mycrusader-f3 finies : reprise. Essai `blocked_bodies` (J5c) relancé pour l'utilisateur.
  **MV.4.5.1** lancée (sous-agent, Elrond) : règles pures de « tenir le rang » (rang et file, comblement, front aligné,
  décalage d'une file, glissement des files vides) et leurs tests, rouges d'abord.
- **Essai de l'utilisateur sur `blocked_bodies` (J5c, correctif)** : murs à nous (cas 2) et ennemi (cas 3) : « ils
  tapent un peu les gens immobiles mais les contournent après » : contournement OK, mais **contact avant le détour**
  (D293 : contourner avant de bousculer). Question posée : commit maintenant puis boucle « détour sans contact », ou
  avant le commit.
- **Choix de l'utilisateur** : commit maintenant, puis une boucle « détour sans contact » (test rouge « aucun contact
  avec le mur »). Commit `213f470` sur `mv4-bodies` (EtapeDefini : ligne des marcheurs bloqués réduite aux renforts et
  aux foules ; ajoutées : contact avant de contourner, ligne qui perd sa forme au combat) ; `main` 233e31e (D345.2a)
  fusionné dedans (464c9f0) ; compilé PASS ; `quick_check -Parts tests,move,formation,bodies,combat` **PASS** (376 s,
  tests 100/100 ; bodies : passage 3 s, mur ennemi 4,5 s, mur à nous 4,5 / 5,8 s, croisement 6 volte-face).
  Avance de `main` demandée à mycrusader-8c et mycrusader-f3 (EtapeDefini.md modifié non commité dans la copie
  principale). Essai `crossing` pas encore fait par l'utilisateur.
- **`main` avancé à 464c9f0** (accord de mycrusader-8c et mycrusader-f3) : EtapeDefini.md non commité (d'une autre
  session) remis par stash ; perte de BOM de `quick_check.ps1` dans la copie principale (changement de personne)
  écartée. **Attention** : `quick_check.ps1` a perdu son BOM UTF-8 dans 213f470 : remis par la boucle en cours (J5c).
- **MV.4.5.1 règles pures de « tenir le rang »** (sous-agent, Elrond, 10 min, rien de commité) :
  `HOLFormation::GetRankAndFile`, `FHeldLineMember` / `FHeldLineSlot`, `MakeHeldLine` (tri stable file / rang / id ;
  comblement ; glissement : seul le **dernier vivant** de la voisine la plus fournie (≥ 2) glisse, au plus un par appel ;
  médiane des premiers vivants, jamais au-delà de la case d'origine ; les morts restent passés tant que l'ordre vit),
  `HOLCombat::ELineFight { No, Strike, Step, Walk }`, `GetLineFight` (rang 0 : Walk ; rang 1+ à portée : Strike ; point
  à portée à une file au plus et pas en avant : Step ; sinon No), `ShouldTurnOnAttacker` (R8). Tests
  `Formation.HeldLine.RankAndFile` / `FillsDead` / `EmptyFileSlides` / `FrontMedian` / `Deterministic`,
  `Combat.MayWalkToFight` : **rouge** (6 tests sur 27, 41 vérifications) puis **vert** (27/27). Boucle « détour sans
  contact » lancée dans J5c (sous-agent, test rouge d'abord).
- **Pause** pour l'A/B au repos de S3 de mycrusader-f3 (~8 min) : sous-agents arrêtés, machine libérée.
  - Elrond : traces `MV4DIAG` retirées (`HOLSimulationProcessors.cpp` sans diff), `main` 464c9f0 amené sans conflit
    (stash / avance rapide / pop) ; reste : compiler, `quick_check -Parts tests,move,formation,bodies,combat`.
  - J5c (« détour sans contact », non commité) : compteur `hol.Units.ContactReport` (marcheur hors combat plus près que
    la somme des rayons d'un corps immobile < 10 cm/s ; « Contact report: N contacts with still bodies in M
    walker-steps, closest gap X cm at (x, y) ») ; vérification « no still body touched » (plus petit écart ≥ −3 cm)
    dans « bodies » (les trois cas ensemble : risque de faux rouge au passage étroit, filtrer par zone au besoin) ; BOM
    de `quick_check.ps1` remis. Compilé. Reste : mesurer le rouge ; dev prévu : horizon des corps immobiles 1,0 s
    (`hol.Units.AvoidStillTime`), « bloqué » = un corps trouvé (plus seulement avant l'horizon), angle max 90° près d'un
    corps immobile (`hol.Units.AvoidStillMaxAngle`) ; puis vert.
- **« Détour sans contact » (sous-agent, J5c, ~12 min, rien de commité)** : `hol.Units.ContactReport` (pas où un
  marcheur hors combat est plus près que la somme des rayons d'un corps immobile : sans ordre de marche et < 10 cm/s) ;
  vérification « no still body touched » dans « bodies » (plus petit écart ≥ −3 cm). **Rouge** (réglages d'avant :
  `AvoidStillTime 0.5`, `AvoidStillMaxAngle 60`) : 43 pas en contact sur 864, −12 cm au bord du mur ennemi. Dev
  (`HOLSimulationProcessors.cpp/.h`) : horizon des corps immobiles `hol.Units.AvoidStillTime` 1,0 s (plafonné par la
  distance à la case) ; un corps trouvé dans sa limite bloque (au lieu de `>= Horizon`) ; angle max 90° près d'un corps
  immobile (`hol.Units.AvoidStillMaxAngle`, 70-90° essayés seulement si rien n'est libre à 60°). **Vert** `-Parts
  bodies` (48 s) : 0 contact / 844 pas, écart minimal +6 cm ; passage 3 s, mur ennemi 3,5 s, mur à nous 5 s / 6 s (ils
  contournent au lieu de bousculer), croisement 2,5 s et 6 volte-face. BOM de `quick_check.ps1` remis. Reste : `-Parts
  tests,move,formation,bodies`, essai de l'utilisateur, commit.
- Nouvelle pause (dernier A/B de S3 de mycrusader-f3, ~3 min) ; l'arrêt de mon jeu de test d'Elrond m'a été refusé
  (auto mode) : attendu sa fin d'elle-même.
- **Elrond après fusion de `main` 464c9f0** : compilé PASS ; `quick_check -Parts tests,move,formation,bodies,combat`
  **FAIL** (381 s) : (1) tests 105/107 : `Formation.HeldLine.Deterministic` et `EmptyFileSlides` (MV.4.5.1) :
  `MakeHeldLine` ne fait pas glisser le dernier de la file -1 (attendu un id, obtenu -1) ; verts 27/27 juste avant
  (lancés seuls) : à comprendre ; (2) `combat arrows flee` (première flèche à 1613 cm en 430 ms, vitesse apparente
  38 m/s refusée) : passait sur 464c9f0 dans J5c → MV.4.4 ou hasard, à vérifier. Le reste PASS (dont formation
  reinforce, bodies, combat bodies).
- J5c (« détour sans contact »), `quick_check -Parts tests,move,formation,bodies,combat` : tout PASS (tests 100/100 ;
  bodies : 0 contact, écart +6 cm ; passage 3 s, mur ennemi 3,5 s, mur à nous 5 / 6 s, croisement 6 volte-face)
  **sauf `combat arrows flee`** (même échec que dans Elrond : première flèche à 1586 cm en 423 ms, « 37 m/s
  apparent »). Il passait dans J5c sur 464c9f0 nu : instable, ou lié aux deux changements ; à l'étude (sous-agent
  d'Elrond).
- **Elrond (sous-agent, 14 min)** : (1) vrai bug de `MakeHeldLine` : `INDEX_NONE` (-1) servait de « personne » pour
  la file qui cède, or -1 est aussi la file à gauche de l'axe : corrigé (`HOLFormationRules.cpp` ~1193) ; `Formation`
  19/19, `tests` 107/107. (2) `combat arrows flee` : **test fragile** (pas MV.4) : la première flèche venait d'un Archer
  laissé par un cas précédent (camp 0) ; correctif du test : `hol.Units.Remove` du camp 0 à 40 m du Lancier au début du
  cas (`quick_check.ps1`). Vérifié par la session principale : Elrond `-Parts formation,combat` PASS (332 s) ; même
  correctif porté dans J5c : `-Parts tests,move,formation,bodies,combat` **PASS** (375 s, tests 100/100, 0 contact).
- J5c prêt pour l'essai de l'utilisateur (`blocked_bodies` : murs contournés sans contact ; puis `crossing`).
  **MV.4.5.2** lancée dans Elrond : champs de la ligne tenue dans l'ordre, remplissage à l'ordre, `hol.Combat.HoldLine`.
- **MV.4.5.2 (sous-agent, Elrond, 15 min)** : `FHOLMoveOrderFragment` : `bHoldsLine`, `LineFile` (int16), `LineRank`
  (uint8), `LineSerial` (uint32, distinct d'`OrderSerial`) ; 0 bit répliqué. Remplis à l'ordre si `bFormation &&
  AttackMove && HOLCombat::IsHoldLineOn()` (R1), rang / file par `GetRankAndFile` ; remis à zéro par tout autre ordre
  (marche, tâche, `ApplyCombatOrder`). Cvar `hol.Combat.HoldLine` (1). Rapport `hol.Units.LineReport [Slot]`. Cas
  `quick_check` « formation held line » (12 Hommes d'armes en attaque-déplacement : 1 ligne de 12, ≥ 2 rangs ; simple
  marche : 0 ligne) : **rouge** (0 ligne) puis **vert**. `quick_check -Parts tests,move,formation,combat` PASS (381 s).
  Files / rangs exacts à figer à la prochaine passe. **MV.4.5.3** lancée : passe du processeur de combat.
- **MV.4.5.3 boucle 1 (sous-agent, Elrond, 13 min)** : cvar `hol.Combat.HoldLineFight` (0 = comportement d'avant,
  sert au rouge) ; `bLineMember` / `bLineBack` ; zone de contrôle fermée au rang ≥ 1 ; `RearFlank` coupé (R11) ; R8
  (dernier attaquant à portée) ; hors de portée au rang ≥ 1 : `GetLineFight` (Step : marche de combat vers le point à
  portée le plus proche de sa case, ≤ 200 cm ; sinon cible lâchée, `StopWalk`) ; une marche de combat d'un membre garde
  sa ligne ; `StopWalk` garde déjà la ligne (`StandInLine` inutile). Rapport étendu (`back N walking N striking N off
  <médiane>, front <largeur> of <largeur des cases>`). Cas `quick_check` « combat held line » (12 contre 12 Hommes
  d'armes, rapports à 2 s pendant 20 s ; à partir du 6e : walking 0, médiane ≤ 60 cm, front ≥ 80 %). **Rouge**
  (`HoldLineFight 0`) : walking 2-3, médiane 373-377 cm, front 810-883 / 600 ; **avec le dev : encore rouge** (walking
  1, médiane 222-257 cm) : manque la passe `MakeHeldLine` (la mêlée se fait avant les cases d'origine, R5). Boucle 2 :
  passe `MakeHeldLine`, compteur « walking » sans les pas de côté, fin R9, puis `-Parts tests,formation,combat`.
- **MV.4.5.3 boucle 2 (sous-agent, Elrond, 12 min)** : `UHOLCombatProcessor::UpdateHeldLines` (groupe par
  `LineSerial`, active au premier `bAttacking`, ancre recalculée, `MakeHeldLine`, glissement ≤ 1 / s, réécrit rang /
  file) ; case tenue dans un **nouveau champ `LinePlace`** (écrire dans `AttackMoveGoal` cassait la reprise de
  l'attaque-déplacement après le combat) ; état par ligne `HeldLines` ; fin R9 (moins de 2 vivants, ou 2 s sans membre
  engagé) ; retour à la case (rang ≥ 1 sans cible arrêté à > 50 cm de `LinePlace`) ; compteur « walking » = buts à plus
  d'une file de la case. « combat held line » : walking 0, front 668 / 600, ligne tenue (12 → 10 membres, 5 files) mais
  **écart médian 167 / 159 cm (seuil 60) : KO** ; tout figé du 3e au dernier rapport. Autres cas `combat` PASS.
  Boucle 3 : pourquoi les rangs ≥ 1 ne rejoignent pas `LinePlace` (retour non lancé ? bloqué ? ancre fausse ?).
- **Essai de l'utilisateur** : `blocked_bodies` (détour sans contact) : « good ». Commit `6ab675c` (EtapeDefini : ligne
  « contact avant de contourner » retirée), `main` 3f59aec fusionné (02d8ea4 ; seulement des docs et
  `rest_measures.ps1`, pas de code), **`main` avancé à 02d8ea4** (`quick_check.ps1` local de la copie principale =
  seulement le BOM, que le commit remet : écarté). Essai `crossing` lancé (J5c).
- **Essai de l'utilisateur sur `crossing`** (avec détour sans contact) : « tout est bon ». Cas 1 et 3 du chantier
  (croisement, marcheurs contre des murs) clos ; restent au cas 3 : renforts derrière leur ligne, foules ennemies.
- **Renforts derrière leur ligne** : à la question A / B / C, l'utilisateur répond « on n'a pas déjà défini ce point ? » :
  oui, il découle de R3 / R8 (rangs de derrière : frappe à portée, décalage d'une file, se retourne si frappé, ne
  marche pas vers l'ennemi). **Retenu : A**, les renforts suivent les règles des rangs de derrière (R1 ne couvrait
  formellement que l'attaque-déplacement). À coder après MV.4.5.3, test rouge d'abord (`melee_reinforce` : plus aucun
  renfort en approche de combat bloquée derrière la ligne).
- `main` avancé par mycrusader-8c à 1473564 puis 2c2e574 (D345.2b : unités qui attaquent les bâtiments ;
  `FHOLCombatFragment::TargetBuilding`, approche des bâtiments dans `HOLCombatProcessor`) : **conflits à prévoir** avec
  MV.4.5.3 (Elrond) dans `HOLCombatProcessor.cpp` à la prochaine fusion de `main`.
- **MV.4.5.3 boucle 3 (sous-agent, Elrond, 12,5 min)** : cause (trace `MV453 TEMP`, retirée) : le retour à la case
  n'était jamais lancé (aux pas de recherche, la branche `No` lâchait la cible puis `continue` ; hors pas de recherche,
  le retour exigeait `bSearchStep`) ; ancre, cap et `LinePlace` justes. Correctif : même retour dans la branche `No`.
  « combat held line » **vert** (walking 0 ; écart médian 42 → 10 → 5 cm ; front 603-630 / 600) et **rouge** avec
  `HoldLineFight 0` (walking 1-2, écart 286-381 cm). Rangs ≥ 1 jamais atteints dans ce cas (R3 Step / R8 non exercés).
  `combat away` a échoué une fois dans le run rouge (avant le cas, sans ligne) : instable ? Reste : `quick_check`.
- Vérifié par la session principale : Elrond `quick_check -Parts tests,move,formation,bodies,combat` **PASS** (416 s,
  tests 107/107, formation held line, combat held line, formation reinforce, bodies, combat away PASS).
  Suite : fusion de `main` 2c2e574 (D345.2b, conflits dans `HOLCombatProcessor.cpp`), puis renforts = rangs de
  derrière (test rouge), puis essai de l'utilisateur (`melee_formed`, `melee_reinforce`).
- **Fusion de `main` dans Elrond** (sous-agent, 15 min, rien de commité ; sauvegarde
  `scratchpad\elrond_mv45_before_merge.patch`) : avance rapide 464c9f0 → 3b53b51, un conflit de texte
  (`HOLCombatTests.cpp` : `Combat.BuildingAttack` et `Combat.MayWalkToFight` gardés). Choix bâtiments : frapper un
  bâtiment ne compte pas comme contact / engagement de la ligne ; rang ≥ 1 : bâtiment seulement à portée de frappe
  (comme Hold), jamais de marche vers lui ; l'ordre explicite d'attaquer un bâtiment retire la ligne. Compilé PASS ;
  `quick_check` : tests 108/108 et combat PASS **sauf « combat held line »** (écart médian des rangs ≥ 1 : 107 → 76
  → 51 puis **151 / 190 cm** après les morts du front ; avant la fusion 42 → 10 → 5). Suspect : le détour sans contact
  (corps immobiles vus à 1 s, `AvoidStillTime`) gêne les rangs de derrière qui avancent vers une case juste derrière
  un ami immobile. Stash `stash@{0}` « mv45 wip before merge main » resté (contenu déjà appliqué ; son retrait a été
  refusé au sous-agent : à faire avec l'accord de l'utilisateur).
- Stashs supprimés avec l'accord de l'utilisateur : « mv45 wip before merge main » (déjà appliqué dans Elrond) et
  « mv2 wip » (état intermédiaire de la boucle 1 de MV.2, version finale dans `main`). Plus aucun stash.
- **Correctif après fusion (sous-agent, Elrond, 14 min)** : départage : avec `AvoidStillTime 0.5` /
  `AvoidStillMaxAngle 60`, « combat held line » repasse vert → cause = détour sans contact (un rang de derrière qui
  revient à une case à moins de (60 + 60) × 1,1 = 132 cm d'un ami immobile voit ce corps comme un mur). Correctif :
  `AvoidBodies(..., const FVector2D* Goal)` : un corps immobile **qui touche le but** du marcheur (but à moins de la
  distance de contact) n'est pas un mur (le marcheur passe `&Order.Slot` dès qu'il a une phase). Ligne entière : écart
  médian 0-25 cm. Mais après des morts (3 → 2 rangs) : **171-175 cm**, avec 1-4 rangs de derrière qui frappent
  (`striking`) : pas de côté R3 (légitime) ou marcheur gêné ? Décision de la session principale : la médiane d'écart ne
  porte que sur les rangs de derrière **qui ne frappent pas** (R3 les autorise à frapper à une file de leur case) ; un
  contrôle à part vérifie que ceux qui frappent restent à une file (200 cm) au plus de leur case. Boucle suivante.
- **Boucle 2 (sous-agent, Elrond, 14 min)** : trace : au moment des 153-260 cm, les rangs ≥ 1 éloignés sont **tous
  des frappeurs** (pas de côté puis frappe ; dérive de 60-110 cm au-delà du point de pas, #56 à 260 cm pendant 5
  rapports) ; ceux qui ne frappent pas sont à 0-36 cm. Mesure appliquée : médiane « off » sur les rangs ≥ 1 qui ne
  frappent ni ne font un pas de côté (`bApproaching` avec cible compté comme frappeur), et contrôle nouveau « striking
  off max ≤ 200 cm » (R3). `-Parts combat` PASS une fois (off 4 → 31 cm, striking off max 0). Reste : la **dérive des
  frappeurs** au-delà d'une file (rouge possible selon le passage) : correctif R3 (un rang ≥ 1 qui frappe à plus d'une
  file de sa case lâche sa cible et revient), répété plusieurs fois ; puis `quick_check` complet (« bodies » 0 contact).
- **Fin de séance (2026-10-22, demande de l'utilisateur)** : sous-agent de la boucle 3 (dérive des frappeurs) arrêté.
- **Reprise (2026-10-23)** : Elrond repris (IN_USE.txt). Boucle « correctif R3 douteux » lancée (sous-agent) : points 1
  et 2 de la liste ci-dessous.
- **Boucle « correctif R3 » (sous-agent, Elrond, 15 min, rien de commité)** : **sans correctif, rouge reproduit** (un
  frappeur à 260 cm de sa case pendant 5 rapports) : le correctif est nécessaire. Correctif **restreint**
  (`HOLCombatProcessor.cpp` ~1567-1582) : seulement un membre arrêté (`Phase == None`) dont la cible est à portée ;
  marche de retour donnée une fois, plus à chaque pas. `-Parts combat` ×3 : vert, rouge (striking off max 417-466 cm
  avec encore 3 rangs), rouge (médiane des non-frappeurs 213 cm juste après le passage de 3 à 2 rangs). Pistes : (1) les
  frappeurs à 417-466 cm ne sont pas arrêtés, ou sont comptés `bApproaching` (pas de côté qui continue) : trace
  ponctuelle ; (2) la médiane compte des membres qui marchent vers leur nouvelle case après `MakeHeldLine` : les sortir
  du rapport (ou ignorer les rapports juste après une refonte). Boucle suivante lancée.
- **Boucle 2 (sous-agent, Elrond, 15 min)** : deux causes de dérive : (a) un rang ≥ 1 qui marche vers sa case (nouvelle
  après refonte) s'arrête pour frapper tout ennemi à portée, et le correctif restreint n'agissait qu'à l'arrêt ; (b) un
  pas de côté pouvait l'emmener à plus d'une file (#56 à 260 cm en marche, but à 151 cm de sa case : contournement ou
  poussée). Correctifs (`HOLCombatProcessor.cpp`) : ~1567-1587 R3 sans condition d'arrêt (cible à portée et > 200 cm de
  sa case : lâche ; marche de retour seulement s'il est arrêté ou si son but est à > 50 cm de sa case) ; ~1599-1604 pas
  de côté refusé (`No`) au-delà de 200 cm de sa case. Mesure (`HOLUnitOrderProcessor.cpp` ~3868) : un non-frappeur en
  marche vers sa case (but à ≤ 50 cm d'elle) sort de la médiane ; arrêté hors de sa case, il compte toujours. Seuils
  inchangés. Traces retirées, compilé PASS (BOM ajouté par erreur puis retiré). `-Parts combat` (avant le correctif (b)) :
  PASS, FAIL (#56), PASS. Reste : 3 passages sur l'état final (lancés par la session principale).
- **3 passages `-Parts combat` sur l'état final (session principale)** : **FAIL ×3**, toujours « combat held line »
  médiane des non-frappeurs : 3 rangs, `striking 3 off 144 striking off max 124, front 529 of 600` **figé** du 4e au
  9e rapport (12 s), puis 2 rangs off 0-40. Le reste de `combat` PASS. Régression du correctif (b) ou de la mesure :
  des non-frappeurs arrêtés à ~144 cm de leur case ne reviennent pas. Boucle 3 lancée.
- **Boucle 3 (sous-agent, Elrond, 18 min)** : cause : ces rangs ≥ 1 (#55, #61, #64, 115-250 cm) étaient encore en
  phase `Block` (marche de bloc de l'attaque-déplacement, but à 120-400 cm de `LinePlace`) derrière le front arrêté ;
  les deux retours à la case exigeaient `Phase == None`. Le refus du pas de côté (b) n'y était pour rien : gardé.
  Correctif (`HOLCombatProcessor.cpp`) : `IsWalkingToLinePlace(Order)` (~133-144 : en marche ET but à ≤ 50 cm de la
  case) ; retour à la case si `!IsWalkingToLinePlace` dans la branche sans cible (~1414), la branche `No` (~1651) et le
  lâcher R3 (~1595, même règle factorisée). Seuils inchangés. `-Parts combat` **PASS ×3** (médiane 7 / 7 / 11 cm,
  walking 0, striking off max 7 / 7 / 160-171 cm, front 609-724 / 600). Traces retirées, compilé PASS, pas de BOM.
  Reste : `-Parts tests,move,formation,bodies,combat` (lancé par la session principale).
- Elrond, `quick_check -Parts tests,move,formation,bodies,combat` **PASS** (424 s ; tests 108/108 ; bodies 0 contact
  / 820 pas, écart +6 cm ; formation reinforce, formation held line, combat held line PASS). Points 1-3 de la liste
  faits. **Point 4 lancé** (sous-agent) : renforts = rangs de derrière, test rouge d'abord.
- **Renforts, boucle 1 (sous-agent, Elrond, 15 min)** : cas `quick_check` « combat reinforce » (partie `combat`, pas
  ~1301-1320, contrôles ~3197-3215 : 12 Hommes d'armes en attaque-déplacement sur 12, 6 renforts en simple marche 6 s
  après ; à partir du 4e rapport : walking 0, « free » final ≤ « free » d'avant, frappeurs à ≤ 200 cm de leur case) ;
  `LineReport` + « ; free N walking M » (`HOLUnitOrderProcessor.cpp` ~3824-3846, ~3901). Rouge vu (walking 6 → 1, free
  12) **mais faussé** : l'ordre tombait sur un ennemi (ordre d'attaque sur #101) ; corrigé (sol 7 m derrière, attente
  « Order: 6 units »), pas relancé. Dev compilé, pas lancé : `HOLFormation::GetReinforcementFile` (file la moins
  fournie, puis la plus proche, puis la plus petite ; test `Formation.HeldLine.ReinforcementFile`),
  `UHOLCombatProcessor::JoinReinforcements` (~422, appelé par `UpdateHeldLines` ~486 : nos unités libres derrière une
  ligne active, à une file de ses ailes et 4 rangs derrière, rejoignent le fond de leur file ; tireurs, occupés, Hold et
  cible ordonnée exclus), `Execute` ~1057 (attaque-déplacement, cible effacée, marche vers sa case). Boucle 2 lancée :
  rouge refait avec l'ordre corrigé, puis vert ×3.
- **Renforts, boucle 2 (sous-agent, Elrond, 15 min)** : cvar `hol.Combat.ReinforceJoin` (1 ; 0 = avant, pour le rouge ;
  `HOLCombatProcessor.cpp` ~88-91, garde ~490-493) ; cas : caméra à MeetTo-1400 avant l'ordre au sol, attente « Order:
  6 units to ( », contrôle « simple marche au sol (pas d'ordre d'attaque) » (~1314, ~3207). **Rouge** (`ReinforceJoin
  0`, simple marche confirmée) : free 4 → 10 sur les 10 rapports, walking 0, 5, 3, 3, 3, 4, 3, 4, 4, 3 (ils marchent sur
  l'ennemi sans rejoindre). **Vert** (1 passage `-Parts combat`, tout PASS) : ligne 12 → 18 membres (5 files, 5 rangs),
  walking 0 partout, free revenu à 6, frappeurs à 198 / 90 / 0 cm de leur case. Reste : 2 passages verts de plus et
  `-Parts tests,move,formation,bodies,combat` (lancés par la session principale), puis essai de l'utilisateur.
- **Session principale** : `-Parts combat` **FAIL** sur « combat reinforce » seul : « striking ranks behind within a
  file » : `13 members, 4 files, 5 ranks, striking 3, striking off max 156` (×3) puis, après une mort, `12 members,
  striking 2, striking off max 211` ; free 5. Puis `-Parts tests,move,formation,bodies,combat` **PASS** (445 s, tests
  109/109, 0 contact ; combat reinforce : 16 membres, 5 files, off 15-94, striking off max 0). Bilan : 2 verts sur 3,
  et le cas n'est pas déterministe (13 contre 16-18 membres selon le passage). Boucle 3 lancée.
- **Renforts, boucle 3 (sous-agent, Elrond, 15 min, pas de code)** : 1 passage `-Parts combat` **vert** (17 → 13
  membres, striking off max 184 / 25 / 0) : 211 cm non reproduit. Lecture du code : `UpdateHeldLines` (~1061) tourne
  avant la boucle des combattants, donc le lâcher R3 (~1703) et le refus du pas de côté (~1734) voient la nouvelle case
  dans le même pas ; mais le combat tourne avant marche, mouvement et écartement, et `LineReport` lit les positions
  après : **piste = frappeur à ≤ 200 cm au combat, poussé de ~11 cm avant le rapport** (non confirmé). Ralliement
  partiel : **pas un bug** (« free » compte ~5 unités qui ne sont pas des soldats : 5 avant et après, les 6 renforts
  rejoignent à chaque fois). Boucle 4 lancée : rouge avec trace, puis marge de décision sous 200 cm.
- **Renforts, boucle 4 (sous-agent, Elrond, 15 min)** : pas de rouge en 3 passages (combat 161-174 cm, rapport 129-173
  cm) : poussée après le combat non montrée. Marge posée quand même (`HOLCombatProcessor.cpp` 118-122 :
  `LinePushSlack` 25 cm, `LineHoldReach` = 200 − 25 = 175 cm pour le lâcher R3 ~1709, le refus du pas de côté et la
  limite de `GetLineFight` ~1739-1742 ; `LineStepReach` 200 reste la règle visible ; seuil `quick_check` 200 cm
  inchangé). `-Parts combat` **PASS ×3**. Trace retirée, compilé PASS. Reste : `-Parts tests,move,formation,bodies,
  combat` + 2 passages `combat` de plus (le rouge venait 1 fois sur 4), lancés par la session principale.
- **Session principale** : `-Parts tests,move,formation,bodies,combat` **PASS** (452 s, tests 109/109, 0 contact) puis
  `-Parts combat` **PASS ×2** (striking off max 55 / 175) : 6 verts de suite avec la marge. Points 1-4 faits. Essai de
  l'utilisateur lancé : `melee_formed` (Elrond), puis `melee_reinforce`.
- **Essai de l'utilisateur sur `melee_formed` : refusé** (« c'est pas du tout ça ! ils ne sont pas censés rester en
  bloc, une ligne / vague devrait se former » ; capture : deux blocs de ~10 rangs ; GIF de référence : une ligne mince
  de 2 rangs). Lecture : « tenir le rang » garde chaque rang de derrière à sa case du bloc et R11 coupe `RearFlank` :
  c'est ce qui défait la vague validée au 7e essai de J7c.4 (rangs tassés derrière les amis, glissade, débordement par
  les bouts, ligne qui s'allonge et enroule le flanc). Scénario de comparaison `melee_formed_wave` (Elrond, non commité :
  `hol.Combat.HoldLine 0`, donc comportement de J7c.4) lancé. Question à l'utilisateur sur la règle voulue.
- **Essai `melee_formed_wave`** : « ah oui, c'est beaucoup mieux ». **Retenu : A** (la recommandation) : au contact,
  la vague de J7c.4 (rangs tassés, glissade, débordement par les bouts) ; « tenir le rang » (rangs gardés à leur case,
  R11) **abandonné** pour la mêlée. Écartés : B déploiement en ligne mince au contact ; C formation déjà mince à
  l'ordre (à revoir si la ligne de départ reste trop épaisse). Essai `melee_reinforce_wave` (même chose, renforts)
  lancé. Puis : que garder du code de MV.4.5 (question à l'utilisateur).
- **Essai `melee_reinforce_wave`** : « super », mais **des renforts ne vont jamais au front** (capture : ~10 renforts
  arrêtés à 10-15 m derrière la mêlée ; simple marche, arrivés au repos, ennemi hors de leurs 16 m d'agression). Deux
  soldats debout **sur les rochers** : bogue à part, pour EtapeDefini. Question : A rejoindre la mêlée à la fin d'une
  simple marche ; B simple marche près d'une mêlée changée en attaque-déplacement à l'ordre ; C **porter secours**
  (façon AoE4 : un soldat de mêlée au repos qui voit un des siens se battre à ~20 m va l'aider). **Choix : C** (la
  recommandation).
- **Retrait de « tenir le rang »** (décision de la session principale, suite du choix A) : code de MV.4.5 et ralliement
  des renforts à la ligne tenue retirés ; gardés : MV.4.4 (bloc qui se défait près de la mêlée), `AvoidBodies(...,
  Goal)`. Sauvegarde complète d'avant retrait : `scratchpad\elrond_mv45_holdline_full.patch` (session 0b11c55e,
  vérifiée par `git apply --check --reverse`). Sous-agent lancé (Elrond).
- **Retrait fait (sous-agent, Elrond)** : `HOLCombatProcessor.cpp/.h`, `HOLCombatRules.cpp/.h`,
  `HOLSimulationFragments.h`, `HOLFormationRules.cpp`, `HOLCombatTests.cpp` revenus à 3b53b51 (processeur de combat =
  celui de `main`, D345.2b compris : ce que donnait `HoldLine 0`) ; retirés : règles de la ligne tenue, champs, remplissage
  à l'ordre, `hol.Units.LineReport`, tests `Formation.HeldLine.*` et `Combat.MayWalkToFight`, cas « formation held line »
  / « combat held line », scénarios `*_wave`. Gardés : MV.4.4 (`ShouldBreakBlock`, `IsFightingFriend`, `IsMemberStuck`,
  `Formation.BreakAtMelee`, `AdvanceBlocks`, cas « formation reinforce »), `AvoidBodies(..., Goal)` (but = case de
  l'ordre pour tout marcheur en phase). « combat reinforce » : mise en place gardée, contrôles remplacés par « Order: 6
  units to ( » vu (TODO : porter secours). Reste 236 lignes ajoutées / 6 retirées (6 fichiers). Compilé PASS ;
  `-Parts tests,move,formation,bodies,combat` **PASS** (427 s, tests 102/102, 0 contact). Note : un bloc en
  attaque-déplacement reste formé jusqu'à la mêlée (`ShouldBreakBlock` : simple marche seulement) : c'est l'état vu et
  validé dans `melee_formed_wave`.
- **« Porter secours » (choix C)** lancé (sous-agent, Elrond) : test rouge d'abord.
- **Porter secours, boucle 1 (sous-agent, Elrond, 19 min, rien de commité)** : règle (`HOLCombatRules.h:223-235` /
  `.cpp` : `DefaultAidRadius`, `IsFriendFighting`, `ShouldComeToAid` ; corps écrits, pas encore compilés) : soldat en
  recherche libre (mode Auto : ni Hold ni attaque-déplacement, aucun ordre en marche, pas de cible ordonnée), de mêlée,
  agressivité > 0 (pas les Paysans), sans bâtiment visé, sans cible trouvée par sa propre recherche ; un ami du même
  joueur à `hol.Combat.AidRadius` (2000 cm bord à bord ; 0 = coupé) qui se bat (`bAttacking`, `AttackerCount > 0` ou
  `LastAttacker`) ; cible = l'ennemi le plus proche parmi les `Target` / `LastAttacker` de ces amis, par l'approche
  existante, au pas de recherche (grille dense, sans allocation). `GatherFriends` (~268), commande `hol.Combat.AidReport
  Slot [Radius]`. Test `Combat.ComeToAid` (14 vérifications) **rouge** sur la règle vide (102/103). Cas « combat
  reinforce » (but à MeetTo − 1700, `AidReport` chaque seconde) : **pas rouge** (« idle melee 5, near a fighting friend
  0 » : sans doute des restes d'autres cas ; un seul renfort a combattu). Boucle 2 : rouge fiable (6 soldats posés au
  repos par `SpawnHere` derrière notre ligne, à > 16 m de tout ennemi et < 20 m d'un ami qui se bat ; rapport limité à
  une zone), puis vert ×3.
- **L'utilisateur est absent** (« j'afk ») : on continue jusqu'au vert et à l'essai prêt, sans commit.
- **Porter secours, boucle 2 (sous-agent, Elrond, 12,5 min)** : cas déterministe « combat aid » (`quick_check.ps1`
  ~1287-1297, rapport ~3171-3178 ; `$AidBack = 1850` ~386 ; `SpawnSeed 141`) : 6 contre 6 Hommes d'armes au contact à
  `$MeetTo`, 6 des nôtres posés au repos 18,5 m derrière ; `AidReport Slot [Radius] [X Y Area]` (~1634-1690, zone
  seulement). **Rouge** (`AidRadius 0`) : à 2, 3, 4 s idle 6, near 6 (#67-#72), ennemi le plus proche 18,45 m.
  **Vert** (2000) : idle 0 dès +1 s. Règle réelle compilée ; tests 103/103 (`Combat.ComeToAid`). Mais **« combat
  bodies » FAIL** sur 2 passages (sans doute « bodies kept » : plus vieux corps 4,1 s pour ≥ 4,5) : aide en cause ou
  non ? Le « combat reinforce » en simple marche passe mais ne prouve rien (0 même sur la règle vide). Boucle 3 lancée.
- **Porter secours, boucle 3 (sous-agent, Elrond, 16 min)** : « combat bodies » PASS avec `AidRadius 0` : l'aide est
  en cause **mais fait ce qu'il faut** : le Lancier #11 resté de « orders attack » tuait nos Paysans #7-#10 un à un (les
  « plus vieux corps » vus) ; avec l'aide, les Hommes d'armes #1-3 au repos à < 20 m viennent les secourir et tuent #11.
  Reste le Paysan du cas, posé à la fin de ses 1 s de chute (`DyingSteps` 20) : 5 s après la mort il a toujours ~4,1 s.
  Le seuil `Oldest -ge 4.5` ne passait que grâce aux restes. Correctif du test (`quick_check.ps1` ~3073-3087) : seuil
  3,9 s (5 s d'attente − 1 s de chute − marge), commentaire. `-Parts combat` : PASS, PASS, puis « combat reinforce »
  FAIL (`0,…,0,1,0,0` : une unité au repos près d'un ami qui se bat pendant un seul relevé, à 21 s). Boucle 4 lancée.
- **Boucle 4 arrêtée au-delà de 15 min** (rappel de l'utilisateur : **minuteur oublié pour toute la séance**, noté en
  mémoire). Laissé : contrôle de « combat reinforce » changé en « aucun id dans deux relevés de suite à partir de 12 s »
  (`AidReport` avec les ids ; une unité qui vient de tuer attend sa prochaine recherche), contrôles « aid off » / « aid
  on » du cas « combat aid » ; code inchangé depuis la boucle 3, déjà compilé. 3 passages `-Parts combat` lancés par la
  session principale.
- **Session principale** : `-Parts combat` **PASS ×3** (combat aid PASS), puis `-Parts tests,move,formation,bodies,combat`
  **PASS** (429 s, tests 103/103, 0 contact). **Prêt pour l'essai de l'utilisateur** (absent) : `play.ps1
  melee_reinforce -Root ..\HeroesOfLiberty-Elrond` (renforts en simple marche : plus aucun ne doit rester au repos
  derrière la mêlée). Après l'essai : commit (EtapeDefini : lignes « renforts » et « ligne qui perd sa forme » ; ajouter
  « soldats debout sur les rochers »), fusion dans `main` (accord des sessions), puis foules ennemies (`crowds_cross`),
  puis MV.6 (au GDD : « tenir le rang » essayé et refusé, vague de J7c.4 gardée, porter secours C).
- **Essai de l'utilisateur sur `melee_reinforce`** : « super », mais deux renforts ne vont jamais au combat : ce sont
  les deux **sur les rochers**, déjà au même endroit à l'essai d'avant (jamais bougé, même avec l'ordre ; apparus sur
  un rocher, et à > 20 m de la mêlée). Question : A commit maintenant, rochers en ligne à part ; B rochers avant le
  commit. **Choix : A.** Commit `7776dab` sur `mv4-diag` (EtapeDefini : lignes renforts et « ligne qui perd sa forme »
  retirées, « unité apparue ou poussée sur un rocher : y reste coincée » ajoutée), `main` f5f4be3 fusionné (`44e51d6`,
  un conflit de texte dans EtapeDefini, lignes des deux côtés gardées). Compilation et `-Parts
  tests,move,formation,bodies,combat` en cours, puis avance de `main`.
- Compilé PASS, `-Parts tests,move,formation,bodies,combat` **PASS** (429 s, tests 107/107, 0 contact). `main` avancé
  entre-temps à 239a9a9 (D324 : unité immobile qui sort du mur visible, `HOLSimulationProcessors.cpp` touché) :
  fusionné (`8021c71`, conflit EtapeDefini : ligne « unité bousculée » retirée par `main`, nos deux lignes gardées) ;
  compilé PASS ; même vérification **PASS** (436 s, tests 108/108, 0 contact). Avance rapide de `main` 239a9a9 →
  8021c71 possible sans toucher les fichiers non commités de la copie principale : accord demandé aux 5 sessions
  actives (mycrusader-e8, playable-79, mycrusader-97, mycrusader-3c, mycrusader-d4).

- **`main` avancé à 8021c71** (accord des 5 sessions : mycrusader-97, -e8, -d4, -3c ; playable-79 hors projet).
  mycrusader-3c (J5c, `build-red`) cherche pourquoi `-Parts build` est rouge sur `main` (suspect : 6ab675c, détour sans
  contact) : prévenue de ce que touche MV.4 (`AvoidBodies` Goal, `ShouldBreakBlock`, `AidRadius`). mycrusader-e8
  fusionnera MV.4 dans `victory-v1` avant d'avancer `main`.
- **Foules ennemies (« go » de l'utilisateur)** : la piste A du diagnostic (le bloc évite le bloc) n'a jamais été
  tranchée par l'utilisateur, et le diagnostic date d'avant le mur ennemi contourné, le détour sans contact et MV.2.
  Essai `crowds_cross` sur l'état actuel (Elrond = `main` 8021c71) lancé avant toute question.
- **Essai de l'utilisateur sur `crowds_cross`** : « c'est bon ». Les correctifs déjà dans `main` (mur ennemi contourné,
  détour sans contact, MV.2) suffisent : **cas clos sans code** (piste A abandonnée). EtapeDefini : ligne des foules
  retirée. `main` avancé par mycrusader-e8 à 4541266 (Victory V1, accord de cette session).

### Reprise exacte (MV.4)

État :
- **Dans `main` (8021c71, essais de l'utilisateur OK)** : MV.4.2 mur ennemi contourné, détour sans contact devant un
  mur d'unités immobiles, partie `quick_check` « bodies » ; MV.4.4 bloc de renforts qui se défait près de la mêlée ;
  **porter secours** (choix C : soldat de mêlée au repos qui voit un des siens se battre à `hol.Combat.AidRadius` 20 m
  va l'aider ; cas « combat aid ») ; `AvoidBodies(..., Goal)`. **« Tenir le rang » (MV.4.5) essayé et refusé** : la
  vague de J7c.4 est gardée (sauvegarde du code retiré : `scratchpad\elrond_mv45_holdline_full.patch`, session
  0b11c55e). Elrond (`mv4-diag`) = `main` 8021c71.

À faire, dans l'ordre :
1. ~~Foules ennemies qui se croisent~~ : clos à l'essai sans code (« c'est bon »).
2. (ligne EtapeDefini à part) Unité apparue ou poussée sur un rocher : y reste coincée (vue dans `melee_reinforce`).
3. MV.6 : décisions au GDD (MV.2, MV.5, MV.4 : détour sans contact, renforts, porter secours ; « tenir le rang »
   essayé et refusé au profit de la vague de J7c.4), mesures S1-S5, clôture.
Règles : test rouge d'abord (mémoire `red-test-first`) ; boucles de 15 min **avec minuteur lancé dans le même message
que le sous-agent** ; rien de commité avant l'essai.
- (2026-10-23, session EtapeDefini) **Retour d'essai de l'utilisateur**, noté pour ce chantier : quand il sélectionne
  des unités au sein d'un groupe et leur donne l'ordre de bouger d'un rien (juste se mettre en formation là où elles
  sont), elles peinent, car elles ne poussent presque pas les autres. Ligne ajoutée à `EtapeDefini.md` (main 9c8e5f4).
