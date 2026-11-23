# ELROND-1123pb : ai_garrison montre l'entree des archers de l'IA (S4-b), 2026-11-23 15:18 -> 15:24

Impact joueur : le joueur voit les archers de l'adversaire disparaitre dans sa tour attaquee (3 sur 4) ; il ne voit PAS la garnison d'une tour ennemie (information cachee par regle).

- Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, branche integ-25, commit bbd8947 (Agent: ELROND-1123pb) : Tools/Scenarios/ai_garrison.txt et .look (.json inchange). Aucun Source\ ni Script\ touche, pas de build. integ-25 PRETE a bbd8947 (merge main non refait : deja fusionne dans 3994a82).
- Comment le jeu montre une garnison (lecture) : un soldat garnisonne sort de la liste des unites dessinees (HOLUnitSubsystem.cpp GetUnitViews, `!Unit.Value.Garrison.IsValid()`) = il disparait ; occupants repliques AHOLBuilding::GarrisonOccupants en COND_OwnerOnly (HOLBuilding.cpp:136) et vue/compteur « n / places » (HOLGarrisonView.as, HOLSelectionReadLibrary) : proprietaire seul. Journal de l'entree effective, distinct de l'ordre : `Garrison: slot 1 puts 3 of 3 units in TowerWood (3 places, 0 inside before)` (HOLGarrisonSubsystem.cpp:193) ; l'ordre est `Garrison order: slot 1, 3 unit(s) queued (AI)` (HOLAIGarrisonOrders.cpp:38).
- Scenario : zoom 4 crans (hol.Camera.Zoom 4), capture `ai_garrison_during` apres waitunits 4, puis `waitlog Garrison order: slot 1` + `waitlog Garrison: slot 1 puts` (plus de `wait 3` aveugle), frames 20, capture `ai_garrison_after`. 3 notes. Tour selectionnee : non (c'est la tour ennemie, aucune vue d'occupants pour le joueur).
- Preflight : 2 rejeux (1er : FAIL seulement `.look` non commite ; dernier apres commit : PASS entier, 31 s, journal propre, audit sanity OK, 3 PNG). Un seul WARN : pas de `HUD overlap:`.
- PNG ouverts (dernier rejeu) : before = tour seule ~105 x 130 px, Forum de l'IA en haut a droite ; during = tour ~300 x 260 px, Hommes d'armes bleus a gauche, groupe d'environ 4 silhouettes rouges serrees au pied (compte exact illisible, elles se chevauchent) ; after = plus qu'une silhouette rouge dehors, bleus toujours la, panneau de notes en retard d'une note.
- Limite honnete : "during" est pris au moment de l'ordre (les archers apparaissent a ~3 m et l'entree suit 1 frame plus tard, journal frames 704 -> 705) : on ne voit pas les archers marcher, seulement avant/apres.
- Fiche Retours\E-ai_garrison.md : VU A L'ECRAN, GESTE interne, 1re ligne « branche integ-25, pas encore dans main », play.ps1 en chemin absolu, ~35 s.
- Suivis : FortificationsInProgress.md l.264, EtapeDefini.md l.49. Auto_Journey_File.md : non touche (ligne deja cochee par 1123pa). Placeholders.txt : aucun provisoire ajoute ni retire (non relu en entier : NON VERIFIE).
- Proposition de ligne `[ ]` (design a trancher, pas inventee) : affichage de la garnison d'une tour ENNEMIE (indicateur « n archers dedans » visible de l'adversaire ?), aujourd'hui COND_OwnerOnly.

## Champs
- TEST_ROUGE: sans objet (scenario ; le rouge de l'etat precedent = LOGIQUE SEULEMENT de 1123pa) ; TEST_ROUGE_SEPARE: non
- BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: sans objet (aucun .as touche)
- EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun ; IN_USE.txt entree et sortie faites (fichier non suivi par git).
- NON VERIFIE : compte exact des archers sur "during" ; marche des archers avant l'entree ; Placeholders.txt.
- Anomalie : warning LF -> CRLF au commit (fichiers scenario en LF) ; la note 1 du scenario garde des guillemets « » d'origine.
