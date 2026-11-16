# RECETTE-1116a : recette indépendante des essais du 2026-11-16 (règle Q5/L5-4)

Worktree de recette : `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` (branche galadriel-tab, avance rapide sur main 2beb47d à 16:03 ; binaire UI du 15:22, `Get-StaleBinaryProblem` vide). Aucune fiche E modifiée, aucun commit, aucun build.

## Verdicts (une ligne par essai)

- **alert_population : FAIL** : rejoué caché 2 fois, la capture tient (10 / 10, ligne « Population pleine » en haut à droite), MAIS le pré-vol est rouge sur main (`FAIL map: alert_population.json has no mapSize`, D820) et l'essai a échoué dans les mains de l'utilisateur (son journal : `timeout at step 30: waitlog HUD cells: A:`, panneau « failed »).
- **arquebusier : PASS sur le build de main (Galadriel)** : pré-vol PASS 41 s, arq_before 155 / 155 (Homme d'armes rouge seul, herbe, arbre), arq_hit 117 / 155 (Arquebusier bleu au contact). La COMMANDE de la fiche (worktree Sauron) est à refaire : binaire périmé.
- **nomad_start : PASS avec 2 réserves sur le build de main (Galadriel)** : pré-vol PASS 49 s, 4 PNG vus, chantier ouvert, cadre « 2 en construction · 3 en marche » ; un seul Paysan visible sur nomad_start_site (le .look en promet 2) ; achèvement du forum : voir « Essai de casse B ». La COMMANDE de la fiche (worktree Celeborn) est à refaire : binaire périmé.
- **deploy : PASS sur le build de main (Galadriel)** : pré-vol PASS 61 s, 5 PNG vus (D « Monter », D « Démonter » + statut « Attaque », Maison 555 / 600 puis 510 / 600, D redit « Monter »). La COMMANDE de la fiche (worktree Faramir) est à refaire : binaire périmé, WIP C++ non compilé.

## alert_population (détail)

Ce qui tient, vu sur `Saved_Tests\Screenshots\WindowsEditor\HOL\alert_population.png` (2 passages, 16:05 et 16:07) : barre « Population 10 / 10 », « Inactifs : 9 » ; en haut à droite, sous « Vous 2 vies », la ligne « Population pleine » ; « Centre principal » sélectionné, case A « Paysan », panneau de production « Paysan » barre au début ; un paquet de Paysans bleus au nord du forum (paquet serré, 9 non dénombrables à l'image). Journal : `Mouse click 960,540 x1 at 960,540: on SViewport`, `Selection: MainCentre of slot 0 (own)`, `Key pressed: A`, `Production: slot 0 queued Peasant`, `HUD report: ... population 10/10`, `HUD messages: [Population pleine]`.

Ce qui casse :

1. **Pré-vol rouge sur main** : `preflight.ps1 alert_population -Root ...Galadriel` = `FAIL (1)` : `FAIL  map: alert_population.json has no mapSize, the trial plays on the grey test map (D820...)`. La fiche écrit « preflight PASS (71 s) » (15:47) : vrai avant l'arrivée de la règle b1e97e3 (visual, 15:39) dans ce worktree. Le sol gris de la carte de test se voit sur la capture.
2. **Le passage de l'utilisateur** (son journal `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Saved\Logs\HeroesOfLiberty.log`, fenêtre 1920x1080 posée en 320,156 sur un écran 2560x1440, 16:05:21 à 16:09:45, heures locales = heures du journal + 1 h) :
   - 16:05:25 `Scenario waits for the key: Entree : clic sur ton forum, puis la touche A (Paysan)` ;
   - 16:05:33 `Selection: MainCentre of slot 0 (own)` puis 16:05:36 `Production: slot 0 queued Peasant` : l'utilisateur a fait LUI-MÊME le clic et la touche A, AVANT Entrée (le texte du waitkey se lit comme une consigne) ;
   - 16:05:42 Entrée (`Scenario key: go on (key)`), puis le clic du scénario : `Mouse click 960,540 x1 at 1280,696: on SViewport` -> **`Selection: none`** (en caché : `Selection: MainCentre`) ; le rapport HUD de cet instant dit `population 10/10`, `HUD cells: none`, **`HUD messages: [Population pleine]`** ;
   - 16:06:04 `HUD: grid cell A clicked` -> `Command feedback: population full`, `HUD message: PopulationFull "Population maximale — construisez des maisons"` ET `Production: slot 0 queued Peasant (1 in the queue, F 600 ...)` (mis en file et payé à 10 / 10) ;
   - 16:06:42 **`LogHOLTest: Error: Test sequence: timeout at step 30: waitlog HUD cells: A:`**, `Scenario panel: scenario alert_population failed, 1 notes` : les notes 2 et 3 ne sont jamais venues.
   Lecture : la ligne est entrée au fil vers 16:05:36 (le rapport de 16:05:42 la contient encore), pendant le geste de l'utilisateur, alors que la note à l'écran disait « Le fil de messages est vide » ; elle dure environ 8 s et ne revient pas. Le constat de D829 (« la ligne n'est jamais apparue ») ne correspond donc pas au journal : elle était dans le fil 6 s après la touche A. Si elle était visible à l'écran de l'utilisateur : NON VÉRIFIÉ (seul le contenu du fil est journalisé).
3. **Essai de casse A** (scénario temporaire `recette_alert_pre`, non commité, supprimé : clic sur le forum + A, 6 s, puis Entrée, comme l'utilisateur) : `HUD messages: [Population pleine]` à +6 s, puis après Entrée 2e touche A -> `Command feedback: population full` + `Production: slot 0 queued Peasant (2 in the queue, F 600 ...)`, `HUD messages: []` 8 s après la 1re ligne, et **`timeout at step 42: waitlog Population pleine]`**, panneau « failed ». En caché le 2e clic garde le forum sélectionné : le `Selection: none` de l'utilisateur n'est PAS reproduit (cause non trouvée : curseur réel ou caméra déplacée, hypothèses).
4. **« GESTE: réel »** de la fiche : prouvé seulement en fenêtre cachée ; dans la fenêtre de l'utilisateur le clic du scénario n'a rien sélectionné.
5. Sur la capture, le panneau du scénario affiche encore « Population 9 / 10 ... Le fil de messages (en haut à droite) est vide » à côté de « 10 / 10 » et de la ligne (la note suivante arrive 1,4 s après la capture).

## arquebusier (détail)

Pré-vol `preflight.ps1 arquebusier -Root ...Galadriel` PASS (41 s, carte générée `mapSize Small`, audit Sanity passé). PNG ouverts : arq_before = sol d'herbe, arbre en haut à gauche, Homme d'armes rouge seul sélectionné, « Points de vie 155 / 155 », pas d'Arquebusier, pas d'avis « Shaders en préparation » cette fois ; arq_hit = Arquebusier bleu au contact, « Points de vie 117 / 155 », statut « Attaque ». Journal : `ManAtArms slot 1 hp 155/155`, `Right click on unit #13`, `Order: 1 units attack #13 ManAtArms`. La fiche est honnête (GESTE: interne, duel de la note 5 non rejoué).
Réserves : `SON: sans objet` n'est pas une des deux valeurs du gabarit (`entendu` / `non entendu`) ; l'en-tête de `arquebusier.txt` dit encore « Geste reel » alors que les clics sont `hol.Select.ClickUnit` ; la commande donnée pointe sur Sauron (voir anomalie 1).

## nomad_start (détail)

Pré-vol `preflight.ps1 nomad_start -Root ...Galadriel` PASS (49 s, audit Sanity passé). PNG ouverts :
- nomad_start_wide : aucun bâtiment à toi, un Paysan bleu en bas à gauche, une unité rouge à gauche, « Population 5 / 0 » ; minicarte agrandie 4 fois : 5 points bleus éloignés les uns des autres (minuscules à taille réelle).
- nomad_start_grid : « Paysan (5) », pages I et II ouvertes, C « Centre principal » en clair, Marché / Atelier de siège / Chapelle / Écurie / Forge gris ; l'avis « Shaders en préparation (1) » chevauche le haut du panneau Triches.
- nomad_start_forum : fondation en fil de fer bleu clair à gauche de la forêt, Bois 200, Pierre 200, « 5 en marche ». La fondation n'est PAS « au centre du cadre » (.look) : décalée à gauche, en partie sous les panneaux du HUD ; journal `placed MainCentre foundation at (-15200, -3600)` alors que la fiche cite (-14800, -3600).
- nomad_start_site : chantier bleu (dalle, 4 tours basses), pas un bâtiment achevé ; cadre « 2 en construction · 3 en marche » (l'écart « 1 en construction » de la fiche est corrigé par 31a01c9) ; **un seul Paysan bleu visible** (coin haut gauche), le second est caché par le HUD (liseré bleu au bord de la grille) : le point « au moins 2 Paysans bleus à côté » du .look n'est pas vu. Un trait blanc fin part d'un point blanc au sol vers le haut de l'écran sur nomad_start_forum et nomad_start_site : non expliqué.
Journal : `first work ... 1 builders` 5,4 s après la pose, `2 builders at work` 14,6 s après la pose, aucun `finished MainCentre` avant la capture.

**Essai de casse B** (scénario temporaire `recette_nomad_done`, non commité, supprimé : même scénario puis 195 s d'attente) : forum posé à T0, `first work` à +5,4 s, 2 / 3 / 4 / 5 bâtisseurs à +14,7 / +21,0 / +33,9 / +51,9 s, puis `Construction: slot 0 finished MainCentre at (-15200, -3600) after 1380 steps of work (by builders: 1 x185, 2 x125, 3 x258, 4 x360, 5 x452), speed 1` à **+74,5 s** ; population 5 / 0 -> 5 / 10. Capture `Saved\Screenshots\WindowsEditor\HOL\recette_nomad_t160.png` vue : forum achevé (toit, 4 tours, drapeaux bleus), « Population 5 / 10 », « 5 inactifs », page II refermée. La durée « environ 3 min » de la fiche tient (forum fini 1 min 15 après la pose, 5 Paysans). Les 150 s pour un Paysan seul ne sont PAS chronométrées : 1380 pas à 20 pas/s = 69 s de chantier à 1 puis 5 bâtisseurs ; le forum de l'IA (`1491 steps`, autre répartition) finit au même instant, ce qui est cohérent avec 3000 pas pour un seul si N bâtisseurs valent environ N^0,63 (loi déduite des deux chantiers, non lue dans le code).

## deploy (détail)

Pré-vol `preflight.ps1 deploy -Root ...Galadriel` PASS (61 s). PNG ouverts : deploy_packed (Mangonneau 200 / 200, Attaque 45, D « Monter », Maison rouge intacte au-dessus), deploy_deployed (D « Démonter », statut « Attaque »), deploy_fire (Maison, IA Normal 2, 555 / 600, cadre blanc), deploy_fire_hit (510 / 600), deploy_packed_again (D « Monter », statut parti, Maison visible). Journal : `Key pressed: D`, `Deploy: #13 phase 0 -> 1`, `phase 1 -> 2 (delay over)` 8 s après, `Mangonel hits a building for 45` deux fois à 5 s d'écart, `phase 3 -> 0` 5 s après la 2e touche D.
Réserves : sur deploy_packed, deux messages du moteur (« Shaders en préparation (1) » et une ligne « ...AllScreenMessages" pour supprimer ») recouvrent le haut du panneau Triches ; Mangonneau = mannequin gris, aucun projectile (connus) ; les notes écrivent « Demonter » sans accent.

## Anomalies

1. **Les commandes données à l'utilisateur pointent sur des worktrees au binaire périmé** : Sauron (arquebusier ; UI dll du 11-15 14:45, source écrite 16:06 ; worktree OCCUPÉ par SAURON-1116e en C++), Celeborn (nomad_start ; UI dll du 11-15 14:43, source écrite 16:01), Faramir (deploy ; dll du 11-15 13:52, sources écrites 16:04 et 16:07 ; `IN_USE.txt` : « WIP de79413 non compilé »). `Get-StaleBinaryProblem` non vide pour les trois. Leur `Script\UI\HOLGameScreenScript.as` (a11ed2a fusionné) appelle `GetPopulationNow` / `AddThreadLine`, et la chaîne `GetPopulationNow` est ABSENTE de leur `UnrealEditor-HeroesOfLibertyUI.dll` (présente dans celle de Galadriel) : erreur de script du HUD attendue au lancement (prédit par lecture, NON exécuté). `play.ps1` ne contrôle pas le binaire (aucune ligne Stale / older). Seul Galadriel porte un binaire à jour : commande à donner `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 <nom> -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`.
2. **`session_gate.ps1 -Phase Essay -Root ...Galadriel` = 1 FAIL pour les 4 essais** : « binaire du 15:22 PLUS ANCIEN que le dernier commit de code du 15:27 » alors que `Get-StaleBinaryProblem` est vide et que le jeu montre l'effet d'a11ed2a (la ligne au fil) : faux rouge par date de commit (a11ed2a commité 5 min après le build des mêmes fichiers).
3. La recette a tourné dans le worktree où l'utilisateur jouait au même moment (son jeu 16:05:01 à 16:09:45, mes passages cachés 16:05:39 à 16:08:15) : dossiers `Saved` et `Saved_Tests` distincts, aucune interférence vue au journal, mais non exclue.
4. Mise en file à population pleine : `Command feedback: population full` + message « Population maximale — construisez des maisons » ET `Production: slot 0 queued Peasant` avec 50 de nourriture payés (journal de l'utilisateur et essai de casse A) : refus et acceptation à la fois (domaine de D829).

## Rapport (13 champs)

```
APPELS_DECLARES: 36
HEURE_DEBUT/HEURE_FIN: 16:03 / 16:26
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: 16:03 vide (Galadriel)
TEST_ROUGE: sans objet (recette, aucun code)
COMMITS: 0, merge_main_fait: oui (avance rapide f0f8ddd..2beb47d a 16:03)
TESTS: preflight alert_population FAIL(1) x2, arquebusier PASS, nomad_start PASS, deploy PASS ; TESTS_TAUTOLOGIQUES: sans objet
ESSAIS: alert_population FAIL ; arquebusier PASS ; nomad_start PASS avec reserves ; deploy PASS (tous rejoues sur Galadriel, 12 PNG ouverts)
QUESTIONS: aucune
FAIT / RESTE: 4 essais rejoues, 2 essais de casse (A, B) ; reste : corriger alert_population (mapSize, texte du waitkey, clic du scenario en fenetre reelle), redonner les 3 commandes sur un binaire a jour
NON VÉRIFIÉ: ligne visible a l'ecran de l'utilisateur (seul le contenu du fil est journalise) ; cause du « Selection: none » en fenetre reelle ; erreur de script sur Sauron / Celeborn / Faramir (predite, non executee) ; son (-nosound) ; duel complet de l'Arquebusier ; maison + 2e remplissage d'alert_population
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: gate Essay faux rouge (anomalie 2) ; recette lancee dans le worktree ou l'utilisateur jouait (anomalie 3) ; 1 appel PowerShell perdu sur une parenthese
GIT_STATUS_PRINCIPALE: [M Auto_Journey_Jour.md / ?? Docs/Priorite_Socle_Factions_2026-11-16.md / ?? Docs/Rapports/] : inchangee hors Docs/Rapports/RECETTE-1116a.md (non suivi) : oui ; worktree Galadriel : []
```
