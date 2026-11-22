# GLORFINDEL-1122j : rejeu follow_short / follow_crowd apres 1122i (scenarios seuls)

Impact joueur : des soldats qui suivent un allie s'arretent a son contact, repartent quand il repart, et un ordre manuel termine le suivi une seule fois sans retour.

## Resultats (build 17:25, .as de 1122i charges)
- (1) 0 erreur "Angelscript ... rror", 0 "requires WorldContext" sur les 4 lancements (3 follow_short, 1 follow_crowd). Aucune erreur de compilation sur ConsumeMoves(...).Num() : le piege n'a pas joue.
- 2e trajet suivi : OUI. Relevés (Paysan #5, hommes #1-#4) : avant (contact apres trajet 1) Paysan (-11791,5496), hommes a 100-300 uu ; pendant le trajet 2 (+3 s) hommes Moving a (-13550..-13165, 5444..5582), Paysan arrive (-13318,5534) ; apres (+12 s) hommes Idle a (-13168..-13435, 5483..5694), a 100-300 uu du Paysan (-13274,5586). Les hommes ont donc parcouru ~1700 uu avec lui apres s'etre arretes au contact.
- Une seule ligne de fin : OUI. "FollowBridge: manual order ended the follow of 4 units" x1 (3e trajet du Paysan, ordre manuel des hommes vers (-13292,3497)). Les hommes finissent a (-13292,3497) Idle, le Paysan a (-11791,3496) : ils ne reviennent pas.
- (4) cible morte : NON FAIT (hol.Cheat.Wound blesse toutes les unites, ne tue pas ; pas de passage restant).
- Soin d'une unite saine : OUI, follow_crowd. hol.Cheat.Wound 50 blesse TOUTES les unites (22 blessees) ; l'unite saine #23 est creee apres (hp absent = plein). Ordre "heals #23 first" : le Moine s'arrete a portee (600 cm), aucun PV ne change sur #23, aucune erreur. Le Moine soigne ensuite #22 (78 -> 155) puis #19 en soin auto.
- follow_crowd : 3 clics = "Order: 10 units to" (OwnUnits 0), aucun "follow started". Les waitlog "Order: 10 units to" passent sans timeout.

## Corrections de scenarios (731c931)
- follow_short : trajets 1, 2 (apres FocusNear 1500 4500, ClickUnit -1, OrderNear 1500 5500) et 3 (Paysan reste selectionne, sinon ClickUnit -1 tombait sur un homme au contact : constate au 2e passage, un ordre de 1 homme), relevés avant / pendant / apres, ordre manuel pendant le 3e trajet. Un 1er essai avec OrderNear 6000 5500 n'a emis aucun ordre (point hors champ).
- follow_crowd : Moine + Homme d'armes #22 crees, hol.Cheat.Wound 50 (sur tout), puis creation de #23 saine.

2e trajet suivi : oui
une seule ligne de fin : oui
soin d'une unite saine : oui
branche prete : oui (ability-columns, 731c931 au-dessus de ece3ce2 ; merge main non fait)

APPELS_DECLARES: 14 vus a l'ecriture (arret 25 ; arret dur 18:02 ; heures Get-Date 17:48 -> 17:54)
HEURE_DEBUT/HEURE_FIN: 17:48 / 17:54
PREMIER_BUILD: sans C++   BUILD: sans objet (dll 17:25)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenarios) ; TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 (731c931 17:54), merge_main_fait: non
TESTS: play follow_short x3, follow_crowd x1 ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun
ESSAIS: aucun liste ; PNG non ouverts ; LOGIQUE SEULEMENT
QUESTIONS: aucune
FAIT / RESTE: voir resultats ; reste : cible morte, deux NativeManualOrder par clic droit cote C++ (non necessaire)
NON VERIFIE: cible morte, captures PNG, DIAGNOSTICS_AS (pas de serveur de langage)
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: crashpad_handler et zenserver fermes par play.ps1 au 3e lancement
GIT_STATUS_PRINCIPALE: docs seuls (EtapeDefini.md, Auto_Journey_File.md, OrdersPatrolInProgress.md + fichiers deja la) : oui
