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
| M1 — cœur de la simulation (`UnitData`, identifiants, apparition/disparition, file de commandes) | **fait** (2026-10-11) : D221 à D229 implémentés ; **revue traitée le 2026-10-12** (deux bugs corrigés, D230) ; S1 2,25/2,10 ms, S2 2,21/2,25 ms ✅, croisement 2,7 cm (limitation connue) |
| M2 — rendu (instances par type et par joueur, animations, interpolation) | à faire |
| M2.5 — tranche réseau Iris (D216) | à faire |

## Prochaine étape exacte

1. ~~Table de données, fabrique, identifiants, naissances/destructions, file de commandes~~ → **fait (2026-10-11)**.
2. ~~S1 et S2 en non-régression~~ → **fait** : S1 2,25 et 2,10 ms, S2 2,21 et 2,25 ms, rendu ≥ 144 i/s.
3. ~~Revue de l'utilisateur : deux bugs, dette inexistante, axes d'amélioration~~ → **fait (2026-10-12)** : bugs corrigés et
   mesurés, journée d'évitement instruite (**D230**), journaux dédoublonnés, graine fixée, test de croisement ajouté.
4. **M2 — rendu**, au-dessus du cœur de M1 : instances par type et par joueur, animations cuites en textures
   (`AnimToTexture`), interpolation déjà en place (`UMCInterpolatedISMProcessor`) ; garder S1 et S2 au vert, et
   une vérification à 2 clients dès M2.5 (D216).
5. **Avant M5, deux points ouverts à instruire** (voir « Questions ouvertes ») : le **croisement frontal** (comportement
   non expliqué, test à durcir) et la **vitesse d'arrêt nulle** qui empêche la séparation de résorber les recouvrements
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
| D230 | **Cadence de l'évitement** (revue du 2026-10-12). | **tranché** : cadence de D220 conservée **et** impulsion d'évitement retenue 5 pas, sans décroissance ; croisement frontal consigné comme limitation **non expliquée** ; graine fixée |

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

**Cœur de M1 implémenté (2026-10-11).** Fichiers créés : `Simulation/MCUnitTypes.h`, `Simulation/MCUnitTrait.*`,
`Simulation/MCUnitMovementTrait.h`, `Simulation/MCUnitSubsystem.*`, `Commands/MCUnitCommand.h`,
`Commands/MCCommandSubsystem.*`, `Commands/MCUnitOrderProcessor.*`, `Tools/Data/UnitTypes.csv`,
`Tools/Editor/make_unit_types.py`. Fichiers supprimés : `Simulation/MCLightUnitTrait.*`,
`UMCWanderProcessor` (+ `FMCWanderTag`), dépendance `NavCorridor`.

## Questions ouvertes

- **Croisement frontal de deux unités — comportement non expliqué (2026-10-12)** : avec l'évitement à **chaque
  pas**, deux unités ordonnées à travers le même point se frôlent à 66,5 cm (somme des rayons 80 cm) ; à 1 pas
  sur 2, 3 ou 6, l'écart tombe à 1,3 / 3,0 / 4,7 cm — **il ne suit donc pas la fréquence** — et la force retenue
  le dégrade (2,4 à 7,4 cm). L'explication par le nombre d'impulsions est fausse. À instruire **avant M5** :
  durcir le test (plusieurs décalages de phase, angles, graines — le test actuel est un croisement unique) et
  regarder ce qui se passe sur les pas **sans** solveur (force, vitesse désirée, pilotage).
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

