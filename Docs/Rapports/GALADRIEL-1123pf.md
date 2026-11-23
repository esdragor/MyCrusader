# GALADRIEL-1123pf (2026-11-23, 16:51 - 16:57, worktree Galadriel, branche socle-ai, base a3de24e)

NON COMPILE : deux commits C++ sans build ni quick_check (consigne). La session principale lance build + tests.

## Impact joueur
Une attenuation de camp (commande provisoire hol.Morale.Attenuate) ou un ordre de moral en file ne survit plus a la fin d'une partie : la partie suivante demarre propre.

## (G1) Etats statiques du moral (HOLMoraleProcessor.cpp)
- `SlotAttenuationPercent[MaxSlots]` (l.61) : table globale, jamais remise a 0.
- `PendingAround` et `PendingClear` (l.56-57, verrou `PendingAroundLock`) : files d'ordres globales, ne se videntque pour un monde mort ou le monde qui execute (un ordre pose sur un monde qui ne fait jamais tourner le processeur reste).
- Aucun autre compteur statique dans HOLMoraleProcessor / HOLMoraleLibrary (la bibliotheque ne garde rien, elle appelle `QueueEffectAround` / `QueueClearMalusAround`).
- Precedent de remise a zero d'un etat statique de simulation : AUCUN trouve (Grep `Reset|Clear|OnWorldCleanup|FWorldDelegates` dans Source\ : seul `UHOLAIToolsLibrary::ClearEvolveTimes`, commande de console, pas un cycle de monde). Le point de debut/fin de monde de la simulation est `UHOLSimulationSubsystem::OnWorldBeginPlay` (cree et initialise les processeurs : `Processor->CallInitialize`, HOLSimulationSubsystem.cpp:258) et `::Deinitialize` (l.292). Choix :
  - fin : `UHOLSimulationSubsystem::Deinitialize` appelle `UHOLMoraleProcessor::ResetForWorld(GetWorld())` ;
  - debut : surcharge `UHOLMoraleProcessor::InitializeInternal` (appelee par `CallInitialize`, donc en jeu par OnWorldBeginPlay ET dans les tests C++ qui font `NewObject + CallInitialize`) qui appelle `ResetForWorld(Owner.GetWorld())`. `OnWorldBeginPlay` direct ne convenait pas : les mondes de test (`CreateWorld` + `InitializeActorsForPlay`) ne l'appellent pas.
- `ResetForWorld(World)` : table a 0 ; retire des deux files les ordres de ce monde et ceux des mondes morts (un ordre d'un autre monde vivant continue d'attendre). Table d'attenuation : toujours globale, remise a 0 pour tous les camps (l'attenuation n'est pas par monde, redesign non fait).
- Accesseurs ajoutes (lecture pour le test) : `GetSlotAttenuation(Slot)`, `NumPendingOrders()`.

Commits (NON COMPILES) :
- 5f27811 : test + bouchon (`ResetForWorld` ne fait rien) + accesseurs. Test : `HeroesOfLiberty.Morale.ProcessorStaticStateEndsWithTheWorld` (HOLMoraleRulesTests.cpp, fin de fichier, sous `WITH_DEV_AUTOMATION_TESTS`). Monde 1 : camp 1 a 50 %, un ApplyEffectAround + un ClearMalusAround en file, monde detruit -> attenuation 0 et file a 0 ; puis attenuation 50 laissee entre deux parties, monde 2 + processeur -> 0 / file vide, et premier pas : un Demoralised pose sur une unite du camp 1 garde sa duree pleine (pas de moitie).
- 2f8a019 : la remise a zero (corps de ResetForWorld, `InitializeInternal`, appel dans `Deinitialize`, include).
- TEST_ROUGE : NON VU (rien compile). Avec le bouchon le test doit etre rouge sur les assertions « after the end » (attenuation 50, 2 ordres).
- TEST_ROUGE_SEPARE: oui (deux commits).

## (G4) HOLMoraleRules.as
Par les faits : le jeu LIT le C++ pour la regle (combat, processeur) ; le `.as` n'est pas un jumeau mais un relais : 119 lignes, chaque corps = un appel `HOLMorale::Native*` de `UHOLMoraleLibrary` (sauf `Count` = `State.Effects.Num()`), plus les constantes de contrat Lever*/Apply*. Il a des appelants reels HORS tests : `Script/Morale/HOLMoraleTable.as:177` (`HOLMorale::MakeEffect`), `Script/UI/HOLMoraleBlock.as:40` et `:61` (`HOLMorale::IsMalus`). `HOLDuelLedger.as:118` appelle `HOLMorale::ApplyEffectAround`, qui est dans `UHOLMoraleLibrary` et non dans le `.as`. Donc : RIEN supprime. Ligne `[ ]` ecrite en file (Auto_Journey_File.md l.49) : rediriger ces 3 appels vers `NativeMakeEffect` / `NativeIsMalus`, grep des autres noms et constantes, puis `git rm` du `.as` et de `HOLMoraleRules_Test.as` (il ne teste que le renvoi ; la regle est couverte par HOLMoraleRulesTests.cpp). TYPES_A_EXPOSER : aucun.
Le constat G5 de la revue est donc inexact sur le fond (pas de seconde regle) ; reste vrai : un relais inutile a terme.

## Champs
BUILDS_DEMANDES: 1 (lot C++ : build + tests, a lancer par la session principale)
TESTS_SCRIPT_AJOUTES: 0
TESTS_TAUTOLOGIQUES: 0 (la duree vient du processeur et de HOLMorale::Apply)
TYPES_A_EXPOSER: aucun
EDITS_REFUSES: 1 (Edit « not read » sur HOLMoraleProcessor.h, remplace par remplacement exact en PowerShell)
DIAGNOSTICS_AS: sans objet (aucun .as modifie)
Merge main : non fait (consigne).

## NON VERIFIE
- Compilation (signature `InitializeInternal(UObject&, const TSharedRef<FMassEntityManager>&)` verifiee dans le MassProcessor.h du moteur C:\UE\UE_5.8_AS_3, ligne 218 ; protected non verifie, la surcharge est placee dans `protected:`).
- Que `World->DestroyWorld(false)` appelle bien `Deinitialize` des sous-systemes dans un monde de test (le test le suppose ; sinon la partie « apres la fin » reste rouge, et le debut de monde est couvert a part).
- Les tests existants qui posent une attenuation (HOLMoraleRulesTests.cpp:1059-1061, 1156) le font apres leur `CallInitialize` : non affectes.

## A lancer
`<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check` du worktree sur les tests (attendu : 2151 dont le nouveau, 0 echec).

## Appels
~14 outils, fin 16:57.
