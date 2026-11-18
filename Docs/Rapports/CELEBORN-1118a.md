# CELEBORN-1118a : lot R partie H (HOLPlayerController.h + HOLHealProcessor.h)
NON COMPILE (C++ sans build, voulu). Commit 8d81190 sur hero-j8-close (base 0bff683), 2 fichiers, pas de push, pas de fusion.

APPELS_DECLARES: 12 (vus par moi)
HEURE_DEBUT/HEURE_FIN: 13:42 / 13:45
PREMIER_BUILD: sans C++ (aucun build)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (pas de test neuf ; la porte no_cpp_rpc_gate est la cible du lot R, voir CARTE-LOTR-1118 section 4, lot T) ; assertit l'effet : non
COMMITS: 1, dernier a 13:44, merge_main_fait: non (consigne : pas de fusion)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT: HOLPlayerController.h 1009 -> 949 lignes (-60) : supprime de l.773 a 832 (commentaire D223 + 16 blocs UFUNCTION(Server, Reliable) + commentaires + ligne vide + commentaire orphelin D303 "The player's own unit selected alone" juste avant DoWatchUnit, perime). Commentaire l.756 reecrit : "D846: the bodies of the 16 order RPCs (the script RPC actor's bridge calls them); no C++ RPC." 16 Do<X> gardes (0 ligne void Do supprimee), ServerRequestMatchStats intact (absent du diff). Fichier : UTF-8 sans BOM, CRLF, fin de fichier CRLF conservee. HOLHealProcessor.h:46 : ServerHealUnit -> RequestHealUnit (1 ligne).
CONTROLE FINAL : Grep Source de Server(MoveUnits|...|SetRallyPoint)\b : 35 occurrences, TOUTES dans Player\HOLPlayerController.cpp (traite par HALDIR-1118a) ; aucun autre fichier. Gate (Get-NoCppRpcLeft sur HOLPlayerController.h, liste par defaut lots A-E) : sortie vide (rien a signaler) ; avec en plus les 16 noms du lot L2 en argument : sortie vide aussi (les 16 sont retires du .h). Note : Get-NoCppRpcLeft exige le parametre -HeaderPath (pas de defaut), appele sans argument il n'a rien a lire.
PONT (lecture seule) : Net\HOLScriptRpcPlayerActor.cpp l.385-490 appelle les 16 Do<X> (DoMoveUnits 385, AttackUnit 392, HealUnit 399, AttackBuilding 406, AttackMove 413, StopUnits 420, ToggleDeploy 427, MoveUnitsInLine 434, GatherUnits 441, PlaceBuilding 448, BuildUnits 455, DropOffUnits 462, GarrisonUnits 469, UngarrisonAll 476, CancelBuilding 483, SetRallyPoint 490) ; aucun Server<X> dans tout le fichier.
RESTE: le .cpp (autre agent, 35 occurrences Server<X> : forwarders _Implementation, appels RPC des Request, OrderScriptOn/OrderSend/OrderRecv) ; sans lui le .cpp ne compile pas (les _Implementation n'ont plus de declaration) : ne pas builder cette branche seule. Puis UN build.
NON VERIFIE: compilation (aucun build) ; que le .h seul ne laisse pas d'autre reference ailleurs que dans Source (Script, Tools non greppes par moi, la carte dit aucun).
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune. IN_USE.txt du worktree ecrit a l'entree (OCCUPE, CPP: oui) puis remis a "libre" ; il n'apparait pas dans git status (non suivi/ignore). Les fichiers Tools/Scenarios/c28_*, c31_* non ajoutes.
GIT_STATUS_PRINCIPALE: non relu par moi (copie principale non touchee par cet agent) ; inchangee hors docs voulus : oui (je n'y ai ecrit que ce rapport)