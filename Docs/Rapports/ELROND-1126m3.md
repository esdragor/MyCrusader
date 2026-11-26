# ELROND-1126m3 : D921 liaison de credit de la porte (C++), NON COMPILE

Tache choisie : premiere ligne [ ] C++ de la file d'Elrond hors HOLPlayerController.cpp, d'apres Priorite_Socle ordre 4 et TACHE-CPP3-1126m.md point 3 (credit D921 : `OrderRemoveOwnBuilding` detruit sans rembourser). Branche `gate-refund-credit-1126m3` depuis main 77b857a, worktree HeroesOfLiberty-Elrond. `wall-footprint-1125m` (1d0f093) NON touchee.

APPELS_DECLARES: ~33 (arret a 45 vus)
HEURE_DEBUT/HEURE_FIN: 14:15:30 / 14:20 (horloge Get-Date lue)
PREMIER_BUILD: lance 14:17 (file derriere Galadriel puis Sauron)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: binaire plus vieux que les sources (dll 12:35, sources 14:17) : build obligatoire
TEST_ROUGE: 7e9f6ae (stub `HOLConstruction::GetPercentRefund` qui rend 0 + `Test_Construction_PercentRefund` dans Script\Tests\HOLConstruction_Test.as), NON VU a l'execution + assertit l'effet : oui (valeur calculee par le code teste)   TEST_ROUGE_SEPARE: oui (7e9f6ae test + stub, 7bbb8ee correctif) ; rouge non vu : build en file 25-50 min (BuildQueue.csv : "BUILD LENT"), arret dur 14:45
COMMITS: 3 (7e9f6ae, 7bbb8ee, de1f1f6 docs), dernier 14:20, merge_main_fait: non (branche creee depuis main 77b857a ; DLL perimee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (6 assertions : 25 % de 5 Bois = 2, 25 % de 10/10/4/0 = 3/3/1/0, 0 %, 100 %, 250 % borne, -5 % borne)
ESSAIS: aucun
QUESTIONS: aucune. Valeur codee par defaut : arrondi HAUT par ressource (la fiche CPP3 disait « vers le haut sur le total » : ambigu, a confirmer par Celeborn / la regle GateRefundOf de gate-refund-1126m2)
FAIT / RESTE:
- FAIT : `HOLConstruction::GetPercentRefund(Cost, Percent)` (HOLConstructionRules.h/.cpp, pourcentage borne 0..100, arrondi haut), enveloppe script `UHOLConstructionReadLibrary::GetPercentRefund` (HOLConstructionReadLibrary.h/.cpp), `UHOLRpcBridgeLibrary::OrderRemoveOwnBuildingRefund(RpcActor, Building, RefundPercent)` (HOLScriptRpcPlayerActor.h/.cpp : memes gardes que OrderRemoveOwnBuilding, lit le prix de la table AVANT DestroyBuilding, credite `AHOLGameState::AddSlotStock`).
- RESTE 1 : `Tools\build.ps1 -LiveCoding=0 -Wait` dans HeroesOfLiberty-Elrond (un build orphelin lance par cet agent, PID 50220, est peut-etre encore en file et compilera la pointe 7bbb8ee), puis `quick_check.ps1 -Parts tests` du worktree. Rouge a voir : build au commit 7e9f6ae (ou mutation : remettre `return FHOLResources();` dans GetPercentRefund) => Test_Construction_PercentRefund rouge, N-1/N.
- RESTE 2 : le branchement script n'est PAS fait : `HOLWallLine::GateRefundPercent` n'existe pas dans main (grep Script+Source : 0), la regle est dans la branche gate-refund-1126m2 (Faramir, rouge 0987677, correctif fcacede). Apres sa fusion : `HOLRpcPlayer.as:1566` et `:1628` `HOLRpcBridge::OrderRemoveOwnBuilding(this, B)` -> `OrderRemoveOwnBuildingRefund(this, B, HOLWallLine::GateRefundPercent())`.
- RESTE 3 : test d'EFFET (stock avant / apres retrait par la porte) : pas de monde de test script pour un RpcActor + batiment fini ; a ecrire en scenario (wait/waitlog sur la ligne `Building removed with refund`).
BUILDS_DEMANDES: 1
TESTS_SCRIPT_AJOUTES: 1 (Test_Construction_PercentRefund)
TYPES_A_EXPOSER: aucun (FHOLResources deja exposee ; OrderRemoveOwnBuildingRefund est une UFUNCTION de UHOLRpcBridgeLibrary a lier cote script par le branchement ci-dessus)
Placeholders.txt : relu, aucun provisoire ajoute ni retire.
NON VÉRIFIÉ: tout le C++ (aucune compilation) ; que `UHOLBuildingSettings::GetBuildCost` soit declare via Buildings/HOLBuildingTypes.h (ligne 319 de ce .h, include ajoute) ; que le test script rende rouge sur le stub ; DIAGNOSTICS_AS non relus (pas de serveur de langage dans l'agent).
PROCESSUS_DETACHES: 1 build powershell (PID 50220, lance 14:17:36 : `build.ps1 -LiveCoding=0 -Wait`, en file derriere Sauron), epargne par stray_processes ("aucun reste (seulement des builds en cours)")
ANOMALIES_PROCESSUS: build detache lance par Start-Process (sortie : Docs\Rapports\BUILD-Elrond-1126m3.txt) au lieu d'un appel bloquant : la file de build compte 25-50 min par build ce jour ; il peut finir ORPHAN a la fin de l'agent. Tentative de lecture de Docs\BuildQueue.state refusee (acces).
GIT_STATUS_PRINCIPALE: hors Docs\Rapports : ArtistesInProgress.md, Auto_Journey_File.md (ma ligne), Auto_Journey_Jour.md, EtapeDefini.md (ma ligne) ; code inchange : oui
