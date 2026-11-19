# GALADRIEL-1119co : portage des tests de deconnexion (D853 point 4)

Branche `disconnect-3` (partie de integ-22 9ed53fd), worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`. Commit 7270421 (non compile, non teste : aucun build ni quick_check, par consigne). Aucun test C++ retire. Pas de push, pas de fusion.

## 1. Table assertion C++ -> assertion script (HOLDisconnect2_Test.as)
Les messages sont identiques d'un cote a l'autre (verifie ligne a ligne sur les 4 tests).

| Test C++ | Script | Etat avant | Etat apres ce commit |
|---|---|---|---|
| DisconnectRules (1) | Test_DisconnectRules | tout sauf `DefaultWindowSeconds == 180` | complet : `AssertEquals(180, int(RulesGetDefaultWindowSeconds()))` |
| DisconnectGains (3) | Test_DisconnectGains | sauf "player state gone" et "load emptied" | complet : `CountPlayerStatesOfSlot == 1` avant, `== 0` apres `Bob.DestroyActor()`, `Remaining == 0` |
| DisconnectTechnologies (6) | Test_DisconnectTechnologies | sauf 2 comptes de journal "researched PloughI" (slot 0 : 1, slot 3 : 0) | complet via capture de journal (Begin/Count/End) |
| DisconnectWindowEnd (15) | Test_DisconnectWindowEnd | sauf 2 assertions Diplomacy | complet via `RulesGetWindowEndInMode` |

Differences de forme gardees : Technologies fait 3000 pas au lieu de 8 (pas de hol.Prod.BuildTime en script) ; Gains : Bob est maintenant ajoute au PlayerArray (Seat(..., true)) pour que le compte "gone" ne soit pas vacu (en C++ Bob n'est pas ajoute non plus : l'assertion C++ "gone" etait peut-etre vacue, le compte 1 -> 0 du script la rend reelle).

## 2. Enveloppes C++ ajoutees (liaison de test, D839)
- `UHOLTestWorldLibrary::DepositLoadWithRemaining(TestWorld, Owner, Resource, Amount, int32& OutRemaining)` : comme DepositLoad, plus le reste du chargement. `DepositLoad` inchange.
- `UHOLTestWorldLibrary::BeginLogCapture()`, `CountCapturedLogLines(A, B)`, `EndLogCapture()` : capture de GLog (FOutputDevice dans un TUniquePtr statique, Flush avant lecture). Begin met fin a une capture en cours.
- `UHOLGameStateTestLibrary::RulesGetWindowEndInMode(Json, Slot, Mode)` : fixe `Config.Mode` apres lecture du JSON.
- `UHOLGameStateTestLibrary::RulesGetDefaultWindowSeconds()`.
Fichiers : Tests\HOLTestWorldLibrary.h/.cpp, Tests\HOLGameStateTestLibrary.h/.cpp.
BUILD REQUIS avant de lancer les tests script (le script appelle ces 6 fonctions neuves). Les UFUNCTION `int32&` en sortie : appel script `int Remaining = -1; ...(…, Remaining)` ; si l'AS refuse la reference, remplacer par une enveloppe qui renvoie le reste.

## 3. Falsifications d'une ligne (fichier `Script\Tests\HOLDisconnect2_Test.as`)
- Gains (load) : `T.AssertEquals(0, Remaining, "its load emptied");` -> `T.AssertEquals(1, Remaining, "its load emptied");`
- Gains (state gone) : `T.AssertEquals(0, HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1), "his player state gone` -> `T.AssertEquals(1, HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1), "his player state gone`
- Rules : `T.AssertEquals(180, int(HOLGameStateTest::RulesGetDefaultWindowSeconds())` -> `T.AssertEquals(181, int(HOLGameStateTest::RulesGetDefaultWindowSeconds())`
- WindowEnd : `T.AssertEquals(DisconnectTest2::Defeat, DisconnectTest2::EndInMode(DisconnectTest2::Ints2(1, 1)` -> `T.AssertEquals(DisconnectTest2::AIHolds, DisconnectTest2::EndInMode(DisconnectTest2::Ints2(1, 1)`
- Technologies : `T.AssertEquals(1, AliceLogged,` -> `T.AssertEquals(0, AliceLogged,`
Chaque texte "avant" est unique dans le fichier (les nouvelles lignes ont des noms propres).

## NON VERIFIE
- Rien n'a ete compile ni lance : les 6 enveloppes C++ (compilation), les noms de journal ("slot 0" / "slot 3" contenus dans la ligne "researched PloughI"), le passage de `int32&` en sortie depuis AS, et `CountPlayerStatesOfSlot == 1` avant le depart de Bob (suppose que `AddPlayerState` du test place Bob dans PlayerArray, comme le fait Test_DisconnectFakeBack ligne ~395).
- Si le compte du journal est 0 en script (journal de simulation filtre en verbosite) : verifier les lignes capturees ; le test C++ a le meme principe.
- Le ROUGE puis le VERT sont a voir par la session principale avant de retirer les 4 tests C++ (Rules, Gains, Technologies, WindowEnd).
- Commentaires de tete (l.5 "The C++ tests stay...") non modifies.

## Champs de rapport (agent_rules, resume)
BUILDS_DEMANDES: 1 (lot C++ : enveloppes ci-dessus, build puis `quick_check -Parts` pour les tests script + Match.Disconnect*) / TESTS_CPP_RETIRES: 0 / TESTS_SCRIPT_AJOUTES: 0 (5 tests script completes) / LIGNES_CPP_SUPPRIMEES: 0 / TYPES_A_EXPOSER: aucun / EDITS_REFUSES: 0 / PROCESSUS_DETACHES: aucun lance.
