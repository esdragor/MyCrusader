# Implementation in progress

**Journal de reprise du chantier de code.** Il dit où on en est et quoi faire ensuite, pour qu'une
reprise se fasse en disant simplement **« continue ImplementationInProgress »**.

Compagnon du [GDD](./GDD.md) : le GDD garde les décisions de conception (§ 21, journal `D…`) et le point
de reprise du § 22 ; ce fichier-ci garde l'**état d'avancement du code** (fait, en cours, à faire, questions
ouvertes, commandes utiles). Toute décision de conception va dans le GDD, jamais ici.

Règles de tenue : une entrée par étape ou analyse (la plus récente en haut de « Journal d'avancement »),
des faits mesurés plutôt que des intentions, et jamais de mise à jour silencieuse d'une mesure.

---

## Où on en est

| Bloc | État |
|---|---|
| M0 — migration 5.8, modules, plugins, carte de test | **fait** (2026-10-11) |
| M1 — essai Mass (D215) | **fait** (2026-10-11) : seuils D218 tenus sur S1 et S2 après **D220**, hypothèse `Stand` vérifiée |
| M1 — cœur de la simulation (`UnitData`, identifiants, apparition/disparition, file de commandes) | **fait** (2026-10-11) : D221 à D229 implémentés ; **revue traitée le 2026-10-12** (deux bugs corrigés, D230) puis **suites de la revue** (maintien corrigé, A/B reproductible) ; S1 2,42 ms, S2 2,48/2,58 ms ✅, recouvrements S2 102 → 68 (173 → 120 sans maintien), croisement 3,5 cm de face, 6,1 cm à angle droit (limitation connue) |
| M2 — rendu (instances par type et par joueur, animations, interpolation) | à faire |
| M2.5 — tranche réseau Iris (D216) | à faire |

## Prochaine étape exacte

1. ~~Table de données, fabrique, identifiants, naissances/destructions, file de commandes~~ → **fait (2026-10-11)**.
2. ~~S1 et S2 en non-régression~~ → **fait** : S1 2,25 et 2,10 ms, S2 2,21 et 2,25 ms, rendu ≥ 144 i/s.
3. ~~Revue de l'utilisateur : deux bugs, dette inexistante, axes d'amélioration~~ → **fait (2026-10-12)** : bugs corrigés et
   mesurés, journée d'évitement instruite (**D230**), journaux dédoublonnés, graine fixée, test de croisement ajouté.
3b. ~~Suites de la revue (`ReviewM1Followup.md`)~~ → **fait (2026-10-12)** : maintien de l'impulsion corrigé et vérifié,
   A/B reproductible de D230 (gain net confirmé), spawn robuste, scénario C paramétré par angle, drapeau « solveur actif »
   posé par le pas, stress test avancé par pas. Détail dans le journal.
4. **M2 — rendu**, au-dessus du cœur de M1 : instances par type et par joueur, animations en **squelettes instanciés**
   à l'essai (D232, `AnimToTexture` en repli), interpolation déjà en place (`UHOLInterpolatedISMProcessor`) ; garder S1
   et S2 au vert, et une vérification à 2 clients dès M2.5 (D216). **En cours : plan de M2, question par question**
   (voir « Plan de M2 ») ; **plan complet (D232 à D236)** ; étapes 1 à 3 faites (squelettes instanciés animés et
   colorés, voir le journal) ; **en cours : étape 4**, vues en place (D236, D237) ; reste à mesurer machine libre
   (classique contre Nanite) et à régler la foulée.
5. **Avant M5, deux points ouverts à instruire** (voir « Questions ouvertes ») : le **croisement** de deux unités (de face,
   même le solveur à chaque pas échoue ; à angle droit, tout solveur ralenti échoue — outillage prêt : `hol.StressTest.CrossingAngle`,
   `hol.Units.AvoidanceTrace`) et la **vitesse d'arrêt nulle** qui empêche la séparation de résorber les recouvrements
   d'unités immobiles.
6. **M4 — à prévoir** : un ordre de groupe vers un **même point** ne verra jamais ses unités s'arrêter (tolérance
   d'arrivée de 50 cm pour toutes) : il faudra une formation ou une tolérance liée à la taille du groupe.
7. Proposer avant d'écrire tout choix d'implémentation non trivial qui n'est pas déjà couvert par D221 à D230.

## Choix d'implémentation tranchés (détail au GDD, journal § 21)

| # | Question | État |
|---|---|---|
| D227 | **Fabrication de la table des types** : un `.uasset` ne s'écrit pas à la main. | **tranché** : CSV source `Tools/Data/UnitTypes.csv` + script d'éditeur `Tools/Editor/make_unit_types.py` → `/Game/Units/UnitTypes`, lu par configuration |
| D228 | **Mouvement de l'ordre** « va à cet endroit ». | **tranché** : cible directe (`FMassMoveTargetFragment`, action `Move`, arrivée détectée par nous), destination ramenée au navmesh **à l'émission** (une requête de point par ordre) ; chemins à M3 |
| D229 | **S2 du stress test**. | **tranché** : S2 passe aux ordres de M1, l'errance d'essai est supprimée, le scénario est requalifié (M3 le rendra exigeant) |
| D230 | **Cadence de l'évitement** (revue du 2026-10-12). | **tranché** : cadence de D220 conservée **et** impulsion d'évitement ré-appliquée exactement 5 pas, sans décroissance, bornée à `MaxAcceleration` ; gain confirmé par un A/B reproductible (102 → 68 contre 173 → 120) ; croisement consigné comme limitation **non expliquée** ; graine fixée, stress test par pas |

## Plan de M2 (à valider question par question)

Rappel de l'étape (§ 16.6) : *« rendu : instances par type et par joueur, animations, interpolation »*, mesuré sur S2.
État de départ : une instance **statique** par type (`SM_Mannequin`, `UMassMovableVisualizationTrait` posé par la
fabrique), interpolée entre deux pas (`UHOLInterpolatedISMProcessor`) ; ni joueur propriétaire, ni animation.
Numérotation : **D231 est réservé** au serveur dédié (relais `TMP.md`, autre chantier), d'où D232 pour M2.

| # | Question | État |
|---|---|---|
| Q1 | **Technique d'animation** : squelettes instanciés natifs de Mass, textures d'animation (`AnimToTexture`) ou acteurs près de la caméra ? | **tranché (D232)** : squelettes instanciés à l'essai borné, `AnimToTexture` en repli |
| Q2 | **Joueur propriétaire** : où vit l'appartenance d'une unité, et comment le rendu la montre (couleur par instance, ou instances séparées par joueur). | **tranché (D233)** : octet `OwnerPlayer` dans `FHOLUnitStateFragment`, couleur par valeur d'instance et palette côté client, un composant par type |
| Q3 | **Animations jouées** : lesquelles en M2 (repos, marche), qui les choisit (état d'activité, vitesse), décalage pour désynchroniser. | **tranché (D234)** : repos et marche, choisis par le client d'après la vitesse affichée (seuil ~15 % avec hystérésis), fondu ~0,2 s, départ décalé au hasard, vitesse de lecture par type |
| Q4 | **Ressources de test et niveaux de détail** : mannequin du pack Third Person, Nanite ou non, maillages par distance. | **tranché (D235)** : mannequin squelettique en deux copies (Nanite, classique), deux lignes de la table des types, chemin tranché sur mesure |
| Q5 | **Mesure** : ce que le stress test montre et mesure pour « 2 000 unités à l'écran » (caméra, scénario). | **tranché (D236, cadrage corrigé par D237)** : vue large à ~100 m, 2 000 unités entières sur une nuée S2 de ~45 m ; vue de jeu à hauteur de RTS sur une nuée de ~30 m, en partie hors cadre, unités à l'écran comptées ; appels de dessin au rapport (triangles non mesurables) ; S2 requalifié |

**Plan complet (D232 à D236).** Ordre d'implémentation proposé :
1. **Ressources** : importer le mannequin squelettique et ses animations de repos et de marche (`TP_ThirdPerson.upack`) par
   script d'éditeur (`Tools/Editor/`), en deux copies (Nanite, classique avec niveaux de détail générés) ; créer le
   fournisseur d'animation (`UAnimSequenceTransformProviderData`, repos et marche) et le matériau teinté par une valeur
   d'instance (palette de joueurs).
2. **Table des types** : colonnes squelette (maillage, fournisseur, indices des séquences, vitesse de référence de la
   marche) ; deux lignes d'essai (Nanite, classique) dans `Tools/Data/UnitTypes.csv`.
3. **Code** : `OwnerPlayer` dans `FHOLUnitStateFragment` (D233) ; la fabrique pose `SkinnedMeshInstanceDesc` et la
   représentation `SkinnedMeshInstance` ; variante interpolée de `UMassUpdateSkinnedMeshProcessor` (comme
   `UHOLInterpolatedISMProcessor`) ; processeur de présentation qui choisit l'animation d'après la vitesse affichée
   (D234) et écrit la couleur du joueur ; le stress test répartit les unités entre joueurs.
4. **Stress test** : nuée de S2 à ~45 m, deux vues (D236), triangles et appels de dessin au rapport ; nouvelle référence
   de S1/S2, puis mesures Nanite contre classique et décision consignée (D235). Bascule sur `AnimToTexture` si
   144 i/s ne tient pas (D232).

Repères vérifiés dans le moteur 5.8.2 (`C:/UE/UE_5.8_AS_1`) : type de représentation `EMassRepresentationType::SkinnedMeshInstance`,
`FSkinnedMeshInstanceVisualizationDesc` (dans `UMassVisualizationTrait::SkinnedMeshInstanceDesc`, un fournisseur de
transformations par maillage), `UMassUpdateSkinnedMeshProcessor`, `UMassConsumeInstancedSkinnedMeshAnimationProcessor`
(consomme les `FAnimSequenceTrackAutoPlayData` : séquence, position, vitesse, durée de fondu, boucle) ;
`UInstancedSkinnedMeshComponent` fonctionne avec ou sans Nanite (`ShouldNaniteSkin`). Ressources : `TP_ThirdPerson.upack`
(`FeaturePacks/`) contient le mannequin squelettique et ses animations.

## Plan de M1 (à valider question par question)

Rappel de l'étape (§ 16.6) : *« cœur de la simulation : entités, identifiants stables, apparition et
disparition, `UnitData` »*. Rien de tout cela n'existe : les unités ne naissent aujourd'hui que par la
commande de stress test, sans type, sans identifiant et sans disparition.

| # | Question | État |
|---|---|---|
| Q1 | **Données d'unité — forme du fichier** : table de données, DataAsset, ou les deux ? | **tranché (D221)** : table de données, références souples |
| Q2 | **Données d'unité — à l'exécution** : où vivent les valeurs et quels champs sont statiques ou variables. | **tranché (D222)** : fragment partagé constant par type + fragment par entité |
| Q3 | **Identifiant stable** : taille et forme, recyclage, table identifiant ↔ entité, ce qu'il faut prévoir pour Iris (M2.5). | **tranché (D223)** : identifiant maison de 32 bits, jamais recyclé, + table identifiant ↔ entité |
| Q4 | **Apparition et disparition** : qui fait naître une unité, comment on regroupe les naissances, comment on fait disparaître. | **tranché (D224)** : file de naissances avec budget par pas, disparition différée à la fin du pas |
| Q5 | **Archétypes et types** : un archétype par type, mis en cache, comment un type se déclare. | **tranché (D225)** : fabrique en code, un archétype par type |
| Q6 | **Périmètre exact de M1** : ce qu'on modélise maintenant et ce qu'on repousse. | **tranché (D226)** : le cœur + la file de commandes, un seul ordre |

**Plan complet (D221 à D226).** Prochaine étape : implémenter M1, dans l'ordre du plan — table de données et
fragments, fabrique, identifiants, file de naissances/destructions, file de commandes. Toute étape
d'implémentation est consignée dans le journal ci-dessous, et les valeurs chiffrées restent provisoires
(fiche de valeurs, à régler en test).

**Cœur de M1 implémenté (2026-10-11).** Fichiers créés : `Simulation/HOLUnitTypes.h`, `Simulation/HOLUnitTrait.*`,
`Simulation/HOLUnitMovementTrait.h`, `Simulation/HOLUnitSubsystem.*`, `Commands/HOLUnitCommand.h`,
`Commands/HOLCommandSubsystem.*`, `Commands/HOLUnitOrderProcessor.*`, `Tools/Data/UnitTypes.csv`,
`Tools/Editor/make_unit_types.py`. Fichiers supprimés : `Simulation/HOLLightUnitTrait.*`,
`UHOLWanderProcessor` (+ `FHOLWanderTag`), dépendance `NavCorridor`.

## Questions ouvertes

- **Croisement de deux unités — comportement non expliqué (2026-10-12, remesuré à passages reproductibles)** : écart
  minimal pour une somme des rayons de 80 cm, maintien coupé sauf mention :

  | Cadence du solveur | 0° (face-à-face) | 90° | 135° |
  |---|---|---|---|
  | chaque pas | 4,5 cm ❌ | **65,9 cm** ✅ | — |
  | 1 sur 2 | 9,2 ❌ | 0,9 ❌ | — |
  | 1 sur 3 | 7,3 ❌ | 3,8 ❌ | — |
  | 1 sur 6 (D220) | 10,7 ❌ | 5,2 ❌ | 1,0 ❌ |
  | 1 sur 6 + maintien 5 pas (D230) | 3,5 ❌ | 6,1 ❌ | **88,9** ✅ |

  Ce qu'on sait : **de face**, l'échec ne vient pas de la cadence (même chaque pas échoue) ; la trace montre une
  impulsion de pur **freinage** (−250 sur l'axe de marche, composante latérale inférieure à 0,1 cm/s²), dans une
  géométrie parfaitement symétrique. **À angle droit**, toute cadence ralentie échoue. Entre deux passages du solveur,
  le **pilotage ramène la vitesse** vers la vitesse voulue (trace : force de pilotage +42, +76, +105… cm/s² pendant
  que l'impulsion freine), hypothèse 1 de la revue confirmée. Reste à instruire **avant M5** : décalages de phase de
  l'ordre (0 à 5 pas), angles intermédiaires, effet d'une légère dissymétrie de face ; la trace par pas existe
  (`hol.Units.AvoidanceTrace 1+2`).
- **Séparation des unités immobiles** : `MassStandingAvoidanceProcessor` pousse un **fantôme** et la conduite
  d'arrêt déplace le corps **en appliquant `MoveTarget.DesiredSpeed`** — que la naissance met à 0 pour une unité
  immobile : le fantôme s'écarte, le corps ne bouge pas (les recouvrements de S1, ~215 paires à la naissance,
  restent inchangés). Piste à mesurer : une vitesse d'arrêt non nulle, ou une résorption explicite des
  chevauchements à la naissance (répulsion de spawn).
- **Rendu : écart de mesure inexpliqué** : les passages du 2026-10-11 donnaient 145 à 170 i/s à 2 000 unités, ceux
  du 2026-10-12 (mêmes unités, même carte, même commande) 247 à 258 i/s, pour un plafond de scène vide de 284 i/s
  (scénario de croisement à 2 unités). Le seuil de 144 i/s passe dans les deux cas ; l'écart (état de la fenêtre de
  jeu ? charge de la machine ?) reste à comprendre avant de s'appuyer sur la marge de rendu.
- **Marge de rendu** : le seuil de 144 i/s est tenu avec une marge désormais confortable dans les derniers passages,
  mais l'écart ci-dessus invite à la prudence avant de conclure ; à surveiller quand le contenu s'ajoute (animations,
  effets, bâtiments).
- **Réserve si un seuil casse** (mêlées de M5, unités plus rapides ou plus nombreuses) : paralléliser l'évitement de
  Mass (option B1) ou l'écrire dans la couche RTS (option A). Non nécessaires aujourd'hui.
- ~~**Table identifiant ↔ entité et changement d'archétype**~~ → **n'existe pas (2026-10-12)** : le
  `FMassEntityHandle` d'une entité **reste valide** quand elle change d'archétype (`FMassArchetypeData::MoveEntityToAnotherArchetype`
  réinsère le même handle, seul l'emplacement mémoire change) ; il ne devient invalide qu'à la destruction. La
  table n'a donc rien à rafraîchir. Le motif de D223 a été corrigé en conséquence (recyclage des index).
- ~~Hypothèse `Stand`~~ → **résolue (2026-10-11)** : voir le journal. Ce n'était pas un gain mais un transfert
  de coût ; les deux solveurs d'évitement sont désormais ralentis (D220).

## Pièges de Mass connus

Savoir d'implémentation (déplacé du GDD le 2026-10-12, où il ne reste qu'un renvoi). Chaque piège a coûté une
erreur silencieuse ; la plupart sont désormais gardés par le code.

- **Fragment partagé : tous les champs en `UPROPERTY`.** Mass identifie une instance partagée par une empreinte
  calculée sur les champs **réfléchis**, puis compare ces mêmes champs : des champs ordinaires rendent deux types
  d'unité interchangeables (l'archer héritait des constantes de l'infanterie). Garde-fou : la fabrique compare
  l'empreinte de chaque type et journalise une erreur si deux types se ressemblent.
- **Identité d'un `FMassEntityConfig`** : un GUID **dérivé du chemin de son propriétaire** ; une fabrique qui donne
  le même propriétaire à tous les types les fait répondre par un seul template. Un GUID neuf par type
  (`MakeShared<FMassEntityConfig>()` puis `SetOwner`).
- **`ProjectPointToNavigation` avec une boîte large** accroche la position sur une grille grossière du navmesh de la
  carte de test (le spawn y perdait sa répartition) : boîte serrée en horizontal, large en Z.
- **`DistanceToGoal` n'est entretenu par Mass que derrière un chemin navmesh.** En cible directe il reste à 0 : Mass
  croit l'unité arrivée et ramène l'évitement à 10 % (`EndOfPathAvoidanceScale`). Nous le réécrivons à chaque pas.
- **`MoveTarget.Forward` décide du pilotage** : `MassSteerToMoveTargetProcessor` ramène l'unité sur la ligne
  `Center` / `Forward`. Réécrit à chaque pas vers la cible (ce que nous faisons), l'écart latéral est toujours nul
  et l'unité va droit vers la cible (D228).
- **Mass remet la force à zéro après l'avoir appliquée** (`MassApplyForceProcessor`) : un évitement ralenti est
  oublié au pas suivant, d'où le maintien de l'impulsion (D230).
- **Seul le solveur d'évitement borne la force** à `MaxAcceleration` ; le pilotage ne la borne pas
  (`(vitesse voulue − vitesse) / ReactionTime` : 1 000 cm/s² au départ d'une unité à 300 cm/s). Le maintien de D230
  borne sa propre somme comme le fait le solveur.
- **API synchrone d'entités interdite pendant un processeur** (`IsProcessing()`, assertion de
  `FMassEntityManager`) : naissances et destructions se font à la tête et à la queue du pas (D224).
- **Une graine fixe ne suffit pas à rendre un test reproductible** : tout ce qui pilote la simulation (ordres,
  phases du test) doit avancer **par pas**, jamais par image.
- **Squelettes instanciés : position, animation et valeurs d'instance sont appariées par leur ordre d'ajout**
  (`MassVisualizationComponent.cpp`, `EndVisualChanges` : un même index dans les trois tableaux). Les ajouter dans
  **une seule boucle** (`UHOLInterpolatedInstancesProcessor`) ; les processeurs de Mass séparés
  (`UMassUpdateInstancedSkinnedMeshProcessor`, `UMassConsumeInstancedSkinnedMeshAnimationProcessor`, ce dernier sans
  filtre de visibilité) peuvent les désaligner — désactivés dans `DefaultMass.ini`.
- **Mass ne transmet l'animation d'une instance que si sa séquence change** (vitesse de lecture fixée au changement, D234).
- **Le fournisseur d'animation exige « Optimize for Instancing »** sur chaque niveau de détail du maillage squelettique
  (carte d'os unifiée), et le matériau les usages `SkeletalMesh` **et** `InstancedSkinnedMesh` — sinon matériau par
  défaut, avec un avertissement `Material with missing usage flag` au lancement.
- **Le compteur de triangles du RHI (`GNumPrimitivesDrawnRHI`) vaut 0** pour les squelettes instanciés et Nanite :
  dessins indirects pilotés par le GPU, dont le processeur ne connaît pas la taille.

## Journal d'avancement

### 2026-10-13 (suite) — Projet renommé en Heroes of Liberty (D248)

- Renommage scripté (46 fichiers modifiés, 40 renommés), correspondance complète dans D248 : module `HeroesOfLiberty`,
  préfixe **`HOL`** (classes, fichiers, `LogHOL…`), variables console **`hol.*`**. Dossier du projet et historique git
  inchangés. `Binaries` et `Intermediate` supprimés puis recompilés (365 s) ; `UnitTypes` régénérée depuis le CSV.
- Vérification : S2 court (2 000 unités, 5 s) lancé sur le projet renommé : module chargé, constantes de type lues
  dans la table régénérée, squelettes instanciés affichés, aucune erreur du projet. **Chiffres non significatifs** (la
  compilation du moteur 5.8.3 occupait le processeur) : pas de nouvelle référence.
- Piège : lancer l'éditeur depuis **PowerShell** ; Git Bash convertit `/Game/Maps/StressTest` en chemin Windows et
  l'éditeur se bloque sur « carte introuvable ».

### 2026-10-13 (suite) — M2, étapes 1 à 3 : squelettes instanciés animés et colorés, vues de D236 en cours

- **Ressources** (`Tools/Editor/make_skinned_mannequin.py`, idempotent) : le **mannequin UE4 du projet**
  (`CloseCombat/Character/UE4Mannequin`, source de `SM_Mannequin`) plutôt que Manny du pack Third Person — même
  modèle que l'historique, animations d'**épéiste** (repos `A_SW_Idle` 11,33 s, marche `A_SW_Walk_F` 0,87 s, **sans
  déplacement de la racine**), rien à importer ; précision consignée dans D235. Deux copies : `SKM_Mannequin_Classic`
  (4 niveaux de détail : 23 309 / 6 733 / 3 803 / 2 152 sommets) et `SKM_Mannequin_Nanite` ; fournisseurs
  `TP_Mannequin_*` (séquence 0 = repos, 1 = marche) ; matériau `M_UnitPlayerColor` (couleur de base = valeurs
  d'instance 0-2, gris clair par défaut). Pièges rencontrés : voir « Pièges de Mass connus » ; en Python, modifier les
  `SkeletalMaterial` renvoyés par `get_editor_property` ne change que des copies (il faut des structures neuves), et
  supprimer puis recréer un asset référencé échoue (l'objet reste chargé) — d'où la mise à jour en place, y compris
  dans `make_unit_types.py`.
- **Table des types** : colonnes `SkinnedMesh`, `AnimationProvider`, `IdleSequence`, `WalkSequence`, `WalkPlayRate`
  (constantes du type, D222, plus les durées des séquences lues par la fabrique) ; lignes `LightInfantry` et `Archer`
  en classique, **`LightInfantryNanite`** pour la comparaison de D235. Sans squelette valide, la fabrique retombe sur
  le maillage statique (repli de D232).
- **Code** : `OwnerPlayer` dans `FHOLUnitStateFragment` et dans la requête de naissance (D233) ; la fabrique pose
  `SkinnedMeshInstanceDesc` et la représentation `SkinnedMeshInstance` ; `UHOLInterpolatedISMProcessor` renommé
  **`UHOLInterpolatedInstancesProcessor`** : interpolation des instances statiques et squelettiques, choix repos/marche
  d'après la vitesse affichée (seuils 15 % / 10 % de la vitesse du type, fondu 0,2 s, départ décalé par une empreinte
  de l'identifiant), couleur du joueur (palette de 8 couleurs côté client) ; fragment `FHOLUnitAnimationStateFragment`
  (animation démarrée ou non). Le stress test répartit ses unités entre **8 joueurs** (`hol.StressTest.Players`), en
  une requête par joueur : même nuée qu'avant (102 recouvrements au début de S2, comme l'historique).
- **Vérifié visuellement** (captures S2) : 2 000 squelettes instanciés, animés, désynchronisés, aux 8 couleurs ; aucun
  `ensure`. `WalkPlayRate` reste à 1,0, à régler à l'œil (foulée).
- **Stress test** : `hol.StressTest.View Wide|Game` (D236) avec la nuée de chaque vue (`hol.StressTest.SpawnRadius`
  négatif = 45 m / 30 m), ligne `Draw` (appels de dessin) au rapport. **Triangles retirés** : compteur du RHI à 0 pour
  ces dessins (voir les pièges) — écart à D236, à signaler.
- **Mesures provisoires, non recevables** (le moteur 5.8.3 compile en parallèle, 3 `cl.exe`) : S2 classique, vue de
  jeu 135 i/s, vue large 107 i/s ; **GPU 4,1-4,2 ms** dans les deux (sous les 6,9 ms de 144 i/s), fil jeu 7,3-9,3 ms,
  fil de rendu 3,6-3,9 ms ; 88-95 appels de dessin. Tendance : le GPU tient, le fil jeu limite — à remesurer machine libre.
- **Problème de cadrage (D236 à reprendre avec l'utilisateur)** : avec un champ vertical de 59° (90° horizontal en
  16:9), voir **en entier** une nuée de 45 m de rayon (~50 m après la marche) demande ~100 m de recul, pas 70 m
  (estimation faite sur le champ horizontal) ; à hauteur de RTS (~40 m), 2 000 unités ne tiennent dans le cadre qu'à
  ~1 m² chacune. Les deux captures coupent le bas de la nuée.
- **Cadrage tranché (D237)** : vue large à 100 m, tangage 45° (2 000 unités entières) ; vue de jeu à 46 m, tangage 55°
  (~38 m de hauteur), nuée en partie hors cadre. Le rapport compte les unités à l'écran (projection des pieds dans la
  vue du joueur, fin de la mesure). Vérifié : vue large **1 994 / 2 000** à l'écran (quelques isolés sortent par le bas
  en marchant), vue de jeu **1 838 / 2 000** ; 61 et 94 appels de dessin. Toujours sous compilation du moteur
  (126 et 119 i/s, GPU 4,1 ms, fil jeu 7,8-8,4 ms) : non recevable.
- **Prochaine étape** : mesurer **machine libre** (fin de la compilation du moteur 5.8.3) : S1 et S2 dans les deux vues,
  `LightInfantry` (classique) contre `LightInfantryNanite`, deux passages chacun ; nouvelle référence de S2 (D236) ;
  trancher le chemin de rendu (D235) ; régler `WalkPlayRate` à l'œil.

### 2026-10-13 — Plan de M2 ouvert : technique d'animation tranchée (D232)

- **Constat dans les sources du moteur** : Mass 5.8 sait représenter une entité en **maillage squelettique instancié**
  (`SkinnedMeshInstance`), animé par le GPU depuis des séquences, en plus des instances statiques ; mode absent quand le
  § 16.4 a été écrit (« textures de sommets, ou équivalent »).
- **D232** : squelettes instanciés natifs de Mass dans un **essai borné** (2 000 unités en marche et à l'arrêt sur S2),
  bascule sur `AnimToTexture` si 144 i/s ne tient pas ou si la technologie résiste trop. Consigné au GDD (§ 16.4,
  § 16.6, journal).
- **D233** : propriétaire = octet `OwnerPlayer` de `FHOLUnitStateFragment` ; couleur par valeur d'instance, palette choisie
  par le client, un seul composant d'instances par type. Consigné au GDD (§ 16.6, journal).
- **D234** : repos et marche, choisis par le client d'après la vitesse affichée ; fondu, départ décalé, vitesse de
  lecture par type. **Contrainte vérifiée dans le moteur** : `UMassVisualizationComponent` (`MassVisualizationComponent.cpp`,
  vers la ligne 768) n'appelle `SetAutoPlayData` que si l'index de séquence change ; la vitesse de lecture ne suit donc pas
  la vitesse instantanée sans chemin à nous (`UAnimSequenceTransformProviderDataInstance::SetPlayRate`). Les fondus de
  `BlendSpace` existent dans le fournisseur, mais le chemin de Mass ne les utilise pas.
- **D235** : mannequin squelettique du pack Third Person en deux copies (Nanite, classique), deux lignes de
  `Tools/Data/UnitTypes.csv`, comparaison par `hol.StressTest.UnitType` ; chemin tranché sur mesure, remesuré avec les
  modèles définitifs. Le projet n'a aucun réglage de rendu dans `DefaultEngine.ini` (réglages par défaut du moteur ;
  le log montre Lumen et Nanite actifs).
- **D236** (précision de l'utilisateur : caméra rapprochée) : vue large à ~70 m sur une nuée S2 resserrée à ~45 m,
  vue de jeu à hauteur de RTS sur ~30 m ; triangles et appels de dessin au rapport ; S2 requalifié.
- **Plan de M2 complet (D232 à D236).** Prochaine étape : implémenter dans l'ordre du plan (ressources, table, code,
  stress test). **Aucune mesure tant que la compilation du moteur 5.8.3 tourne** (autre chantier, `TMP.md`) : elle
  occupe les cœurs et fausserait les temps.

### 2026-10-12 (suite) — Suites de la revue de M1 (`ReviewM1Followup.md`) : maintien corrigé, A/B reproductible de D230

- **Tâche 1 — maintien de l'impulsion corrigé** (`HOLAvoidanceHoldProcessors.cpp`). Défaut confirmé : au pas du solveur,
  l'impulsion (déjà dans la force) était **ajoutée une seconde fois**, puis ré-appliquée **4 pas** au lieu de 5, et la
  somme dépassait le plafond `MaxAcceleration` que le solveur pose lui-même (`MassAvoidanceProcessors.cpp:1078`). Le
  solveur muet laissait en outre courir l'ancienne impulsion. Désormais : au pas du solveur, on mémorise (ou on efface)
  sans rien ajouter ; aux autres pas, on ajoute, on borne à `MaxAcceleration` (fragment partagé `FMassMovementParameters`),
  puis on applique la décroissance. **Vérifié par trace** (`hol.Units.AvoidanceTrace 1+2`, scénario C) : 8 cycles avec
  impulsion → **exactement 5 ré-applications** chacun, 32 cycles sans → 0 ; force ≤ 250 sur **tous** les pas ré-appliqués.
  Les seuls pas au-delà du plafond sont ceux du **pilotage** de Mass au départ (1 000 = 300 / 0,3 s), qui ne nous
  appartiennent pas (voir « Pièges de Mass connus »).
- **Conventions des CVars alignées** (corrections mineures) : `hol.Units.AvoidanceHold` et `hol.Units.AvoidanceHoldDecay`
  valent **négatif = réglage** (défaut) ; `AvoidanceHold 0` coupe le maintien, `AvoidanceHoldDecay 0` force une décroissance
  totale (impossible avant). `GetDefault` n'est plus appelé deux fois.
- **Découverte en route : les passages n'étaient pas reproductibles**, graine fixe ou non. Premier A/B (même binaire,
  mêmes réglages) : S2 sans maintien 171 → 135 puis 179 → 151, avec maintien 104 → 67 puis 104 → 108 ; croisement de
  face 3,5 puis 11,6 cm. Cause : ordres, échauffement et mesure du stress test suivaient les **images** ; une image lente
  fait tourner deux pas d'un coup et saute une passe d'ordres. **Corrigé** : `UHOLSimulationSubsystem` diffuse le début de
  chaque pas (`GetOnStepStarted`, avant le chrono du pas), et le stress test y émet ses ordres, surveille le croisement et
  compte ses phases **en pas** (3 s → 60, 10 s → 200). Vérifié : deux S2 de suite donnent **102 → 68 deux fois**, deux
  croisements **3,5 cm deux fois**. Les mesures antérieures de D230 sont marquées invalides dans l'entrée précédente.
- **Tâche 2 — A/B de D230 à graine et pas égaux** (2 000 unités, `LightInfantry`, seuil de pas 3 ms ; passages alternés) :

  | Variante | Pas S2 | Évitement de marche (par passage) | Recouvrements S2 (début → fin) | Rendu |
  |---|---|---|---|---|
  | sans maintien (`AvoidanceHold 0`) | 2,331 / 2,425 ms ✅ (2,807 / 2,849 dans une série antérieure) | 6,31 / 6,67 ms | **173 → 120** (identique aux 4 passages) | 223-235 i/s |
  | maintien 5 pas (réglage) | 2,581 / 2,481 ms ✅ | 7,30 / 6,86 ms | **102 → 68** (identique aux 4 passages) | 204-218 i/s |

  Le « début » est l'état après 60 pas de marche depuis la même nuée (215 recouvrements à la naissance). **Gain net
  confirmé** : −41 % de recouvrements après l'échauffement, −43 % en fin de mesure. **Coût** : non distinguable du bruit
  (± 0,3 ms d'un passage à l'autre ; les deux processeurs de maintien coûtent ~0,08 ms chacun et tournent aussi quand le
  maintien est coupé). D230 n'est donc **pas** remise en question. S1 (référence, même binaire) : 2,423 et 2,637 ms
  avec maintien, 2,403 sans, 215 → 215 recouvrements dans les trois cas.
  « 137 → 58 » remplacé partout avec explication : GDD § 16.6 et D230, `DefaultGame.ini`, `HOLSimulationSettings.h`,
  ce journal.
- **Tâche 3 — naissance hors navmesh** (`HOLUnitSubsystem::CreateUnitEntities`) : jusqu'à **4 tirages** par unité ; après le
  dernier, l'unité naît au point tiré (plus jamais empilée au centre) ; les échecs sont comptés et journalisés **une fois
  par lot** (`LogHOLSimulation` Warning). Sur la carte de test : **aucun échec**, S1 garde ses 215 recouvrements (même nuée).
  La graine est réinitialisée à chaque test (`ResetSpawnSeed`, appelé par le stress test).
- **Tâche 4 — scénario C paramétré** : `hol.StressTest.CrossingAngle` (0° = face-à-face, **défaut** ; 90° = l'ancien
  scénario ; 150° au plus), affiché dans le rapport ; commentaires corrigés (« 30 m » → 15 m, « swap places » →
  croisement sous un angle). Mesures 0°, 90° et 135° : voir « Questions ouvertes ». Les anciens chiffres de croisement
  (1,3 à 4,7 cm, 66,5 cm) étaient des passages uniques **à 90°**.
- **Tâche 5 — une seule source pour « le solveur tourne à ce pas »** : le pipeline garde le statut ralenti de chaque
  processeur (`ProcessorThrottled`), hors des statistiques ; `RunStep` décide une fois par pas, à partir de la CVar lue en
  direct, et publie la décision (`AreThrottledProcessorsRunningThisStep`) que lit le processeur de maintien. La statistique
  `StepInterval` n'est plus qu'un affichage.
- **Rapport du stress test** : nouvelle ligne `Avoidance cadence` (cadence du solveur, pas de maintien, décroissance) pour
  que deux rapports d'un A/B se distinguent ; capture nommée `StressTest_C.png` pour le croisement.
- **Corrections mineures** : indentation de `OrderRandom.Initialize` ; changement de pilotage dû à `Forward` consigné
  (D228 et « Pièges de Mass connus ») ; GDD : titre « en cours de planification » → « implémenté », bloc « Pièges de Mass »
  déplacé ici (renvoi d'une ligne au GDD), D230 « cinq pas » désormais exact. Trailer `Co-authored-by: Copilot` : déjà
  retiré (`8464f4c` est `a49aed7` réécrit, même arbre, sans trailer).
- **Commandes qui ont servi** : compilation et lancement habituels (voir « Commandes utiles ») ; A/B :
  `-ExecCmds="…,hol.Units.AvoidanceHold 0,hol.StressTest S2 2000 10 quit"` contre la même commande sans `AvoidanceHold` ;
  croisement : `…,hol.StressTest.CrossingAngle 90,hol.Simulation.ThrottledStepInterval 1,hol.StressTest C 2 10 quit` ;
  trace : `…,hol.Units.AvoidanceTrace 1+2,hol.StressTest C 2 10 quit` (CSV dans `Saved/AvoidanceTrace.csv`).
- **Prochaine étape** : M2 (rendu) ; le croisement reste à instruire avant M5.

### 2026-10-12 — Revue traitée : deux bugs corrigés, cadence d'évitement instruite (D230), mesures rapatriées ici

- **Bug 1 — un seul template pour tous les types d'unité** (signalé par la revue, cause trouvée dans le moteur).
  `FMassEntityConfig(UObject& InOwner)` dérive son identité (`ConfigGuid`) du **chemin de son propriétaire**
  (`MassEntityConfigAsset.cpp:30-34`) : mes configs partageaient le sous-système pour propriétaire, donc le même
  identifiant de template, et le registre répondait le **premier** template à tous les types (l'Archer naissait
  avec les constantes de l'infanterie). Reproduit en faisant naître **les deux types dans le même processus**
  (un seul passage de type ne pouvait pas le montrer) : `Unit constants: LightInfantry … x 1000` alors que
  500 Archer étaient vivants. Corrigé par un **GUID neuf par type** (`MakeShared<FMassEntityConfig>()` puis
  `SetOwner`). Avant correctif, le log de la fabrique montrait `birth Archer … template values speed 300`
  (au lieu de 280).
- **Bug 2 — champs de fragment partagé sans `UPROPERTY`** (signalé, confirmé et **indépendant** du premier) :
  Mass identifie une instance partagée par une empreinte calculée sur les champs **réfléchis**
  (`GetStructInstanceCrc32` → `SerializeItem`) puis compare **ces mêmes** champs (`CompareScriptStruct`) : sans
  `UPROPERTY`, l'empreinte est constante et la comparaison ne compare rien, donc deux types partagent une copie.
  Prouvé en deux temps : (a) avec le bug 1 corrigé et les `UPROPERTY` retirés, l'Archer reprend la vitesse 300 et
  le garde-fou journalise `Unit types 'LightInfantry' and 'Archer' have the same constants fingerprint` ;
  (b) avec les `UPROPERTY`, chaque type porte les siennes (`Archer: radius 35, speed 280, 80 HP, avoidance 220 x 500`).
  Corrigé (7 champs en `UPROPERTY`) **et gardé** : la fabrique compare l'empreinte de chaque type et journalise
  une erreur si deux types se ressemblent. Outillage associé : le rapport affiche les **constantes réellement
  portées** par les unités (lues dans le fragment, pas dans la table) — c'est cette ligne qui a révélé les deux bugs.
- **Bug 3 — `DistanceToGoal` jamais entretenu** (signalé, confirmé) : l'ordre posait `DistanceToGoal = 0` et rien
  ne le maintenait ; Mass ne l'entretient que derrière un chemin navmesh, donc il croyait l'unité **arrivée** et
  ramenait la force d'évitement à 10 % (`EndOfPathAvoidanceScale`, `MassAvoidanceProcessors.cpp:439`) pendant
  toute la marche. Corrigé dans `HOLUnitOrderProcessor::StopArrivedUnits` : distance et direction vers la cible
  réécrites à chaque pas. Effet isolé par un nouveau **test de croisement** (deux unités ordonnées à travers le
  même point, écart minimal rapporté, scénario `C`) : avec l'évitement à chaque pas, l'écart passe de **2,2 cm**
  (avant) à **66,5 cm** (après) — somme des rayons 80 cm.
- **D230 — cadence de l'évitement** (choix de l'utilisateur : garder D220 + retenir l'impulsion). Mesures,
  2 000 unités, seuil de pas 3 ms, écart au croisement pour une somme de rayons de 80 cm :

  | Variante | Croisement | Pas S2 | Recouvrements S2 (début → fin) |
  |---|---|---|---|
  | solveur **à chaque pas** | **66,5 cm** ✅ | ≈ 6,9 ms ❌ | — |
  | 1 pas sur 2 | 1,3 cm ❌ | 4,299 ms ❌ | 114 → 103 |
  | 1 pas sur 3 | 3,0 cm ❌ | 3,295 ms ❌ | 121 → 122 |
  | 1 pas sur 6 (D220) | 4,7 cm ❌ | ≈ 2,2 ms ✅ | 135 → 137 |
  | 1 sur 6 + **impulsion retenue 5 pas**, sans décroissance | 2,4 cm ❌ | 2,438 ms ✅ | ~~**101 → 58**~~ *(invalide, voir l'entrée suivante)* |
  | 1 sur 6 + impulsion retenue, décroissance 0,6 | 7,4 cm ❌ | 2,173 ms ✅ | 126 → 92 |
  | 1 sur 6 + impulsion retenue, décroissance 0,3 | 4,1 cm ❌ | (non mesuré) | — |

  Retenu : **cadence de D220 conservée + impulsion retenue 5 pas, sans décroissance** (`AvoidanceHoldSteps = 5`,
  `AvoidanceHoldDecay = 1`) — seuil tenu et foule qui se sépare (~~137 → 58 recouvrements~~) pour +0,2 ms de pas.

  > **Correction (2026-10-12, suite de la revue)** : toutes les lignes de ce tableau sont des passages uniques **non
  > reproductibles** (le stress test avançait par images, pas par pas), et les lignes « impulsion retenue » ont été
  > mesurées avec une version **fautive** du maintien (impulsion ajoutée deux fois au pas du solveur, puis quatre pas
  > seulement, plafond d'accélération contourné). « 137 → 58 » comparait en outre deux points de départ différents
  > (135 → 137 et 101 → 58). Chiffres comparables : **173 → 120 sans maintien, 102 → 68 avec** (entrée suivante).

  Implémentation : `FHOLAvoidanceHoldFragment` + deux processeurs nôtres encadrant les solveurs
  (`HOLAvoidanceSnapshotProcessor` après le pilotage, `HOLAvoidanceHoldProcessor` avant l'application des forces) ;
  Mass **remet la force à zéro** après application (`MassApplyForceProcessor`, « Reset to zero after force is
  applied »), d'où la nécessité de la réinjecter. Le savoir en clair est aussi consigné au GDD (§ 16.6, D230).
- **Limitation consignée, cause non élucidée** : l'écart au croisement **ne suit pas** la fréquence du solveur
  (1 sur 2 : 1,3 ; 1 sur 3 : 3,0 ; 1 sur 6 : 4,7 ; chaque pas : 66,5 cm) et l'impulsion retenue le **dégrade**
  (2,4 à 7,4 cm) : l'explication par le nombre d'impulsions est fausse. À instruire **avant M5** — durcir le test
  (décalages de phase, angles, graines) et instrumenter les pas **sans** solveur (force, vitesse désirée, pilotage).
- **Séparation** (question de la revue) : ce n'est pas un mécanisme distinct. Côté **marche**, la séparation est
  **dans** le solveur d'évitement (`SeparationRadiusScale`/`SeparationStiffness`), donc soumise à la même cadence.
  Côté **arrêt**, `MassStandingAvoidanceProcessor` pousse un **fantôme** (`FMassGhostLocationFragment`) et c'est
  la conduite d'arrêt qui déplace le corps — or elle applique `MoveTarget.DesiredSpeed`, que la naissance met à
  **0** pour une unité immobile : le fantôme s'écarte, le corps ne bouge pas. D'où les recouvrements de S1
  inchangés (~215 paires, soit la valeur théorique du tirage aléatoire). Piste à mesurer plus tard : vitesse
  d'arrêt non nulle, ou résorption explicite à la naissance.
- **Outillage et reproductibilité** : scénario **C (croisement)** au rapport du stress test ; **graine fixée** sur
  la nuée (`HOLUnitSubsystem::SpawnRandom`) et sur les destinations de S2 (`OrderRandom`). Deux passages identiques
  le confirment : S1 215 → 215 recouvrements deux fois, S2 100 → 87 deux fois, 2 352 ordres, 19,3 m parcourus.

  > **Correction (2026-10-12, suite de la revue)** : la graine ne suffisait pas. Deux S2 de suite donnaient 171 → 135
  > puis 179 → 151 (sans maintien), 104 → 67 puis 104 → 108 (avec) : les ordres, l'échauffement et la mesure suivaient
  > les **images**, et une image lente (naissance, capture) fait tourner deux pas d'un coup en sautant une passe
  > d'ordres. L'accord « 100 → 87 deux fois » était une coïncidence. Corrigé : le stress test avance **par pas**.

  Piège trouvé en route : `ProjectPointToNavigation` avec une **boîte de recherche large** (8 000 cm) accroche les
  positions sur une grille grossière du navmesh de la carte de test — les Y sortaient en multiples de 988 cm et le
  spawn passait de 215 à 1 555 recouvrements ; boîte serrée en XY et large en Z pour ne corriger que la hauteur.
  Vérifié au passage : la projection des **ordres** (boîte de 1 000 cm) ne déplace aucun point sur ce plan.
- **Corrections de nos propres écrits** (jamais silencieuses) :
  - la « dette » de la table identifiant ↔ entité **n'existait pas** : `FMassArchetypeData::MoveEntityToAnotherArchetype`
    réinsère le **même** `FMassEntityHandle` — vérifié dans le moteur — donc rien à rafraîchir ; ligne retirée et
    **motif de D223 corrigé** (le vrai motif est le recyclage des index après destruction) ;
  - les **mesures de l'essai Mass** (coût de l'évitement ~7-10 ms par pas à 2 000 unités, **par unité** et mono-fil,
    insensible à la densité et aux bords du navmesh ; deux solveurs, marche et arrêt) ont été **rapatriées ici** :
    le § 22 du GDD ne garde plus que les décisions et une synthèse chiffrée, la mesure vivant dans ce journal.
- **Mesures finales de l'état livré** (`AvoidanceHoldSteps=5`, graine fixe, 2 000 unités, seuil de pas 3 ms,
  rendu 144 i/s) : S1 **2,251 et 2,095 ms** ✅ (215 recouvrements, inchangés), S2 **2,248 et 2,209 ms** ✅
  (100 → 87), croisement **2,7 cm** ❌ (limitation connue) ; rendu **247 à 258 i/s** ✅.
  *(Version fautive du maintien et passages non reproductibles : remplacées par les mesures de l'entrée suivante.)*
  *Écart à surveiller* : les passages de la veille donnaient 145 à 170 i/s pour la même scène (scène vide : 284 i/s) —
  différence inexpliquée, probablement l'état de la fenêtre de jeu ; consigné en question ouverte, aucune conclusion
  tirée sur la marge de rendu.

### 2026-10-11 (suite) — M1 : cœur de la simulation implémenté, mesuré et vérifié

- **Implémenté dans l'ordre de D226.** Table de données : `FHOLUnitTypeRow` (`FTableRowBase`) et cache partagé
  `FHOLUnitTypeConstants` (`FMassConstSharedFragment`), plus `FHOLUnitStateFragment` (identifiant, points de vie
  courants, état marche/arrêt) — D221, D222, D226. Fabrique : `UHOLUnitSubsystem` construit et met en cache une
  `FMassEntityConfig` **par type** à partir de la ligne de table, les fragments communs étant posés par
  `UHOLUnitTrait` (identifiant d'état, transform précédent, cache du type) et les valeurs du type par
  `UHOLUnitMovementTrait` (vitesse) et `UHOLLightUnitAvoidanceTrait` (distance d'évitement, sortie du trait pour
  entrer dans la table, D220 → D221) — D225. Identifiants : compteur 32 bits jamais recyclé, table
  identifiant ↔ entité **et** l'inverse — D223. Naissances et destructions : file avec budget par pas, et
  commandes de destruction vidées à la queue du pas — D224. Ordres : `Commands/HOLCommandSubsystem`
  (file + destination ramenée au navmesh par une requête de point **par ordre**) et
  `Commands/HOLUnitOrderProcessor` (pose du `FMassMoveTargetFragment`, action `Move`, arrivée sous tolérance →
  `Stand` + état `Idle`) — D226, D228. L'errance d'essai et `UHOLLightUnitTrait` sont **supprimés** (D229).
- **Écart d'implémentation mesuré, corrigé : naissances et destructions ne peuvent pas être des processeurs.**
  Première tentative avec deux processeurs (naissance à la tête, destruction à la queue) : **crash** au premier
  pas — `Assertion failed: IsProcessing() == false` dans `FMassEntityManager::BatchCreateEntities` (« Use
  asynchronous API instead »). Vérifié dans le moteur : `MassExecutor` ouvre une portée de traitement autour de
  **chaque** processeur, et `FlushCommands` refuse lui aussi de tourner pendant ce temps. Les deux opérations
  sont donc portées par le **pas**, dans `UHOLSimulationSubsystem` (tête et queue, hors des processeurs), et
  chronométrées à part dans le rapport. Consigné au GDD (§ 16.6).
- **Défaut du stress test corrigé au passage :** S2 réémettait un ordre **à chaque image** aux unités encore
  `Idle` (l'ordre n'est appliqué qu'au pas suivant), donc leur destination changeait en boucle — 20 984 ordres
  émis en 10 s à la première mesure. Corrigé par une passe **par pas de simulation** (`GetStepIndex`), ce qui
  est la cadence réelle du système d'ordres : 2 565 ordres sur la mesure finale, cohérent avec ~1,3 trajet par
  unité.
- **Vérifications de fond** (rapport du stress test) : 2 000 unités nées par la file (0 naissance en attente),
  table identifiant ↔ entité **0 écart** sur 2 000 unités, `Move action` et `Unit state` concordent
  (`Stand`/`Idle` en S1, `Move`/`Moving` x 1991 en S2), `Units left after the run: 0` (destruction différée
  appliquée à la queue du pas), et le second type crée bien son propre archétype.
- **Budget par pas vérifié (D224)** : sans `hol.Units.SpawnBudget -1`, les 2 000 unités demandées sont servies
  **500 par pas** (valeur des réglages) et arrivent toutes (rapport : `2000 units of 2000 requested, 0 births
  waiting`) — la file est bien servie partiellement sur plusieurs pas, pas seulement en mode « tout d'un coup ».
- **Mesures (2026-10-11, éditeur en `-game` non cuit, 2 000 unités, `hol.Units.SpawnBudget -1`)** — seuils
  D218 : simulation ≤ 3 ms, rendu ≥ 144 i/s :

  | Scénario | Simulation | Rendu (1 % bas) | Action / état |
  |---|---|---|---|
  | S1 immobiles, `LightInfantry` | **2,096 ms ✅** | 145,8 i/s (72,8) ✅ | `Stand`/`Idle` x 2000 |
  | S1 immobiles, `Archer` (2ᵉ archétype) | **2,040 ms ✅** | 167,7 i/s (81,4) ✅ | `Stand`/`Idle` x 2000 |
  | S2 en marche, ordres (D229) | **2,158 ms ✅** | 167,0 i/s (89,7) ✅ | `Move`/`Moving` x 1991, `Stand`/`Idle` x 9 |
  | S1 immobiles, budget par pas par défaut (500/pas) | **1,935 ms ✅** | 163,5 i/s (83,1) ✅ | `Stand`/`Idle` x 2000 |
  | S1 500 unités (contrôle de destruction) | **0,542 ms ✅** | 171,5 i/s (92,9) ✅ | `0 unit left` après l'arrêt |

  Détail du pas en S1 à 2 000 unités : évitement d'arrêt 7,37 ms ramené à 1/6 (1,23 ms), transform précédent
  0,12, pilotage 0,18, orientation 0,24, ordre 0,05, déplacement 0,05, reste ≈ 0,1 ; **naissances et
  destructions 0,000 ms pendant la mesure** (elles ont lieu à l'échauffement).
- **Correction de la note S2** : la mesure précédente (2,80 ms, D220) mesurait l'errance d'essai **avec**
  suivi de navmesh ; la nouvelle (2,158 ms) mesure l'**ordre** en cible directe (D229). Les deux ne sont pas
  comparables ; aucune n'est écrasée. Le rendu, lui, s'améliore (145→167 i/s, 1 % bas 72→90) : mêmes unités,
  comportement de marche différent.
- **Commandes qui ont servi** : compilation `Build.bat HeroesOfLibertyEditor Win64 Development -project=…` ;
  création de l'asset `UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="Tools/Editor/make_unit_types.py" -unattended -nop4 -nosplash` ;
  tests `UnrealEditor.exe <uproject> /Game/Maps/StressTest -game -windowed -ResX=1920 -ResY=1080 -nosplash -ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,hol.Units.SpawnBudget -1,hol.StressTest S1|S2 2000 10 quit"`.
- **Prochaine étape** : M2 (rendu), dette de la table identifiant ↔ entité notée dans « Questions ouvertes » à
  traiter avant M5.

### 2026-10-11 (suite) — M1 : trois choix d'implémentation tranchés avant d'écrire (D227 à D229)

- **D227 — fabrication de la table des types** : CSV source `Tools/Data/UnitTypes.csv` + script d'éditeur
  `Tools/Editor/make_unit_types.py` qui importe l'asset `/Game/Units/UnitTypes`, le code lisant l'asset par
  configuration. Motif : un `.uasset` ne s'écrit pas à la main, et le CSV reste relisible en revue. Écartés :
  l'asset saisi à la main (rien de testable tant qu'il n'existe pas, valeurs dans un binaire) et les lignes
  par défaut en code (deux sources, ce que D221 écarte).
- **D228 — mouvement de l'ordre « va à cet endroit »** : cible directe (`FMassMoveTargetFragment` : centre,
  avant, vitesse du type, `IntentAtGoal = Stand`, action `Move`), arrivée détectée par nous (tolérance →
  `Stand`), destination ramenée au navmesh **à l'émission de l'ordre** (une requête de point par ordre, pas par
  unité). Écartés : la cible brute (une destination hors navmesh en fait sortir l'unité) et le chemin navmesh
  par unité (amène M3 en avance ; c'est la façon de faire d'AoE4, mais pour M3). Vérifié au passage dans le
  moteur : `MassNavMeshPathFollowProcessor` n'agit que s'il y a un chemin court (≥ 2 points), donc une cible
  directe n'est pas écrasée.
- **D229 — S2 passe aux ordres de M1** : les unités de S2 marchent par ordres réémis à l'arrêt, l'errance
  d'essai (`UHOLWanderProcessor` et son étiquette) est supprimée. Conséquence assumée : **S2 change de nature**
  (plus de suivi de navmesh, coût plus bas), sa mesure n'est plus comparable aux 2,80 ms de D220, et M3 devra
  le rendre exigeant en ajoutant les chemins. Seuil D218 inchangé.
- Tous trois consignés au GDD : journal **§ 21 (D227 à D229)**, corps du **§ 16.6**, et point de reprise du
  **§ 22**.
- **Prochaine étape** : écrire le code dans l'ordre de D226 — table et fragments, fabrique et cache
  d'archétypes, identifiants, file de naissances et destructions différées, file de commandes, puis remesure
  de S1 et S2.

### 2026-10-11 (suite) — M1, Q6 tranchée : le cœur plus la file de commandes (D226) — plan complet

- Choisi : **périmètre M1 = le cœur + la file de commandes**, avec un seul ordre (« va à cet endroit »), ce que
  D216 demande dès M1. Repoussés : sélection et interface (M4), combat et cadavres (M5), StateTree, visuels par
  type et animations (M2), scénarios S3 à S6 de D190.
- Écartés : le strict minimum sans file de commandes (reporte la file à M4, contredit D216) ; l'ajout de la
  sélection et d'une interface minimale (empiète sur M4 et sur le § 15).
- **Le plan du cœur de M1 est complet : D221 (table de données), D222 (fragments), D223 (identifiant),
  D224 (naissances/destructions), D225 (fabrique et archétypes), D226 (périmètre).** Consigné au GDD : journal
  § 21 et § 16.6 (paragraphe « Cœur de la simulation »).
- Prochaine étape : **implémenter M1** dans l'ordre du plan, en consignant chaque étape ici.
- **Fin de séance (2026-10-11)** : tout est commité — commit le plus récent de `main`, message
  `ADD : D220 follow-up (standing avoidance throttled, idle units standing, movement action report), M1 core plan D221-D226, implementation journal and agent rule`
  (local, non poussé). Reprise : lire ce fichier et repartir de « Prochaine étape exacte », point 3
  (implémenter M1).

### 2026-10-11 (suite) — M1, Q5 tranchée : une fabrique en code, un archétype par type (D225)

- Choisi : **fabrique en code** — un sous-système construit et met en cache une configuration d'entité Mass
  (donc un archétype) par type, à partir de la ligne de table (D221) ; les fragments communs (identifiant,
  transform précédent, cache des données de type) sont posés par la fabrique pour tous les types.
- Écartés : un archétype unique avec le type en champ (incompatible avec D222) ; un DataAsset de
  configuration Mass par type (un fichier par type, écarté par D221).
- Consigné au GDD : journal **D225** (§ 21) et § 16.6.
- Suite du plan : **Q6** (périmètre exact de M1) — dernière question.

### 2026-10-11 (suite) — M1, Q4 tranchée : file de naissances, disparition différée (D224)

- Choisi : naissances **par lots mis en file**, servies avec un **budget par pas fixe** (réglable ; illimité
  pour les stress tests, qui doivent pouvoir mesurer un pic) ; **disparitions différées à la fin du pas**,
  jamais pendant le parcours des processeurs.
- Écartés : création immédiate à l'appel (pic dans le pas, mesures faussées) ; tout différer au pas suivant
  (50 ms de délai visibles pour un joueur).
- Consigné au GDD : journal **D224** (§ 21) et § 16.6.
- Suite du plan : **Q5** (archétypes et types), puis Q6 périmètre exact de M1.

### 2026-10-11 (suite) — M1, Q3 tranchée : identifiant maison de 32 bits (D223)

- Constat au passage : le couple index/version de `FMassEntityHandle` **n'est pas stable** (il change quand
  l'entité change d'archétype), donc il ne peut pas servir d'identifiant d'unité. C'est le fond de la question.
- Choisi : **identifiant maison de 32 bits**, compteur croissant côté serveur, **jamais recyclé** dans une
  partie, porté par le fragment d'état, avec une **table identifiant ↔ entité** dans un sous-système.
- Écartés : le couple de Mass ; 64 bits (*inutile sans recyclage*, et le recyclage introduirait des références
  périmées à gérer) ; le recyclage des numéros.
- Consigné au GDD : journal **D223** (§ 21) et § 16.6.
- Suite du plan : **Q4** (apparition et disparition), puis Q5 archétypes par type, Q6 périmètre exact de M1.

### 2026-10-11 (suite) — M1, Q2 tranchée : données partagées par type, état par entité (D222)

- Trois options proposées (tout par entité / partagé constant par type + état par entité / fragment réduit à
  l'identifiant de type avec lecture de la table) ; l'utilisateur choisit le **fragment partagé constant par
  type** plus un **fragment par entité** pour l'état — l'idiome de Mass.
- Consigné au GDD : journal **D222** (§ 21) et § 16.6.
- Suite du plan : **Q3** (identifiant stable), puis Q4 apparition/disparition, Q5 archétypes par type,
  Q6 périmètre exact de M1.

### 2026-10-11 (suite) — M1, Q1 tranchée : les types d'unité en table de données (D221)

- Trois options proposées (table de données / un DataAsset par type / les deux) ; l'utilisateur choisit la
  **table de données** : une ligne `FTableRowBase` par type d'unité, valeurs et **références souples**
  (`SoftObjectPath`) vers maillages et animations. Rationale : un seul endroit à équilibrer, valeurs
  provisoires par nature, conforme au « valeurs provisoires dans les Data Tables » déjà écrit au § 22.
- Consigné au GDD : journal **D221** (§ 21) et § 16.6.
- Suite du plan : **Q2** (où vivent les valeurs à l'exécution, et quels champs sont statiques ou variables),
  puis Q3 identifiant stable, Q4 apparition/disparition, Q5 archétypes par type, Q6 périmètre exact de M1.
- *Note de tenue* : une insertion dans le journal du GDD a d'abord abîmé la ligne `D74` (anomalie de
  placement préexistante, la ligne traîne en fin de tableau au lieu d'être entre `D73` et `D75`) ; ligne
  restaurée à l'identique. Le mauvais placement, antérieur à cette séance, n'a **pas** été corrigé.

### 2026-10-11 (suite) — Vérification de l'hypothèse `Stand` : deux solveurs, pas un

- **Contexte** : D220 ne ralentissait que `MassMovingAvoidanceProcessor`. Restait à savoir si une unité
  immobile était traitée comme « en marche » (d'où S1 cher).
- **Vérifié dans le moteur** : `FMassMoveTargetFragment` démarre en `Move` (fichier
  `MassNavigationFragments.h`, `CurrentAction = EMassMovementAction::Move`) et `FMassMovingAvoidanceProcessor`
  saute les entités en `Animate` ou `Stand`. Nos unités de S1 n'ayant aucune cible de déplacement, elles
  étaient donc toutes en `Move`.
- **Correction apportée** : les unités de S1 naissent en `Stand` (`CreateNewAction(EMassMovementAction::Stand, World)`),
  ce qui est le sens de « 2 000 unités immobiles » ; le rapport du stress test affiche désormais la
  répartition des actions (`Move action: Stand x 2000` / `Move x 2000`), via `LogMovementActionSummary()`.
- **Mesure, et surprise** : l'évitement de marche tombe bien à **0,04 ms**, mais
  `MassStandingAvoidanceProcessor` prend le relais à **6,98 ms** — il n'était pas ralenti. S1 total : 7,8 ms ❌.
  **Une unité immobile ne coûte pas moins, elle change de solveur.** (Correction d'une note du GDD qui
  attendait un gain : c'était un transfert de coût.)
- **Corrigé** : `MassStandingAvoidanceProcessor` ajouté à `ThrottledProcessors` (même cadence, 1 pas sur 6).
- **Résultats après correction** (2 000 unités, seuil D218 = 3 ms) :

  | Scénario | Avant | Après | Action | Évitement, par exécution |
  |---|---|---|---|---|
  | S1 immobiles | 7,80 ms ❌ | **1,96 ms ✅** | `Stand` x 2000 | arrêt 6,86 ms (1/6) |
  | S2 en marche | — | **2,80 ms ✅** | `Move` x 2000 | marche 7,79 ms (1/6) |

- **À retenir pour la suite** : les deux solveurs sont mono-fil et facturés par unité (≈ 3,4 à 4 µs) ; le
  budget d'évitement est donc le même que les unités marchent ou s'arrêtent. La marge est mince (2,8 ms pour
  3 ms) : à surveiller dès que les unités auront une vitesse ou des rayons différents (M3/M5).
- **GDD mis à jour** : journal **D220** (§ 21), § 16.6, point de reprise du § 22 (hypothèse marquée vérifiée,
  correction de la note erronée, prochaine étape = plan de M1).

### 2026-10-11 — D220 : évitement ralenti et resserré, seuils tenus *(commit `7439918`, poussé)*

- Mesures : `MassMovingAvoidanceProcessor` coûtait 9,3 à 10,5 ms par pas à 2 000 unités, soit presque tout
  le pas fixe ; coût **par unité** (≈ 4 µs, linéaire, quasi indépendant de la densité et des bords du
  navmesh), mono-fil.
- Règlé par `ThrottledProcessors` / `ThrottledProcessorStepInterval` (un pas sur **six**, 3,33 Hz) dans
  `UHOLSimulationSettings` et une distance de détection resserrée à **250 cm** dans `UHOLLightUnitAvoidanceTrait`.
- Résultat : S1 11,5 → **1,9 à 2,3 ms ✅**, S2 10,9-12,2 → **2,6 à 2,8 ms ✅** (seuil 3 ms). Rendu inchangé.
- Défaut d'outillage corrigé au passage : la capture d'écran du stress test plantait le processus (assertion
  D3D12) quand la fenêtre de jeu était minimisée ; capture sautée avec un avertissement, vérifié sur trois
  scénarios.

## Commandes utiles

```powershell
# Compiler (éditeur fermé, sinon Live Coding bloque)
& "C:\UE\UE_5.8_AS_1\Engine\Build\BatchFiles\Build.bat" HeroesOfLibertyEditor Win64 Development -Project="C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject"

# Lancer un stress test : la valeur de -ExecCmds DOIT être entre guillemets,
# sinon la ligne de commande est coupée au premier espace et le test ne démarre pas.
# Ne pas minimiser la fenêtre pendant la mesure (la capture est sautée, avec un avertissement).
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" /Game/Maps/StressTest -game -windowed `
  -ResX=1920 -ResY=1080 -nosplash -ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,hol.Units.SpawnBudget -1,hol.StressTest S2 2000 10 quit"
# Rapport : Saved/Logs/HeroesOfLiberty.log, lignes LogHOLStressTest ; capture : Saved/Screenshots/WindowsEditor/

# Créer/remplacer la table des types d'unité depuis le CSV (D227), éditeur fermé
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" -run=pythonscript `
  -script="C:\Users\baudo\Documents\MyCrusader\Tools\Editor\make_unit_types.py" -unattended -nop4 -nosplash
```

Leviers de mesure (ligne de commande ou console) :

| Levier | Effet |
|---|---|
| `hol.StressTest S1\|S2\|C [Count] [Seconds] [quit]` | lance le scénario (S1 immobiles, S2 en marche par ordres D229, C croisement de deux unités D230) ; échauffement et mesure comptés **en pas** (3 s → 60 pas, 10 s → 200 pas), donc deux passages identiques donnent les mêmes recouvrements |
| `hol.StressTest.SpawnRadius` | rayon de la nuée, en cm (densité) |
| `hol.StressTest.UnitType` | ligne de table utilisée par le test (`LightInfantry`, `Archer` — D221, D225) |
| `hol.StressTest.OrderRadius` | rayon (cm) des destinations de S2 (D229) |
| `hol.StressTest.MaxOrdersPerFrame` | ordres émis par pas au plus (D229 ; le nom date de la passe par image) |
| `hol.StressTest.CrossingHalfDistance` | distance (cm) des deux unités du scénario C à l'origine (D230 ; 1 500 = 15 m) |
| `hol.StressTest.CrossingAngle` | angle (°) entre les deux trajectoires du scénario C : 0 = face-à-face (défaut), 90 = angle droit, 150 au plus |
| `hol.Units.SpawnBudget` | naissances par pas (D224) : 0 = réglage, −1 = tout en un pas (stress test) |
| `hol.Units.AvoidanceHold` | pas pendant lesquels l'impulsion d'évitement est ré-appliquée (D230) : négatif = réglage (défaut), **0 = éteint** |
| `hol.Units.AvoidanceHoldDecay` | facteur appliqué à l'impulsion retenue après chaque ré-application (D230) : négatif = réglage (défaut), 1 = aucune décroissance, 0 = une seule ré-application |
| `hol.Units.AvoidanceTrace` | identifiants d'unités séparés par `+` (ex. `1+2`, les deux unités du scénario C) : forces d'évitement écrites à chaque pas dans `Saved/AvoidanceTrace.csv` (outil de diagnostic : l'écriture coûte ~3 ms de pas) |
| `hol.Simulation.ThrottledStepInterval` | cadence des processeurs ralentis (0 = valeur de la configuration) |
| `hol.Avoidance.ObstacleDetectionDistance` | distance de détection des unités, en cm (0 = valeur de la ligne de table) |
| `hol.StressTest.ResolutionQuality` | échelle de rendu (`sg.ResolutionQuality`, %) imposée par un passage au-dessus des réglages enregistrés : 87 par défaut, la valeur de toutes les mesures d'avant le menu ; négatif = garder le réglage enregistré (D245) |

**Du chantier menu (2026-10-13, `FrontEndInProgress.md`)**, à connaître pour M2 :
- **Garde-fou D245** : l'écran Réglages enregistre maintenant échelle de rendu, synchro, limite d'i/s et qualité dans
  `GameUserSettings.ini` (section `[/Script/HeroesOfLiberty.HOLGameUserSettings]`, plus `[ScalabilityGroups]`). Un
  passage impose l'échelle de rendu (`hol.StressTest.ResolutionQuality`) ; la ligne de commande habituelle impose déjà
  synchro, limite d'i/s et `scalability 2`. Le rapport a une ligne `Display (D245)` (échelle, synchro, limite, viewport,
  niveau de chaque groupe) : deux rapports se comparent seulement si ces lignes sont identiques.
- **`OwnerPlayer` (D233)** : quand une unité naîtra pour un joueur, son `OwnerPlayer` est
  `AHOLPlayerState::GetPlayerIndex()`, l'emplacement que le mode de jeu a donné au joueur dans la configuration de partie
  (D243, D244) ; la palette commune est `Match/HOLPlayerColors` (index de couleur dans `GetColorIndex()`).
- **Lancements visibles** (stress test compris) : ajouter `-saveddirsuffix=Tests` et `-WinX=3840 -WinY=0` (1920 x 1080)
  ou `-WinX=3940 -WinY=100` (1600 x 900) : fenêtre sur le deuxième écran de l'utilisateur, et dossier `Saved_Tests/`
  qui ne touche pas ses réglages (journal et captures y vont aussi). Sans le dossier de test, le moteur recentre la
  fenêtre sur l'écran principal ~15 s après le démarrage. Détail dans `FrontEndInProgress.md` (« Commandes utiles »).

**Ordre d'avancement après M2 (D252, 2026-10-13, `JourneyInProgress.md`)** : le plan suit désormais le parcours du
joueur, **J1** emplacement de départ et caméra RTS, **J2** centre principal par joueur et sélection, puis **M2.5**
réseau, **J3** production d'un Paysan, **J4** ordres de déplacement et chemins (M3), **J5** récolte… Le plan de J1 est
en cours dans une autre session : il remplacera le pion par défaut de la carte de bataille par une caméra RTS ; les vues
de mesure du stress test (D236, D237) seront gardées, et vérifiées sur S1 et S2.

**D258 (2026-10-13, demande de l'utilisateur) : aucune ombre dans le jeu**, pour éviter tout problème de performances.
Consigné au GDD (§ 16.6, § 21). **Pas encore appliqué** au moteur : quand il le sera (réglage global), les mesures S1 à
S6 faites avec ombres ne se compareront plus ; une nouvelle référence sera à prendre.
**Appliqué le 2026-10-13** (choix de l'utilisateur : tout de suite, partout) : `[SystemSettings]` de `DefaultEngine.ini`
(`r.ShadowQuality=0`, `r.Shadow.Virtual.Enable=0`, `r.DistanceFieldShadowing=0`, `r.CapsuleShadows=0`,
`r.ContactShadows=0`), au-dessus des niveaux de qualité. Le rapport du stress test écrit encore `Shadow 2` dans la ligne
`Display (D245)` : c'est le niveau du groupe, sans effet désormais ; à remplacer par `r.ShadowQuality` si utile.
Premiers passages courts sans ombres (machine chargée par la compilation du moteur 5.8.3, chiffres non significatifs) :
S1 3,53 ms, 110 i/s, GPU 3,98 ms, fil de jeu 8,93 ms ; S2 3,38 ms, 119 i/s, GPU 3,92 ms, fil de jeu 7,90 ms ; complets,
0 unité restante.
