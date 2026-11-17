# CELEBORN-1117j : MIG-C-Volcano (2026-11-17), worktree HeroesOfLiberty-Celeborn, branche hero-j8-close

Aucun test C++ supprime. Binaire perime au lancement (Get-StaleBinaryProblem non vide : HOLTerrainTests.cpp arrive par `git merge main`), donc AUCUN rouge vu (L15-5) :
les tests script ci-dessous sont ecrits, non executes. Rouge + vert a faire par la session principale apres le build.

## Commits (anglais, pas de push, pas de fusion)
1. `fae492c` Script: AltarSend + AltarHoldSquad en script via `UHOLVolcanoLibrary::DecideAltarSquad` (deja expose) : `Script\Tests\HOLVolcanoAltarSquad_Test.as` (3 tests). Compile avec le binaire actuel.
2. Lot C++ minimal + tests script du reste : `Source\HeroesOfLiberty\Simulation\HOLVolcanoRulesLibrary.h/.cpp` (enveloppes seules, ~290 lignes avec les 6 structs de vue) + `Script\Tests\HOLVolcanoRules_Test.as` (15 tests). A compiler SEULEMENT apres le build.

## Registre : nom C++ -> test script / a exposer
| Test C++ (HeroesOfLiberty.Volcano.*) | Test script | Expose par |
|---|---|---|
| StandardStaysDormant | Test_VolcanoStandardStaysDormant | lot : GetPhase |
| CatastropheCycle | Test_VolcanoCatastropheCycle | lot : MakeCycle, GetPhase |
| LavaSpeed | Test_VolcanoLavaSpeedIs70PercentOfTheManAtArms | lot : ManAtArmsSpeed (lit la table, 4 lignes) |
| FlowFront | Test_VolcanoFlowFront | lot : FlowLength, EruptionDuration, FrontDistance, PointAt |
| LavaReach | Test_VolcanoLavaReach | lot : IsInLava, LavaDestroys |
| ObstacleBoxes | Test_VolcanoObstacleBoxesCoverTheLavaReach | lot : GetObstacleBoxes |
| EruptionFollowsTheFlow | Test_VolcanoEruptionFollowsTheFlow | lot |
| DrawFlow | Test_VolcanoDrawFlow | lot : DrawFlow |
| SiteState | Test_VolcanoSiteStateFollowsTheCycles | lot : SiteStateAt, SiteSeed |
| MatchSeed | MOITIE script : Test_VolcanoSiteStateSameSeedSameEruption (meme graine, meme eruption) ; la moitie .json (HOLTestHooks::MakeScenarioConfig / GetScenarioSeed) RESTE C++ | lot : SiteStateAt |
| EvacuationPoint | Test_VolcanoEvacuationPoint (inclut DistanceToFlow de AltarHoldSquad) | lot : IsInCorridor, EvacuationPoint, EvacuationReach, DistanceToFlow |
| AltarAdvance | Test_VolcanoAltarAdvance | lot : AdvanceForAltar |
| AltarHold | Test_VolcanoAltarHoldCount | lot : StepAltar, StepAltarOwned |
| AltarWake | Test_VolcanoAltarWakeStartsTheAlert | lot : SiteStateAt (un seul reveil : WakeCycle/WakeAt) |
| AltarSend | Test_VolcanoAltarSendSizesTheSquad, Test_VolcanoAltarSendNeverOscillates | DEJA expose : DecideAltarSquad |
| AltarHoldSquad | Test_VolcanoAltarHoldSquadFollowsTheShare (le cas « adversaire dans la zone » n'a pas d'equivalent : DecideAltarSquad route alors vers AltarSend ; couvert par AltarSend) | DEJA expose : DecideAltarSquad |
| FreshDeposit | Test_VolcanoFreshDeposit | lot : FreshDeposit, FreshDepositDue |

Restent en C++ : la moitie .json de MatchSeed. Les 17 tests = StandardStaysDormant + 16 autres ; 16 sont couverts par script (MatchSeed a moitie). Suppression C++ possible (apres rouge + vert) : tout `HOLVolcanoTests.cpp` sauf la moitie .json de MatchSeed (a garder en C++ ou a deplacer dans un petit test C++ de 15 lignes avant de supprimer le reste).

