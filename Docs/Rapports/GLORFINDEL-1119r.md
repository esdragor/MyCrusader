# GLORFINDEL-1119r : retrait du C++ mort du tribut (R1)

Impact joueur : aucun (le tribut reel passe deja par HOLTribute::Settle en script) ; on retire du code mort.

AGENT: GLORFINDEL-1119r
WORKTREE: C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, branche net-playtest (partage avec 1119q ; seuls mes chemins ajoutes un par un)
COMMITS:
- 282252d : test script seul (Script\Tests\HOLTributeRules_Test.as, Test_TributeReceived, +3 assertions : 7->5, 99->70, 1000->700 a 30 %)
- 74d29eb : retrait C++ + surcharge script, un seul commit (5 fichiers, +7 / -56)
BUILDS_DEMANDES: 1 (celui du lot groupe de la session principale ; rien lance ici)
TESTS_SCRIPT_AJOUTES: 3 assertions dans Test_TributeReceived (pas de nouveau test)
TESTS_CPP_RETIRES: 0 test entier ; 10 assertions retirees de HeroesOfLiberty.Market.ServerBoard (ReceivedNow x3, Received statique x7), le reste du test (recover, SettleTribute) reste.
LIGNES_CPP_SUPPRIMEES: ~46 lignes C++ (HOLMarketSubsystem.cpp 21, .h ~14 hors commentaires, test 11) ; script -8 (surcharge TributeReceived). `git diff --stat` : -56 / +7 au total (commentaires reecrits compris).
LIGNES_MIGREES_PAR_LOT: 0
TYPES_A_EXPOSER: aucun
TEST_ROUGE: NON VU (aucun build/jeu autorise). Valeur a fausser pour le rouge : dans HOLTributeRules_Test.as, `T.AssertEquals(70, HOLTribute::Received(99, ...), "99 at 30 %...")` -> mettre 71 (attendu : 1 echec, Test_TributeReceived). Variantes : 5 -> 6 (ligne 7), 700 -> 701 (ligne 1000). Vecteurs verifies a la main contre HOLTributeRules.as:36-43 : 7-2=5, 99-29=70, 1000-300=700.
TESTS_TAUTOLOGIQUES: 0

## Fait
- HOLMarketSubsystem.h : retire `static Received`, `ReceivedNow`, `TributeReceived` (BlueprintNativeEvent) + `_Implementation` ; commentaire de classe reecrit (Settle applique Received).
- HOLMarketSubsystem.cpp : retire `Received`, `ReceivedNow`, `TributeReceived_Implementation`.
- HOLMarketBridgeTests.cpp : retire l'ancien bloc « tribute » (lignes 91-101) ; SettleTribute (ancien l.102+) conserve.
- HOLMarketBoardScript.as : retire la surcharge `TributeReceived` (l.62-66) + commentaire d'en-tete mis a jour.
- HOLMarketRules.h:15 : commentaire -> `UHOLMarketSubsystem::SettleTribute`.
- Grep `Source\`, `Script\`, `Tools\` apres retrait : plus aucun appelant de `ReceivedNow`, `UHOLMarketSubsystem::Received`, `TributeReceived`/`_Implementation`. Restent seuls `HOLGameState::NoteTributeReceived` (autre fonction, non touchee) et le nom du test `Test_TributeReceived`.
- Diff relu : suppressions de fonctions entieres, accolades equilibrees ; includes du test (`HOLMarketRules.h`, `HOLTribute::DefaultTaxPercent` encore utilise par SettleTribute) toujours necessaires.

## Reste
- Commande du lot groupe : build C++ du worktree puis quick_check -Parts des parties script/market (dont HeroesOfLiberty.Market.ServerBoard et Test_TributeReceived) ; faire le rouge (99 -> 71) avant ou sur un build separe si on veut le voir.
- Placeholders.txt : rien a changer (aucun provisoire touche). Auto_Journey_File.md ligne 47 (R1) : a cocher par la session principale apres le build vert.
- Pas de merge main, pas de push.

## NON VERIFIE
Compilation C++ et script, quick_check, rouge/vert : rien lance (consigne). processus detaches : aucun lance.

## Anomalies
Aucune. Le worktree est partage avec 1119q (un commit aa2c5d2 de lui s'est intercale entre les miens, sans conflit).
