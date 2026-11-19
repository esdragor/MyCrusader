# ELROND-1119ab : retrait des tests C++ de déconnexion entièrement couverts par le script (NON COMPILÉ)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche `integ-16`, commit 6fed720 (2 fichiers C++, 508 suppressions, 0 ajout). Aucun build, aucun quick_check, aucun jeu. Pas de push, pas de merge.

Impact joueur : aucun (tests seulement).

## Tableau test C++ -> test script -> assertions

Relu moi-même, assertion par assertion (chaque `TestTrue/Equal/False/NotNull` du C++ face au `T.Assert*` script, même valeur attendue).

| Test C++ | Test script | Assertions C++ / couvertes | Décision |
|---|---|---|---|
| DisconnectGameState (l.96-161) | `HOLDisconnect_Test.as` Test_DisconnectGameState | 22 / 22 (la raison est comparée en int 5 = `EHOLDefeatReason::Disconnected`, vérifié : None0 Surrender1 Cheat2 Forum3 King4 Disconnected5 ; `GetReasonName(5)` = "Disconnected" est assertée) | RETIRÉ |
| DisconnectBanner (l.243-333) | Test_DisconnectBanner | 33 / 33 (FormatAbsentLine / FormatBackLine directs, champs de la notice en texte tab-joint "1", "Bob", "190.000", "0.000", propriétés répliquées, lignes de bannière) | RETIRÉ |
| DisconnectHeroSheet (l.336-387) | Test_DisconnectHeroSheet | 13 / 13 (voir note 1) | RETIRÉ |
| DisconnectPause (l.799-855) | Test_DisconnectPause | 20 / 20 | RETIRÉ |
| DisconnectBannerClock (l.857-904) | Test_DisconnectBannerClock | 8 / 8 | RETIRÉ |
| DisconnectReportMode (l.968-1008) | Test_DisconnectReportMode | 6 / 6 | RETIRÉ |
| DisconnectFakeBack (test 12) | Test_DisconnectFakeBack | 13 / 12 | GARDÉ (voir note 2) |
| DisconnectRules (1), DisconnectLateReturn (15), WindowEnd | script quasi entiers | 1 assertion manquante chacun (constante `DefaultWindowSeconds == 180` ; `GetSlotLevel` ; mode Diplomacy) | GARDÉS |
| DisconnectGains, Technologies, Held, HeldStock, Replace | script en partie | voir ELROND-1119z (remboursements, journal, GetSlotLevel, IssueAIProduceOrder, PlayerArray) | GARDÉS |
| DisconnectTable (HOLDisconnectDisplayTests.cpp) | `HOLDisconnectDisplay_Test.as` Test_DisconnectTable | 10 / 10 (les 2 `TestFalse(a || b)` / `TestTrue(a && !b)` sont séparés en 2 assertions chacun, plus strict) | RETIRÉ |
| DisconnectMessage | Test_DisconnectMessage | 5 / 5 | RETIRÉ |
| DisconnectLeftIsDefeat | Test_DisconnectLeftIsDefeat | 10 / 10 (le dernier `Events.Num()==2 && Kind && Reason` est éclaté en 3 assertions) | RETIRÉ |
| DisconnectReport | Test_DisconnectReport | 7 / 7 | RETIRÉ |

Note 1 (HeroSheet, HeroSubsystem et stats) : `TestNotNull("hero subsystem")` du C++ n'a pas d'assertion jumelle explicite en script (le script passe par `HOLTestWorld::SetHeroXP`, qui rend vrai seulement si le sous-système existe, puis lit `GetHeroSheetXP == 12` du sous-système). Je l'ai jugée couverte par implication (une garde de précondition dont dépendent 3 assertions de valeur). Même raisonnement pour `TestNotNull("stats subsystem")` de LeftIsDefeat (`CountStatsDefeats == 1` exige le sous-système). Si tu veux le critère strict (une assertion nommée par garde), ces deux tests sont à remettre : `git revert 6fed720` partiel ou `git checkout 6fed720^ -- <fichier>`.

