# GLORFINDEL-1116a (net-playtest, 2026-11-16)

APPELS_DECLARES: 45 (arret a 45 vus)
HEURE_DEBUT/HEURE_FIN: 15:02 / 15:31
PREMIER_BUILD: sans C++   BUILD: aucun (session_gate Launch 15:13 FAIL plafond C++ : Elrond IN_USE, Galadriel + Haldir file de build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: vu a 15:16 : tests 1320/1321 (Test_ChatSeatlessSpectatorIsADefeatedOne, D806) ; vu a 15:26 : tests 1321/1322 (Test_ChatInputChannelCycle, D807, stub) ; commits verts test + correctif ensemble d3fd15f, bd2ca42 ; assertit l'effet : oui (destinataires et canal calcules par HOLChat)
COMMITS: 2 (d3fd15f 15:21, bd2ca42 15:30) + merge main ece1fa3 15:30, merge_main_fait: oui
TESTS: quick_check -Parts tests : 1320/1320 (reference 15:10), 1320/1321 rouge, 1321/1321 vert, 1321/1322 rouge, 1322/1322 PASS 15:30 ; TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 2 (Edit sans Read prealable)
ESSAIS: aucun
QUESTIONS: aucune (representation de l'observateur = slot hors sieges, meme choix que le C++ FSeats::IsSpectator ; D807 : canal que personne ne lirait saute, choix note au journal)
FAIT / RESTE:
- FAIT D806 partie script : HOLChatRules.as, un slot hors sieges = spectateur sans siege traite comme un vaincu (Tous et Allies avec les spectateurs, jamais en prive meme vers un vaincu, lit le Tous des vivants, jamais leur Allies).
- FAIT D807 partie script : HOLChat::OpeningChannel (Allies, Tous sans allie) et NextChannel (Tab Allies -> Tous -> Prive -> Allies, saute Allies sans allie et Prive sans joueur choisi).
- RESTE (file l. 59, C++) : jumeau HOLChatRules.cpp : prive d'un observateur aujourd'hui livre au vaincu vise ; Allies observateur <-> vaincu aujourd'hui refuse ; cas observateur/observateur de ServerPostChat a passer par HOLChat::Reaches ; cas dans HOLChatTransportTests.cpp ; quick_check tests,net.
- RESTE (file l. 60, C++) : champ de saisie D807 (Entree, Tab, RequestChat), scenario chat_input.
NON VÉRIFIÉ: le comportement reseau (net/netsig3) non rejoue (aucun C++ change) ; la lecture « Allies d'un vaincu -> observateurs » est une lecture litterale de D806 (« lit avec les spectateurs sur Tous et Allies »), le C++ ne la fait pas encore.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune ; session_gate Launch WARN : DLL du worktree (11-15 14:12) plus ancienne que le dernier commit de code de main (11-15 14:26), 0 commit Source de retard ; Edit refuse 2 fois faute de Read prealable.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, ?? Docs/Priorite_Socle_Factions_2026-11-16.md, ?? Docs/Rapports/, ?? Retours/E-audio_compare_buildings_{range,stable}.md ; inchange hors docs voulus : oui (mes ecritures : lignes 59-60 d'Auto_Journey_File.md, une ligne d'EtapeDefini.md ; le reste vient d'autres sessions)
