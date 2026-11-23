# SAURON-1123pb : wall_line_long au geste reel (integ-55)

APPELS_DECLARES: ~19 (arret a 25 / 15:34)   HEURE_DEBUT/HEURE_FIN: 15:18 / 15:30
COMMITS (branche integ-55, Agent: SAURON-1123pb) : 6eb590a test play_hidden_test (ROUGE vu 3:20 : FAIL sur le cas hol.Mouse.Drag) ; 9e285e1 play.ps1 (test vert : play_hidden_test PASS) ; 0e7144e scenario + .look. Tip 0e7144e. merge main : non fait (DLL non rebuildee avec le Source de main).
TEST_ROUGE: vu a 15:20 : play_hidden_test FAIL (1) / TEST_ROUGE_SEPARE: oui   TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 1 (cas 1b de play_hidden_test.ps1, existant)   TYPES_A_EXPOSER: aucun   DIAGNOSTICS_AS: sans objet   EDITS_REFUSES: 0   PROCESSUS_DETACHES: 0

## Fait
1. play.ps1 (pas de BOM, analyse OK) : `$UsesRealMouse` (ligne `hol.Mouse.Drag` dans le .txt) ; sous -Hidden, `hol.Camera.TestCursor 0.5 0.5` n'est plus pose pour ce scenario ; les autres scenarios inchanges (play_hidden_test PASS, dry-run de wall_line_long sans TestCursor).
2. wall_line_long.txt : Paysan spawne et selectionne (TestCursor 0.5 0.5 le temps du ClickUnit, puis off), hol.Select.Grid 0 puis 6 (A puis D = Palissade), camera centree (-17400, 0), hol.Mouse.Drag 200 360 1000 360 90, waitlogs sur les textes exacts du code (`Placement: Palisade (cost`, `Drag end: foreground=`, `Placement: Palisade wall line (`, `RpcOrder: place wall line received by the server`), captures during / after. .look : objet, taille en pixels.
3. preflight wall_line_long : 1er essai FAIL (ClickUnit a 40 images de la camera, fichier .look non commite), 2e essai PASS (47 s). PNG ouverts : during = Paysan selectionne, grille age I, 4 boites vertes plates (tiers du glisser), aucune rouge, Bois 20200 ; after = 9 fondations de palissade alignees, plus de boite verte, Bois 20155 (-45 = 9 x 5). Journal : ligne (-17711,-1375) -> (-17711,1982), pas de refus.
4. Refus : `refused (3)` = EHOLWallLineRefusal::Occupied ; HOLWallLineRules.as Validate : « All or nothing: one refusal and no cell is laid ». D867 et D691 du GDD ne le disent pas : question Retours\Q-ligne-case-bloquee.md [A REPONDRE], defaut A (comportement actuel).
5. Retours\E-wall_line_long.md : VU A L'ECRAN, GESTE: reel, branche integ-55 en tete. File (Auto_Journey_File.md, ligne wall_line_long cochee), FortificationsInProgress.md, EtapeDefini.md mis a jour. IN_USE.txt : mis a "libre" a la sortie (l'entree OCCUPE n'a pas ete ecrite : oubli).

## NON VERIFIE / anomalies
- Le viewport cache fait 1066x600 ici (pas 1280x720 : le .look chiffre a 1280x720 est donc approximatif ; le .txt marche car la fraction de souris est proportionnelle) ; la capture during tombe au tiers du glisser (4 boites, pas toute la ligne).
- Construction des fondations par les Paysans, diagonale, ligne croisant un obstacle, souris materielle : non joues.
- Placeholders.txt : aucun provisoire ajoute ou retire.
- integ-55 PRETE a 0e7144e pour cet essai (binaire a jour 15:12, aucun C++ touche).