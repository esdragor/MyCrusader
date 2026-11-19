# SAURON-1119cm : scenario d'essai forge_lignes (D856)

Branche `forge-essai` (de `forge-lines` 30632dc), commit d70c184, non pousse, non fusionne. Fichiers : `Tools\Scenarios\forge_lignes.txt`, `.look`, `.json` (copie de `ecran_hud_grille.json`). AUCUN build, AUCUN jeu, AUCUN preflight lance : tout est NON VERIFIE. Lance 18:28, fini ~18:36, ~19 appels vus.

## Deroule (Entree = etape suivante, 4 waitkey, 4 captures, ~1 min de jeu + lecture)
Mise en place : Plain, 5 Paysans, IA figee, combat coupe, langue fr, 1920x1080, `hol.Cheat.Resources 5000`, `hol.Cheat.Tier 1` (PAR TRICHE : palier 1), `hol.Prod.BuildTime 2` (recherche de 2 s), Forge posee finie par `hol.Cheat.SpawnBuilding Forge 0 2500 2500` (TRICHE).
1. F3 (`hol.Select.Category Technological`, `waitlog Selection: Forge`) ; capture `forge_4_lignes` : W Lames affûtées, X Empennage, C Maille, V Barde.
2. VRAI clic souris `hol.Mouse.Click GridCell08` (W = case 8, modele ecran_hud_grille) ; waitlog `Production: slot 0 queued research MeleeAttackI` puis `Technology: slot 0 researched MeleeAttackI` (2 s, pas de triche d'octroi) ; capture `forge_niveau_2` : W = Lames affûtées II GRISEE (palier 2 manque, dit dans la note), X C V inchanges.
3. Ferme posee finie par triche en (-2500, 2500), selectionnee par F1 + Tab x2 (`hol.Select.Category Economic`, `hol.Select.Subgroup` x2, `waitlog Subgroup: Farm`, chaine de tech_grid.txt) ; capture `ferme_2_lignes` : W Charrue, X Brouette, C V vides.
4. Camp de bûcherons pose fini par triche en (2500, -2500), selectionne par `hol.Select.Click 0.5 0.5` apres `hol.Camera.FocusNear` (`waitlog Selection: LumberCamp`) ; capture `camp_1_ligne` : W Haches doubles seule.
Fin : `hol.Test.Sanity`. Camp de mineurs non fait (meme regle que le camp de bûcherons, dit dans la note 4).
Chaque capture : `hol.HUD.Click ScenarioClose`, `frames 10`, `hol.HUD.Report`, `capture`. Les captures `.look` nomment l'objet (texte exact des cases, accents du jeu : "Lames affûtées", "Camp de bûcherons", "Haches doubles").

## NON VERIFIE (a regarder au preflight)
- Tout : rien lance. `hol.Cheat.Tier 1` sans 2e argument vise le joueur local (usage `<0..3|off> [Slot|all]`, defaut -1) et le palier 1 suffit a allumer les niveaux I (comme forge.txt avec Hero.SetXP 0 600 ; ici pas de heros, json sans `startingHero`) : a confirmer sur le PNG 1 (cases allumees).
- Etape 4 : le batiment est-il bien au centre de l'ecran (0.5 0.5) apres `Camera.FocusNear` ? Sinon `waitlog Selection: LumberCamp` echoue (60 s) : ajuster le point de clic ou passer par F1 + Tab. Ordre des sous-groupes de F1 avec Ferme + Camp + Forum non connu : c'est pourquoi la Ferme est selectionnee AVANT la pose du Camp.
- Etape 2 : apres le clic la souris reste sur la case W : son infobulle (Lames affûtées II) peut s'afficher au-dessus de la case sur les captures 2 a 4 (n'empiete pas sur la rangee du bas ; a dire a l'utilisateur si gene).
- `waitlog Camera above the start of slot 0`, `Cheat: spawned LumberCamp of slot 0` : formats pris de ecran_hud_grille / forge.txt / tech_grid.txt, LumberCamp non vu dans un scenario existant (nom de type deduit de HALDIR-1119cl : `LumberCamp`).
- Accents UTF-8 sans BOM dans le `.txt` (waitkey, notes) et `.look` : seulement "affûtées" et "bûcherons" ; si l'affichage de la note est casse, remettre sans accent.
- Les lignes `note` etapes 1 a 4 sont en ASCII + ces accents ; le preflight verifie notes, waitkey, 1 capture par point `.look`.

## Ecart
Pas de clic souris sur un batiment pour l'etape 3 (Tab, deja prouve dans tech_grid.txt) ; etape 2 = vrai clic ; selection des batiments = commandes `hol.Select.*` (internes, pas de geste de souris sur le batiment sauf etape 4 via `hol.Select.Click`). Donc GESTE: interne pour la selection, reel pour la recherche.

## BROUILLON de la fiche d'essai (`Retours\E-forge_lignes.md`, a creer apres preflight PASS)
```
# [À ESSAYER] E-forge_lignes : la Forge montre 4 cases (Lames affûtées, Empennage, Maille, Barde), chaque case passe au niveau II après recherche ; la Ferme en montre 2, le Camp de bûcherons 1 (D856)

STATUT : VU À L'ÉCRAN | LOGIQUE SEULEMENT   (à fixer après lecture des 4 PNG)
GESTE: réel pour la recherche (clic souris sur la case W) | interne pour la sélection des bâtiments (hol.Select.*)
SON: sans objet
branches: forge-essai, forge-lines   (le tip de forge-lines 30632dc doit etre dans le worktree de la commande ; forge-essai = le scénario)

Pré-vol (<date heure>, agent <nom>, worktree <chemin>, main <sha>) : `powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\preflight.ps1 forge_lignes -Root <worktree>` PASS (<durée>, 4 PNG).
PNG OUVERTS et vus :
- forge_4_lignes : <W Lames affûtées, X Empennage, C Maille, V Barde allumées ?>
- forge_niveau_2 : <W Lames affûtées II grisée, X C V inchangées ?>
- ferme_2_lignes : <W Charrue, X Brouette, C V vides ?>
- camp_1_ligne : <W Haches doubles seule ?>
NON VÉRIFIÉ : <à remplir : palier par triche, centrage du clic de l'étape 4, tous les camps (mineurs) non montrés>

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\play.ps1 forge_lignes -Root <worktree>`
Durée : environ 3 min (Entrée à chaque étape : 1 Forge, 2 niveau II après ton clic sur W, 3 Ferme, 4 Camp de bûcherons).
Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`. A regarder : (1) 4 cases de la Forge, aucune "II" au départ ; (2) W devient Lames affûtées II, grisée (palier 2 requis), les 3 autres inchangées ; (3) Ferme : Charrue + Brouette seulement ; (4) Camp de bûcherons : Haches doubles seule.
```
Commande de pre-vol : `powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\preflight.ps1 forge_lignes -Root <worktree>` (ouvrir les 4 PNG : forge_4_lignes, forge_niveau_2, ferme_2_lignes, camp_1_ligne). Prerequis : binaire/DLL de forge-lines compile (HALDIR : lot C++ de 5 fichiers, build `-LiveCoding=0 -Wait` a faire par la session principale).

## Champs
APPELS_DECLARES: ~19 ; PREMIER_BUILD: sans C++ ; BUILD: NON COMPILE (sans objet) ; LAUNCHCHECK_DEBUT: sans C++ ; TEST_ROUGE: sans objet (scenario, aucun code) ; COMMITS: 1 (d70c184, 18:3x), merge_main_fait: non ; TESTS: aucun, TESTS_TAUTOLOGIQUES: 0 ; ESSAIS: forge_lignes : preflight NON LANCE, PNG NON OUVERTS, LOGIQUE SEULEMENT en attendant ; QUESTIONS: aucune ; PROCESSUS_DETACHES: aucun (rien lance) ; ANOMALIES_PROCESSUS: aucune ; BUILDS_DEMANDES: 0.