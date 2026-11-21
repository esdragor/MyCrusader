# DIAG-1121sa (2026-11-21, 19:32 - 19:35)

Worktree : C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branche integ-42.

CAUSE : piste n°1. TowerWood est maintenant en case F (age I) de la grille des Paysans (fc901f7 / D878 : BuildingTypes.csv,
HOLCommandGrid_Test). L'attente de `Tools\Tests\quick_check.ps1` n'avait pas la case : elle etait fausse, pas le code
(le rapport HUD donne bien `...S=ArcheryRange greyed F=TowerWood greyed W=Farm...`, cellules `I F: Tour de guet en bois`,
tooltip `I F: Tour de guet en bois | W 100 missing`). Piste n°2 (fragment de moral) : sans rapport, aucun test build touche.

Regexes corrigees (quick_check.ps1, 6 chaines, 1 occurrence chacune) :
- L4267/4268 (build hud, grille) : `F=TowerWood greyed` entre S=ArcheryRange et W=Farm + libelle.
- L4271 (build hud, cellules) : `I F: Tour de guet en bois` entre `I S` et `I W`.
- L4273 (build hud, tooltips) : `I F: Tour de guet en bois \| W 100 missing` entre `I S` et `I W`.
- L4513 (build barracks) : `I S: Champ de tir; I F: Tour de guet en bois; I W: Ferme` + libelle.
BOM conserve, analyse PowerShell : 0 erreur.

Correctif : commit 9e7a324 sur integ-42 (quick_check.ps1 seul, aucun C++, aucun build).
quick_check -Parts build apres correctif : PASS in 115 s (build hud PASS, build barracks PASS, toutes les 37 etapes PASS, aucune REGRESSION).

Autres branches :
- Elrond integ-25 (TowerWood case 7) : meme correctif necessaire, sinon build hud / build barracks y rougissent aussi
  (cherry-pick 9e7a324 ou meme edit des 6 chaines). Deja inclus dans integ-42, donc a l'integration finale il n'y aura
  pas de conflit si integ-25 ne touche pas ces lignes ; sinon prendre cette version.
- Haldir wall-line (SecondaryCentre page II case 1) : meme famille de regexes, a prevoir. La page II attendue dans
  quick_check (L4268 `II=locked [A=Market ... W=Forge greyed]`, L4271 `II A: March.+`, L4273 `II A: March.+ \| W 150 ...`,
  `II Z: Palier II` etc.) devra recevoir la case SecondaryCentre a sa place (selon la case 1 reelle de la grille), meme
  ordre que le rapport HUD ; verifier aussi `Script\Tests\HOLCommandGrid_Test.as`. A confirmer par le quick_check build de
  cette branche (non lance ici).

NON VERIFIE : l'ordre exact d'un `II` avec SecondaryCentre (Haldir) ; quick_check autres parties que `build`.
Aucun processus lance hors quick_check (termine). Aucun autre worktree touche.
