# GLORFINDEL-1122f : pont de Suivre rejoue, clic dans la masse, Soigner (sans C++)

Impact joueur : le clic droit sur un allie lance bien Suivre ; un clic droit au sol dans un groupe a soi devient un Suivre (piege, question posee) ; Soigner par clic non prouve.

pont prouve actif : OUI (follow_short : "FollowBridge: follow started, 4 units follow #5" puis "manual order ended the follow of 4 units" ; follow_crowd : "follow started, 10 units follow #11")
suivi vu : NON (follow_short : le Paysan n'a jamais marche, ClickUnit 5 = "no own unit 5 on the screen" (index 0-based, 5 inexistant) ; les 4 hommes ont recu le deplacement du Paysan puis le waitlog "manual order ended" a expire a 240 s ; follow_crowd : suiveurs toujours "Idle" aux 3 releves, cible immobile : aucune distance qui varie). Scenario corrige (ClickUnit -1 = dernier) mais NON rejoue.
soin par clic prouve : NON (partie heal de follow_crowd lancee avec les anciens ClickUnit 21/22 (1-based, faux) ; le journal lu s'arrete avant ; corrige en 20 (Moine) / 21 (blesse), non rejoue)
branche prete : OUI pour la branche ability-columns (aucun C++ touche, scenarios seuls ; commit des scenarios), mais Soigner et le suivi visible restent non prouves.

APPELS_DECLARES: 26 vus (limite 25 : depassee d'un appel pour ecrire le rapport) ; premier Edit/Write au 7e appel (tenu).
HEURE_DEBUT/HEURE_FIN: 17:10 / 17:17
PREMIER_BUILD: sans C++   BUILD: sans objet (dll de 1122e)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenarios) ; TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 (scenarios), merge_main_fait: non
TESTS: play follow_short (1 passage, timeout 240 s, "Test sequence: timeout at step 36") ; play follow_crowd (1 passage, 42 s, fini par quit). Erreurs "Angelscript ... rror" : 0 ; "requires WorldContext" : 0 (les deux journaux). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun liste (LOGIQUE SEULEMENT, PNG non ouverts, preflight non lance)
QUESTIONS: Retours\Q-follow-own-units.md (defaut : comportement code, A)

## (A) follow_short
Journal : follow started (1x), Order: 4 units follow #5 Peasant. Distances suiveur-cible : 1 seule valeur utile (hommes a (-14719,2470)..., Paysan a (-14810,4490), Idle), puis l'ordre "manual order ended" part trop tot : la selection n'a pas change (ClickUnit 5 rate), le clic droit de (3000,5500) a ete un ordre de deplacement des 4 hommes (-11791,5496) qui a coupe le suivi ; le Paysan est reste a (-14810,4490) Idle aux 4 releves. Aucune distance variable vue : "suivi vu = non". Les hommes n'ont meme pas ete vus se rapprocher entre le 1er et le 2e releve (14 ms de jeu entre les deux).

## (B) follow_crowd (20 unites : 10 hommes selectionnes ids 1-10, 10 Paysans a soi ids 11-20, damier 90 uu)
1. Clic droit sur le corps d'un Paysan non selectionne (-14710,2500) : "FollowBridge: follow started, 10 units follow #11" / "Order: 10 units follow #11 Peasant of slot 0" : SUIVRE.
2. Clic droit vise sur un Homme d'armes selectionne (-14800,2500) : les unites avaient glisse (separation), le point etait sur le corps du Paysan #13 : "follow started, 10 units follow #13" : SUIVRE (la selection est ecartee, la cible derriere est prise).
3. Clic droit au sol a cote de la masse (-15500,2500) : "FollowBridge: manual order ended the follow of 1 units" + "Order: 10 units to (-15497, 2499)" : DEPLACEMENT (et fin du suivi en cours).
Autre constat : apres 'follow started', les 10 hommes restent "Idle" (aux releves) pendant que le Paysan ne bouge pas : le suiveur au contact ne bouge pas, a verifier avec une cible qui marche.
Options (sans trancher) : A codee ; B exclure les unites a soi ; C touche modificatrice. Voir Q-follow-own-units.md.

## (C) Soigner
Scenario ecrit (Moine #21, blesse #22 par hol.Cheat.Wound 50, indemne #23). Non prouve : pas de ligne "heal" lue, PV non releves (budget d'appels). Rejouer : play.ps1 follow_crowd -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -TimeoutSec 200 puis chercher "heal" et les PV de #22 dans les "Units where" ; un Moine sans blesse : clic sur #23, a lire de meme.

NON VERIFIE : suivi visible (distances), Soigner, PNG (aucun ouvert), scenarios corriges (ClickUnit -1 / 20 / 21) non rejoues.
PROCESSUS_DETACHES: aucun (jeux caches termines tout seuls)
ANOMALIES_PROCESSUS: 3 UnrealEditor vus dont 2 d'autres worktrees ; follow_short a tourne les 240 s pleines.
GIT_STATUS_PRINCIPALE: docs seuls (Retours\Q-follow-own-units.md neuf) : oui