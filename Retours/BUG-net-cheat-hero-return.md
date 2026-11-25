# BUG (ouvert, cause inconnue) : 2 rouges réseau sur main c386ed2 (net 2 clients, net hero return) — CLAUDE-1125ar

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
