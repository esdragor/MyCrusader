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
| M1 — cœur de la simulation (`UnitData`, identifiants, apparition/disparition) | **en cours** : plan à valider |
| M2 — rendu (instances par type et par joueur, animations, interpolation) | à faire |
| M2.5 — tranche réseau Iris (D216) | à faire |

## Prochaine étape exacte

1. ~~Vérifier l'**hypothèse `Stand`** et la consigner ici.~~ → **fait (2026-10-11)**, voir le journal.
2. ~~Valider le **plan de M1** question par question.~~ → **fait** : D221 à D226, plan complet ci-dessus.
3. **Implémenter M1**, dans cet ordre, en consignant chaque étape dans le journal :
   1. table de données `UnitData` + fragments (partagé par type, état par entité) ;
   2. fabrique de types et cache d'archétypes (D225) ;
   3. identifiant stable et table identifiant ↔ entité (D223) ;
   4. file de naissances (budget par pas) et destructions différées (D224) ;
   5. file de commandes et ordre « va à cet endroit » (D226) ;
   6. remettre S1 et S2 en non-régression (seuils D218) et étendre le stress test au besoin.
4. Proposer avant d'écrire tout choix d'implémentation non trivial qui n'est pas déjà couvert par D221 à D226.

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

## Questions ouvertes

- **Marge de rendu** : 146 à 169 i/s pour un seuil de 144, 1 % bas entre 50 et 87 i/s. Passe, mais mince et
  bruité ; à surveiller quand le contenu s'ajoute (animations, effets, bâtiments).
- **Réserve si un seuil casse** (mêlées de M5, unités plus rapides ou plus nombreuses) : paralléliser
  l'évitement de Mass (option B1) ou l'écrire dans la couche RTS (option A). Non nécessaires aujourd'hui.
- ~~Hypothèse `Stand`~~ → **résolue (2026-10-11)** : voir le journal. Ce n'était pas un gain mais un transfert
  de coût ; les deux solveurs d'évitement sont désormais ralentis (D220).

## Journal d'avancement

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
  -ResX=1920 -ResY=1080 -nosplash -ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,mc.StressTest S2 2000 10 quit"
# Rapport : Saved/Logs/MyCrusader.log, lignes LogMCStressTest ; capture : Saved/Screenshots/WindowsEditor/
```

Leviers de mesure (ligne de commande ou console) :

| Levier | Effet |
|---|---|
| `mc.StressTest S1\|S2 [Count] [Seconds] [quit]` | lance le scénario (S1 immobiles, S2 en marche) |
| `mc.StressTest.SpawnRadius` | rayon de la nuée, en cm (densité) |
| `mc.Simulation.ThrottledStepInterval` | cadence des processeurs ralentis (0 = valeur de la configuration) |
| `mc.Avoidance.ObstacleDetectionDistance` | distance de détection des unités, en cm (0 = valeur du trait) |