## Choix de conception du lot (a relire)
- `FHOLVolcanoTuning` : struct de vue des reglages numeriques, ses valeurs par defaut sont lues sur `FHOLVolcanoSettings` (une seule source), `ToSettings()` les recopie.
- Un flot = `TArray<FVector>` ; un site = `FHOLVolcanoSiteLayout` ; `SiteStateOfMatch` non expose (une ligne : `SiteStateAt(Config.bCatastrophe, Config.Seed, ...)`).
- Pas de regle dans le lot, aucune logique : une appel + une recopie par fonction. Classe script : `HOLVolcanoRules::`.
- Risques de compilation non verifies (pas de build) : `UPARAM(ref) float&` sur BlueprintCallable, TArray<FVector> en parametre const& de BlueprintPure, noms de parametre `ManAtArmsSpeed` du .h vs fonction du meme nom (la .cpp utilise `ManAtArmsSpeedValue`), `FName ==` dans le script, `Math::DegreesToRadians`, `bool&out`/`const FHOLAIAltar&` du test d'autel.

## Commande de build (session principale)
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait`
puis `quick_check -Parts tests` (rouge : fausser une valeur attendue dans HOLVolcanoAltarSquad_Test.as, vu N-1/N, la remettre, vert).

## CELEBORN-1117k (2026-11-17, apres build) : script compile, rouge puis vert, C++ retire
- Cause de l'echec : `float` du script = double ; les parametres `float&` (UPARAM(ref)) veulent `float32`. 3 variables (Held, Owned, Count) passees en `float32` dans Test_VolcanoAltarHoldCount (7d31f07). Seules erreurs du log (22, toutes StepAltar/StepAltarOwned).
- Vert : quick_check tests 1420/1422, seuls rouges NomadFirstForum et NomadReplicated (connus).
- Rouge : une assertion faussee dans chacun des 18 tests script Volcan (AssertTrue(!(cond)) ou AssertEquals(x + 1, ...)), UN quick_check : 1402/1422, les 18 nommes rouges ; valeurs remises (git checkout), vert revu.
- Comparaison assertion par assertion (comptes C++ -> script) : identiques sauf SiteState 18 -> 17 (« same machine, same state » passe dans SameSeedSameEruption), AltarSend 12 -> 11 (« squad > 1 » impliquee par « == 2 »), EvacuationPoint 17 -> 19 (+ DistanceToFlow de HoldSquad), MatchSeed 11 -> 3 (reste la moitie .json), AltarHoldSquad 12 -> 8.
- Retire de HOLVolcanoTests.cpp (325c96f, C++ NON COMPILE, build a relancer) :

| Test C++ retire | Test script (rouge puis vert) |
|---|---|
| StandardStaysDormant | Test_VolcanoStandardStaysDormant |
| CatastropheCycle | Test_VolcanoCatastropheCycle |
| LavaSpeed | Test_VolcanoLavaSpeedIs70PercentOfTheManAtArms |
| FlowFront | Test_VolcanoFlowFront |
| LavaReach | Test_VolcanoLavaReach |
| ObstacleBoxes | Test_VolcanoObstacleBoxesCoverTheLavaReach |
| EruptionFollowsTheFlow | Test_VolcanoEruptionFollowsTheFlow |
| DrawFlow | Test_VolcanoDrawFlow |
| SiteState | Test_VolcanoSiteStateFollowsTheCycles |
| EvacuationPoint | Test_VolcanoEvacuationPoint |
| AltarAdvance | Test_VolcanoAltarAdvance |
| AltarSend | Test_VolcanoAltarSendSizesTheSquad + Test_VolcanoAltarSendNeverOscillates |
| AltarHold | Test_VolcanoAltarHoldCount |
| AltarWake | Test_VolcanoAltarWakeStartsTheAlert |
| FreshDeposit | Test_VolcanoFreshDeposit |

- GARDES en C++ : MatchSeed (moitie .json : MakeScenarioConfig / GetScenarioSeed, 5 assertions ; la moitie etat est Test_VolcanoSiteStateSameSeedSameEruption) ; AltarHoldSquad (le script n'a pas le cas « adversaire dans la zone », ni 40 m exact, ni « 100 cm : pas de rappel », ni le defaut 4000 : DecideAltarSquad route autrement) ; les autres tests Volcano C++ (Altar*, Burn, Evacuate, Layout*, Obstacle, FreshDepositAppears...) sont dans d'autres fichiers, hors lot.
- Apres le build : quick_check -Parts tests doit rester 1420 - 15 = 1405/1407 (2 rouges connus) ; Test_VolcanoAltarHoldSquadFollowsTheShare ne remplace pas AltarHoldSquad C++.
