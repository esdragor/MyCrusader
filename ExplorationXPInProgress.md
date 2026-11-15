# XP d'exploration (GDD § 9.4, D05, D06) — chantier B2

Branche `visual`, worktree Sauron. Ouvert le 2026-11-15 10:2x par SAURON-1115b. Sans C++ (règle pure Angelscript + test).

## Ce que le GDD fixe

- § 9.4 : source **Exploration** = « découverte de zones, de lieux importants, d'événements ».
- D05 : Exploration + Événements ~15 % de l'XP d'une partie (niveau 6 = 3 300 XP au prototype, `HOLHeroXPRules.h` : ~500 XP pour les deux).
- D06 : l'exploration **par les unités** continue de rapporter pendant la récupération du héros (seuls combat et commandement s'arrêtent).
- § 8.4 : neutres = XP modérée dans la même part.

## Ce qui existe

- `Source\HeroesOfLiberty\Simulation\HOLHeroXPRules.h` : `EHOLXPSource::Exploration` « no carrier yet », poids 25 %, `CountsWhileDead` vrai pour l'exploration.
- Brouillard F0-F9 dans main (`FogInProgress.md` § 7) : deux états par case (4 m) et par camp, **explorée** (jamais perdue) et en vue ; le serveur garde l'exploré par camp (D520).

## Question

`C:\Users\baudo\Documents\MyCrusader\Retours\Q-exploration-xp.md` [À RÉPONDRE] : porteur (surface par tranche / lieux / les deux) et valeur. Défaut codé : **C** (les deux), `HOLExplorationXP::Preset(0)` ; A = `Preset(1)`, B = `Preset(2)` (une ligne à changer).

## Plan (étapes de 15 min)

1. [ ] Règle pure `Script\Heroes\HOLExplorationXPRules.as` en bouchons (renvoient 0) + `HOLExplorationXPRules_Test.as`. **Premier test rouge** : « une tranche neuve paie une seule fois, aussi pendant la récupération du héros (D06) » (`Test_ExplorationXPNewTranchePaysOnce`), vu rouge à l'exécution, commit du test.
2. [ ] Vert : tranche = carré de `CellsPerTranche` cases du brouillard ; la 1re case explorée d'une tranche paie `XPPerTranche` une fois ; un lieu (relais, camp, autel) paie `XPPerPlace` une fois quand sa case est explorée ; l'exploré du départ (vue initiale, mode « Carte explorée ») ne paie rien ; hors grille rien. `quick_check -Parts tests` vert, commit.
3. [ ] (C++, agent suivant, worktree Faramir/fog) branchement : lecture du brouillard, voir ci-dessous.

## Ce qu'il faudra lire du brouillard (C++, plus tard)

- La **transition par case et par camp « inexplorée -> explorée »** (pas l'état « en vue ») au pas de mise à jour du brouillard, côté **serveur** (l'exploré par camp vit sur le serveur, D520) : pour chaque case nouvellement explorée, appeler `HOLExplorationXP::OnCellExplored(Settings, State, Places, X, Y, bHeroDead)` avec l'état du camp, et ajouter l'XP rendue au héros du camp par la source `Exploration`.
- **Au départ** (vue initiale des bases, ou tout en mode « Carte explorée », D389/D522) : `MarkExploredAtStart` pour chaque case explorée avant la 1re seconde, sans XP.
- La **taille de la grille** (cases par côté, 110 à 220 selon la taille de carte, D490) pour `NewState`, et la **case** de chaque relais, camp neutre, autel pour la liste des lieux.
- Le poids de 25 % de `HOLHeroXP::GetWeightPercent(Exploration)` : la valeur de la règle est l'XP finale (pas repassée par le poids) ; à confirmer au branchement.
- Mode `Disabled` (tests, tout visible) : rien n'est « nouvellement exploré », donc aucune XP ; à vérifier au branchement.

## Suivi

- 2026-11-15 10:2x : chantier ouvert, journal + question écrits (SAURON-1115b).

## Appels (un trait par appel)

|||||||||||||||||||
