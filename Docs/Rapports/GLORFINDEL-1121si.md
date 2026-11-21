# GLORFINDEL-1121si (2026-11-21, 20:21 -> 20:24)

NON COMPILE : script seul, aucun build, aucun quick_check, aucun test lance.

APPELS_DECLARES: ~22 vus (arret a 25 ; premier Edit au 13e appel, avant-8e NON applique : lectures de decouverte)
HEURE_DEBUT/HEURE_FIN: 20:21 / 20:24
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: e861db4 (3 tests + stub RightClickUnitOrder qui rend Move, Book intact), assertit l'effet : oui (Book reel : followers, cibles, cible non suiveuse). Rouge NON VU a l'execution.
COMMITS: 3 (e861db4 test rouge + stub ; 18d5922 regle + Book du joueur + helper ; 3bad0bc appel dans DoForController), dernier 20:23, merge_main_fait: non (DLL non verifiee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 3 (Script\Player\HOLControllerCommandsC10a_Test.as) : Test_ControllerCommandsC10aRightClickOnAllyStartsFollow, Test_ControllerCommandsC10aRightClickOnEnemyKeepsAttack, Test_ControllerCommandsC10aRightClickSlotKeepsOrdinaryPath
BUILDS_DEMANDES: 0 ; apres : `quick_check -Parts tests` dans le worktree Glorfindel
TYPES_A_EXPOSER: (1) le VRAI clic droit souris sur une unite est traite par AHOLPlayerController (C++, ScriptRightClick / OrderAtFraction) : le script ne recoit que le sol (OrderGround) ; la cible sous le curseur n'est connue du script que dans la commande console hol.Select.ClickUnit ... order (UnitScreenPoint). Liaison manquante : un point d'entree C++ -> script au clic droit reel qui passe (bOnUnit, TargetId, bTargetIsAlly) a HOLFollowOrders::ApplyRightClick / HOLControllerCommandsC10a::RightClickUnitOrder. (2) appelant de StepUnit a chaque pas + bPlayerOrderActive (inchange). Aucun C++ ecrit.
DIAGNOSTICS_AS: non releves

## Fait
- HOLControllerCommandsC10a.as : RightClickUnitOrder (Slot != -1 : Move, camp inconnu ; sinon ApplyRightClick avec bTargetIsAlly = !bEnemy, soi-meme compris), RightClickStartsFollow (lit HOLControllerCommand::OwnSelectedUnitIds), et dans DoForController (hol.Select.ClickUnit ... order) : Follow = log "Right click on unit #N: the selected units follow it" et AUCUN ordre ordinaire ; ennemi ou slot= = OrderAtFraction inchange (l'attaque existante n'est pas touchee). Le sol (OrderGround) n'est pas modifie.
- HOLFollowOrders.as : HOLFollowOrders::PlayerBook (FHOLFollowBook de namespace, variable globale script) rempli par le clic droit.

## Rouges attendus nom par nom
- Sur e861db4 (stub Move) ROUGES : Test_ControllerCommandsC10aRightClickOnAllyStartsFollow, Test_ControllerCommandsC10aRightClickOnEnemyKeepsAttack. VERT par construction : Test_ControllerCommandsC10aRightClickSlotKeepsOrdinaryPath (garde de regression).
- Sur 3bad0bc (tip) : les 3 verts attendus.

## NON VERIFIE
Compilation script ; la variable globale `FHOLFollowBook PlayerBook` en namespace (hot reload la reinitialise ; persistance reelle a revoir cote C++) ; `OwnSelectedUnitIds` visible en script comme HOLControllerCommand::OwnSelectedUnitIds (meme espace que UnitScreenPoint, a confirmer) ; fins de ligne (fichiers mixtes, Edit multi-lignes refuse une fois, remplace par PowerShell).
Limite : sans appelant de Step, un Follow demarre par la console n'envoie aucun mouvement (pas d'ordre ordinaire non plus).
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: premier Edit apres le 8e appel ; un commit intermediaire (18d5922) a laisse le helper non appele, corrige par 3bad0bc. EtapeDefini.md non mis a jour (temps).
GIT_STATUS_PRINCIPALE: seul ce rapport ajoute par moi ; aucun fichier de code touche dans la copie principale : oui.