## Journal d'avancement

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
  toute la marche. Corrigé dans `MCUnitOrderProcessor::StopArrivedUnits` : distance et direction vers la cible
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
  | 1 sur 6 + **impulsion retenue 5 pas**, sans décroissance | 2,4 cm ❌ | 2,438 ms ✅ | **101 → 58** |
  | 1 sur 6 + impulsion retenue, décroissance 0,6 | 7,4 cm ❌ | 2,173 ms ✅ | 126 → 92 |
  | 1 sur 6 + impulsion retenue, décroissance 0,3 | 4,1 cm ❌ | (non mesuré) | — |

  Retenu : **cadence de D220 conservée + impulsion retenue 5 pas, sans décroissance** (`AvoidanceHoldSteps = 5`,
  `AvoidanceHoldDecay = 1`) — seuil tenu et foule qui se sépare (137 → 58 recouvrements) pour +0,2 ms de pas.
  Implémentation : `FMCAvoidanceHoldFragment` + deux processeurs nôtres encadrant les solveurs
  (`MCAvoidanceSnapshotProcessor` après le pilotage, `MCAvoidanceHoldProcessor` avant l'application des forces) ;
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
  la nuée (`MCUnitSubsystem::SpawnRandom`) et sur les destinations de S2 (`OrderRandom`). Deux passages identiques
  le confirment : S1 215 → 215 recouvrements deux fois, S2 100 → 87 deux fois, 2 352 ordres, 19,3 m parcourus.
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
  *Écart à surveiller* : les passages de la veille donnaient 145 à 170 i/s pour la même scène (scène vide : 284 i/s) —
  différence inexpliquée, probablement l'état de la fenêtre de jeu ; consigné en question ouverte, aucune conclusion
  tirée sur la marge de rendu.

### 2026-10-11 (suite) — M1 : cœur de la simulation implémenté, mesuré et vérifié

- **Implémenté dans l'ordre de D226.** Table de données : `FMCUnitTypeRow` (`FTableRowBase`) et cache partagé
  `FMCUnitTypeConstants` (`FMassConstSharedFragment`), plus `FMCUnitStateFragment` (identifiant, points de vie
  courants, état marche/arrêt) — D221, D222, D226. Fabrique : `UMCUnitSubsystem` construit et met en cache une
  `FMassEntityConfig` **par type** à partir de la ligne de table, les fragments communs étant posés par
  `UMCUnitTrait` (identifiant d'état, transform précédent, cache du type) et les valeurs du type par
  `UMCUnitMovementTrait` (vitesse) et `UMCLightUnitAvoidanceTrait` (distance d'évitement, sortie du trait pour
  entrer dans la table, D220 → D221) — D225. Identifiants : compteur 32 bits jamais recyclé, table
  identifiant ↔ entité **et** l'inverse — D223. Naissances et destructions : file avec budget par pas, et
  commandes de destruction vidées à la queue du pas — D224. Ordres : `Commands/MCCommandSubsystem`
  (file + destination ramenée au navmesh par une requête de point **par ordre**) et
  `Commands/MCUnitOrderProcessor` (pose du `FMassMoveTargetFragment`, action `Move`, arrivée sous tolérance →
  `Stand` + état `Idle`) — D226, D228. L'errance d'essai et `UMCLightUnitTrait` sont **supprimés** (D229).
- **Écart d'implémentation mesuré, corrigé : naissances et destructions ne peuvent pas être des processeurs.**
  Première tentative avec deux processeurs (naissance à la tête, destruction à la queue) : **crash** au premier
  pas — `Assertion failed: IsProcessing() == false` dans `FMassEntityManager::BatchCreateEntities` (« Use
  asynchronous API instead »). Vérifié dans le moteur : `MassExecutor` ouvre une portée de traitement autour de
  **chaque** processeur, et `FlushCommands` refuse lui aussi de tourner pendant ce temps. Les deux opérations
  sont donc portées par le **pas**, dans `UMCSimulationSubsystem` (tête et queue, hors des processeurs), et
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
- **Budget par pas vérifié (D224)** : sans `mc.Units.SpawnBudget -1`, les 2 000 unités demandées sont servies
  **500 par pas** (valeur des réglages) et arrivent toutes (rapport : `2000 units of 2000 requested, 0 births
  waiting`) — la file est bien servie partiellement sur plusieurs pas, pas seulement en mode « tout d'un coup ».
- **Mesures (2026-10-11, éditeur en `-game` non cuit, 2 000 unités, `mc.Units.SpawnBudget -1`)** — seuils
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
- **Commandes qui ont servi** : compilation `Build.bat MyCrusaderEditor Win64 Development -project=…` ;
  création de l'asset `UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="Tools/Editor/make_unit_types.py" -unattended -nop4 -nosplash` ;
  tests `UnrealEditor.exe <uproject> /Game/Maps/StressTest -game -windowed -ResX=1920 -ResY=1080 -nosplash -ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,mc.Units.SpawnBudget -1,mc.StressTest S1|S2 2000 10 quit"`.
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
  d'essai (`UMCWanderProcessor` et son étiquette) est supprimée. Conséquence assumée : **S2 change de nature**
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
  `UMCSimulationSettings` et une distance de détection resserrée à **250 cm** dans `UMCLightUnitAvoidanceTrait`.
- Résultat : S1 11,5 → **1,9 à 2,3 ms ✅**, S2 10,9-12,2 → **2,6 à 2,8 ms ✅** (seuil 3 ms). Rendu inchangé.
- Défaut d'outillage corrigé au passage : la capture d'écran du stress test plantait le processus (assertion
  D3D12) quand la fenêtre de jeu était minimisée ; capture sautée avec un avertissement, vérifié sur trois
  scénarios.

## Commandes utiles

```powershell
# Compiler (éditeur fermé, sinon Live Coding bloque)
& "C:\UE\UE_5.8_AS_1\Engine\Build\BatchFiles\Build.bat" MyCrusaderEditor Win64 Development -Project="C:\Users\baudo\Documents\MyCrusader\MyCrusader.uproject"

# Lancer un stress test : la valeur de -ExecCmds DOIT être entre guillemets,
# sinon la ligne de commande est coupée au premier espace et le test ne démarre pas.
# Ne pas minimiser la fenêtre pendant la mesure (la capture est sautée, avec un avertissement).
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\Users\baudo\Documents\MyCrusader\MyCrusader.uproject" /Game/Maps/StressTest -game -windowed `
  -ResX=1920 -ResY=1080 -nosplash -ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,mc.Units.SpawnBudget -1,mc.StressTest S2 2000 10 quit"
# Rapport : Saved/Logs/MyCrusader.log, lignes LogMCStressTest ; capture : Saved/Screenshots/WindowsEditor/

# Créer/remplacer la table des types d'unité depuis le CSV (D227), éditeur fermé
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "C:\Users\baudo\Documents\MyCrusader\MyCrusader.uproject" -run=pythonscript `
  -script="C:\Users\baudo\Documents\MyCrusader\Tools\Editor\make_unit_types.py" -unattended -nop4 -nosplash
```

Leviers de mesure (ligne de commande ou console) :

| Levier | Effet |
|---|---|
| `mc.StressTest S1\|S2\|C [Count] [Seconds] [quit]` | lance le scénario (S1 immobiles, S2 en marche par ordres D229, C croisement de deux unités D230) |
| `mc.StressTest.SpawnRadius` | rayon de la nuée, en cm (densité) |
| `mc.StressTest.UnitType` | ligne de table utilisée par le test (`LightInfantry`, `Archer` — D221, D225) |
| `mc.StressTest.OrderRadius` | rayon (cm) des destinations de S2 (D229) |
| `mc.StressTest.MaxOrdersPerFrame` | ordres émis par pas au plus (D229) |
| `mc.StressTest.CrossingHalfDistance` | distance (cm) des deux unités du scénario C à l'origine (D230) |
| `mc.Units.SpawnBudget` | naissances par pas (D224) : 0 = réglage, −1 = tout en un pas (stress test) |
| `mc.Units.AvoidanceHold` | pas pendant lesquels l'impulsion d'évitement est ré-appliquée (D230) : 0 = réglage, négatif = éteint |
| `mc.Units.AvoidanceHoldDecay` | décroissance par pas de l'impulsion retenue (D230) : 0 = réglage, 1 = aucune |
| `mc.Simulation.ThrottledStepInterval` | cadence des processeurs ralentis (0 = valeur de la configuration) |
| `mc.Avoidance.ObstacleDetectionDistance` | distance de détection des unités, en cm (0 = valeur de la ligne de table) |