Note 2 (FakeBack gardé, "dans le doute") : le C++ compte les player states du siège 1 dans `PlayerArray` (`Seats == 1`, donc UNICITÉ : un doublon serait vu). Le script le remplace par `FindLiveHolder != nullptr`, qui prouve qu'il existe un détenteur vivant mais pas qu'il n'y en a qu'un. La substitution ne prouve pas la même chose, donc le test C++ reste entier. Pour le retirer : exposer le compte des player states d'un siège en script (déjà dans TYPES_A_EXPOSER de ELROND-1119z), puis remplacer l'assertion.

## Suppressions

- `Source\HeroesOfLiberty\Tests\HOLDisconnectDisplayTests.cpp` : fichier supprimé (`git rm`, 147 l.), ses 4 tests étant couverts en entier ; son espace de noms anonyme (`MakeFourHumans`) n'avait aucun autre appelant.
- `Source\HeroesOfLiberty\Tests\HOLDisconnectTests.cpp` : 4 blocs entiers retirés (commentaire + `IMPLEMENT_SIMPLE_AUTOMATION_TEST` + `RunTest` + lignes vides voisines) : l.96-161, 243-388, 799-905, 968-1009 = 361 lignes. `git diff --stat` : 361 suppressions, 0 ajout ; les 9 `IMPLEMENT_SIMPLE_AUTOMATION_TEST` restants relus (Rules, Gains, Technologies, Held, HeldStock, Replace, FakeBack, LateReturn, WindowEnd).
- Aides de fichier : `MakeNamedConfig` (10 emplois restants), `Flags` (10), `FCaptureLog` (Technologies) : toutes gardées, ont encore des appelants. Aucune aide devenue sans appelant. Les `#include` ne sont pas touchés (certains peuvent n'être plus utilisés, sans effet sur la compilation).

## Références restantes (Grep du worktree hors Saved / Intermediate / Binaries / Docs)

- Aucune dans `Tools\`, aucun `known_reds.txt` ni liste de tests ne cite un test retiré.
- Commentaires seulement (non modifiés, la tâche était un commit C++) : `Script\Match\HOLMatchFlow_Test.as:7-8` cite `FHOLDisconnectPauseTest, Tests/HOLDisconnectTests.cpp:800` (désormais retiré) ; les en-têtes de `Script\Tests\HOLDisconnect_Test.as`, `HOLDisconnect2_Test.as` et `HOLDisconnectDisplay_Test.as` disent « the C++ file stays » (périmé pour les 6 + 4 tests retirés) ; docs de suivi (`DisconnectInProgress.md`, `NetPlaytestInProgress.md`, `JourneyInProgress.md`) citent les noms à titre d'historique.

## Champs

BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 10 (HeroesOfLiberty.Match.DisconnectGameState, DisconnectBanner, DisconnectHeroSheet, DisconnectPause, DisconnectBannerClock, DisconnectReportMode, DisconnectTable, DisconnectMessage, DisconnectLeftIsDefeat, DisconnectReport)
TESTS_SCRIPT_AJOUTES: 0
LIGNES_CPP_SUPPRIMEES: 508 (361 dans HOLDisconnectTests.cpp + 147 pour HOLDisconnectDisplayTests.cpp supprimé)
LIGNES_MIGREES_PAR_LOT: 0
TYPES_A_EXPOSER: compte des player states d'un siège (pour retirer DisconnectFakeBack) ; le reste comme ELROND-1119z
TESTS_TAUTOLOGIQUES: 0
EDITS_REFUSES: 0

## Total quick_check attendu après le build

1755 - 10 = **1745** sur `integ-16` (le script en compte 1755 dont les 10 jumeaux restent ; les tests C++ retirés partent du total). Ce total BAISSE de 10 : c'est attendu, la règle « le total ne baisse jamais » porte sur une perte de couverture, ici chaque assertion est couverte par un jumeau script vu rouge puis vert (11:34 pour Display, 14:28 pour les 12 de déconnexion).

NON VÉRIFIÉ : compilation du fichier réduit (aucun build) ; que le build + quick_check donne bien 1745/1745 ; les deux gardes de sous-système (note 1) jugées couvertes par implication ; `GiveSlotStock` sans temps (C++) face à `GiveSlotStock(..., 0.0 ou 100.0)` (script) : même résultat attendu, comportement du défaut non relu dans `HOLGameState.cpp`.
Commande à lancer par la session principale : `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check` complet.
PROCESSUS_DETACHES: aucun lancé (pas de build ni de jeu).
