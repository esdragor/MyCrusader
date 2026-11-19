# ELROND-1119v : port script des tests portables de HOLDisconnectTests.cpp

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche `tiers`. Commit c74712f (1 fichier neuf, `Script\Tests\HOLDisconnect_Test.as`, ~400 l.). Aucun fichier C++ touche, aucun test C++ supprime. Pas de push, pas de merge.

Impact joueur : aucun (outillage de test). Plan : 6 tests script remplacent, apres relecture, 5 tests C++ entiers et la moitie d'un sixieme.

## Tableau d'equivalence (15 tests C++)

| # | Test C++ | Etat | Script | Ce qui reste en C++ |
|---|---|---|---|---|
| 1 | DisconnectRules | non porte | | statiques de `FHOLDisconnectRules` sur `FHOLAbsentSlot` (FindAbsent, GetExpired, GetSecondsLeft, GetStockOnTaking, IsWindowOver, KeepsSlot, MakeDefault) : enveloppe de regles absente |
| 2 | DisconnectGameState | porte en entier, avec ecarts (A) | Test_DisconnectGameState | rien (la raison se compare en int, 5, au lieu de `GetReasonName` = "Disconnected" : 1 assertion C++ reste, ligne 138) |
| 3 | DisconnectGains | non porte | | methodes de `AHOLBuilding` (Setup 3 args, GetConstructionState/Work, GetTotalWork, DepositLoad, GetCancelRefund, TryEnqueue) |
| 4 | DisconnectBanner | en partie | Test_DisconnectBanner | les 7 assertions directes de `FormatAbsentLine` / `FormatBackLine` (161 s, 160.2 s arrondi, 59 s, -3 s, la ligne "de retour" sans heure ni tiret), l. 253-262. Leurs textes sont lus en litteraux via les bannieres |
| 5 | DisconnectHeroSheet | porte, sauf `IsABot` | Test_DisconnectHeroSheet | l'assertion `Holder->IsABot()` (l. 369) : le script verifie seulement "non nul et pas Bob" |
| 6 | DisconnectTechnologies | non porte | | capture de journal, cvar, GetTechnologyBonusPercent, GetGatherStat, FindTechnology, files du batiment |
| 7 | DisconnectHeld | non porte | | cerveau IA (ThinkNow, GetThinkingSlots, IsHeld, GetSlotLevel) |
| 8 | DisconnectHeldStock | non porte | | `GetSlotStock` statique, files de production, technologies, IsABot |
| 9 | DisconnectReplace | non porte (pas essaye, temps) | | tout ; faisable en partie (FindSlotForLogin, FindReplacedForLogin, RetireHolder, GetSlotResources) mais manquent FindLiveHolder, IsOnlyASpectator, file de production, `GetTechnologies().Has` |
| 10 | DisconnectPause | porte en entier | Test_DisconnectPause | rien |
| 11 | DisconnectBannerClock | porte en entier | Test_DisconnectBannerClock | rien |
| 12 | DisconnectFakeBack | non porte (pas essaye, temps) | | tout ; manquent `GetSlotStock` statique et `IsABot` |
| 13 | DisconnectReportMode | porte en entier | Test_DisconnectReportMode | rien |
| 14 | DisconnectLateReturn | non porte | | cerveau IA |
| 15 | DisconnectWindowEnd | non porte | | regles pures (GetWindowEnd, MayReturn, KeepsSlot) |

Porte en entier ou presque (a supprimer apres relecture) : 2 (hors 1 assertion GetReasonName), 5 (hors IsABot), 10, 11, 13. En partie : 4. Le C++ de 2 et 5 ne peut donc pas etre supprime sans perdre ces deux assertions : soit on garde ces deux tests C++ jusqu'a l'exposition de `GetReasonName` et `IsABot`, soit on accepte la perte (decision de la session principale).

## Ecarts script / C++ (A) a connaitre

