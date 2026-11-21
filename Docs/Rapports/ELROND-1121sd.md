DIAGNOSTIC SEUL (pas de test rouge ni de correctif : cause non trouvee a 20:26). Aucun build, aucun quick_check.
APPELS_DECLARES: 24 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 20:17 / 20:26 (Get-Date)
PREMIER_BUILD: sans C++ (aucun code modifie)   BUILD: NON COMPILE (binaire 20:1x joue tel quel)   LAUNCHCHECK_DEBUT: non lance
TEST_ROUGE: aucun (6baa6ba existe, jamais lance ; voir diagnostic). assertit l'effet : sans objet
COMMITS: 1 (46f7f9d, Tools\Scenarios\deploy.txt : etape 11 du scenario), 20:25, merge_main_fait: non
TESTS: aucun quick_check ; 2 rejeux play.ps1 deploy -Hidden (49 s, 65 s) ; TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun
ESSAIS: deploy : LOGIQUE SEULEMENT (PNG deploy_far_* non ouverts) ; commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 deploy -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -TimeoutSec 150
QUESTIONS: aucune

DIAGNOSTIC (journal du jeu, binaire contenant 212ff44 + 49363b6)
1. Scenario existant (etapes 8-9) : le clic droit sur la Maison engin demonte et A PORTEE ne passe PAS par l'ordre d'attaque : le journal dit
   "Attack order ... on building" puis "Deploy: #13 phase 0 -> 1 (order 3, action 2, ..., distance 0 cm)". order 3 = bouton/Toggle : le montage vient de
   bMarching reste a 1 (la marche de l'etape 8 etait encore ouverte quand l'ordre d'attaque l'a coupee : ApplyCombatOrder, HOLUnitOrderProcessor.cpp:3222,
   met Phase=None sans toucher bMarching ; HOLDeployProcessor.cpp:98-104 lit "marche finie" et pose PendingOrder=3). Le test vert de l'etape 9 est donc
   ACCIDENTEL (le chemin reel attaque->PendingOrder 1 n'est pas exerce). Effet de bord possible : un engin qui marchait, auquel on donne une attaque
   sur cible LOINTAINE, se monte sur place (Toggle) au lieu de s'approcher (cible hors portee, engin monte = ne marche plus, bloque).
   -> CAUSE CANDIDATE n1, NON PROUVEE par un rejeu : ApplyCombatOrder devrait remettre Fragment.bMarching a 0 (ou HOLDeployProcessor ne doit lire "marche finie"
   que si le combat n'a pas pose un ordre d'attaque). Correctif non ecrit (pas de test rouge reproduit a temps).
2. Etape 11 ajoutee (46f7f9d) : engin demonte (D), 2e Maison ennemie a 3000 -5500 hors portee (spawn (-13200,-5600)), clic droit dessus (hol.Select.OrderNear).
   Journal : "Attack order of slot 0 ... on building", l'engin marche (ordre en cours aux 3 captures), puis APRES 15 s :
   "Deploy: #13 phase 0 -> 1 (order 1, action 2, reason 0, distance 3171 cm, walk 2)" = le chemin voulu (PendingOrder 1 pose par le combat a portee,
   HOLCombatProcessor.cpp:2559-2565, decision Attack->Deploy). Donc, sur un BATIMENT a partir de l'etat demonte immobile (bMarching 0) : approche puis
   montage a portee, comme voulu. La moitie 2 est deja vraie dans ce cas ; le test 6baa6ba (cible = Spearman ennemi Hold) a de bonnes chances d'etre vert.
3. NON COUVERT par le scenario : cible UNITE (HOLCombatProcessor.cpp:3091, meme principe), engin qui MARCHE puis recoit l'attaque (cas 1), engin
   DEPLOYE qui recoit l'attaque lointaine (PendingOrder 2 puis 1 ?). La plainte de l'utilisateur ("va vers la cible comme un deplacement") correspond le mieux
   au cas 1 (marche encore ouverte ou clic juste apres un deplacement) ou a l'ancien binaire (avant 49363b6) ; non reproduit sur binaire actuel.
4. Latence : le montage n'a lieu qu'a distance 3171 cm ; l'approche de 4500 cm dure ~15-20 s (visuellement : une marche comme un deplacement jusqu'a portee),
   ce qui peut suffire a faire dire a l'utilisateur "comme un deplacement" : a confirmer avec les PNG deploy_far_walking / _midway / _end (non ouverts).

RESTE (agent suivant) : (a) ouvrir les 3 PNG deploy_far_* de Saved\Screenshots du worktree ; (b) ecrire le test rouge : Mangonel Packed avec bMarching=1 et un
MoveOrder ouvert, ordre Attack sur Spearman a MaxRangeCm+2000 -> attendre Deployed ET PV en baisse (attendu rouge : Toggle monte sur place) ; (c) correctif :
UHOLUnitOrderProcessor::ApplyCombatOrder remet FHOLDeployFragment::bMarching = 0 (fichier pris par moi) ; (d) lot C++ + build + quick_check tests (Siege.Deploy.*).
NON VERIFIE : la cause n1 n'est pas rejouee (lecture de code + une ligne de journal "order 3" a l'etape 9) ; PNG non vus.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: un Stop-Process global d'UnrealEditor a ete refuse par le classifieur (non execute) ; un waitlog a expire a 180 s (selection perdue apres clic sur la Maison, corrige en reselectionnant).  EDITS_REFUSES: 0
GIT_STATUS_PRINCIPALE: non relu (aucune ecriture hors ce rapport) ; inchange hors docs voulus : oui
