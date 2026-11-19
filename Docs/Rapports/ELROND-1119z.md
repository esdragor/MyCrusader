# ELROND-1119z : port script de HOLDisconnectTests.cpp, suite (9 tests neufs, 3 completes)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche `integ-16`, commit 22300a0 (2 fichiers script : `Script\Tests\HOLDisconnect2_Test.as` neuf, 589 l. ; `HOLDisconnect_Test.as` complete). Aucun fichier C++ touche, aucun test C++ supprime. Pas de push, pas de merge.

Impact joueur : aucun (outillage de test).

## Tests

Completes dans `HOLDisconnect_Test.as` : GameState (GetReasonName(5) = "Disconnected"), HeroSheet (IsABot du holder), Banner (les 7 assertions directes FormatAbsentLine / FormatBackLine). Neufs dans `HOLDisconnect2_Test.as` : Rules, WindowEnd, Gains (partiel), Technologies, Held, HeldStock, LateReturn, Replace, FakeBack. SpawnPlayerStateNoSlot + SetPlayerMatchSlot = l'ordre du C++ (plus de BobAgain spawne apres le Destroy).

## Boucle

1. 14:1x : compile ; 1752/1755 : Gains (rien rendu par `HOLTestWorld::DestroyBuilding`, qui est un `Destroy()` simple, pas `UHOLConstructionSubsystem::DestroyBuilding` : remboursements de la fondation et de la file absents), HeldStock (`HOLAIOrder::IssueAIProduceOrder` sans monde de test : file vide, food inchangee), et le test C++ DisconnectTechnologies (REGRESSION CAUSEE PAR MON PREMIER ESSAI : `RunConsoleCommand "hol.Prod.BuildTime 0"` pose la cvar en SetByConsole, et le `Set(0.1, SetByCode)` du test C++ est alors ignore : "ignored as it is lower priority than the previous SetByConsole"). Decouverte : un test script ne doit jamais poser une cvar par la console si un test C++ la pose par code.
2. Corrections (aucune valeur attendue changee) : plus de cvar, 3000 pas au lieu de 8 ; Gains reduit a ce qui est portable (depot de 10 bois, valeur attendue = depart + 10 bois, les 2 remboursements restent en C++) ; HeldStock pose sa commande par `EnqueueProduction` paye sur le holder (meme file, meme stock).
3. 14:24 : VERT 1755/1755 (216 s). Total de depart : 1746 ; +9 tests neufs.

## TEST_ROUGE

TEST_ROUGE: vu a 14:28 : 1743/1755 (12 valeurs faussees, 12 tests tombes, eux seuls : les 3 completes GameState/Banner/HeroSheet et les 9 neufs). Valeurs faussees : ReasonName "Disconnectex" ; Banner "0:58" pour 59 s ; HeroSheet !IsABot ; Rules 159 -> 158 ; WindowEnd 1v1 -> AIHolds ; Gains +11 bois ; Technologies compte d'Alice 2 ; Held IsHeld faux ; HeldStock nom "Bab" ; LateReturn 1 defait au lieu de 0 ; Replace slot 2 ; FakeBack notices + 1. Restaure par `git checkout` des 2 fichiers (aucun diff), relance : voir VERT_FINAL.

## Tableau d'equivalence des 15 tests C++ (fichier HOLDisconnectTests.cpp)

