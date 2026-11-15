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
- 2026-11-15 10:2x : etapes 1 et 2 FAITES (SAURON-1115b) : test rouge VU ROUGE (tests 1268/1272, les 4 tests HOLExplorationXPRules_Test KO), commit 2487a66 ; regle codee (tranches, lieux, depart, hors grille, prereglages A/B/C, defaut C), quick_check -Parts tests 1272/1272 PASS, commit cf77faf, merge main de32333 (tests non relances apres le merge). Prochaine etape = 3 (C++, worktree du brouillard) : voir 'Ce qu'il faudra lire du brouillard'. Rien n'est branche au jeu.

- 2026-11-15 12:25 : SAURON-1115c, etape 3 (C++ branchement). merge main (avance rapide cdbfe27), LaunchCheck non vide -> build.
  - Poids 25 % TRANCHE PAR LE CODE : `UHOLHeroSubsystem::AddXP` n'applique aucun poids ; seul `HOLBuildProcessor` passe par `HOLHeroXP::GetXP` (Development) ; les deux porteurs Exploration existants (`HOLRelaySubsystem` XP de territoire, `HOLCampSubsystem` part des camps) ajoutent leur XP telle quelle. Donc la valeur de la regle = XP finale, PAS de `GetWeightPercent` reapplique.
  - Jumeau C++ `Source\HeroesOfLiberty\Simulation\HOLExplorationXPRules.h/.cpp` (memes nombres que le .as ; cvar `hol.HeroXP.ExplorationPreset` 0 = C defaut, 1 = A, 2 = B, lue au Start du brouillard).
  - Test `Source\HeroesOfLiberty\Tests\HOLExplorationXPTests.cpp` (`HeroesOfLiberty.ExplorationXP.FogPaysTheHero`) : vraie chaine (Peasant, UHOLVisionSubsystem::Update, fiche du heros) ; 1re seconde = 0 XP ; deplacement lointain : XP = N tranches neuves (comptees par le test depuis le brouillard) x XPPerTranche ; meme lieu une 2e fois = rien de plus ; camp adverse = 0.
  - Branchement prevu : `FHOLVisionMap::Lay` note les cases passees inexplorees -> explorees par camp (`TakeNewlyExplored`) ; `UHOLVisionSubsystem::Update` (serveur et solo, jamais client) les draine : pendant la 1re seconde (SimulationRateHz pas) `MarkExploredAtStart`, ensuite `OnCellExplored` et `AddXP(Slot, XP, Exploration)` pour CHAQUE slot du camp (camp = equipe ou slot seul ; partage entre allies NON TRANCHE, note pour la question). Mode Carte exploree : ExploreAll ne note rien -> 0 XP. Lieux (relais, camps, autels) : liste VIDE pour l'instant (preset B ne paie donc rien en jeu) -> reste a faire.

- 2026-11-15 12:40 : TEST ROUGE VU A L'EXECUTION (build PASS 12:37 ; quick_check -Parts tests 1303/1304, seul KO `HeroesOfLiberty.ExplorationXP.FogPaysTheHero`), commit 960714b (regle C++ + cvar + test). Detail de l'assertion KO non lu (journal UE non trouve) : a lire au vert.
- ARRET a 45 appels (SAURON-1115c) AVANT le branchement : les 8 Edit du branchement ont ete refuses (« file has not been read yet » : faire un Read de chaque fichier avant). PROCHAIN AGENT : appliquer, build, `quick_check -Parts tests` (attendu 1304/1304), commit, puis `-Parts net` (brouillard cote serveur).
  1. `Map\HOLVisionRules.h`, avant `private:` de FHOLVisionMap : `TArray<int32> TakeNewlyExplored(const int32 Camp) { TArray<int32> Cells; if (NewlyExplored.IsValidIndex(Camp)) { Swap(Cells, NewlyExplored[Camp]); } return Cells; }` ; en prive : `TArray<TArray<int32>> NewlyExplored;`.
  2. `Map\HOLVisionRules.cpp` : `Init` apres `Explored.Init(...)` : `NewlyExplored.Reset(); NewlyExplored.SetNum(NumCamps);` ; `Lay` : remplacer `Explored[Offset + Index] = true;` par `if (!Explored[Offset + Index]) { Explored[Offset + Index] = true; NewlyExplored[Observer.Camp].Add(Index); }`.
  3. `Map\HOLVisionSubsystem.h` : `#include "Simulation/HOLExplorationXPRules.h"` ; membres prives `void PayExplorationXP(); HOLExplorationXP::FSettings ExplorationSettings; TArray<HOLExplorationXP::FState> ExplorationStates; int32 UpdatesSinceStart = 0;`.
  4. `Map\HOLVisionSubsystem.cpp` : includes `Simulation/HOLHeroSubsystem.h`, `Simulation/HOLHeroXPRules.h`, `Simulation/HOLSimulationSettings.h` ; `Start` apres `Map.Init(...)` : `ExplorationSettings = Preset(GetPresetIndex())`, un `NewState(ExplorationSettings, Map.GetGrid().Side, 0)` par camp, `UpdatesSinceStart = 0` ; fin de `Update()` : `PayExplorationXP();` ; `PayExplorationXP` : rien si `bClient` ou NM_Client ; `bAtStart = UpdatesSinceStart < Max(1, SimulationRateHz)` puis `++UpdatesSinceStart` ; par camp : chaque `Map.TakeNewlyExplored(Camp)` -> `Grid.ToCell(Index)` -> `MarkExploredAtStart` (debut) ou `XP += OnCellExplored(..., {}, Cell, false)` ; si XP > 0 : `Heroes->AddXP(Slot, XP, EHOLXPSource::Exploration)` pour chaque slot avec `CampOfSlot[Slot] == Camp`.
  - Reste ensuite : lieux (relais, camps neutres, autels) dans `Places` (vide = preset B ne paie rien) ; test d'accord .as / C++ (modele `Test_RelayScriptAndSimulationRulesAgree`) ; partage entre allies d'un meme camp (chaque slot du camp paye = defaut, a ajouter a Q-exploration-xp).

