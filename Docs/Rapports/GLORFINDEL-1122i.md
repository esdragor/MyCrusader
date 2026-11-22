# GLORFINDEL-1122i : Suivre = garde du corps, le suivi ne s'arrete plus au contact (script seul, NON CHARGE)

Impact joueur : des soldats qui suivent un allie s'arretent a son contact puis repartent quand il repart (avant : le suivi etait supprime au premier arret) ; un ordre manuel le termine toujours, une seule ligne de journal.

TEST_ROUGE_SEPARE: oui (725873c : stubs qui compilent + 4 tests, commit accepte par le classifieur). TEST_ROUGE: 725873c, assertit l'effet : oui (le suiveur obeit aux Move de la file ; lecture de la file ConsumeMoves) ; rouge NON VU a l'execution (aucun chargement des .as dans cette passe : integ-51 tourne) ; rouge attendu :
- Test_FollowOrdersFollowerStaysAtContactThenWalksAgainWithTheAlly : ROUGE sur le stub (StepFollower renvoie false des l'arret au contact : bEnded vrai, MovesSecondLeg 0).
- Test_FollowOrdersEndFollowsCountsOnlyFollowsThatReallyEnded : ROUGE sur le stub (2e appel renvoie 2 au lieu de 0).
- Test_FollowOrdersManualOrderEndsTheFollowForGood et Test_FollowOrdersStepUnitTargetGoneEndsTheFollow : VERTS sur le stub (garde-fous : ils doivent le rester apres le correctif).

## Diagnostic : defaut, pas voulu
- Aucun test ni decision n'ecrit que le suivi finit au contact. Seul test voisin : HOLFollowRules_Test.as:44 (Test_FollowAllyStandsStillWhenClose) = Decide renvoie None de pres (pas d'ordre), pas une fin du suivi. D818 Q2 B (garde du corps) va dans le sens contraire.
- Cause : HOLFollowOrders.as StepUnit (ancienne ligne 166) renvoyait le bool de StepBodyguard / Step = "un ordre est entre dans la file". HOLFollowBridge.as:47-56 (Step du pont) retire du livre tout suiveur dont StepUnit renvoie false. Au contact, Decide = None (HOLFollowRules.as:43), aucun ordre, false : l'entree du livre est supprimee, le 2e trajet de la cible n'est jamais suivi. Meme effet pour un Attack deja file sur le meme ennemi (LastAttackIds, Orders.as:101) : le suivi tombait aussi.
- Le commentaire de StepUnit disait "false quand l'unite ou la cible a disparu" : contrat violé par la valeur renvoyee.

## Correctif (7f2bdee)
- HOLFollowOrders::StepFollower(Commands, PlayerIndex, Book, UnitId, UnitLocation, TargetLocation, bPlayerOrderActive, EnemyId) : appelle StepBodyguard puis renvoie `Book.States[Index].bActive` (vrai tant que le suivi dure, meme sans ordre ce pas ; faux apres un ordre manuel). StepUnit l'appelle : faux reste pour unite ou cible disparue (cible morte / garnisonnee) via ReadUnit.
- Le pont (HOLFollowBridge.as) n'a pas change de logique de retrait (commentaire seul) : une entree inactive (ordre manuel) est toujours retiree au pas suivant.

