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
| AR5 Données | ligne `Crossbowman` de `UnitTypes.csv` (AR-Q1), ligne `ArcheryRange` de `BuildingTypes.csv` (AR-Q2a), `UnitStats.md` regénéré ; colonnes nouvelles (forme, projectile) laissées à AR7 / AR9 (l'import les demande dans la structure C++) | tables relues |

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