- 2026-11-15 12:43 : SAURON-1115d, LaunchCheck vide. Etapes 1-4 du branchement APPLIQUEES (code ci-dessus ; en plus `UpdateClient` appelle aussi `PayExplorationXP`, qui sur un client vide la liste sans payer, pour qu'elle ne grossisse pas). Build lance 12:46. Q-exploration-xp.md complete (chaque siege du camp paye ; lieux vides -> preset B ne paie rien en jeu).
- 2026-11-15 12:55 : SAURON-1115d VERT : build PASS 12:47 (131 s), quick_check -Parts tests 1304/1304 PASS (FogPaysTheHero vert), commit 3d94384, -Parts net PASS (303 s). Etape 3 FAITE. Reste : lieux (relais, camps neutres, autels) dans Places (vide = preset B ne paie rien en jeu) ; test d'accord .as / C++ ; partage entre allies (Q-exploration-xp) ; frequence reelle d'Update (1re seconde = SimulationRateHz appels d'Update, a verifier en partie). Pas dans main.
- 2026-11-15 14:05 : SAURON-1115e (sans C++). merge main (avance rapide 6a77f35) ; LaunchCheck NON VIDE (dll plus vieilles que HOLSpecializationGridTests.cpp, HOLGameScreen.h venus de main : build a faire avant tout test moteur, aucun C++ touche ici).
  - TEST D'ACCORD FAIT : `Tools\Tests\exploration_xp_accord_test.py` lit les defauts des structs (.as `FHOLExplorationXPSettings`, .h `FSettings`) et les 3 branches de `Preset` (.as et .cpp) et compare CellsPerTranche / XPPerTranche / XPPerPlace (defaut, 0 = C 16/3/25, 1 = A 16/6/0, 2 = B 16/0/40) ; auto-test « il mord » (B a 41 dans le script, C++ sans branche B, defaut de tranche 8 refuses). ROUGE vu sur copie jetable du scratchpad (C++ A a 7 XP : `1 XPPerTranche: script 6, C++ 7 <-- FAIL`, exit 1), VERT sur le depot (exit 0). Branche dans la partie values de `quick_check.ps1` (worktree seulement). `quick_check -Parts values` PASS.
  - LIEUX : RECOUVREMENT -> NON BRANCHES. Relais : XP de territoire a la minute tant qu'on le tient (`HOLRelaySubsystem.cpp` ~l.260, D388) ; camps neutres : recompense au nettoyage (`HOLCampSubsystem.cpp` ~l.278) ; les deux sur la source Exploration. La 1re vue (B 40, C 25) crediterait la meme source une 2e fois pour le meme lieu, en plus de la part ~15 % (D05). Autel : seulement la carte Volcan (`HOLVolcano*`). Point [A REPONDRE] ajoute a `C:\Users\baudo\Documents\MyCrusader\Retours\Q-exploration-xp.md` (A tous / B seulement les lieux sans autre paiement, recommande / C aucun / Autre ; defaut en place = liste vide).
  - Reste : reponse sur les lieux (si B : lire la case de l'autel du Volcan dans `UHOLVolcanoSubsystem` et la passer a `Places` dans `UHOLVisionSubsystem::Start`, test rouge d'abord) ; partage entre allies ; frequence reelle d'Update.

## Appels (un trait par appel)

||||||||||||||||||| (SAURON-1115c : ~49)
||||||||||||||||||||||||||||||| (SAURON-1115d : 31 au build)
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D809 (Q-exploration-xp) = Q1 C (défaut, tranche 3 + lieu 25) + Q2 B : seuls les lieux sans autre paiement paient à la 1re vue (autel du Volcan, futurs lieux), pas relais ni camps ; ligne [ ] Sauron (C++) dans Auto_Journey_File.md.
