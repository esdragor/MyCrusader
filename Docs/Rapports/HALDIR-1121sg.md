# HALDIR-1121sg : essai du centre secondaire (D879), worktree HeroesOfLiberty-Haldir, wall-line

Impact joueur : l'essai n'est PAS encore listable (pas de Retours\E-secondary_centre.md) : le geste de pose marche (Z, Z, clic) et les Paysans batissent le centre, mais l'etape "selection du centre fini" echoue.

APPELS_DECLARES: 25 vus (arret atteint ; ~5 de plus pour le rapport)
HEURE_DEBUT/HEURE_FIN: 20:17:39 / 20:25:36 (+ rapport)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario, pas de correctif de code) ; assertit l'effet : oui (waitlog sur "Construction: slot 0 finished SecondaryCentre")
COMMITS: 0 (fichiers non commites : Tools\Scenarios\secondary_centre.txt / .look / .json), merge_main_fait: non
TESTS: preflight secondary_centre lance 3 fois : FAIL (6). TESTS_TAUTOLOGIQUES: 0
ESSAIS: secondary_centre : preflight FAIL, PNG NON ouverts (aucun relu), LOGIQUE SEULEMENT, pas de E-*.md ecrit. Commande de rejeu (quand corrige) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 secondary_centre -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
QUESTIONS: aucune

FAIT :
- secondary_centre.txt / .json / .look ecrits (Plain Small, 5 Paysans, hol.Cheat.Tier 1 0, 5000 ressources, hol.Build.Speed 6, waitkey entre les etapes, captures sc_grille_age2, sc_grille_infobulle, sc_pendant, sc_apres, sc_forum ; OBJET nomme dans le .look).
- GESTE REEL prouve par le journal : hol.Select.Grid 1 (touche Z : "Grid: age 1 chosen"), hol.Select.Grid 1 (touche Z : "Placement: SecondaryCentre (cost F 0 W 150 S 400 G 0)", "Placement ghost: SecondaryCentre at (-16800, -6000) valid"), hol.Build.Click (clic) -> "Construction: slot 0 placed SecondaryCentre foundation ... paid F 0 W 150 S 400", "Build order of slot 0: 5 units", 5 batisseurs, "finished SecondaryCentre after 190 steps ... speed 6". Cout 150 bois + 400 pierre confirme.
- Emplacement : FocusNear 4500 3500 = "off the navmesh" ; FocusNear -3800 4600 = "out of the map" (le slot 0 est au bord ouest, centre ~(-17800,-1800)) ; FocusNear 0 -4500 = valide (-16800, -6000).

RESTE (cause de l'echec, a faire par l'agent suivant) :
1. Etape 4 : `hol.Select.Click 0.5 0.5` sur le centre fini donne "Selection: none" (le clic ecran ne touche pas le batiment ; le forum est trouve ainsi, pas le centre secondaire place a 4,5 m de la camera). Solution : cadrer la camera sur le centre avec hol.Camera.Focus -16800 -6000 (absolu) avant le clic, ou chercher la commande hol.Select.* qui prend le batiment par type (Grep "hol.Select" dans HOLPlayerController.cpp). Les etapes sc_apres et sc_forum n'ont donc jamais ete capturees.
2. Page II de la grille : le journal montre seulement les cellules de l'age I dans "HUD tooltips" ; "Z=SecondaryCentre" en page II n'est PAS lu dans la sortie (la ligne "grid ages II" du HUD report non relevee faute d'appels). A verifier en relisant Saved_Tests\Logs\scenario_secondary_centre.log (HUD report apres hol.Select.Grid 1).
3. preflight : FAIL "user's launch: 3 uncommitted file(s)" = commiter secondary_centre.txt/.look/.json ; WARN frames (corrige : wait) ; WARN "no Sanity" apparait seulement a cause du timeout (hol.Test.Sanity est en fin de scenario).
4. Puis ouvrir chaque PNG (Saved_Tests\Screenshots\WindowsEditor\HOL\sc_*.png), verifier "seul dans son groupe" (hol.Select.Report) et la grille du centre = celle du forum, puis ecrire Retours\E-secondary_centre.md depuis Tools\AutoJourney\E_template.md (GESTE: reel, SON: non entendu, branches: wall-line).

NON VERIFIE : aucun PNG ouvert ; page II de la grille ; selection seul-dans-son-groupe ; grille du centre = grille du forum ; infobulle de la case Z ; modele SM_Forum a l'ecran ; E-secondary_centre.md non cree ; EtapeDefini.md et SecondaryCentersInProgress.md non mis a jour (plafond d'appels).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: preflight dure 85 a 100 s par passage ; le check refuse le scenario tant que ses 3 fichiers ne sont pas commites (FAIL "uncommitted").
GIT_STATUS_PRINCIPALE: non relu (aucun fichier de la copie principale touche hors ce rapport) : oui