## Doublon "FollowBridge: manual order ended" (2 fois dans le journal, 1122h)
- Source C++ : NativeManualOrder est appele a CHAQUE ordre, et un clic droit en appelle deux : HOLPlayerController.cpp:1449 (avant la resolution du clic) puis le chemin d'ordre (:2933, :2942, :2951, :3002, :3011, :3030 selon l'ordre ; deplacement au sol = un de ceux-la). Pas modifie (C++).
- Cote script : le pont comptait toute entree encore presente dans le livre, meme deja terminee (OnManualOrder ne retire pas l'entree, il met bActive a false) : le 2e appel recomptait. Corrige : HOLFollowOrders::EndFollows(Book, UnitIds) ne compte que les follows actifs ; le pont l'appelle (HOLFollowBridge.as:25) : le 2e appel renvoie 0 et reste silencieux. Test_FollowOrdersEndFollowsCountsOnlyFollowsThatReallyEnded.
- Reste possible cote C++ : supprimer l'un des deux appels (lot C++ de la session principale, non necessaire pour la ligne unique).

## Scenarios (ece3ce2)
Le waitlog ne lit que les lignes ecrites depuis la derniere commande (HOLTestSequence.cpp:35, :646) : il se place donc IMMEDIATEMENT apres la commande qui ecrit la ligne, sans `wait` entre les deux.
- follow_short.txt : `hol.Select.OrderNear 1500 3500` puis `waitlog FollowBridge: manual order ended` (a la place du `wait 1` de 0751ab6), puis `wait 1`. Commentaire du scenario complete ("corrige en 1122i, non rejoue").
- follow_crowd.txt : les trois `OrderNear` (90 / 0 / -700) sont suivis de `waitlog Order: 10 units to` (a la place des `wait 1`), puis `wait 3`.
- Risque : si la ligne n'est pas ecrite, le scenario s'arrete sur le delai du waitlog (c'est le but : plus de faux vert) ; si le waitlog d'Order: expire encore comme en 1122h, remettre `wait 1` en cause = le journal de ce passage.

## Pieges 1 a 9 relus (brief)
`Cast` non utilise ; pas de conteneur imbrique ; variables locales avant AssertEquals (First / Second, bLasts) ; pas de fonction globale hors namespace ; noms nouveaux dans `namespace HOLFollowOrders` ; fichiers dans Script\Orders (pas de dossier saute). Appel de `HOLCommandScript::ConsumeMoves(Commands).Num()` direct dans AssertEquals (Test_..._TargetGone) : un tableau temporaire, pas une methode de classe script : a surveiller au chargement (un chargement refuse = affecter d'abord a une variable locale).

APPELS_DECLARES: 20 vus (arret 25 ; heures Get-Date 17:36 -> 17:39 ; premier Edit au 7e appel, tenu)
HEURE_DEBUT/HEURE_FIN: 17:36 / 17:40
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, aucun chargement)   LAUNCHCHECK_DEBUT: sans C++
COMMITS: 3 (725873c rouge, 7f2bdee vert, ece3ce2 scenarios), dernier 17:39 ; merge_main_fait: non (non demande, passe integ-51 en cours)
TESTS: aucun lance (pas d'editeur, pas de quick_check) ; TESTS_SCRIPT_AJOUTES: 4 (Test_FollowOrdersFollowerStaysAtContactThenWalksAgainWithTheAlly, Test_FollowOrdersManualOrderEndsTheFollowForGood, Test_FollowOrdersStepUnitTargetGoneEndsTheFollow, Test_FollowOrdersEndFollowsCountsOnlyFollowsThatReallyEnded) ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun
ESSAIS: aucun (pas de rejeu : integ-51 en cours) ; a faire apres chargement : quick_check -Parts tests (rouge a voir sur 725873c : 2 tests rouges attendus, puis vert sur 7f2bdee), puis play follow_short -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel (le Paysan s'arrete, les hommes arrivent, il repart : les hommes le suivent ; ajouter un 2e trajet du Paysan au scenario, non fait ici)
QUESTIONS: aucune
FAIT / RESTE: voir ci-dessus ; reste : chargement des .as, rejeu, 2e trajet dans follow_short, eventuellement retirer l'un des deux NativeManualOrder (C++).
NON VERIFIE: tout (aucun chargement, aucun test lance) ; le retrait du livre d'une cible garnisonnee ; DIAGNOSTICS_AS : non relus (pas de serveur de langage dans la passe).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune. Note : edit_line.ps1 refuse un -NewLine contenant des guillemets doubles (liaison de parametres) : texte sans guillemets.
GIT_STATUS_PRINCIPALE: docs seuls (EtapeDefini.md, OrdersPatrolInProgress.md, ce rapport, plus Auto_Journey_Jour.md et les rapports non suivis deja la au depart) : oui
