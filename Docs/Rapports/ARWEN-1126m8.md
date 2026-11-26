# ARWEN-1126m8 (16:20 - 16:37) : pixels de clic des scenarios contre le viewport reel

APPELS_DECLARES: ~17 (arret 16:49 non atteint)
HEURE_DEBUT/HEURE_FIN: 16:20 / 16:37
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build lance ; binaire de f887556 inchange)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: scenario alert_population sur HEAD f887556 (timeout etape 30 waitlog HUD cells: A:, vu 16:23, 16:25 et 16:32) ; assertit l'effet : oui (HUD cells A: Paysan apres un vrai clic sur le forum)   TEST_ROUGE_SEPARE: non (le scenario est lui-meme le test : le rouge est le HEAD inchange, un seul commit de correctif)
COMMITS: 1 (c38428d sur integ-100b, 16:36), merge_main_fait: non (branche prete ; c38428d ne touche que alert_population.txt)
TESTS: play.ps1 -Hidden -Root Arwen : signals_altclick VERT (57 s, signal recu a 1400,350, viewport 1920x1080) ; nomad_start VERT (62 s, fondation posee a 960,540) ; alert_population VERT apres correctif (sequence done 51,8 s, HUD cells A: Paysan, Population pleine ; play sort 2 faute de quit final, pas un echec). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- Les 3 scenarios cites posent r.SetRes 1920x1080w et le jeu masque le respecte (log "HUD layout: viewport 1920x1080") : leurs pixels sont CORRECTS ; seuls deploy / deploy_far / arwen_pick (sans SetRes, 1280x720) avaient un pixel faux (m7).
- Essai de mettre 640 360 dans alert_population : toujours rouge, annule (git checkout) : le pixel n'etait pas la cause.
- alert_population : le clic 960,540 touche le sol (selection none) car la camera n'a pas fini de se poser apres le dernier cran de zoom (la ligne "Camera height 3000 cm" precede la fin du mouvement) ; sonde avec capture avant le clic (3 s) = "selected MainCentre" ; correctif : wait 3 avant le clic (c38428d). Un waitlog sur une fin de mouvement de camera serait plus propre (pas de ligne existante vue).
- grep hol.Mouse.Click : les autres clics sont des widgets nommes (pas de pixel), sauf faction_aim_free.txt (600 380, 1030 300, commentaire "viewport 1066x600") : NON rejoue, a verifier par un agent suivant (pas de SetRes dans ce fichier : viewport 1280x720 probable).
- deploy.txt / deploy_far.txt / arwen_pick.txt : toujours 533 300 dans integ-100b ; correctif sur arwen-1126m7 (ceeffd1).
NON VERIFIE: faction_aim_free ; que le wait 3 suffit sous charge (camera dependante du temps) ; mes appels sont un decompte de memoire.
PROCESSUS_DETACHES: voir ci-dessous
ANOMALIES_PROCESSUS: premier replay alert_population : 5 processus fermes par play (3 powershell + 2 UnrealEditor), tous du jeu masque
GIT_STATUS_PRINCIPALE: docs seulement : oui (rien de code ecrit dans la copie principale)