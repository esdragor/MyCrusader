# GLORFINDEL-1122l : preflight follow_short / follow_crowd

APPELS_DECLARES: ~21 (arret 20 ou 18:51)
HEURE_DEBUT/HEURE_FIN: 18:36:31 / ~18:45
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, aucun quick_check)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenarios seuls) ; assertit l'effet : oui (waitlog)   TEST_ROUGE_SEPARE: non (pas de code)
COMMITS: 3 (67d6bf7, 8f359e7, 716a751 sur ability-columns), merge_main_fait: non (integ-52 non fusionne, consigne)
TESTS: preflight follow_short -TestMap x1, follow_crowd -TestMap x2 ; TESTS_TAUTOLOGIQUES: 0

## preflight follow_short / follow_crowd : FAIL (sur des controles statiques, le scenario lui-meme PASS)
- check follow_short : PASS (58 s). check follow_crowd : PASS (49 s) aux deux passages. Aucun waitlog expire (le waitlog '#22 (slot 0) is healed to 155' est passe).
- 1er passage follow_short : FAIL pan exposure ligne 22 (ClickUnit -1 a 32 images de la camera) + FAIL mapSize. Corrige : camera replacee + frames 2 avant ClickUnit -1 (67d6bf7). mapSize : non ajoute au .json (changerait la geometrie, positions absolues de la carte de test) ; passage avec -TestMap (WARN seulement, intentionnel : carte de test voulue).
- 2e passage follow_short : seul FAIL = « 1 fichier non commite » (le .txt venait d'etre corrige) ; commite en 67d6bf7, NON REJOUE (2 passages faits).
- follow_crowd passage 1 et 2 : FAIL pan exposure ligne 61 (hol.Select.ClickUnit 21, 32 images apres la camera). 1re correction (8f359e7) a vise la mauvaise ligne (le ClickUnit 21 apres le waitlog heal) ; la ligne 61 etait l'autre ClickUnit 21 (Moine/#22 apres frames 30) : corrigee en 716a751. NON REJOUE : 3e passage a faire pour obtenir PASS.
- Aucune assertion affaiblie. Comptes dans les journaux (scenario_follow_short.log, scenario_follow_crowd.log) : « Angelscript ... rror » = 0, « requires WorldContext » = 0.

## PNG ouverts (Read), point par point
follow_short :
- start : 4 Hommes d'armes groupes (panneau « Homme d'arm. (4) ») au centre-gauche, Paysan seul plus a droite (pas « plus haut » : perspective) : VU. Cercles de selection : DOUTEUX (non distincts a cette echelle).
- walk : un seul amas bleu en haut a droite, panneau Paysan (selectionne), les 4 hommes plus pres du Paysan : DOUTEUX (impossible de separer hommes et Paysan dans l'amas ; l'eloignement des hommes de leur position de depart est visible).
- second_leg : camera recentree, 4 hommes + Paysan (cercle) groupes au contact a droite : VU.
- cancelled : 4 hommes selectionnes (cercles) sous le Paysan isole en haut a gauche, ils ne sont plus autour de lui : VU ; « partis vers la droite » : DOUTEUX (ils sont au centre, pas a droite).
follow_crowd :
- start : masse serree a droite des 6 spheres vertes, panneau « Homme d'arm. (10) » : VU en masse ; damier Hommes/Paysans et cercles : PAS VU (amas trop dense), position « en haut de l'image » : pas vu (milieu).
- body_peasant : masse etalee, 10 hommes selectionnes (cercles), pas de trace de Suivre : etat de la masse VU.
- body_selected : idem, VU.
- ground : 10 hommes (cercles) en ligne en bas a gauche de la masse, Paysans restes en haut a droite : VU (ils marchent vers la gauche/bas).
- heal_start : Moine selectionne (panneau Moine 40/80), unites alignees a droite de la masse : VU ; « un blesse » : PV du Moine 40/80 vus, barres des Hommes d'armes DOUTEUX.
- heal_done : panneau « Homme d'armes 155 / 155 » sur l'unite selectionnee (cercle, a droite du Moine) : VU ; barre de vie de #22 PLEINE au-dessus de l'unite : DOUTEUX (traits minuscules, couleur non distinguable a 1066x600).

## Statut
Les deux scenarios restent LOGIQUE SEULEMENT (geste interne OrderNear / ClickUnit, aucune fiche E). Aucune fiche Retours\E-*.md ecrite.

## Reste
- Rejouer preflight.ps1 follow_crowd -TestMap (3e passage) puis follow_short -TestMap pour PASS formel (un passage chacun).
- Decision hors perimetre : ajouter mapSize Small au .json (D820) = changement de geometrie, a trancher.

QUESTIONS: aucune
NON VERIFIE: PASS formel des deux preflight (corrections statiques non rejouees), barre de vie de #22, cercles de selection a l'echelle des PNG.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchange hors docs voulus : oui (EtapeDefini.md, OrdersPatrolInProgress.md ajoutes)
