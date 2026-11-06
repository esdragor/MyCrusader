# PerfSpikeInProgress — lisser la pointe du pas à 6 et mettre le Soin en veille (ouvert le 2026-11-06)

État : **préparé, rien codé, rien commité.** Aucun worktree réservé : l'utilisateur donne le nom (règle « nouveau worktree : demander »).
Reprise = « continue PerfSpikeInProgress ».

## D'où ça vient

Mesures au repos du 2026-11-06 (`Saved_Tests/Logs/rest_measures_1106_0838.txt`, `main` 4a8e6e1) comparées à celles du 2026-11-01 21:11
(journaux `chapter_rest_<tag>_main_<s>_<n>.log`, lignes `Step breakdown`) :

- Presque tous les processeurs sont 10 à 30 % plus lents, **y compris ceux du moteur** (`MassApplyForceProcessor` +22 à +33 %,
  `MassApplyMovementProcessor` +28 à +34 %) : facteur machine (fréquence, température, `IMF` ouvert), pas une régression du code.
- Deux vrais coûts de code :
  1. **`HOLHealProcessor`** (nouveau, Moine) : 0,10 à 0,12 ms par pas dans **tous** les scénarios, même S1 où personne ne soigne.
  2. **La pointe du pas à 6** : en S3 le pas moyen est de 6,80 ms mais le **pas le plus lourd de 9,16 ms** (`max 9.163 ms` dans le
     journal). À 144 images par seconde une image dure 6,94 ms : ce pas-là dépasse l'image.
- Le serveur dédié n'est pas concerné (50 ms de budget par pas à 20 Hz, D217) ; le problème est le solo, où simulation et dessin
  partagent le processus.

## Partie 1 — Soin en veille (la plus simple, à faire en premier)

Constat dans `Simulation/HOLHealProcessor.cpp` (`Execute`, l. 71-139) : le `bAnyHealer` n'est connu qu'**après** la boucle par unité
qui construit le tableau `Step` de toutes les unités légères (camp, vie max avec le pourcentage de technologie, position...). Sans
Moine, tout ça est jeté (`return` l. 134-139).

Plan :
- **Pré-passe au niveau des chunks** : `HealRate` et `HealRange` sont dans le fragment partagé `FHOLUnitTypeConstants`, donc le même
  pour tout un chunk ; une première boucle `ForEachEntityChunk` qui ne lit que `Chunk.GetConstSharedFragment` (aucune boucle par
  entité) dit s'il existe un soigneur. Sinon : même sortie qu'aujourd'hui (`Kept.Reset()`, `Orders.Reset()`, `Links.Reset()`).
- Comportement inchangé avec un Moine (mêmes soins, mêmes liens, même ordre) : tests existants `HOLMonkTests.cpp` et
  `HOLHealRules` doivent rester verts sans changement.
- **Test rouge d'abord** (règle permanente) : une règle pure `HOLHeal::...` ou un compteur lu par un test (`Heal: candidates built N`
  dans `hol.Heal.Report` si ça existe, sinon à ajouter) qui dit « sans soigneur, 0 unité construite » ; rouge aujourd'hui (N = nombre
  d'unités), vert après. Et un test « avec un Moine, le tableau est construit et le soin a lieu ».
- Mesure : `Step breakdown: HOLHealProcessor` doit passer de ~0,10 ms à < 0,01 ms en S1, S2, S7.

## Partie 2 — Lisser la pointe du pas à 6 — **EN RÉSERVE (2026-11-06, décision de l'utilisateur : risque trop haut pour le gain)**

Ne pas coder tant que la pointe n'est pas confirmée comme gênante en jeu (saccade vue en solo dans une grande bataille) et que
la cause (évitement en mouvement) n'est pas mesurée pas par pas. Seule la partie 1 (Soin) reste à faire.

