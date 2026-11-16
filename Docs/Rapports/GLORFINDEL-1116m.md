# GLORFINDEL-1116m : migration, pilote 1 (le chat), 2026-11-16 soir

APPELS_DECLARES: 50 (arrêt au 44e appel de travail, puis clôture ; la limite de 45 est dépassée de 5 appels, à cause de la clôture et de l'attente du build)
HEURE_DEBUT/HEURE_FIN: 20:11 / voir la ligne BUILD (clôture vers 20:35)
PREMIER_BUILD: 20:26   BUILD: voir l'addendum en bas (lancé à 20:26:23 après session_gate Launch PASS)   LAUNCHCHECK_DEBUT: non lancé avant le code (écart à la règle 10). Binaire donné à jour à 19:23 et confirmé par le compte de tests de 20:20 (Chat.Observer absent). session_gate de 20:26 : « DLL 19:23 >= dernier commit de code de main 17:21 PASS ».
TEST_ROUGE: P1a : rouge VU à 20:20, tests 1326/1332 (KO : Test_ChatGateRefusals et Test_ChatGateRate contre un bouchon CheckPost qui renvoie Ok ; Test_ChatRouteBySeats et Test_ChatObserverBySeats avec une valeur attendue faussée, "1,2" pour "1,0" et "-1" pour "-1,2"), puis vert à 20:25 (1330/1332), commit a9e8811 (le rouge seul n'est pas commité). Le test assertit l'effet : oui (les destinataires calculés par RecipientsBySeats, les réponses calculées par CheckPost). P1b : rouge de Chat.ServerBoard pas encore vu.
COMMITS: 1 (a9e8811 à 20:26) ; merge_main_fait : oui à 20:12 (avance rapide, rien à fusionner à la fin)
TESTS: quick_check -Parts tests : 20:20 1326/1332 (rouge voulu), 20:25 1330/1332. Les 2 KO restants sont HeroesOfLiberty.Match.StartingHero.NomadFirstForum et .NomadReplicated, rouges D831 venus de main (ETAPE0 les annonce ; ils ne sont pas dans known_reds.txt, d'où « REGRESSION »). TESTS_TAUTOLOGIQUES: 0.
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- FAIT MIG-P1a a9e8811 : Script\Chat\HOLChatRules.as donne la vue du serveur : matrice d'alliances MateByPair (From*Slots+To, non transitive), ReachesBySeats, RecipientsBySeats ; la règle par équipes devient un cas (MatesFromTeams). Garde CheckPost (Ok..BadTarget ; 200 caractères, 5 messages par 10 000 ms). Quatre tests script portés avec les mêmes vecteurs.
- ÉCRIT, NON COMMITÉ, build lancé : MIG-P1b. Pont neuf Source\HeroesOfLiberty\Match\HOLChatSubsystem.h/.cpp (UHOLChatSubsystem abstrait, FHOLChatSeats, FHOLChatGateAnswer, ReadSeats et ParseChannel déplacés). ServerPostChat passe par le pont : sans board en script, la réponse est -1 et rien n'est envoyé. Les cvars hol.Chat.* valent 0 par défaut, ce qui veut dire « défaut du script ». git rm de HOLChatRules.cpp/.h et de Docs\Patches\D806_cpp_twin_observer.patch. Chat.Route et Chat.Gate retirés, Chat.ServerBoard ajouté, Chat.ThreadLine gardé (constantes de canal du pont). ValueSheet comm.chat.* lit désormais Script/Chat/HOLChatRules.as.
- RESTE P1b : poser Script\Chat\HOLChatBoardScript.as (copie : Docs\Patches\HOLChatBoardScript.as.txt dans le worktree), d'abord en bouchon pour voir le rouge de Chat.ServerBoard, puis la vraie version ; ensuite -Parts tests,net, puis le commit.
- RESTE P1c : mesure sans build (MaxTextLength = 1 dans le .as : Chat.ServerBoard doit devenir rouge, puis remettre 200) et attentes de netsig3.
- RESTE P1d : non commencé.
- Branche net-playtest : PAS prête (P1b n'est pas commité).
NON VÉRIFIÉ: le build de P1b (voir l'addendum) ; HOLChatBoardScript.as jamais compilé : signature BlueprintOverride avec FHOLChatGateAnswer en retour et const TArray<int>& en paramètre ; le chat sur le réseau (netsig3) après la bascule ; la RPC en script (P1d).
PROCESSUS_DETACHES: voir l'addendum (stray_processes est lancé en dernier geste)
ANOMALIES_PROCESSUS: (1) Deux Edit refusés pour « fichier non lu » sur HOLChatRules.as, refaits après Read. EDITS_REFUSES: 2. (2) Mes remplacements PowerShell ont retiré le BOM de quick_check.ps1 (WriteAllText sans BOM) ; je l'ai remis avant tout lancement. (3) Je n'ai pas lancé LaunchCheck avant le code. (4) La limite de 45 appels a été atteinte avant P1b vert : le build part à 20:26 alors que le budget était presque épuisé ; il aurait fallu moins d'appels de lecture.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md (ma ligne et celle d'avant), ?? Docs/Priorite_Socle_Factions_2026-11-16.md, ?? Docs/Rapports/ARWEN-1116m.md, ?? Docs/Rapports/ETAPE0-1116-MIG.md (pas à moi), plus ce rapport ; EtapeDefini.md modifié (ma ligne 93). Inchangée hors docs voulus : oui.

## Champs de migration
BUILDS_DEMANDES: 1 (P1b, 20:26)
TESTS_CPP_RETIRES: 2 (HeroesOfLiberty.Chat.Route, HeroesOfLiberty.Chat.Gate). Retirés dans l'arbre seulement, pas commités. Chat.Observer n'a jamais existé dans les sources.
TESTS_SCRIPT_AJOUTES: 4 (Test_ChatRouteBySeats, Test_ChatGateRefusals, Test_ChatGateRate, Test_ChatObserverBySeats), plus 1 test C++ du pont (HeroesOfLiberty.Chat.ServerBoard, non commité)
LIGNES_CPP_SUPPRIMEES: 293 (HOLChatRules.cpp 133 + .h 69 + HOLPlayerController.cpp 16 + HOLChatTransportTests.cpp 75) contre 259 ajoutées (pont 195 + contrôleur 22 + tests 42), soit −34 lignes nettes de C++
LIGNES_MIGREES_PAR_LOT: P1a : +222 lignes de script (règle 104, tests 118) ; P1b : +26 (board script, non posé)
DURÉE DU LOT: P1a de 20:12 à 20:26 (14 min, dont deux passages de tests de 3,5 min) ; P1b écrit de 20:17 à 20:26, puis le build
RPC EN SCRIPT: non vérifié (P1d non fait)
TYPES_A_EXPOSER: aucun pour le chat : FHOLChatSeats et FHOLChatGateAnswer sont des USTRUCT neufs, UHOLChatSubsystem expose BlueprintNativeEvent. Pour P1d, à vérifier : Server et Client en UFUNCTION de script avec Iris.

## Registre nom à nom
| Test C++ | Test script en regard | Statut |
|---|---|---|
| Chat.Route (9 assertions) | Test_ChatRouteBySeats (9) | retiré dans l'arbre, pas commité |
| Chat.Gate (5 refus + cadence) | Test_ChatGateRefusals (6 + refus non comptés) + Test_ChatGateRate (cadence + défaut 201) | retiré dans l'arbre, pas commité |
| Chat.Observer (patch, 10 assertions) | Test_ChatObserverBySeats (10) | patch git rm, pas commité |
| Chat.ThreadLine | (reste en C++ : délégué natif OnSignalMessage) | gardé |
| (neuf) Chat.ServerBoard | pont C++ vers le script | rouge pas encore vu |
Total de quick_check tests : 1328 avant, puis 1332 avec P1a ; après P1b, 1332 − 2 + 1 = 1331, attendu (il ne baisse pas sous la référence d'entrée de 1328).
