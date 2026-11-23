# GLORFINDEL-1123i : rejeu ai_sacred_light (worktree HeroesOfLiberty-Glorfindel, ability-columns, ffd0325)

STATUT : LOGIQUE SEULEMENT (lancer vu au journal ; effet non vu sur PNG)

APPELS_DECLARES: 12 (arrêt à 45 vus par toi)
HEURE_DEBUT/HEURE_FIN: 10:27:49 / ~10:32
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build demandé)   LAUNCHCHECK_DEBUT: sans C++ (preflight : binaire non plus vieux que les sources C++ : OK)
TEST_ROUGE: sans objet (rejeu, aucun code touché) ; TEST_ROUGE_SEPARE: non (sans objet)
COMMITS: 0, merge_main_fait: non (aucun changement)
TESTS: preflight ai_sacred_light PASS (check PASS 25 s) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: ai_sacred_light : preflight PASS, 2 PNG ouverts, LOGIQUE SEULEMENT ; commande : play.ps1 ai_sacred_light -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
QUESTIONS: aucune
FAIT / RESTE:
- Preflight ai_sacred_light PASS, scénario non modifié.
- Journal (Saved_Tests\Logs\scenario_ai_sacred_light.log), pas 44 (image 145) : « powers think, Honor 100, wounded 1 of 7, decision cast=true row 0, banner (14939, -46), nearest wounded 253.0 cm, range 800 cm » ; « Zone effect: slot 1 puts a Sacred Light at (14895, 203) » ; « RpcPower: Sacred Light put down by slot 1, Honor now 75 » ; « AI slot 1 step 44: Sacred Light cast on 1 wounded unit(s) ».
- Pas 49 (image 164) : « wounded 0 of 7 », cooldown left 19716 ms : le blessé n'est plus compté blessé (soin indirect, pas de PV chiffrés au journal).
- PNG before : forum de l'IA (château), 6 soldats rouges devant + quelques paysans ; aucun combat, aucune zone de lumière. Conforme.
- PNG after (capturé image 163, 18 images = 0,4 s après le lancer) : mêlée rouge/bleu devant le forum, 3 rouges à gauche ; panneau de note ouvert qui masque le haut du château ; AUCUNE zone de lumière visible. La zone (14895, 203) est dans la mêlée mais rien de lumineux n'apparaît.
- RESTE : décider si la zone n'a pas de rendu visible sous -RenderOffScreen / à 0,4 s de la pose (effet non fiable à voir), ou si le visuel de Sacred Light est absent côté zone d'IA ; une capture plus tardive (frames 40-60 après le lancer) ou une capture de la zone posée par le joueur trancherait. Non fait (consigne : pas de changement de seuils, un seul rejeu).
NON VÉRIFIÉ: effet visuel de la zone ; PV chiffrés (soin déduit de wounded 1 -> 0) ; jeu avec fenêtre.
PROCESSUS_DETACHES: voir sortie ci-dessous (stray_processes -Stop)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md modifiés par moi) : oui
