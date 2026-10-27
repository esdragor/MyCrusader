# Crossbow in progress — l'Arbalétrier

> Ligne d'`EtapeDefini.md` : « Bonus de l'Arbalétrier contre Lourd » (D428). Ouvert le 2026-10-26 par la session
> principale (copie `MyCrusader`, `main`), pendant la recompilation du moteur Hazelight avec la cible Server (D483).

## Pourquoi maintenant

Le moteur est en recompilation : rien ne se compile ni ne se lance dans Unreal. Peuvent se faire **sans le moteur** :
les questions (une à la fois, options A / B / C, recommandation), le plan, la ligne de `Tools/Data/UnitTypes.csv`,
`UnitStats.md` (`python Tools/Data/make_unit_stats.py`), les gestes (`Tools/Editor/peasant_poses.py`), la pièce
arbalète (comme `peasant_props.py`) et leurs **aperçus PNG** (`preview_animation.py`), tous en Python pur.
**Après la recompilation** : fabrication dans Unreal (commandlets `make_peasant_mesh.py --soldiers`,
`make_peasant_animations.py`), tests C++ (rouge d'abord), `quick_check`, scénario, essai de l'utilisateur.

## Ce qui existe

- Catégorie **Lourd** (D428) : portée par l'Homme d'armes (`ArmorCategories` = Infanterie lourde, Lourd).
- Bonus par catégorie (`Bonuses` du CSV, D345) : dégâts = attaque + bonus − armure du type, 1 au moins.
- Tir de l'Archer (J7c.2) : flèche à 37,5 m/s qui touche toujours ; **une seule vitesse pour tous**
  (`HOLCombat::GetArrowSpeed()`).
- Production : la Caserne fait Lancier (A), Homme d'armes (Z), Archer (E) ; le Champ de tir du GDD (D128) n'existe
  pas encore, les paliers non plus (l'Arbalétrier est au palier 1 dans le GDD).
- Gestes et pièces de l'Archer : `SKM_Archer`, `TP_Archer`, `A_Archer_*` (journal de J7c.1 dans
  `JourneyInProgress.md`, fabrication dans `AnimationsInProgress.md`).

## Questions à trancher (une à la fois)

- **AR-Q1 Stats** : combat, coût, vitesse.
- **AR-Q2 Production** : où, quelle touche.
  - **AR-Q2a Champ de tir** : stats, case de la grille du Paysan, touches de production.
  - **AR-Q2b Allure du Champ de tir** : aujourd'hui tous les bâtiments ont la même forme provisoire (bloc et tour,
    D261), seule la taille change.
- **AR-Q3 Allure** : arbalète, carreaux, tenue.
- **AR-Q4 Geste de tir** : épauler, tirer, recharger ; le hit and run (retour d'essai 8 de l'Archer).
- **AR-Q5 Projectile** : carreau visible, vitesse.
- **AR-Q6 Chute** : celle de l'Archer ou une à lui.

## Décisions

