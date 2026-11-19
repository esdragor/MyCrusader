# RECETTE-1119d : bloc de selection du HUD, AVANT (C++) / APRES (script), worktree HeroesOfLiberty-Haldir, branche selection 3be6faf

## Pre-vols (lignes finales mot pour mot)
- ecran_hud_selection_batiment : `preflight ecran_hud_selection_batiment : FAIL (1)` (check : PASS (34 s) ; le seul FAIL est attendu : `user's launch: 1 uncommitted file(s)... ?? Content/Maps/Crown.umap` ; tous les OK : pas de HUD panels cover, 6 HUD rects, 5 captures presentes)
- ecran_hud_selection_unite : `preflight ecran_hud_selection_unite : FAIL (8)` = DEFAUT DU PANNEAU (voir ci-dessous)
- ecran_hud_selection_puces : `preflight ecran_hud_selection_puces : FAIL (1)` (check : PASS (39 s) ; seul FAIL = Crown.umap attendu)

## Defaut du panneau (ecran_hud_selection_unite), rien corrige
- `check ecran_hud_selection_unite : FAIL (a step timed out) (82 s)`
- `[2026.11.19-08.52.05:119][549]LogHOLTest: Error: Test sequence: timeout at step 38: waitlog HUD hover 0: attack: 8`
- Journal (C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Saved_Tests\Logs\scenario_ecran_hud_selection_unite.log), juste apres la commande hol.HUD.Hover 0 : `[2026.11.19-08.51.05:113][277]LogHOLUI: Display: HUD hover 0: no statistic shown` (attendu : `HUD hover 0: attack: 8`). Le survol de la premiere puce de stats ne trouve aucune puce / infobulle.
- Lignes de rapport a la selection du Lancier : `HUD selection name: none` ; `HUD stats: attack 8, melee armor 0, ranged armor 0, hero armor 0, speed 7.8` ; `HUD stats tip: none` ; `HUD report: ... selection Spearman (own), hp 80/80`. Le panneau affiche bien le Lancier (image), mais `HUD selection name` vaut none : a comparer a la ligne du binaire C++ (je n'ai pas le journal AVANT).
- Consequence : le scenario s'arrete a l'etape 38, donc 4 captures APRES manquent : hud_sel_lancier_survol, hud_sel_paysan_recolte, hud_sel_groupe_paysans, hud_sel_unite_ennemie (5 FAIL `no new capture`). Seule hud_sel_lancier existe (horodatee 09:51:05).

## Pixels differents (zone x 0..45 %, y 55..100 %, 864x486, seuil 30/canal ; meme resolution 1920x1080 pour toutes les paires presentes)
capture	pixels_diff	pct_zone	zone
ecran_hud_selection_batiment/hud_sel_arbre	8	0.002	864x486
ecran_hud_selection_batiment/hud_sel_chantier	212	0.050	864x486
ecran_hud_selection_batiment/hud_sel_fondation	0	0.000	864x486
ecran_hud_selection_batiment/hud_sel_forum_ami	10	0.002	864x486
ecran_hud_selection_batiment/hud_sel_forum_ennemi	6	0.001	864x486
ecran_hud_selection_unite/hud_sel_groupe_paysans	APRES ABSENT	-	-
ecran_hud_selection_unite/hud_sel_lancier	26	0.006	864x486
ecran_hud_selection_unite/hud_sel_lancier_survol	APRES ABSENT	-	-
ecran_hud_selection_unite/hud_sel_paysan_recolte	APRES ABSENT	-	-
ecran_hud_selection_unite/hud_sel_unite_ennemie	APRES ABSENT	-	-
ecran_hud_selection_puces/hud_puces_archers	17	0.004	864x486
ecran_hud_selection_puces/hud_puces_batiments_sous	9	0.002	864x486
ecran_hud_selection_puces/hud_puces_batiments_tous	9	0.002	864x486
ecran_hud_selection_puces/hud_puces_hommes_darmes	25	0.006	864x486
ecran_hud_selection_puces/hud_puces_tous	19	0.005	864x486

Fichier : C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\compare\selection_pixels.txt ; images : C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\compare\<scenario>\<capture>.png

## Les 3 images les plus ecartees (ouvertes)
1. hud_sel_chantier (212 px, 0,050 %) : seule difference = la barre de construction qui avance (191/600 31 % AVANT, 196/600 32 % APRES) ; mise en page identique (attendu, la construction progresse).
2. hud_sel_lancier (26 px, 0,006 %) : mise en page, textes et grille identiques ; difference de 1 a 2 px sur la position des points « · » separateurs des puces de stats (apres « Attaque 8 », « Armure mélée 0 », « Armure distance 0 », « Armure héros 0 »).
3. hud_puces_hommes_darmes (25 px, 0,006 %) : puces Tous/Archer/Homme d'arm./Paysan, contour jaune de la puce active, « Vous », « 3 inactifs » identiques a l'oeil ; ecart de quelques pixels au bord des puces (bruit ou decalage sub-pixel).

## Anomalies
- Defaut hover (ci-dessus) bloque ecran_hud_selection_unite. Pixels sinon tres proches (max 0,05 %, hors chantier en progression <= 0,006 %).
- Dessin du texte probablement leger decalage sub-pixel entre C++ et script (puces, points separateurs) : a regarder a l'echelle 1 par les agents de comparaison fine.