- `GiveSlotStock(Taking, Starting)` du C++ n'a pas de jumeau script sans heure : le script passe `Now` (0 a la premiere connexion, 100 au retour). Aucun effet sur les valeurs attendues.
- `KeepAbsentSlot(.., Now)` du C++ (fenetre par defaut) : le script passe `WindowSeconds = 0`, qui prend `FHOLDisconnectRules::DefaultWindowSeconds` (180, verifie dans `HOLDisconnectRules.h:35`).
- `HOLTestWorld::SpawnPlayerState(.., Slot, ..)` pose le slot des le spawn, ce que le C++ fait plus tard par `SetMatchSlot` : un second player state spawne avant le `DestroyActor` du premier est vu comme "un second detenteur vivant" (`Disconnect: slot 1 left by an old connection, a newer one holds it: not kept`, premier essai rouge vu a 11:43). Le script spawn donc `BobAgain` apres la destruction de `Bob`. La chaine testee est la meme (SetMatchSlot puis GiveSlotStock). Decouverte : pas un bug du jeu, une propriete de la bibliotheque de test.
- `Bob.DestroyActor()` remplace `Bob->Destroy()`.

## Boucle de correction

1. 11:40 : erreur de compilation script (`FString::Printf("%d lines", int)` : pas de signature ; remplace par la chaine formatee `f"{...}"`).
2. 11:43 : 1689/1691, Banner et GameState rouges pour la raison du second player state ci-dessus (script corrige, aucune valeur attendue touchee).
3. 11:48 : VERT 1691/1691 (167 s). Total de depart de la branche : 1685 (1691 moins mes 6).

## TEST_ROUGE

Rouge vu a 11:51 : 1685/1691, exactement les 6 tests portes sont tombes et eux seuls (Banner, BannerClock, GameState, HeroSheet, Pause, ReportMode). Valeurs faussees, une par test : GameState `AliceSlot` 0 -> 7 ; Banner `EndsAt` "190.000" -> "191.000" ; HeroSheet `KeptXP` 12 -> 13 ; Pause `GetWindowClock(200)` 140 -> 141 ; BannerClock "0:50" -> "0:51" ; ReportMode "AI mode hold" -> "AI mode hodl". Restaure par `git checkout` (aucun diff), relance : voir VERT_FINAL ci-dessous.

TEST_ROUGE: vu a 11:51 : 1685/1691 (6 tests portes tombes, eux seuls)
VERT_FINAL: 11:55 : 1691/1691 PASS (203 s), fichier restaure par git checkout, PROCESSUS_DETACHES: aucun

## TYPES_A_EXPOSER (signatures)

- `static FString GetReasonName(int32 Reason)` (`AHOLGameState::GetReasonName(EHOLDefeatReason)`), tests 2.
- `static bool IsABot(const AHOLPlayerState*)`, tests 5, 8, 12.
- `FHOLDisconnectRules::FormatAbsentLine(const FString& Name, double SecondsLeft) -> FString`, `FormatBackLine(const FString& Name) -> FString` (test 4), plus les statiques de regles du test 1 et 15 (enveloppe plate).
- `AHOLGameState::GetSlotStock(World, Slot, FHOLResources& OutStock)`, `FindLiveHolder(World, Slot, Except)`, `AHOLPlayerState::IsOnlyASpectator()`; lecture de la file de production d'un batiment, `GetTechnologies().Has(Name)`; `UHOLAIBrain` (ThinkNow, GetThinkingSlots, IsHeld) (tests 7, 14, 8, 9, 12).

## Champs

BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 6 (Test_DisconnectGameState, Test_DisconnectBanner, Test_DisconnectHeroSheet, Test_DisconnectPause, Test_DisconnectBannerClock, Test_DisconnectReportMode)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~400 (script neuf)
TESTS_TAUTOLOGIQUES: 0 (chaque test passe par le vrai GameState, la vraie chaine KeepAbsentSlot / GiveSlotStock / DefeatExpiredAbsences, les commandes console reelles pour ReportMode)
EDITS_REFUSES: 0
NON VERIFIE : 5 des 6 tests ne sont portes qu'a la valeur pres ; la suppression des tests C++ correspondants (a decider apres lecture du tableau) ; la relance verte apres restauration est consignee en VERT_FINAL.
Anomalies : git signale LF -> CRLF sur le .as (sans effet). merge main non fait. Tests 9 et 12 (partiels selon ELROND-1119u) non portes faute de temps (arret 12:01).
