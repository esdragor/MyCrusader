# GLORFINDEL-1122h : rejeu de Suivre, du clic dans la masse et de Soigner (sans C++, build PASS 17:25)

Impact joueur : suivre un allie marche et s'annule par un ordre manuel ; avec hol.Follow.OwnUnits 0 un clic sur une unite a soi est un deplacement ; un Moine soigne par clic droit sur un soldat blesse a soi.

suivi vu : OUI
deplacement garde : OUI
soin par clic prouve : OUI (soldat blesse) ; cas "unite non blessee" NON isole (voir C)
branche prete : OUI (ability-columns, tip e3ee5ad ; scenarios modifies non commites par moi : Tools\Scenarios\follow_short.txt, follow_crowd.txt dans le worktree)

APPELS_DECLARES: 22 vus (arret 25) ; premier Edit au 7e appel (tenu)
HEURE_DEBUT/HEURE_FIN: 17:26 / 17:33
PREMIER_BUILD: sans C++   BUILD: PASS 17:25 (donne par l'enonce, non relance)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenarios) ; TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 (scenarios follow_*), merge_main_fait: non (non demande)
TESTS: play follow_short x3 (37.5 s, 28.3 s, 24.5 s, tous finis par quit), play follow_crowd x1 (44.5 s, quit) = 4 passages. Erreurs "Angelscript ... rror" : 0, 0, 0, 0 ; "requires WorldContext" : 0, 0, 0, 0. TESTS_TAUTOLOGIQUES: 0
ESSAIS: LOGIQUE + PNG ouverts (follow_short_walk.png : Paysan selectionne en marche, trait jaune, 3 hommes derriere ; follow_crowd_heal_done.png : Moine 40/80 selectionne, groupe a gauche) ; pas d'E-*.md liste (geste interne hol.Select.*, -nosound)
QUESTIONS: aucune neuve (Retours\Q-follow-own-units.md inchangee, defaut 0 en vigueur)

## (A) follow_short (hol.Follow.OwnUnits 1, 3 passages)
Correctifs de scenario : ClickUnit -1 (Paysan #5) garde ; releves Where plus serres (wait 1 + Where, deux fois pendant la marche) ; le waitlog final "manual order ended" (qui expirait et laissait le jeu tourner) remplace par des `wait` : le scenario finit toujours par quit.
Passage 3, cible en marche (Paysan #5 : (-14810,4490) vers (-11791,5496)) :
- t+1 s : #1 (-14549,3100), Paysan (-14074,4735) : distance 1703
- t+2 s : #1 (-14191,3698), Paysan (-13310,4990) : distance 1564 (les 4 hommes "Moving" a chaque releve)
- depart : distance 2000 (hommes a (-14719,2470)) ; passage 2 : au contact (Paysan a l'arret) #1 a ~244 ; passage 1 : ~120 : distance bornee, qui ne diverge pas.
Annulation : ordre manuel (Select.Army + OrderNear 1500 3500) PENDANT la marche du Paysan : "FollowBridge: manual order ended the follow of 4 units" (ligne ECRITE 2 FOIS dans le journal : doublon d'appel, a regarder), "Order: 4 units to (-13291, 3500)" ; releves suivants : hommes (-13542,3445) puis arretes en (-13350..-13231, 3362..3638), Paysan arrete a (-11791,5496) : 2400 uu plus loin, ils ne reviennent pas.
Constat de conception (non tranche) : le suivi S'ARRETE AU CONTACT. Passage 2 : les hommes arrives (Idle) ne suivent PAS un 2e trajet du Paysan (la ligne "manual order ended" n'est alors jamais ecrite). A confirmer par le proprietaire de Suivre si c'est voulu (AoE4 : l'unite suit tant que la cible vit).

## (B) follow_crowd (hol.Follow.OwnUnits 0, 1 passage)
Les trois clics (corps d'un Paysan a soi (90,2500), homme selectionne (0,2500), sol a gauche de la masse (-700,2500)) : "Order: 10 units to (-14709, 2495)", "... (-14795, 2497)", "... (-15497, 2499)" ; aucune ligne "follow started" ni "FollowBridge" dans tout le journal. Les hommes se deplacent (et se dispersent, damier serre).
Correctif de scenario : les trois `waitlog Order: 10 units to` remplaces par `wait 1` (un waitlog expire bloquait le scenario ; le controle se fait au journal).

## (C) Soigner par clic (partie Moine de follow_crowd, meme passage)
Moine #21 (80 PV, hol.Cheat.Wound 50 : 40/80), soldats #22 et #23. hol.Cheat.Wound 50 blesse TOUTE la faction ("Cheat: slot 0 wound to 50 %, 23 units wounded", HOLControllerCommands.as:185, ServerCheatWound) : #22 et #23 etaient tous deux blesses.
- Avant tout ordre : "Heal: step 266 #21 heals #22, health 77.50/155" : le Moine soigne seul (auto) le blesse le plus proche.
- Clic droit du Moine sur #23 (hors selection) : "Order: 1 units heal #23 ManAtArms of slot 0", "RpcOrder: heal unit received ... target 23", "Heal: step 271 #21 stops in reach of #23 at 600 cm (heal range 800 cm, at range)", "heals #23 ... (ordered)" ; PV #23 : 78 -> 98.
- Clic droit sur #22 : "Order: 1 units heal #22", "Heal: step 352 ... heals #22 (ordered)" ; PV #22 : 79 -> 109 -> 139 (+30 par releve de 6 s) ; #23 reste a 98 (le Moine l'a quitte).
- Cas "unite non blessee" : NON VERIFIE (aucune unite saine). Pour le lire : dans follow_crowd.txt, faire le Spawn de l'unite saine APRES `hol.Cheat.Wound 50` (passage non fait : 4 passages utilises).

NON VERIFIE : soin d'une unite saine ; suivi d'un coequipier (un joueur) ; doublon de la ligne "manual order ended" ; arret du suivi au contact (voulu ?).
PROCESSUS_DETACHES: aucun (4 jeux caches finis par quit)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seuls (EtapeDefini.md, Auto_Journey_File.md, OrdersPatrolInProgress.md, ce rapport) : oui
