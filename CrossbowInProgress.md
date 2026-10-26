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
