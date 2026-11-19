# HALDIR-1119t : trois scenarios de preuve du bloc de selection du HUD (reprise 14)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (branche selection), commit 489d723 (9 fichiers neufs dans `Tools\Scenarios\`, jamais joues : ni build, ni preflight, ni play).
Forme commune : `.json` = modele ecran_hud_groupes (Plain, "mapSize": "Small", 0 Paysan, pas de heros, IA slot 1) ; `hol.Test.Sanity` en derniere ligne ; par etape `panel show`, `notereset`, `waitkey`, `panel hide` (panneau de notes ferme avant la capture, commande `panel hide|show` de HOLTestSequence.cpp:52), actions, `capture` ; IA figee (`hol.AI.ThinkSteps 100000`), `hol.Combat.Enabled 0`, `hol.Cheat.Resources 5000`, `hol.Cheat.Population 200`, langue fr, 0,5 s apres chaque naissance d'unites.

## 1. ecran_hud_selection_batiment (5 captures)
1. `hud_sel_forum_ami` : FocusSlot own, `hol.Select.Click 0.5 0.5`, waitlog `Selection: MainCentre of slot 0 (own)` (nom, Vous, sante, vies).
2. `hud_sel_fondation` : `hol.Build.PlaceNear House 3500 -2500` (aucun Paysan : personne ne batit), FocusNear, clic centre, waitlog `Selection: House of slot 0 (own), foundation` (texte de quick_check.ps1:1596).
3. `hud_sel_chantier` : 2 Paysans (SpawnHere), `hol.Select.Type Peasant`, touches A A (`hol.UI.Key A 0 0.1`, geste de quick_check.ps1:1701-1706), `hol.Build.Click`, waitlog `Build order of slot 0: 2 units to House at` + `Construction: slot 0 first work on House at`, `wait 2`, clic centre : barre + %.
4. `hud_sel_arbre` : `hol.Sources.Grove 160 -12000 3000 1 7` (un arbre), `hol.Camera.Focus`, `hol.Select.ClickSource -12000 3000`, « Reste : N ».
5. `hud_sel_forum_ennemi` : un Lancier a moi pose pres du forum ennemi pour la vue (`hol.Camera.FocusNear 3000 0 enemy` puis `0 0 enemy`), clic centre.

## 2. ecran_hud_selection_unite (5 captures)
1. `hud_sel_lancier` : Lancier pose seul (SpawnHere), `hol.Select.ClickUnit -1`, waitlog `HUD stats: attack 8,` (6 puces) ; `hud_sel_lancier_survol` : `hol.HUD.Hover 0`, waitlog `HUD hover 0: attack: 8` (unit_stats_attack.txt:54-55).
2. `hud_sel_paysan_recolte` : Paysan pose pres d'un arbre (Grove), `ClickUnit -1`, `hol.Select.OrderSource -12000 3000`, waitlog `Gather order of slot 0: 1 units to source`, `wait 4`, `hol.Units.Where 0` : ligne d'activite.
3. `hud_sel_groupe_paysans` : 4 Paysans de plus au repos, `hol.Select.Type Peasant`, waitlog `Selection: 5 units (own): Peasant x5` : 1 recolte + 4 inactifs.
4. `hud_sel_unite_ennemie` : Homme d'armes a moi + Lancier du slot 1 (`hol.Units.SpawnHere 1 Spearman 1 250`), `hol.Select.ClickUnit -1 enemy`.

## 3. ecran_hud_selection_puces (5 captures)
1. `hud_puces_tous` : 3 Paysans + 2 Hommes d'armes + 3 Archers (SpawnSeed 103, recette de quick_check.ps1:1434), `hol.Select.Box 0.25 0.25 0.75 0.7`, waitlog `Selection: 8 units (own)` : « Tous (8) » + 3 types.
2. `hud_puces_archers` : `hol.Select.Subgroup`, waitlog `Subgroup: Archer x3` ; 3. `hud_puces_hommes_darmes` : encore `hol.Select.Subgroup`, waitlog `Subgroup: ManAtArms x2` (ordre vu dans quick_check.ps1:1439-1441).
4. `hud_puces_batiments_tous` : 3 maisons par `hol.Cheat.SpawnBuilding House 0 4500 -1600|0|1600`, `hol.Select.Category Economic` (= F2, quick_check.ps1:1723), waitlog `Category Economic: ` : « Tous (4) » + « Maison (3) » + forum.
5. `hud_puces_batiments_sous` : `hol.Select.Subgroup`, waitlog `Subgroup: `.

## Commandes dont je ne suis pas sur
- Aucune commande inventee : toutes vues dans Source\ (definitions) ou quick_check.ps1 / scenarios existants. Fichiers : `hol.Select.Subgroup|Category|Box|Type|ClickSource|ClickUnit` et `hol.Build.PlaceNear|Click`, `hol.Camera.FocusNear|FocusSlot|Focus` dans `Source\HeroesOfLiberty\Player\HOLPlayerController.cpp` ; `hol.Sources.Grove` dans `Resources\HOLSourceSubsystem.cpp:117` ; `hol.HUD.Hover` dans `Source\HeroesOfLibertyUI\Dev\HOLUIHUD.cpp:144` ; `panel hide|show` dans `Dev\HOLTestSequence.cpp`.
- `hol.UI.Key A 0 0.1` : forme vue dans ecran_hud_file.txt ; la meme touche que le `Key A` de quick_check.ps1 (equivalence supposee).
- Texte du journal `Gather order of slot 0: 1 units to source` : copie de quick_check.ps1:1536 ; non retrouve par grep dans Source (format probablement construit autrement).

## NON VERIFIE / non mis en scene
- Rien n'a ete joue. Le clic au centre de l'ecran (`hol.Select.Click 0.5 0.5`) pour la fondation, le chantier et le forum ennemi suppose que le point de visee de FocusNear est au centre : si un Paysan est sous le curseur au chantier, la capture montre un Paysan (la ligne waitlog ne garde pas ce cas : j'ai laisse `hol.Select.Report` sans waitlog pour ne pas faire echouer le scenario).
- Etape arbre : `Selection:` pas de waitlog (texte du journal d'une source non connu). Le point (-12000, 3000) suppose le forum slot 0 en (-14800, 0), Plain a 2 joueurs (quick_check.ps1:984).
- Forum ennemi : visibilite sous brouillard non verifiee (je pose une unite a moi a 30 m pour le voir).
- Le % du chantier avance pendant la capture : comparer la mise en page, pas la valeur.
- Paysan qui recolte : l'etat d'activite change (marche / recolte / depose) : l'ordre est donne a 2 m de l'arbre, mais le forum est a ~28 m.
- Les etiquettes de puces (« Homme d'armes (2) », « Tous (4) » avec forum + 3 maisons) sont attendues dans les `.look` : a corriger apres le premier passage si l'etiquette reelle differe (la categorie Economic peut contenir plus que le forum et les maisons).
- Les notes contiennent des guillemets francais en UTF-8 (comme les modeles).