| # | Test C++ | Etat | Ce qui reste en C++ (ligne) |
|---|---|---|---|
| 1 | DisconnectRules | porte en entier sauf 1 assertion | l.75 `DefaultWindowSeconds == 180` (constante, pas d'accesseur) |
| 2 | DisconnectGameState | PORTE EN ENTIER (le test C++ peut partir) | rien |
| 3 | DisconnectGains | EN PARTIE | l.201-205 et 224-225 : remboursement de la fondation et de la file par `UHOLConstructionSubsystem::DestroyBuilding` ; l.213 "player state gone" ; l.221 "load emptied" ; l.235 la valeur attendue avec les 2 remboursements (le script attend depart + 10 bois) |
| 4 | DisconnectBanner | PORTE EN ENTIER | rien |
| 5 | DisconnectHeroSheet | PORTE EN ENTIER | rien |
| 6 | DisconnectTechnologies | EN PARTIE | l.507 et 510 : les comptes du journal "researched PloughI" (capture de journal absente) ; l.495-509 : la lecture IA `ReadAITechnologiesDone` est remplacee dans le script par les technologies du player state IA holder (autre chemin) ; cvar BuildTime remplacee par 3000 pas |
| 7 | DisconnectHeld | EN PARTIE | l.568, 575, 588 `UHOLAIToolsLibrary::GetSlotLevel` (non essaye : appel a contexte de monde implicite) |
| 8 | DisconnectHeldStock | EN PARTIE | l.660-662 : la commande passe par `UHOLAIOrderLibrary::IssueAIProduceOrder` (script : EnqueueProduction paye sur le holder) ; l.685-691 : le compte des player states du siege (script : FindLiveHolder == BobAgain) |
| 9 | DisconnectReplace | EN PARTIE | l.744-745 : l'ordre IA (script : EnqueueProduction sur l'ancien player state) ; l.770 `GetPlayerIndex() != 1` |
| 10 | DisconnectPause | PORTE EN ENTIER | rien |
| 11 | DisconnectBannerClock | PORTE EN ENTIER | rien |
| 12 | DisconnectFakeBack | PORTE (substitution) | l.943-949 : le compte des player states du siege est remplace par FindLiveHolder ; sinon entier |
| 13 | DisconnectReportMode | PORTE EN ENTIER | rien |
| 14 | DisconnectLateReturn | EN PARTIE | l.1052 et 1068 `GetSlotLevel` (non essaye) |
| 15 | DisconnectWindowEnd | porte sauf 2 assertions | l.1153-1154 : mode Diplomacy (pas de reglage du mode dans les JSON) |

Peuvent partir sans perte : 2, 4, 5, 10, 11, 13 (et 12 si la substitution FindLiveHolder est acceptee). Les autres gardent les lignes ci-dessus.

## TYPES_A_EXPOSER

- `UHOLConstructionSubsystem::DestroyBuilding` (version qui rembourse) dans `HOLTestWorld::` (test 3).
- `HOLAIOrder::IssueAIProduceOrder` avec le monde du test : `IssueAIProduceOrder(TestWorld, Slot, Location, Type)` (tests 8, 9).
- `GetSlotLevel(TestWorld, Slot)` avec monde explicite (tests 7, 14) ; a essayer d'abord tel quel.
- Capture de journal (demarrer / arreter / compter les lignes contenant A et B) (test 6) ; `ReadAITechnologiesDone` (test 6).
- Mode de match (Standard / Diplomacy) dans `HOLMatchConfigRead` (test 15).
- Nombre de player states d'un siege (PlayerArray) ; `GetPlayerIndex` ; `DefaultWindowSeconds` (tests 1, 8, 9, 12).

## Champs

BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 9 (Test_DisconnectRules, WindowEnd, Gains, Technologies, Held, HeldStock, LateReturn, Replace, FakeBack) ; 3 completes (GameState, HeroSheet, Banner)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~590 (script neuf)
TESTS_TAUTOLOGIQUES: 0
EDITS_REFUSES: 0
NON VERIFIE : Held et LateReturn passent sans `GetSlotLevel` (non essaye) ; le cerveau IA existe bien dans le monde de test (Held et LateReturn verts, GetAIBrain non nul) ; la suppression des tests C++ (decision de la session principale).
Anomalies : cvar posee par console = priorite SetByConsole qui bloque un `Set(SetByCode)` ulterieur (voir boucle 1) ; `HOLTestWorld::DestroyBuilding` ne rembourse pas ; `IssueAIProduceOrder` en script n'atteint pas le monde du test (file vide).
PROCESSUS_DETACHES: voir la sortie finale (stray_processes lance en dernier geste)

VERT_FINAL: 14:31 : 1755/1755 PASS (167 s), 2 fichiers restaures par git checkout (aucun diff), commit 22300a0. PROCESSUS_DETACHES: stray_processes -Stop lance en dernier geste (aucun signale)
