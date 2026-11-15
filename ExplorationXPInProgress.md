# XP d'exploration (GDD Â§ 9.4, D05, D06) â€” chantier B2

Branche `visual`, worktree Sauron. Ouvert le 2026-11-15 10:2x par SAURON-1115b. Sans C++ (rÃ¨gle pure Angelscript + test).

## Ce que le GDD fixe

- Â§ 9.4 : source **Exploration** = Â« dÃ©couverte de zones, de lieux importants, d'Ã©vÃ©nements Â».
- D05 : Exploration + Ã‰vÃ©nements ~15 % de l'XP d'une partie (niveau 6 = 3 300 XP au prototype, `HOLHeroXPRules.h` : ~500 XP pour les deux).
- D06 : l'exploration **par les unitÃ©s** continue de rapporter pendant la rÃ©cupÃ©ration du hÃ©ros (seuls combat et commandement s'arrÃªtent).
- Â§ 8.4 : neutres = XP modÃ©rÃ©e dans la mÃªme part.

## Ce qui existe

- `Source\HeroesOfLiberty\Simulation\HOLHeroXPRules.h` : `EHOLXPSource::Exploration` Â« no carrier yet Â», poids 25 %, `CountsWhileDead` vrai pour l'exploration.
- Brouillard F0-F9 dans main (`FogInProgress.md` Â§ 7) : deux Ã©tats par case (4 m) et par camp, **explorÃ©e** (jamais perdue) et en vue ; le serveur garde l'explorÃ© par camp (D520).

## Question

`C:\Users\baudo\Documents\MyCrusader\Retours\Q-exploration-xp.md` [Ã€ RÃ‰PONDRE] : porteur (surface par tranche / lieux / les deux) et valeur. DÃ©faut codÃ© : **C** (les deux), `HOLExplorationXP::Preset(0)` ; A = `Preset(1)`, B = `Preset(2)` (une ligne Ã  changer).

## Plan (Ã©tapes de 15 min)

1. [x] RÃ¨gle pure `Script\Heroes\HOLExplorationXPRules.as` en bouchons (renvoient 0) + `HOLExplorationXPRules_Test.as`. **Premier test rouge** : Â« une tranche neuve paie une seule fois, aussi pendant la rÃ©cupÃ©ration du hÃ©ros (D06) Â» (`Test_ExplorationXPNewTranchePaysOnce`), vu rouge Ã  l'exÃ©cution, commit du test.
2. [x] Vert : tranche = carrÃ© de `CellsPerTranche` cases du brouillard ; la 1re case explorÃ©e d'une tranche paie `XPPerTranche` une fois ; un lieu (relais, camp, autel) paie `XPPerPlace` une fois quand sa case est explorÃ©e ; l'explorÃ© du dÃ©part (vue initiale, mode Â« Carte explorÃ©e Â») ne paie rien ; hors grille rien. `quick_check -Parts tests` vert, commit.
3. [ ] (C++, agent suivant, worktree Faramir/fog) branchement : lecture du brouillard, voir ci-dessous.

## Ce qu'il faudra lire du brouillard (C++, plus tard)

- La **transition par case et par camp Â« inexplorÃ©e -> explorÃ©e Â»** (pas l'Ã©tat Â« en vue Â») au pas de mise Ã  jour du brouillard, cÃ´tÃ© **serveur** (l'explorÃ© par camp vit sur le serveur, D520) : pour chaque case nouvellement explorÃ©e, appeler `HOLExplorationXP::OnCellExplored(Settings, State, Places, X, Y, bHeroDead)` avec l'Ã©tat du camp, et ajouter l'XP rendue au hÃ©ros du camp par la source `Exploration`.
- **Au dÃ©part** (vue initiale des bases, ou tout en mode Â« Carte explorÃ©e Â», D389/D522) : `MarkExploredAtStart` pour chaque case explorÃ©e avant la 1re seconde, sans XP.
- La **taille de la grille** (cases par cÃ´tÃ©, 110 Ã  220 selon la taille de carte, D490) pour `NewState`, et la **case** de chaque relais, camp neutre, autel pour la liste des lieux.
- Le poids de 25 % de `HOLHeroXP::GetWeightPercent(Exploration)` : la valeur de la rÃ¨gle est l'XP finale (pas repassÃ©e par le poids) ; Ã  confirmer au branchement.
- Mode `Disabled` (tests, tout visible) : rien n'est Â« nouvellement explorÃ© Â», donc aucune XP ; Ã  vÃ©rifier au branchement.

## Suivi

- 2026-11-15 10:2x : chantier ouvert, journal + question Ã©crits (SAURON-1115b).

## Appels (un trait par appel)

|||||||||||||||||||