Mécanique actuelle (lue dans le code) :
- `Config/DefaultGame.ini` l. 14-15 : `MassMovingAvoidanceProcessor` et `MassStandingAvoidanceProcessor` sont « throttled » (D220).
- `HOLSimulationSubsystem.cpp` l. 349-350 : `bThrottledProcessorsRunThisStep = (StepIndex % ThrottledInterval) == 0` ; **tous** les
  processeurs throttled tournent le **même** pas. En S3 `HOLParallelMovingAvoidanceProcessor` coûte ~1,2 ms par exécution (donc
  ~0,2 ms lissé), `HOLParallelStandingAvoidanceProcessor` ~0,22 ms. Le pas où ils tournent est le plus lourd.
- `HOLAvoidanceHoldProcessor` (`Simulation/HOLAvoidanceHoldProcessors.cpp` l. 158) lit `AreThrottledProcessorsRunningThisStep()` pour
  savoir si l'impulsion d'évitement est fraîche ou à tenir (D230) : **tout découpage par unité doit lui dire, par unité, si le solveur
  a tourné** (aujourd'hui un seul booléen global).

Pistes (une à trancher, voir questions) :
- **L1 — Décaler les phases** : Moving au pas `% 6 == 0`, Standing au pas `% 6 == 3`. Très simple (un décalage par processeur dans
  `RunStep`), `AvoidanceHold` doit lire un booléen par processeur. Gain : la pointe ne porte plus que Moving (~1,2 ms) au lieu de
  Moving + Standing (~1,4 ms) : **petit** (Standing est faible).
- **L2 — Découper Moving en 6 tranches** : à chaque pas, 1/6 des chunks (`ExecuteChunksInParallel` dans
  `HOLParallelMassProcessors.cpp` l. 30-60 sait déjà collecter la liste des chunks d'une exécution avant de la lancer en parallèle :
  on n'en lance que la tranche du pas). Chaque unité est évitée une fois tous les 6 pas comme aujourd'hui, mais le coût est de
  ~0,2 ms à chaque pas au lieu de 1,2 ms tous les 6. Gain : la pointe tombe de ~+1 ms à ~0. **Risques** : (a) la tranche d'un chunk
  doit être déterministe (clé = premier `UnitId` du chunk, pas l'index du chunk) ; (b) un chunk qui change de composition peut être
  sauté ou doublé une fois (au pire 11 pas sans évitement pour une unité) ; (c) `AvoidanceHold` par unité (voir plus haut) ;
  (d) le déterminisme J7c.4 (mêmes résultats d'une exécution à l'autre) à re-prouver avec `hol.Simulation.ParallelStandingAvoidance`
  et `DeterministicSteering` en A/B.
- **L3 — Intervalle plus court (6 → 3)** : plus de travail total, pointes plus petites ; contraire à D220 (mesuré pour tenir S2).
  Écarté sauf si L2 échoue.

Recommandation : **L2**, avec L1 en repli si (c) ou (d) coûtent trop. Les deux se testent par A/B (`stress_ab.ps1`) sur les mêmes
scénarios, via une variable de console (`hol.Simulation.AvoidanceSlices` 1 = comportement actuel, 6 = tranches).

## Mesure (le point sensible)

- Le chiffre qui compte est le **pas le plus lourd** (`max ... ms` de la ligne `Simulation:` du journal) et pas seulement la
  moyenne : le script `chapter_measures.ps1` ne lit que la moyenne. À ajouter à la sortie (lecture seule) avant de coder, pour avoir
  le « avant » : S3 max 9,16 ms le 2026-11-06 (à reprendre sur 3 passes, le bruit de la machine est de ±0,5 ms).
- Toujours en **A/B alterné le même jour** (`stress_ab.ps1`, ≥ 3 passes de chaque côté), jamais contre une mesure d'un autre jour :
  le 2026-11-06 le facteur machine à lui seul valait +10 à +30 %.
- Critère de réussite proposé : S3 et S4, pas le plus lourd − moyenne < 1 ms (aujourd'hui ~2,4 ms) ; moyenne inchangée à ± bruit ;
  S2 inchangé ; `quick_check -Parts` des parties touchées (simulation, avoidance, formation, combat) vert ; `-Full` à la fusion.

## Questions à trancher avant de coder (réponses de l'utilisateur, D numérotés au GDD ensuite)

- **PS-Q1** Seuil : garder « moyenne ≤ 3 / 6,5 ms » ou ajouter « pas le plus lourd ≤ 1 image à 144 Hz (6,9 ms) » comme critère
  de S3 et S4 ? (Aujourd'hui S3 : moyenne 6,8, max 9,2 : le deuxième critère serait plus dur que le premier.)
- **PS-Q2** Piste : L2 (tranches, recommandée) ou L1 (phases décalées, simple, gain faible) ?
- **PS-Q3** Si L2 : un chunk sauté ou doublé une fois à chaque changement de composition est-il acceptable (évitement légèrement
  décalé d'une unité pendant quelques pas) ?
- **PS-Q4** **Tranchée le 2026-11-06 : ne rien créer, prendre un worktree libre** (IN_USE.txt : Arwen, Celeborn ou Faramir sont marqués
  « Libre » ; proposition Arwen, à réserver dans IN_USE.txt seulement au début du travail) ; la machine reste partagée avec les autres sessions pour les compilations
  (`Tools\build.ps1`), et les mesures A/B demandent la machine seule.

## Étapes (boucles de 15 min)

1. [ ] Lire la sortie `max` dans `chapter_measures.ps1` (lecture seule) + « avant » : S3 et S4, 3 passes, machine seule.
2. [ ] Partie 1 : test rouge « sans soigneur 0 unité construite », pré-passe de chunks, tests Moine verts, `-Parts` soin/combat.
3. [ ] Partie 2 : test rouge (règle pure de tranche : chaque unité tombe dans une tranche, une fois par cycle, déterministe), variable
       de console, `AvoidanceHold` par unité, A/B alterné, J7c.4 déterminisme.
4. [ ] Mesures « après » en A/B le même jour, résumé dans `Docs/Mesures_<date>.md`.
5. [ ] Clôture : GDD (D numérotés), `-Full`, essai de l'utilisateur sur S3 en solo (scénario prêt, règle « essai = scénario prêt »), fusion.

## Suivi

- 2026-11-06 : dossier préparé (lecture du code et des journaux `Step breakdown` seulement). Rien codé.
- 2026-11-06 (validation) : l'utilisateur a validé sur le scénario `monk` (Arwen) ; **commité 0063901** sur `heal-sleep`, puis **fusionné dans main** en avance rapide (2026-11-06, rien poussé) ; Arwen libre. Mesure en ms toujours à faire (passage `rest_morning.cmd` ou A/B machine calme).
- 2026-11-06 (suite) : **Partie 1 faite**, worktree Arwen, branche `heal-sleep` (= main 4a8e6e1).
  `UHOLHealProcessor::GetLastCandidateCount()` ajouté ; pré-passe `ForEachEntityChunk` (fragment partagé `FHOLUnitTypeConstants`,
  `HealRate > 0 && HealRange > 0`, chunk non vide) avant la construction du tableau ; sans soigneur : `Kept.Reset()`,
  `Orders.Reset()`, retour. Test `HeroesOfLiberty.Monk.HealSleep` (`Tests/HOLHealSleepTests.cpp`) : 3 Lanciers sans Moine -> 0 unité
  rassemblée et 0 lien ; un Moine né près d'un Lancier blessé -> 4 unités rassemblées et le Lancier est soigné ; le Moine retiré ->
  0 de nouveau. **Rouge d'abord** (3 au lieu de 0, deux fois, sur l'ancien code), puis vert. `quick_check -Parts tests` : 1039/1039.
  **Pas fait** : la mesure en ms (`Step breakdown: HOLHealProcessor` ~0,10 -> ~0 attendu) : `chapter_measures.ps1` a refusé (BUSY,
  deux éditeurs de Haldir ouverts) ; à refaire machine calme (avant / après sur S1, S2, S7) ; `-Parts net` (soin à 2 clients) non
  lancé non plus (même raison) ; non commité tant que l'utilisateur n'a pas validé.
