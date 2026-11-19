# ELROND-1119q : pre-vol du scenario ecran_hud_fil (version script, worktree Elrond, tip 278ccdd)

## Pre-vol
- `preflight.ps1 ecran_hud_fil` : **PASS** au 1er passage (check 34 s, 50 pas, 17.2 s de jeu, 3 waitkey, 4 notes, aucun warning/erreur LogHOL, "no HUD panel covers another (4 report(s))", sanity OK). Ligne finale : `preflight ecran_hud_fil : PASS`.
- `preflight.ps1 ecran_hud_alerte` : **PASS** (check 32 s). Ligne finale : `preflight ecran_hud_alerte : PASS`.
- 1 passage, aucune correction du scenario, aucun commit (rien a committer).
- Journal : `HUD messages: []` -> `[Votre forum est tombe - 1 vie]` -> `HUD click MessageFeed: done` + `HUD: alert of slot 0 clicked (camera on its forum)` + `Camera above the forum of slot 0` -> `HUD messages: []` apres 8 s. `hol.Mouse.Hover MessageLine0` accepte (pas d'erreur).

## PNG (Saved_Tests\Screenshots\WindowsEditor\HOL\, copies dans scratchpad\apres\ecran_hud_fil\)
- hud_fil_aucune : haut droite, tableau des joueurs seul (Vous 2 vies / IA Normal 2 2 vies), aucune ligne de fil ; le panneau du scenario ("en preparation") est en haut centre, ne recouvre pas la zone.
- hud_fil_ligne : sous le tableau (x~1655-1905, y~120-143) une ligne sombre a bords arrondis, texte "Votre forum est tombe - 1 vie" en petit gris clair, lisible ; la banniere d'alerte est bien en haut centre ; le tableau montre "Vous 1 vie Ruine". Le nom n'est pas colore (le texte de la ligne n'a pas de nom de joueur, juste un libelle gris).
- hud_fil_apres_clic : identique a hud_fil_ligne (ligne encore la, banniere encore la), camera au-dessus du forum en ruine ; la camera avait ete deplacee sur le slot 1 avant le clic, le journal prouve que le clic l'a ramenee ("Camera above the forum of slot 0"), mais la capture ne se distingue pas de la precedente (la camera de hud_fil_ligne etait deja sur le forum) ; aucun effet visuel de survol visible sur la ligne.
- hud_fil_vide : haut droite, le tableau des joueurs seul, plus de ligne ni de banniere.

## Ecarts du panneau
- Aucune ligne absente, aucun texte faux, aucune erreur de script. Remarques d'observation seulement : (1) pas de couleur de joueur dans la ligne du fil (a comparer avec la version C++ / la note 2 du scenario qui annonce "ton nom dans ta couleur") ; (2) pas de changement visible au survol ; (3) la capture apres clic ne prouve pas visuellement le centrage de la camera (preuve par le journal seulement).

## Divers
- Scenario non modifie, pas de commit, pas de build ni quick_check.
