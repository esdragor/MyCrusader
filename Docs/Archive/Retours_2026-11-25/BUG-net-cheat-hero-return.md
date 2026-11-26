# BUG FERMÉ (2026-11-25 fin de soirée, integ-92-art dans main 714fe0b : net 2 clients PASS, net hero return PASS) : 2 rouges réseau causés par 7e0613c, corrigés par 86e5398 — CLAUDE-1125ar

Trouvé par l'intégration integ-86-art (chantier Artistes) le 2026-11-25 20:13 et confirmé SUR MAIN SEUL : ce n'est pas une
régression des branches art-*.

Mesures (quick_check -Parts net) :
- integ-86-art (main + 4 branches art-*), 2 passages (20:13 dans integrate.ps1, puis 20:3x) : mêmes 2 KO.
- main c386ed2 seul, worktree Celeborn (binaire de 17:36, aucun changement de Source/Script/Config entre 8531390 et main),
  21:0x : mêmes 2 KO. Les 18 autres cas net passent.

Lignes KO :
- `net 2 clients` : `[KO] server: client 1's cheat refused (1), the army's granted (4 of 2)`
- `net hero return` : `[KO] client 2 : Alice's gold not lowered by her hero's return (4160, 1000)`

Pistes (non vérifiées, L29-3 : run instrumenté d'abord, aucun correctif avant une cause vue au journal) : commits de code de
main entre la dernière passe net verte et 8531390 (voir `git log 8531390 -- Source Script`), en particulier les triches et le
retour du héros ; « 4 of 2 » ressemble à une triche d'armée comptée deux fois.

Contournement : les 2 cas sont inscrits dans `Tools\Tests\known_reds.txt` (branche integ-86-art) pour ne pas bloquer
l'intégration ; retirer les lignes quand ils repassent.

## Cause (CELEBORN-1125ar10, 2026-11-25 ~23:00, VUE au journal)

- Dernier vert : integ-76 (fcd3250, 2026-11-24 22:51, `net 2 clients : PASS`, `net hero return : PASS`) ; aucune passe net
  entre integ-77 et integ-85 ; premier rouge : integ-86-art (20:13). Plage : `fcd3250..c386ed2`, 65 commits Source/Script/Config.
- Coupable : **7e0613c** (GALADRIEL-1125ma, 09:58, « hol.Cheat.Resources gives the amount to every slot », NON COMPILÉ, sans
  test) : `AHOLPlayerController::DoCheatResources` donne le montant à TOUS les PlayerState de `PlayerArray`.
- Vu au journal serveur (`HeroesOfLiberty-Celeborn\Saved_Tests\Logs\quick_net_server.log`, passe rouge 21:07) : chaque triche
  d'armée écrit `Cheat: slot 0 resources` ET `Cheat: slot 1 resources` dans la même frame (l.2450-2454 : 4 lignes pour 2 triches
  → « 4 of 2 ») ; après `Test mark: net hero return` (l.2781), les triches d'or de Bob touchent aussi le slot 0 (l.2785 `slot 0 ... G 0`,
  l.2795 `slot 0 ... G 1000`) → l'or d'Alice 4160 puis 1000. Journal vert du 2026-11-24 (Sauron) : après la marque, slot 1 seul.
- Test rouge : 5fa19b9 (quick_check, partie net : « Bob's resources cheats reach his slot only ») ; rouge sur le journal rouge
  (10 lignes, 5 sur un autre slot), vert sur le journal vert (5, 0).
- Correctif : 86e5398 (branche net-reds-1125ar) : l'appelant + les slots IA de la LaunchConfig (besoin FARAMIR-1124si gardé),
  jamais un autre humain. Build PASS. quick_check -Parts net 23:02 : 19/19 PASS (les 2 cas verts) ; lignes retirées de known_reds.txt (679d723) ; merge main aa811cb (non retesté).

## Vérification (CELEBORN-1125ar11, 2026-11-25 23:04-23:2x, worktree HeroesOfLiberty-Celeborn, net-reds-1125ar)

- **quick_check -Parts tests 23:05-23:09 : 2101/2303 FAIL (202 KO, REGRESSION)** — NON PROBANT : la DLL du worktree (22:56,
  build de 86e5398) est PLUS VIEILLE que le Source fusionné par aa811cb (merge main = 19 fichiers C++, +790/-283, dont
  l'art sans repli CSV ; `Get-StaleBinaryProblem` : HOLRegicideTests.cpp 23:02:59 > DLL 22:56:42). Au journal
  (`Saved_Tests\Logs\quick_tests.log`) les tests de monde de match tombent sur `Unit type 'Peasant' has no mesh ()` puis
  `MassMovableVisualizationTrait ... validation failed` ; plus `Test_TableImportSiegeTechnologies` (SiegeWorkshop absent de
  la table). L'énoncé disait « binaire à jour » : faux depuis le merge. **À faire par la principale** :
  `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` puis
  `Tools\Tests\quick_check.ps1 -Parts tests,net` dans ce worktree.
- **Besoin FARAMIR-1124si rejoué** : scénario `ai_tower` (Plain, slot 0 humain, slot 1 IA Normale) ; commit 8dfcb70 y ajoute
  `waitlog Cheat: slot 1 resources` juste après `hol.Cheat.Resources 1000`. `preflight.ps1 ai_tower` 23:10-23:13 :
  la séquence va au bout (135 s, toutes les waitlog venues) ; journal `scenario_ai_tower.log` l.2383 :
  `Cheat: slot 1 resources, now F 1150 W 1100 S 1200 G 1200` puis l.2384 `Cheat: slot 0 resources ...` → **la triche
  atteint bien le slot IA en solo** ; l'IA pose sa TowerWood (l.2632, step 2214) et l'achève (l.2670).
  preflight **FAIL (3)** pour des causes hors triche : DLL périmée (lancement utilisateur) et audit de santé (2 unités du
  slot 1 qui tournent sur place, #7 et #19). PNG `ai_tower_built` ouvert : barre du haut, note 1 encore affichée ; la tour
  n'est pas reconnaissable avec certitude (forme provisoire grise + mât sombre à embout orange en haut à gauche, anneau
  gris à droite) : LOGIQUE SEULEMENT pour l'image. Rouge du waitlog non vu à l'exécution (il faudrait un binaire d'avant
  7e0613c, où seul `slot 0` s'écrivait, FARAMIR-1124si).

## Clôture

integ-92-art (art-anim-1125ar + net-reds-1125ar) : build PASS, tests 2314/2314, parties net,build PASS ; `net 2 clients : PASS`, `net hero return : PASS` ; les 2 lignes de known_reds.txt retirées (679d723). main = 714fe0b.
