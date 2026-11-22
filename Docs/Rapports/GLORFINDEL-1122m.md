# GLORFINDEL-1122m : tout ordre manuel du joueur termine le suivi (D818), NON COMPILE

APPELS_DECLARES: 14 (arret a 20)
HEURE_DEBUT/HEURE_FIN: 19:06 / 19:10
PREMIER_BUILD: sans build (consigne : NE COMPILE PAS)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun commit de test, assertit l'effet : non   TEST_ROUGE_SEPARE: non. Raison : l'effet (un suivi en cours se termine apres l'ordre) demande un monde avec unites et un livre de suivi rempli par le script ; il n'existe pas de point d'entree C++ pour demarrer un suivi sans unites reelles (NativeRightClickOnUnit passe par le script), et un rouge ne se prouve qu'a l'execution, impossible sans build. Un test C++ ecrit a l'aveugle risquait de casser le build. Prochaine etape : test d'effet dans HOLFollowBridgeTests.cpp (suivre, puis RequestGatherUnits / RequestGarrisonUnits, relire le livre).
COMMITS: 1 (code), merge_main_fait: non (merge integ-52 fait en premier geste, sans conflit)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
RELECTURE_CPP: diff relu (8 insertions, 0 suppression ; BOM non ajoute, fins de ligne gardees). `Builders` est une TArray<uint32> convertie en TConstArrayView ; `GetCommandUnits()` renvoie une TArray par valeur (temporaire OK pour TConstArrayView). Aucun nouveau include (HOLFollowBridge.h deja inclus pour les 6 sites existants). Non verifie par compilateur.

## Sites modifies (HOLPlayerController.cpp, Request*, un appel chacun)
- RequestGatherUnits (recolte)
- RequestBuildUnits (construction : aider un chantier)
- RequestPlaceBuilding (bati : `Builders = GetCommandUnits()`, appel dans le if `!Builders.IsEmpty()`)
- RequestPlaceWallLine (ligne de mur : `GetCommandUnits()`)
- RequestDropOffUnits (depot)
- RequestGarrisonUnits (garnison)
- RequestAttackBuilding (attaque de batiment)
- RequestToggleDeploy (deploiement : occupe l'unite, barre D869b)
Les 7 sites d'avant (Move, AttackUnit, HealUnit, AttackMove, Stop, MoveInLine + debut de OrderAt) sont inchanges.

## Doublon : pas de nouveau message
OrderAt (clic droit) appelle deja NativeManualOrder (l.1475) avant ses branches, qui passent ensuite par Request* : sur un clic droit l'appel est donc fait deux fois. C'est sans effet visible : `EndFollows` est idempotent (le 2e appel ne trouve plus de suivi, renvoie 0) et `HOLFollowBridge.as` n'ecrit « manual order ended » que si `Ended > 0` : UNE ligne par ordre. L'appel de OrderAt est conserve (il precede la branche Follow qui demarre un nouveau suivi, et couvre le clic droit meme si une branche ne passe pas par Request*). Les boutons, touches et la bibliotheque de commandes (HOLControllerCommandLibrary) passent tous par Request*, donc sont couverts.

## Sites ecartes (aucun deplacement de l'unite)
- RequestHeroAbility (CloseRanks cri, Banner) et RequestCastRage : capacites instantanees lancees sur place.
- RequestUngarrisonAll / RequestUngarrisonUnit : sort des unites de la garnison, aucune unite qui suivait n'est concernee.
- RequestRallyPoint, RequestSetRallyPointOf : reglage de batiment.
- RequestCancelBuilding, RequestProduction, RequestCancelProduction, RequestHeroTalent, RequestSpecialization, RequestStance, RequestShareVision, RequestTribute, RequestMarketTrade, RequestSignal, RequestChat, RequestDuel*, RequestSurrender, RequestReturnHero, RequestPause/Resume : economie, interface, diplomatie.
- RequestCheat* : outils de test.
- Suivi lui-meme : ne passe pas par Request* (script, RightClickOnUnit).
A confirmer par le design : Banner (cible un Location : le heros bouge-t-il ?) reste ecarte faute de preuve qu'il deplace l'unite.

## Soigner et le Moine qui suit
Comportement actuel conserve, non modifie : RequestHealUnit appelait deja NativeManualOrder (GLORFINDEL-1121sm), donc un Moine qui suit et recoit un ordre manuel de soin QUITTE son suivi. Le soin automatique du suiveur est emis par `HOLFollowBridge::Step` (script) et ne passe pas par Request*, donc ne met pas fin au suivi.

QUESTIONS: aucune
FAIT / RESTE: fait = 8 sites + docs (EtapeDefini, OrdersPatrolInProgress). Reste = build C++, test d'effet, rejouer follow_short avec un ordre de recolte pendant le suivi.
NON VERIFIE: tout (aucune compilation, aucun test, aucun scenario).
PROCESSUS_DETACHES: aucun (aucun lancement)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non releve (aucune ecriture hors EtapeDefini.md)
