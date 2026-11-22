APPELS_DECLARES: ~22 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 16:52:52 / 17:00
PREMIER_BUILD: sans C++   BUILD: sans C++ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario, pas de code) ; assertit l'effet : oui (waitlog phase 0 -> 1 puis hits a building, assertions inchangees)   TEST_ROUGE_SEPARE: non (scenario seul)
COMMITS: 1 (57082ea sur integ-25, 17:00), merge_main_fait: non
TESTS: preflight deploy 2 rejeux (16:53 et 16:56) : scenario complet joue jusqu'a "ready" les deux fois, aucune ligne timed-out ; verdict preflight FAIL (1) puis FAIL (2), voir plus bas ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: deploy : preflight FAIL (pan exposure corrige apres le dernier rejeu, NON REJOUE ; "fichier non commite" corrige par le commit) ; PNG far_start, far_walking, far_midway, far_end, arrive_mounting, arrive_mounted OUVERTS et VUS ; LOGIQUE SEULEMENT (GESTE: interne) ; commande : play.ps1 deploy -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond (agent) ; pas de commande donnee a l'utilisateur
QUESTIONS: aucune
FAIT / RESTE: voir ci-dessous
NON VERIFIE: le preflight n'a jamais fini PASS sur la version finale (le correctif de camera avant OrderGround, lignes 82 et 156 de deploy.txt, est posterieur au dernier rejeu) ; la vitesse reelle de l'engin (1,8 m/s lu a l'ecran) et la longueur de marche sont deduites du journal (horodatages) et des pixels, la position de depart n'est pas imprimee ; son non entendu
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune vue
GIT_STATUS_PRINCIPALE: relu : seuls les docs voulus (EtapeDefini.md, Auto_Journey_File.md, Palier3InProgress.md, Retours/E-deploy.md, ce rapport) sont de moi ; le reste (Placeholders.txt, InProgress des autres, E-income_tooltip.md, rapports des autres) vient des autres sessions ; inchangee hors docs voulus : oui
branche prete : non (preflight deploy pas confirme PASS sur le dernier etat : lancer un rejeu final de `Tools\Scenarios\preflight.ps1 deploy` dans C:\Users\baudo\Documents\HeroesOfLiberty-Elrond ; merge main non fait)

CONTRADICTION LEVEE (1122c vs 1122a)
1122c avait raison sur la distance AU MONTAGE (3177 cm, portee 3000) mais a faussement conclu "~180 cm de marche" : 3177 cm est la distance a la cible AU MOMENT du montage, pas au depart. La marche reelle = distance de depart - 3177.
Journal d'UN rejeu (16:53, Maison 2 a (3000,-5500), engin demonte a ~(3000,-1000) apres l'etape 7, soit ~4500 cm de la Maison) :
- ordre "Order: 1 units attack House of slot 1" a 15.55.26.939 ; "Deploy: #13 phase 0 -> 1 (order 1, action 2, reason 0, distance 3172 cm, walk 2)" a 15.55.34.044 : 7,1 s ; "phase 1 -> 2 (delay over)" 1,4 s plus tard ; 1er coup de 45 a 15.55.37.9.
- portee d'attaque 3000 cm (cadre du scenario), portee de montage maximale MaxRangeCm 5000 (D861 a, HOLDeployProcessor.cpp:127-131 : le combat ne pose PendingOrder = 1 qu'a portee d'attaque, donc la distance au montage ~3172-3177) ; marche = ~4500 - 3172 = ~1300 cm = 7,1 s x 1,8 m/s (coherent).
- depart x distance : position de depart non imprimee par le journal ; deduite du repere du scenario.
Marche ~1300 cm : au-dessus de 1000 cm mais sous les 1500 cm voulus. J'ai donc eloigne la cible.

CHANGEMENT (deploy.txt, assertions intactes)
- 2e Maison : hol.Cheat.SpawnBuilding House 1 3000 -6500 (au lieu de -5500) ; hol.Select.OrderGround -13400 -6500 ; camera FocusNear 3000 -3700 (engin a droite, Maison a gauche, 5500 cm = ~830 px).
- Rejeu 2 (16:56, Maison a -6500) : ordre 15.57.56.112, "phase 0 -> 1 (order 1 ... distance 3177 cm, walk 2)" a 15.58.07.598 : 11,5 s, soit ~2070 cm a 1,8 m/s (~2300 cm geometrique) ; coups de 45 a 15.58.11.5 et 15.58.16.5. Approche >= 1500 cm : oui.
- waitlog : ordre inchange (waitlog phase 0 -> 1 et hits a building DEMARRENT apres far_walking (wait 3) et AVANT far_midway : 3 s + marche ~11,5 s ; ils demarrent a 3 s, avant le montage). Les deux waitlog n'ont pas expire sur les deux rejeux.
- deploy.look : far_start, far_walking, far_midway, far_end reecrits avec ce qui est VU.

PNG VUS (rejeu 2)
- far_start : engin demonte (case D = Monter) a droite, Maison 2 (rouge) a gauche, aucun ordre, notes reinitialisees.
- far_walking (3 s) : engin demonte, en marche (case D = Monter, pas d'activite Attaque), a ~80 px de son depart (~530 cm), ligne d'ordre visible vers la gauche.
- far_midway : engin MONTE (case D = Demonter, activite Attaque), ~250 px (~1650 cm) a gauche de far_walking, ligne rouge vers la Maison 2.
- far_end (5 s plus tard) : meme position, monte, Attaque ; PV de la Maison non lisibles (non selectionnee), seul le journal prouve les coups.
- deploy_arrive_mounting : case D = Montage... [--------] (barre vide au debut), engin demonte arrive au point ; deploy_arrive_mounted : case D = Demonter, monte tout seul. Les deux vus.

GESTE (3)
grep "Scripted" dans Source\ et Script\ : seulement des commentaires ("scripted checks"), aucun verbe. grep RightClick/right click : CVar hol.Garrison.RightClickEnters (HOLPlayerOrders.cpp:234) et commentaires ; aucun verbe "Scripted right click". Seul chemin existant : hol.Select.OrderGround -> Controller->OrderAt (meme fonction que le clic souris, confirme par 1122c). GESTE: interne reste (un verbe qui emet un evenement souris Slate n'existe pas ; hol.Select.LeftClickGround est aussi un appel de controleur).

PREFLIGHT (2 rejeux au total)
- Rejeu 1 (16:53, avant mon changement) : scenario complet "ready" ; verdict FAIL (1) = pan exposure : ligne 82 'hol.Select.OrderGround -13400 -1000' 122 frames apres le placement de camera (max 10) (2 occurrences en tout : ligne 82 et la lointaine) ; effet de l'OrderGround de 1122c (OrderNear n'etait pas vu par ce controle).
- Rejeu 2 (16:56) : FAIL (2) = pan exposure + "fichier non commite" (deploy.txt modifie, corrige par le commit 57082ea).
- Correctif pose apres le rejeu 2, NON REJOUE : hol.Camera.FocusNear 3000 1500 + frames 2 avant l'OrderGround de l'etape 7, hol.Camera.FocusNear 3000 -3700 + frames 2 avant l'OrderGround lointain. Effet attendu : le controle pan exposure passe ; le cadrage de deploy_move_unmounting/move_walked correspond a leur .look (camera crete 3000 1500).

DOCS MIS A JOUR (copie principale)
EtapeDefini.md (ligne apres "Siege D877 moitie B"), Auto_Journey_File.md (ligne Elrond sous 1122a), Palier3InProgress.md (ligne apres 1122b), Retours\E-deploy.md (ligne recette: 1122d ; fiche PAS PRET).
