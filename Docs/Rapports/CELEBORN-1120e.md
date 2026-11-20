# CELEBORN-1120e : scenario faction_slot (emplacement de faction du HUD de l'Aube)

NON COMPILE : aucun build, aucun preflight, aucun editeur (consigne). Scenario non rejoue, PNG non vus.

APPELS_DECLARES: 19 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 10:30 / 10:33
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario, pas de regle neuve) ; assertit l'effet : non applicable
COMMITS: 1 (70e953b, duel-rewards, 10:32), merge_main_fait: non (merge integ-26 fait en premier geste, a jour)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: faction_slot : preflight NON LANCE, PNG NON VUS, LOGIQUE SEULEMENT. Commande apres compilation des scripts : `Tools\Scenarios\preflight.ps1 faction_slot` puis `play.ps1 faction_slot -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
QUESTIONS: aucune

FAIT
- Tools\Scenarios\faction_slot.txt / .look / .json (70e953b). JSON : version 2, Plain, mapSize Small, 5 paysans, heros, slot 0 Human DawnOrder, slot 1 AI BlackLegions gele (aiBrain false) ; modele specialization_cheat + legions_roster_gesture_dawn.
- Gestes : waitlog heros slot 0, hol.AI.ThinkSteps, hol.Sanity.Watch 1, Camera.FocusSlot 0 ; puis pour N = 30, 100, 0 : `hol.Cheat.Honor N`, waitlog `RpcCheat: honor set to N for sender slot 0` (HOLRpcPlayer.as:1055), `hol.HUD.Report`, waitlog `faction slot: slot gauge N / 100 powers SacredLight cost 25` (HOLGameScreenScript.as:2614 + HOLFactionSlot::Make), notereset + capture faction_slot_30 / _100 / _0, waitkey entre les captures, hol.Test.Sanity a la fin.
- .look : l'objet attendu nomme (la barre d'Honneur de l'Aube, 30 sur 100, au-dessus du bloc du heros), puis pleine, puis vide.

RESTE
- Compiler les scripts, lancer le preflight, jouer en -Hidden et OUVRIR les 3 PNG (position du slot : 150 px provisoire au-dessus du bloc du heros).

NON VERIFIE
- Tout : le scenario n'a jamais tourne.
- Le waitlog sur le texte du pouvoir s'arrete a `SacredLight cost 25` (sans l'etat) car l'etat depend de bEffectCoded (HOLDawnPowers.as le laisse false = "soon" ; HOLFactionPowerCast.as, arrive par integ-26, pourrait le changer) : je n'ai pas lu ce fichier.
- Cheats actifs en partie solo : specialization_cheat.txt n'a aucune ligne qui les active, j'ai suppose le meme comportement (hol.Cheat.Honor passe par HOLRpcBridge::CheatRefused). Si la ligne `RpcCheat: honor refused (1)` apparait, les cheats sont coupes.
- Le champ "mapSize" combine avec "version": 2 : aucun autre scenario n'a les deux (supporte par le lanceur ? non verifie).
- Honneur au depart (non lu) : le scenario ne s'y fie pas (premier report apres Honor 30).
- Les waitlog avancent-ils par curseur (une seule fois par texte) ? Suppose comme dans specialization_cheat (deux `waitlog HUD overlap:` successifs).
- hol.HUD.Report est-il une seule ligne de journal ou plusieurs (le `\nfaction slot:` de Describe) : waitlog en sous-chaine, devrait passer dans les deux cas.

BESOIN C++ / TYPES_A_EXPOSER: aucun ; BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0
Commande ou ligne de journal manquante : aucune (hol.Cheat.Honor, "RpcCheat: honor set to", "faction slot:" existent).
Pas de slot Legions dans ce scenario (GALADRIEL-1120d proposait un rejeu avec un slot Legions : aucun panneau) : non ecrit.

PROCESSUS_DETACHES: voir la fin du rapport (stray_processes lance en dernier geste)
ANOMALIES_PROCESSUS: aucune ; aucune ecriture de code dans la copie principale (ecritures par l'outil Write avec chemins absolus dans le worktree).
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress.md, Auto_Journey_File.md, EtapeDefini.md et les docs deja modifies) : oui
