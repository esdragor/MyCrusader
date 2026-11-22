# ELROND-1122b : D877 moitie B (montage tout seul a l'arrivee du deplacement) + recadrage far_* (sans C++)

Impact joueur : aucun changement de jeu ; le scenario `deploy` prouve maintenant qu'un engin demonte, dont le deplacement est fini a portee de la Maison, se monte tout seul.

APPELS_DECLARES: ~24 (arret a 25 vus ; arret dur 16:54)
HEURE_DEBUT/HEURE_FIN: 16:38:15 / 16:43:54 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario ; la moitie B passe VERTE : `Deploy: #N phase 0 -> 1 (order 3...)` vu, aucun timeout)   TEST_ROUGE_SEPARE: non (pas de rouge, rien a corriger en C++)
COMMITS: 3 (593c606 etape 9 moitie B ; 3c2fcb1 recadrage far + .look ; d578f88 .look decrit tel que vu), dernier 16:43, merge_main_fait: non (rien a fusionner, integ-25 = main 1447448 + commits de scenario)
TESTS: `preflight.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` UNE fois : PASS, check deploy 91 s (99 s mur) ; aucun warning/erreur, aucun HUD qui se chevauche, sanity OK ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: deploy : preflight PASS ; 19 PNG ouverts (3 planches de contact, 2 planches de 4 PNG a 1000 px et 1 de 11 PNG a 800 px : un PNG n'est PAS lu individuellement a 1280x720, limite de 25 appels) ; statut LOGIQUE SEULEMENT (GESTE: interne pour la partie hol.Select.OrderNear, la fiche reste PAS PRET)
QUESTIONS: aucune

## FAIT
- deploy.txt (apres note 8) : `notereset`, `hol.Camera.FocusNear 4200 -500`, `waitlog phase 0 -> 1 (order 3` (texte litteral du journal, HOLDeployProcessor.cpp:139 ; ordre 3 = bouton/fin de marche, HOLDeployProcessor.cpp:85-100 : D877 "une marche qui finit sur un engin Packed le monte"), `wait 0.3`, capture `deploy_arrive_mounting`, `waitlog (delay over)`, `waitlog D=Pack`, capture `deploy_arrive_mounted`, touche D (demontage), puis l'etape 9 d'origine (ordre d'attaque, montage, tir) INCHANGEE. Le waitlog de la moitie B n'a pas expire (60 s) : le deplacement fini a 2500 cm de la Maison, l'engin se monte seul.
- Recadrage far : `notereset` avant `FocusNear 3000 -2500` ; far_start et far_end montrent l'engin (droite) ET la 2e Maison (grande, rouge, gauche), hors du panneau (une seule note).

## Ce qu'on voit vraiment (PNG)
- deploy_arrive_mounting : engin pres de la crete de 3 rochers, case D = `Montage...` avec barre vide `[-----]` ; Maison ennemie en haut, intacte ; un seul panneau de note (titre seul).
- deploy_arrive_mounted : case D = `Demonter`, activite `Attaque`, note 9a visible ; Maison visible.
- deploy_far_start : engin a droite, case D = `Monter`, 2e Maison a gauche, Maison 1 en haut ; panneau vide ; tout visible.
- deploy_far_walking : engin deja a mi-chemin vers la 2e Maison, case D = `Demonter` : PAS une capture de marche, car les deux waitlog de ELROND-1122a (`phase 0 -> 1`, `hits a building`) la precedent. deploy_far_midway : engin a la meme position (monte, `Attaque`, ligne rouge vers la Maison). deploy_far_end : idem, engin et 2e Maison a l'ecran, panneau court. Les PV de la 2e Maison ne sont pas lisibles (non selectionnee) : seul le journal prouve les coups.
- deploy_packed (D=Monter), deploying (`Montage...` + barre), deployed (D=Demonter), fire (Maison 555/600), fire_hit (510/600), packing (`Demontage...` + barre), packed_again (D=Monter), move_start (D=Demonter), move_unmounting (`Demontage...`), attack_mounting (`Montage...`), attack_hit (Maison 465/600, engin a gauche) : conformes au .look. Dans deploy_deploying le panneau recouvre un peu le haut de la Maison (mineur).
- deploy_move_walked, deploy_attack_mounted : conformes (engin plus bas ; D=Demonter).

## RESTE
- Re-sequencer far_walking/far_midway : mettre `waitlog phase 0 -> 1 (order` et `waitlog Mangonel hits a building` APRES far_midway (la marche sera alors visible sur les deux captures), ou prendre far_walking juste apres `wait 1`. Le re-preflight (une fois autorise) tranchera.
- Fiche E-deploy.md : ligne `recette:` mise a jour, reste PAS PRET.
- far_end : choisir une capture avec la Maison selectionnee si l'on veut ses PV a l'ecran.

NON VERIFIE: la moitie B a l'ecran avec une vraie touche (le deplacement est `hol.Select.OrderNear`, GESTE: interne) ; PV de la 2e Maison a far_end ; les PNG n'ont pas ete lus un par un a pleine resolution (planches de contact) ; la distance de marche et la duree de 16 s (journal de ELROND-1122a) non remesurees ; le comportement du `waitlog (delay over)` de l'etape 9 apres le demontage ajoute (il passe, aucun timeout au journal).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: modifs de docs voulus (Auto_Journey_File.md, EtapeDefini.md, Palier3InProgress.md, Retours/E-deploy.md) ; les autres fichiers (Auto_Journey_Jour.md, Fortifications/StealthInProgress.md, Docs/Rapports/*) ne sont pas de moi : oui hors ces fichiers
