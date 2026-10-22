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