- **AR-Q1 Stats** (2026-10-26, A, la recommandation, avec une correction de l'utilisateur : la vitesse) : valeurs du
  Crossbowman d'AoE4 ramenées à nos échelles — **80 PV**, attaque **12** à distance, **+10 contre Lourd** (seul bonus,
  D428), cadence **2,125 s** (élan 12 %, comme l'Archer), portée **5 cases (20 m)**, armures 0 partout, vitesse
  **1,8 case/s (7,2 m/s, 720 cm/s)** (choix de l'utilisateur, au lieu de 1,74), **80 Nourriture + 40 Or**, **22 s**,
  population 1, catégorie Distance, attaque contre bâtiment comme son attaque (Distance, 12, 2,125 s, 5 cases ;
  1 dégât après les 40 d'armure). Rayon d'agressivité comme l'Archer (portée + 1/3 de case). Écartés : la cadence de
  l'Archer (B), le bonus +15 (C).
- **AR-Q2 Production** (2026-10-26, B, choix de l'utilisateur contre la recommandation A « Caserne, touche R ») : le
  **Champ de tir** est créé maintenant (D128, palier 0 du GDD) ; l'**Archer y passe** avec l'Arbalétrier ; la Caserne
  garde Lancier (A) et Homme d'armes (Z). Écartés : la Caserne sur R (A), le faux palier (C). Conséquences : nouveau
  bâtiment (ligne de `BuildingTypes.csv`, case de la grille du Paysan, nom fr / en / ja, allure), tests et scénarios
  qui produisent l'Archer à la Caserne à reprendre (`HOLProductionTests`, `HOLCommandGridTests`, `quick_check`
  « build barracks » / « build hud », `Tools/Scenarios/archers.txt` et ceux qui partent d'une Caserne).
- **AR-Q2a Champ de tir** (2026-10-26, A, la recommandation) : double de la Caserne, comme dans AoE4 —
  `ArcheryRange`, 3 × 3 cases (1200 × 1200 cm), hauteur 600, **1500 PV**, 40 d'armure distance, **150 Bois**, **40 s**,
  file de 5, catégorie Military, palier 0 ; grille du Paysan **S** (`BuildGridSlot` 5, rangée militaire, à droite de la
  Caserne sur Q) ; production **A Archer, Z Arbalétrier** ; pas de prérequis. Écartés : Caserne exigée (B), plus petit
  et moins cher (C).
- **AR-Q2b Allure du Champ de tir** (2026-10-26, A, la recommandation) : forme provisoire propre, en pièces simples
  du moteur (cube, cylindre) : bloc bas sans tour (long abri sur un côté) et **3 cibles de paille** debout dans la cour
  (disques clairs, rond à la couleur du joueur, lisibles de la caméra du dessus) ; nouvelle colonne « forme » dans
  `BuildingTypes.csv` (resservira à l'Écurie et aux suivants) ; C++ dans `AHOLBuilding::BuildProvisionalShape` et un
  test. Écartés : même forme plus basse (B), vrai maillage modelé (C).
- **AR-Q3 Allure** (2026-10-26, A, la recommandation) : corps du mannequin à la couleur du joueur, comme les autres
  soldats ; **arbalète à étrier** (arbrier ~85 cm, arc d'acier de 70 cm en travers et sa corde, étrier de fer à
  l'avant), tenue **à deux mains, couchée devant le buste** au repos et en marche ; **carquois de carreaux à la hanche
  droite** (carreaux de 35 cm). Pièces dans `Tools/Editor/soldier_props.py`, comme l'arc. Écartés : pavois dans le dos
  (B, identité de l'*Arbalétrier à pavois* des Templiers), chapel de fer (C).
- **AR-Q4 Geste de tir** (2026-10-26, A, la recommandation) : mécanisme de l'Archer (retour d'essai 8) — boucle de
  2,125 s qui **démarre chargée**, arbalète basse ; **épaule et tire à 12 %** (0,26 s, élan de la table), petit recul ;
  puis **recharge à l'étrier** : pointe au sol, pied droit dans l'étrier, les deux mains tirent la corde à la noix,
  carreau pris à la hanche et posé, relevé prêt. Un ordre de marche coupe le geste, la recharge court en marchant
  (hit and run comme l'Archer). Carreau caché du tir à sa pose (comme la flèche, `"hidden"`). Écartés : tir à 25 %
  (B), pas de recharge montrée (C).
- **AR-Q5 Projectile** (2026-10-26, A, la recommandation) : **vrai carreau**, `SM_Bolt` (~35 cm, plus épais, pointe
  carrée, deux empennes), même vitesse (37,5 m/s, `HOLCombat::GetArrowSpeed()`) et même léger arc que la flèche ;
  colonne « projectile » dans `UnitTypes.csv` (maillage par type) ; le client choisit le maillage par le type du
  tireur, déjà connu ; rien ne change côté serveur. Écartés : vitesse et arc propres (B), la flèche de l'Archer (C).
- **AR-Q6 Chute** (2026-10-26, C, choix de l'utilisateur contre la recommandation A « chute des soldats par
  `fall()` ») : **une chute à lui**, nouvelle (par exemple à genoux, puis sur le flanc) ; l'arbalète reste dans la
  main droite (pièces rigides sur un os). Première chute « variante » avant la ligne « Variantes de chutes » d'après le
  prototype. Écartés : chute des soldats (A), calque de celle de l'Archer (B).
  **Précisée le 2026-10-26 (AR4, A, la recommandation)** : l'exemple « à genoux puis sur le flanc » était la chute du
  Paysan ; retenu : **en avant, face contre terre** — touché de flanc, il pivote d'un quart de tour, les jambes cèdent,
  il s'effondre sur le ventre ; l'arbalète lui échappe (`release`, trajet propre comme la lance du Lancier) et glisse
  au sol devant lui. Écartés : à genoux puis sur le flanc (B, trop proche du Paysan), assis puis sur le dos (C).

## Plan

Étapes de moins de 15 min, test rouge d'abord sur la vraie chaîne (C++ : simulation et rendu, D483). Les aperçus
(PNG, GIF par `Tools/Editor/preview_gif.py`) sont montrés à l'utilisateur avant toute fabrication.

**Sans le moteur (pendant la recompilation)**

| Étape | Contenu | Fin |
|---|---|---|
| AR1 Pièces | `soldier_props.py` : arbalète à étrier (sur `ik_hand_gun`, outil de la main droite), carreau en main, carquois de carreaux à la hanche droite ; `SKM_Crossbowman` dans `SOLDIER_MESHES` ; table des pièces | aperçu PNG validé |
| AR2 Repos, marche | `soldier_poses.py` : `A_Crossbow_Idle`, `A_Crossbow_Walk` (arbalète couchée devant le buste, deux mains) | GIF validés |
| AR3 Tir | `A_Crossbow_Attack` : 2,125 s, démarre chargée, tir à 12 %, recharge à l'étrier, carreau caché du tir à sa pose | GIF validé |
| AR4 Chute | `A_Crossbow_Fall` : chute à lui (à genoux puis sur le flanc), arbalète dans la main droite | GIF validé |
| AR5 Données | **reportée à AR7 / AR8** (2026-10-26) : les CSV sont la source des assets (`make_unit_types.py`, `make_building_types.py`) ; une ligne `Crossbowman` sans maillage et l'Archer retiré de la Caserne, commités avant l'import, casseraient la réimport d'une autre session. Valeurs prêtes : AR-Q1, AR-Q2a, `WalkPlayRate` 4,28, séquences comme l'Archer (repos 2, marche 3, tir 4, chute 5), nom dans `make_unit_stats.py` | avec AR7 / AR8 |

**Après la recompilation (moteur Hazelight avec Server)**

| Étape | Contenu | Fin |
|---|---|---|
| AR6 Fabrication | commandlets : `SKM_Crossbowman`, `TP_Crossbowman`, `A_Crossbow_*` (relectures), `SM_Bolt` (`make_arrow_mesh.py` généralisé) | relectures vertes |
| AR7 Champ de tir | colonne « forme » (`FHOLBuildingTypeRow`), `BuildProvisionalShape` (abri bas, 3 cibles), ligne importée, nom fr / en / ja, grille du Paysan S, production A Archer / Z Arbalétrier, Caserne sans Archer ; tests `Construction`, `Production`, `CommandGrid` repris | tests verts |
| AR8 Arbalétrier | ligne importée (maillage, séquences), dégâts contre l'Homme d'armes 18 (bonus Lourd) sur la vraie chaîne, nom fr « Arbalétrier » / en « Crossbowman » / ja « 弩兵 » | tests verts |
| AR9 Carreau | colonne « projectile » (`FHOLUnitTypeRow`), `UHOLArrowSubsystem` choisit le maillage par le type du tireur | test vert, capture |
| AR10 Vérification | `quick_check -Parts` touchées (tests, build, anims, `net 2 clients`) ; scénarios qui produisaient l'Archer à la Caserne repris ; scénario `crossbows` (Champ de tir fini, Arbalétriers contre Hommes d'armes et Lanciers) | **essai de l'utilisateur** |
| AR11 Clôture | GDD (AR-Q1 à AR-Q6, numéros D libres, prochain D485), `EtapeDefini.md`, mémoire | commit |

**Part IA** (règle de D421) : l'IA n'a pas encore d'armée (IA2 pas codée) ; le Champ de tir et l'Arbalétrier
(contre Lourd) entrent dans la liste des unités qu'IA2 utilisera — à noter dans `AIInProgress.md` par sa session.

## Suivi

- 2026-10-26 : chantier ouvert ; questions listées ; AR-Q1 posée.
- 2026-10-26 : AR-Q1 tranchée (A, vitesse 1,8 case/s) ; AR-Q2 posée.
- 2026-10-26 : AR-Q2 tranchée (B, Champ de tir maintenant, l'Archer y passe) ; AR-Q2a posée.
- 2026-10-26 : AR-Q2a tranchée (A, double de la Caserne, S, A Archer / Z Arbalétrier) ; AR-Q2b posée.
- 2026-10-26 : AR-Q2b tranchée (A, abri bas et 3 cibles de paille) ; AR-Q3 posée.
- 2026-10-26 : AR-Q3 tranchée (A, arbalète à étrier, carquois à la hanche) ; AR-Q4 posée.
- 2026-10-26 : AR-Q4 tranchée (A, tir à 12 %, recharge à l'étrier) ; AR-Q5 posée.
- 2026-10-26 : AR-Q5 tranchée (A, `SM_Bolt`, même vol) ; AR-Q6 posée.
- 2026-10-26 : AR-Q6 tranchée (C, chute à lui) ; questions closes, plan écrit.
- 2026-10-26 : AR1 : pièces dans `Tools/Editor/soldier_props.py` (Python pur, rien fabriqué), versions légères comprises.
  **`crossbow`** (id 8) : outil des deux mains sur `ik_hand_gun` (repère « tool » : origine au bout de l'étrier, X le
  long de l'arbrier vers la crosse, Y le dessus (rainure), Z l'arc en travers ; main gauche à 22 cm derrière l'arc
  (`knob_offset`), main droite 45 cm plus loin à la détente) — l'étrier est le « bout du manche » posé au sol à la
  recharge, et `release` libère les deux mains sur la corde ; arbrier 85 cm, arc d'acier 70 cm bandé (pointes 7 cm en
  arrière), corde droite de chaque pointe à la noix (arbalète chargée : un os rigide ne la bouge pas), étrier de fer,
  noix, détente ; 184 triangles (léger 96). **`bolt`** (id 5) : carreau **chargé** dans la rainure, sur `ik_hand_r`
  (enfant de `ik_hand_gun` jamais clé : suit l'arbalète exactement), 42 triangles (léger 14) ; à cacher après le tir
  en clé d'échelle de `ik_hand_r` (AR3 et le baker ; `tool_hidden` cacherait l'arbalète). **Carreau en main** (de la
  hanche à la rainure) : **pas d'os bon marché** — `ik_hand_l` / `ik_hand_r` suivent `ik_hand_gun`, seuls
  `ik_foot_l` / `ik_foot_r` sont libres (enfants de `ik_foot_root`) mais demandent une deuxième piste d'outil
  (`anim_rig`, baker, aperçu) : laissé de côté, le carreau en vol est `SM_Bolt` (AR-Q5). **`bolt_quiver`** (id 6) :
  hanche droite, sur `pelvis` (repère « body », ancre (28, -2, 70)), bande à la couleur du joueur, 4 bouts de
  carreaux, 110 triangles (léger 64). Ids réutilisés (5, 6 de l'Archer) : `prop_bit` n'accepte que 1-8, il suffit
  qu'ils diffèrent dans un maillage. `SKM_Crossbowman` = crossbow, bolt, bolt_quiver. Geste `crossbow_idle`
  (`A_Crossbow_Idle`) dans `PROPOSED` de `soldier_poses.py`, pour voir les pièces : arbalète couchée devant le buste,
  pointe en avant à gauche, crosse à la hanche droite ; mains à l'axe 0,000 cm, aucun point du corps dans les pièces.
  Pièces et gestes existants inchangés (table et primitives identiques à `HEAD`). Aperçus :
  `Saved/AnimPreview/crossbow_idle_contact_sheet.png`, `crossbow_idle_zoom.png`, `crossbow_idle_light_zoom.png`.
  Reste : validation de l'utilisateur ; vrais repos et marche en AR2.
- 2026-10-26 : **AR1 validée** par l'utilisateur (« validé ») ; pas de carreau en main pendant la recharge (geste mimé), épaisseur de l'arc à juger en AR2. Commitée ; AR2 lancée (sous-agent).
- 2026-10-26 : AR2 : `A_Crossbow_Idle` (clé `crossbow_idle`) et `A_Crossbow_Walk` (clé `crossbow_walk`) dans `PROPOSED`
  de `soldier_poses.py` (rien fabriqué), par `crossbow_gesture()` sur `armed()` comme les soldats : repos sur
  `A_SW_Idle` (`steady_spine` 1, souffle et balancement de la base), marche sur `A_SW_Unarmed_Walk` (`steady_chest`
  0,5, jambes de la base : pas de glissement à la bonne cadence), même port dans les deux : arbalète couchée devant le
  buste, pointe en avant à gauche, main gauche sous le fût derrière l'arc, main droite à la détente devant la hanche
  droite (prise (-16, 36, 106), direction (0,8, -0,58, -0,1)) ; coude droit rabaissé et serré (indice (0,3, -0,6, -0,8) :
  coude à droite 21, haut 119 au lieu de 29 et 122). Mains à l'axe 0,000 cm, mains hors cible 0,0 cm, aucun point du
  corps dans l'arbalète, le carreau ni le carquois (repos et marche) ; la crosse bouge de 3 / 3 / 7 cm en marche (le
  balancement du corps). **WalkPlayRate de l'Arbalétrier = 720 / 168,2 = 4,28** (pour AR5 ; 2,85 à l'écran après la
  division par `MeshScale`). L'arc d'acier (1,7 cm) se voit à peine en vue RTS des aperçus et ne se lira sans doute pas
  au LOD 3 (18 m) : l'épaissir à ~3 cm, ou le laisser, à trancher par l'utilisateur. Gestes et pièces existants
  inchangés (`GESTURES` identique à `HEAD`, `soldier_props.py` intouché). Aperçus dans `Saved/AnimPreview/` :
  `crossbow_idle_contact_sheet.png`, `crossbow_idle_zoom.png`, `crossbow_idle_light_zoom.png`,
  `crossbow_walk_contact_sheet.png`, `crossbow_walk_zoom.png`, `crossbow_walk_archer_walk_spear_walk_compare.png`,
  `crossbow_walk.gif`, `crossbow_idle.gif`. Reste : validation de l'utilisateur.
- 2026-10-26 : **AR2 validée** par l'utilisateur (« validé ») ; `WalkPlayRate` 4,28 pour AR5. Arc de l'arbalète : question posée (A épaissir à ~3 cm et éclaircir l'acier / B garder), A choisi par l'utilisateur (la recommandation), fait avec AR3. Commitée ; AR3 lancée (nouveau sous-agent, le premier approchait 300k jetons).
- 2026-10-26 : AR3 (partielle, budget du sous-agent dépassé) : **arc épaissi** (`make_crossbow`, versions légères
  comprises : ~3 cm vu du dessus, 3,6 au centre, 2,25 aux pointes, au lieu de ~1,7 ; acier plus clair
  `CROSSBOW_PROD_STEEL`) ; repos et marche toujours propres (mains à l'axe 0,000 cm, aucun point du corps dans les
  pièces). **`A_Crossbow_Attack`** (clé `crossbow_attack`, `PROPOSED`, rien fabriqué) : **64 images à 30 i/s**
  (2,133 s, jouée à 2,133 / 2,125), **tir sur l'image 8** (`CROSSBOW_SHOT` 0,125 ; 0,12 × 64 = 7,68, écart ~11 ms
  accepté comme l'Archer), clés d'Hermite, bassin en avance (`CROSSBOW_LEADS`). Démarre chargée au port du repos ;
  épaule (tronc tourné à droite, crosse à l'épaule droite), tir, recul (crosse 4 cm en arrière, pointe +4°) ; pointe au
  sol ; **pied droit vraiment dans l'étrier** (clé `feet` d'`anim_rig` : la boule du pied sur la barre de l'étrier,
  2 cm au-dessus du sol, de 1,06 s à 1,45 s, pas levé de 11 cm à l'aller et au retour) ; penché (charnière aux hanches,
  `stoop`), deux mains sur la corde puis à la noix ; main gauche au fût, arbalète relevée, main droite au carquois puis
  à la rainure (carreau posé, mimé), retour à la détente. Mains libres par **`release` en plages** (`anim_rig` : une
  liste de (début, fin) en plus du nombre ; les gestes existants inchangés) : de 0,68 s à 2,0 s. Roulis de l'arbalète
  tenu par les `fingers` de l'outil (rainure = opposé des doigts). **Carreau caché** de 0,276 s à 1,89 s :
  `"bolt_hidden"` (plages en secondes) et `"hidden"` pour l'aperçu ; **le baker (AR6) devra mettre `ik_hand_r` à
  l'échelle ~0 sur ces plages** (comme `ik_hand_gun` pour `tool_hidden`), `make_peasant_animations.py` non touché.
  Gestes et pièces existants inchangés (`GESTURES`, autres pièces, `SOLDIER_MESHES` identiques à `HEAD`). Aperçus :
  `Saved/AnimPreview/crossbow_attack_contact_sheet.png`, `crossbow_attack_zoom.png`,
  `crossbow_attack_light_zoom.png` ; pas de GIF. **Reste** (à reprendre avant de montrer) : main gauche hors cible à la
  visée (4 à 9 cm, images 5-16 : crosse trop en arrière ou torsion insuffisante) ; crosse dans l'épaule (20 à 60
  points, 2 cm) à la visée et arbalète dans le buste en descendant (images 15-23, jusqu'à 170 points) ; mains hors
  cible au relevé (images 42-48, jusqu'à 31 cm : main gauche de la noix au fût) ; saut de 23 cm à l'image 19 (descente
  trop rapide) ; puis GIF.
- 2026-10-26 : bilan de la boucle AR3 (session principale) : un seul sous-agent pour tout le tir, trop gros pour 15 min (225k jetons, arrêté proprement). Vérifié : la retouche de `anim_rig.py` (`release` en plages) ne change rien aux gestes existants (aperçus `archer_attack`, `spear_attack`, `maa_fall` identiques à `HEAD` octet pour octet). **Processus amélioré** : corrections découpées en sous-étapes ciblées avec critères chiffrés : **AR3b** visée et descente (images 5 à 23), **AR3c** remontée (42 à 48), phase penchée et GIF.
- 2026-10-26 : AR3b (partielle, arrêtée au budget) : **visée seule** retouchée (`CB_SHOULDER`, `CB_AIM`, `CB_SHOT`, `CB_KICK`) : torsion 58-60° (bassin 19°), clavicule gauche avancée de 14°, crosse montée sous la joue (`CB_BUTT_AIM` (9, -14, 151)). Main gauche au fût : 0 à 1,3 cm sur les images 5-10 (au lieu de 3 à 7), mais toujours 3 à 9 cm sur 11-16 (`CB_SETTLE`, à refaire avec la descente) ; crosse dans l'épaule : 28 à 51 points, ~2 cm (inchangé : le bras droit levé et le haut du buste sous la crosse) ; images 17-25 et saut de l'image 19 non traités ; saut du relevé à l'image 5 un peu plus fort (22 cm au lieu de 19). Rien changé après l'image 16 ; `GESTURES` et autres gestes proposés identiques. **Constat pour la suite** : la main gauche (22 cm de la pointe) ne peut pas suivre la descente (la pointe parcourt au moins 155 cm de la visée à l'étrier, ~10 cm par image si la descente commence à l'image 10) : avancer le début de `release` juste après le recul et faire glisser les mains vers 80/90 cm sur des clés intermédiaires ; le plus court chemin de la visée à `CB_DOWN` est un demi-tour autour de l'axe (0, -1, 1) : la crosse passe en haut à sa droite, hors du buste (`CB_LOWER` actuel la met dans la poitrine).
- 2026-10-26 : bilan d'AR3b (session principale) : main gauche réglée à la visée (images 5-10), le reste non. **Processus amélioré** : (1) la crosse contre l'épaule est un **contact voulu** : jusqu'à ~2,5 cm d'enfoncement dans l'épaule / le haut du bras à la visée accepté (invisible en jeu), le zéro point n'était pas réaliste ; (2) l'agent suivant (AR3b-2) reçoit la piste trouvée (lâcher tôt, mains qui glissent le long de l'arbrier, demi-tour autour de (0, -1, 1)) au lieu de la rechercher.
- 2026-10-26 : AR3b-2 : **descente refaite** (`soldier_poses.py`, bloc arbalète seul) : `release` dès 0,18 du cycle (juste après le recul, au lieu de 0,32) ; `CB_LOWER` remplacé par `CB_SETTLE` puis `CB_LOWER_1` à `_4` (`cb_lowering`) : demi-tour autour de (0, -1, 1) (crosse en haut à sa droite, pointe à sa gauche), les deux mains posées sur l'arbrier et glissant de 22/67 à 80/90 cm, clés régulièrement espacées le long du chemin de la pointe (~180 cm en 16 images, sans arrêt) ; recul et `CB_DOWN` traversés en mouvement (flow (0, 1) et (0,6, 0)) ; clavicule gauche avancée de 22° dans `CB_DOWN` (la main gauche atteint sa place) ; relevé ralenti (`CB_RAISE_2` à 0,075, `CB_SHOULDER` à 0,10, `CB_READY` qui part en mouvement), tir toujours image 8. Images 0-26 : mains à 1,2 cm au plus (images 7-8, la visée d'AR3b, inchangée ; 0 ailleurs, au lieu de 9,3 et 4,0), mains lâchées à 1,2 cm au plus de l'axe de l'arbrier ; arbalète dans le corps : 0 point de 12 à 26 (au lieu de 160) hors effleurements de l'avant-bras gauche (≤ 0,1 cm, images 12-13) ; sauts ≤ 11,9 cm sur 7-26 (au lieu de 23 à 24), relevé 14,9 cm au plus (au lieu de 21,4 : plus bas impossible sans partir avant l'image 0). Images 27-63 identiques ou meilleures (main gauche 27 : 2,2 → 0). `GESTURES` et autres gestes proposés identiques (8d7e0a23). Reste pour AR3c : l'avant-bras gauche sous l'arbrier à la visée (~0,6-0,9 cm, images 5-10, déjà là avant) ; la remontée 42-48 et la phase penchée.
- 2026-10-26 : AR3b-2 relue par la session principale (aperçu regardé : descente propre, crosse par la droite) ; **accepté** aussi : l'avant-bras gauche sous l'arbrier à la visée (≤ 1 cm, appui voulu) et la levée à 14,9 cm par image. Durée réelle ~35 min (au-delà des 15) : AR3c a un périmètre plus serré et un arrêt ferme à 15 min.
- 2026-10-26 : AR3c : **remontée refaite** (`soldier_poses.py`, bloc arbalète seul, images 0-28 identiques à AR3b-2) : `CB_STEP_BACK`, `CB_CARRY`, `CB_DRAWN` et `_UP_TOOL` remplacés par `cb_lift` : de la prise (`CB_GRAB`, 0,665) à la pose du carreau (`CB_PLACE`, 0,88) l'arbalète va en ligne droite de l'étrier au port bas (`_CARRY_TOOL`), 12 segments réguliers (premier et dernier à moitié : départ et arrivée en douceur), main gauche posée SUR l'arbrier de la clé (42 → 22 cm), corps qui se redresse en retard (part u², portée du bras), pied droit sorti de l'étrier dans la première moitié, main droite à la longueur de chemin corde → bouche du carquois (carreau pris au passage, mimé) → rainure (36 cm) ; clavicule gauche avancée de 22° à la prise ; carreau visible dès 0,88. Images 42-48 : mains à 0,5 cm au plus de leur cible (au lieu de 31), main gauche à 1,5 cm au plus de l'arbrier (au lieu de 14), sauts ≤ 12,0 cm (au lieu de 29,9) ; 49-63 : sauts ≤ 12,4 cm (au lieu de 19,8), main droite à la détente à 2,6 cm au plus (image 58). Phase penchée (36-43) : seul l'avant-bras droit effleure l'arbrier (≤ 1,8 cm, toléré) ; les « 2403 points à 1,13 s » de l'aperçu comptent les mains posées sur l'arbrier pendant `release` (prise voulue, ≤ 2,2 cm), hors mains 1 point (orteil dans l'étrier) : rien à corriger. Boucle 63 → 0 : 4,9 cm (pas de saut). `GESTURES` et autres gestes proposés identiques.
- 2026-10-26 : **AR3 validée** par l'utilisateur (« ça a l'air bon », GIF `crossbow_attack.gif`) ; restes acceptés : main gauche à 4 cm dans la phase de l'étrier (cachée par le corps), 12,4 cm par image sur le trajet carquois → rainure. Commitée ; AR4 lancée.
- 2026-10-26 : AR5 reportée à AR7 / AR8 (voir le plan) : données à committer avec leur import, pas avant. AR4a lancée (premier jet de la chute face contre terre).
- 2026-10-26 : AR4a : **premier jet de `A_Crossbow_Fall`** (`crossbow_fall`, proposé, bloc ajouté à la fin de `soldier_poses.py`) : 1,0 s, 30 images, `release` dès 0 ; touché à l'épaule gauche, buste d'abord (`FALL_LEADS`), quart de tour à sa GAUCHE (-80°) en trébuchant (pied droit puis gauche, chacun son tour), genoux au sol à 0,43 s, à plat ventre à 0,68 s (rebond 3 cm), immobile dès 0,87 s ; corps clé dans son repère tourné (`_cbf`) ; mort : tête tournée à gauche, bras droit jeté devant près de la tête paume en bas, bras gauche le long du flanc paume en haut, jambe gauche tendue, droite un peu fléchie, orteils en arrière ; arbalète lancée devant-gauche, posée à plat devant sa tête à 0,43 s (rebond), carreau laissé visible dans la rainure. Mesures : corps au plus 2,0 cm sous le sol (orteil gauche, 0,63 s ; dernière image 0,8 cm, bassin), arbalète ≥ 0,2 cm, carquois ≥ 8 cm, carreau ≥ 6,5 cm ; image 0 = port de `crossbow_attack` ; mains hors cible ≤ 3,3 cm ; sauts d'os ≤ 31 cm par image (avant-bras droit, images 14 et 18) ; rotation de main 114° en une image (main droite, image 17). `GESTURES` et autres gestes proposés identiques. Reste pour AR4b : sauts des bras (14, 18-21), retournement de la main droite (17), tête 26 cm à l'image 19, cadrage des aperçus (le corps couché sort de la case), lisibilité vue RTS à juger sur GIF.
- 2026-10-26 : AR4a relue (aperçu regardé : la chute se lit). Erreur de la session principale : la chute de l'Homme d'armes est aussi face contre terre. **AR4 bis** (A, la recommandation) : on garde, distinguée par le quart de tour, la pose miroir et l'arbalète qui vole devant lui. Écartés : sur le côté, l'arbalète qui roule derrière (B) ; assis puis sur le dos (C). AR4b lancée (défauts de AR4a).
- 2026-10-26 : AR4b (défauts de AR4a, arrêtée à 15 min). Corps ramené vers son point de départ (`back` de `_cbf` : 14 à 50 cm de 0,43 à 0,75 s, pieds rejetés en arrière) : bassin à 23 cm du départ, tête à 94 cm (avant 73 / 144) ; arbalète posée à plat devant-gauche de sa tête, du côté où il la regarde ; tout dans les cases des trois vues (avant : 83 cm hors case). Retouches : clé de 0,67 s ajoutée (mi-chute), contact au sol 0,68 → 0,75 s, immobile 0,86 s ; paume droite qui tourne peu à peu vers le bas (plus de retournement) ; paumes de 0,25 s à mi-chemin. Mesures : saut d'os max 21 cm (avant-bras droit, image 14 ; avant 31), tête 18 cm à l'image 19 (avant 26), 15-17 cm sur les images 7-12 et 23 (le corps qui tombe) ; rotation de main max 44° par image (main gauche, image 6, la titubation ; avant 114°), poignet droit 56° image 14 ; corps ≤ 1,5 cm sous le sol (0,8 cm à la fin), arbalète, carreau, carquois jamais dessous ; mains hors cible ≤ 3,3 cm ; jambes : genoux ≥ 37 cm, pieds ≥ 44 cm l'un de l'autre, elles ne se croisent pas. `GESTURES` et autres gestes identiques (b6c73750). Reste : avant-bras droit image 14 et mains images 5-7 au-dessus des cibles (15 cm, 35°) ; pied droit qui se pose encore aux images 26-27 (5-7 cm, retard des pieds) ; un essai de retiming (immobile à 0,80 s) l'a réglé mais a remis des sauts de 19 cm et un poignet à 87°, écarté.
- 2026-10-26 : **AR4 validée** par l'utilisateur (« oui, go ») ; restes acceptés (bras 21 cm au choc des genoux, tête 18 cm, main gauche 44° par image en trébuchant, pied droit posé à 0,9 s, dernière image immobile). Commitée. **AR1 à AR4 faites et validées ; AR5 jointe à AR7 / AR8 ; suite (AR6 et après) après la recompilation du moteur.**
- 2026-10-27 : moteur recompilé (Hazelight 5.8.4 avec Server) : AR6 lancée par mycrusader-99. Worktree Elrond pris puis **repris dans la même minute par deux autres sessions** (IA puis Angelscript AS1, `IN_USE.txt` écrasé) : `SM_Bolt` et `make_arrow_mesh.py` déplacés hors de chez elles ; nouveau worktree **`HeroesOfLiberty-Celeborn`** (branche `crossbow` = main 85508be), compilé par `Tools\build.ps1 -Root`. **AR6c `SM_Bolt` fabriqué** (`make_arrow_mesh.py --bolt`, `SM_Arrow` intouché sans l'option) : 35 cm, fût 1 cm de rayon (4 pans), pointe carrée 4,5 cm, deux empennes de 6 × 2,2 cm en travers, origine à la pointe le long de +X, mêmes trois matériaux que la flèche ; 72 triangles, bornes (-35, ±1,7, ±3,2). AR6a (gestes, carreau caché par `ik_hand_r`) relancée dans Celeborn.
- 2026-10-27 : **AR6 faite, relectures vertes** (Celeborn, rien commité). **AR6a** (sous-agent, 12 min) : les 4 gestes passés de `PROPOSED` à `GESTURES` ; baker généralisé (`HIDDEN_BONES` : `tool_hidden` → `ik_hand_gun`, `bolt_hidden` → `ik_hand_r`, piste à la transformée de référence, échelle 0,001 dans les plages) ; relecture : échelle de chaque os caché image par image. Rouge d'abord (« piste ik_hand_r absente », 0 image lue cachée sur 48), puis vert. `A_Crossbow_Idle` 340 images / 11,333 s (brute 0,014, compressée 0,027 cm, boucle 0) ; `_Walk` 30 / 1,0 s (0,000 / 0,005, boucle 0) ; `_Attack` 64 / 2,133 s (0,003 / 0,008, boucle 0, lâcher image 8, **carreau caché images 9 à 56**, lues identiques, mains au manche 0,64 cm hors `release`, main gauche 4,1 cm à l'étrier, accepté) ; `_Fall` 30 / 1,0 s (0,000 / 0,007, dernière image immobile, os le plus bas 2,18 cm). Aperçus existants identiques octet pour octet. **AR6b** : `SKM_Crossbowman` (LOD 23 993 / 7 621 / 4 654 / 2 631 sommets ; arbalète masque 128 sur `ik_hand_gun`, carreau 16 sur `ik_hand_r`, carquois 32 sur `pelvis`, bande masque 0 ; pièces légères au LOD 3 ; relecture OK) et `TP_Crossbowman` (2 `A_Crossbow_Idle`, 3 `_Walk`, 4 `_Attack`, 5 `_Fall`) ; `M_UnitPlayerColorWeapons` réenregistré remis à `main`. **AR6c** `SM_Bolt` (plus haut). Reste à voir en jeu (AR9-AR10) : le carreau à l'échelle 0,001 disparaît bien.
- 2026-10-27 : **AR7a faite** (sous-agent, ~20 min, Celeborn, rien commité) : enum `EHOLBuildingShape { BlockAndTower, Range }`, colonne `Shape` en fin de `BuildingTypes.csv` / `FHOLBuildingTypeRow` (défaut bloc + tour, D261) ; ligne **`ArcheryRange`** (valeurs d'AR-Q2a, forme Range) **pas encore branchée** (`Produces` = Archer seul, `BuildGridSlot` -1, Caserne inchangée) ; `BuildRangeShape` : sol de cour 1140 × 1140 × 6 sable, abri de pierre 342 × 1140 × 270 sur le côté −X (près de la caméra) et toit au ton sombre de la couleur du joueur (sommet 294 cm), 3 cibles de paille (Ø 194, penchées de 45° vers la caméra, marque Ø 87 à la couleur du joueur, poteau) ; `CylinderMesh` chargé en dur. Test `HeroesOfLiberty.Buildings.ArcheryRange` sur la vraie ligne : rouge (« No building type ArcheryRange », puis forme : « top at 600 cm »), puis vert ; `quick_check -Parts tests` 187/187 (48 s) ; DataTable réimportée. Capture `scratchpad\ar7a_range.png` : lisible (marques un peu petites, à revoir en AR10). Notés : le fantôme et le navmesh gardent Height 600 pour un bâtiment fini de 294 cm ; abri fixé côté −X (bâtiments jamais tournés aujourd'hui).
- 2026-10-27 : **AR8 faite** (sous-agent, ~20 min, Celeborn, rien commité ; AR5 jointe) : ligne `Crossbowman` de `UnitTypes.csv` (Archer pour le reste ; 80 PV, 720 cm/s, `SKM_Crossbowman` / `TP_Crossbowman`, WalkPlayRate 4,28, 80 N + 40 Or, 22 s, 12 / 2,125 s, `((Category=Heavy,Damage=10))`, bâtiments 12 / 2,125 s), DataTable réimportée (8 lignes) ; `make_unit_stats.py` + `UnitStats.md` ; nom fr dans `HOLGameNames.cpp`, en / ja par `fill_po.py` (collecte, remplissage, collecte : seule entrée nouvelle `HOLGameNames,Crossbowman` ; ~287 lignes de diff par `.po` = numéros de ligne, se régénère). Tests `HeroesOfLiberty.Combat.Crossbowman` (vraie ligne, `GetDamage` : **18 contre l'Homme d'armes** (12 + 10 − 4), 12 contre le Lancier et l'Arbalétrier, 1 contre un bâtiment) et `Animation.Table` (séquences 2-5) : rouges (186/188) puis verts, **188/188**.
- 2026-10-27 : **AR7b faite** (sous-agent, Celeborn, rien commité) : `Barracks` produit `("Spearman","ManAtArms")`, `ArcheryRange` `("Archer","Crossbowman")` sur la case **S** (`BuildGridSlot` 5) ; nom fr « Champ de tir » (`HOLGameNames.cpp`), en « Archery Range », ja « 射撃場 » (`fill_po.py`, collecte) ; aide de `hol.Build.Place`. Tests repris (`Production.Tables`, `CommandGrid.Tables`, `Construction.Tables`) : rouges (185/188, 11 lignes) puis verts **188/188** ; `quick_check` partie « build » reprise (panneau d'âge I avec S, infobulles « Produit : … », « build barracks » produit Lancier, Homme d'armes, Lancier) : `-Parts tests,build` PASS en 119 s. L'IA ne produit pas d'Archer. Scénarios à reprendre en AR10 : `archers.txt`, `j7_army.txt` (notes « E = Archer » à la Caserne), `building_destroy.txt` (file de 3 à la Caserne → 2).
- 2026-10-27 : **AR9 faite** (sous-agent, ~22 min, Celeborn, rien commité) : colonne `Projectile` (`TSoftObjectPtr<UStaticMesh>`, fin de `FHOLUnitTypeRow` et de `UnitTypes.csv` : `SM_Arrow` Archer, `SM_Bolt` Crossbowman, vide ailleurs ; pas dans le fragment de simulation) ; `UHOLArrowSubsystem` : un composant d'instances par maillage (sans ombre, D258), repli sur `SM_Arrow` ; `FHOLArrowLaunch::ShooterType` rempli aux lancers du serveur (`HOLCombatProcessor`, unité et bâtiment) et des clients (`HOLNetUnitDisplay`), rien de plus sur le réseau, vitesse et arc inchangés ; cuisson : `/Game/Units/Props` toujours cuit + références douces de la table. Commentaire de `BuildGridSlot` corrigé (0 A Maison … 4 Q Caserne, 5 S Champ de tir). Test `HeroesOfLiberty.Combat.Projectiles` (vraies lignes ; 2 tirs d'Arbalétrier + 1 d'Archer → 2 instances `SM_Bolt`, 1 `SM_Arrow`) : rouge (5 erreurs) puis vert ; `quick_check -Parts tests,combat,net` PASS 189/189 en 178 s (net : 17 flèches lâchées et lancées par client). Note : `HOLUnitTypes.h` est aussi modifié par la session héros dans la copie principale (fusion à surveiller). Capture du carreau en vol reportée à AR10.
- 2026-10-27 : **AR10 faite** (sous-agent, ~35 min dont ~10 d'attente : l'emballage d'Elrond puis les compilations d'autres copies bloquaient les lancements). Nouveau scénario **`crossbows`** (+ `.json`, Plain humain contre IA) : Champ de tir et Caserne finis, 6 Arbalétriers, 4 Hommes d'armes + 4 Lanciers ennemis à ~30 m, 5000 de tout, 5 notes. Repris : `archers.txt` (Champ de tir au lieu de la Caserne ; caméra corrigée avant `OrderNear`, le point était hors écran), `j7_army.txt` (Champ de tir ajouté), `building_destroy.txt` (file `Grid 0/1/0`). Les 4 vérifiés hors écran (prêts, 39-62 s). Combat lu au journal : **18 par carreau contre l'Homme d'armes** (mort en 9), 12 contre le Lancier (7), un tir toutes les ~2,1 s ; gestes `A_Crossbow_*` joués. Captures `scratchpad\captures\ar10_*.png` (carreaux en vol, pied à l'étrier lisibles ; carreau caché pas vérifiable à cette taille). `quick_check -Parts tests,build,anims,net` **PASS en 245 s**, 189/189. Aucun défaut du jeu. **Essai de l'utilisateur lancé** (`play.ps1 crossbows -Root …Celeborn`).
- 2026-10-27 : **essai validé par l'utilisateur** (« tout est bon »). Commit **db564be** sur `crossbow` (AR6-AR10). `main` fusionné dans `crossbow` (aedb733 : ligne Héros gardée à côté de l'Arbalétrier, `Projectile` vide ; `GetDamage` rend un float depuis J8a.1 → comparaisons du test en float ; `quick_check -Parts tests,build,combat,net` PASS, 196/196). `main` a encore avancé (7c48518, J8a.2 : `SKM_Hero`) : deuxième fusion en cours, non commitée (conflits `UnitTypes.csv`, `make_peasant_mesh.py` : les deux gardés), compilée, table réimportée ; tests 199/199, `build` et `combat` PASS ; **`net` en échec 4 fois de 4 façons différentes** (camps, client 1 figé après Turnkey, 12 flèches déduites sur 18 avec ~0,5 s de retard, serveur sans partie) **sous CPU à 85-100 %** (deux éditeurs ProjectShiva, un jeu dans Haldir, compilations) : à relancer machine calme avant de commiter la fusion et d'avancer `main`. GDD D486-D489 rédigés (`scratchpad\gdd_rows.md`), à inscrire avec la fusion dans `main` (AR11).
- 2026-10-27 : deuxième fusion (J8a.2) commitée (429a29e) avec deux correctifs de `quick_check` : `UE_SKIP_UBT_SDK_SETUP=1` pour tout jeu lancé (la mise en place AutoSDK par UnrealBuildTool `-Mode=ValidatePlatforms` attendait jusqu'à 4 min 30 derrière les compilations des autres sessions) et clients `net` attendus 300 s depuis leur lancement (au lieu de 120 s depuis les contrôles) ; `net` PASS au dernier essai (148 s). Troisième fusion (D485) sans conflit (ce7cc76, `tests,build` PASS 199/199). **`main` avancé sur ce7cc76** avec l'accord des sessions de la copie principale (Héros : `quick_check.ps1` mis de côté puis repris).
- 2026-10-27 : **AR11, chantier clos.** GDD : **D486** (stats de l'Arbalétrier), **D487** (Champ de tir dès le prototype, l'Archer y passe, forme provisoire), **D488** (allure et gestes), **D489** (carreau en vol) ; prochain numéro D490. `EtapeDefini.md` : ligne « Bonus de l'Arbalétrier contre Lourd » retirée, journal archivé dans `Docs/Archive/`. Part IA (D421) signalée à la session IA pour IA2. Restes notés, sans ligne : fantôme et zone de navigation du Champ de tir hauts de 600 cm pour un bâtiment fini de 294 cm ; marques des cibles un peu petites. Worktree `HeroesOfLiberty-Celeborn` rendu libre.
