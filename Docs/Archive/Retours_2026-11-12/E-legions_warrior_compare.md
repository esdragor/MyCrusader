# [À ESSAYER] E-legions_warrior_compare : les cinq façons de voir la rage du Guerrier damné (D rien, A teinte, B anneau, C case, A+C), Entrée = option suivante

STATUT : LOGIQUE SEULEMENT   (preflight FAIL (1) : seul avertissement vu, « no Sanity: audit line » ; guerriers petits à l'écran, zoom à rapprocher avant de lister prêt)
GESTE: interne   (hol.Select.ClickUnit puis hol.HUD.Click GridCell02 : mêmes chemins que la souris, mais commandes `hol.*`)
SON: non entendu
branches: galadriel-tab

Pré-vol (2026-11-11 17:20, agent Galadriel-9, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, tip 3382f41 + travail non commité puis commité) : `preflight.ps1 legions_warrior_compare -Root ...` FAIL (1), 5 PNG, 27 s de jeu.
PNG VUS (planche des 5 captures, demi-taille, PAS en plein format) :
- legions_warrior_compare_D : rien de dessiné, bloc du bas « Guerrier damné », case E « Rage ».
- legions_warrior_compare_A : un guerrier plus sombre/violacé près de la Caserne (petit, à peine lisible).
- legions_warrior_compare_B : un anneau rouge sombre autour d'un guerrier près de la Caserne (visible, petit).
- legions_warrior_compare_C : case E « Rage 5 s » grisée, corps inchangé.
- legions_warrior_compare_AC : corps sombre ET case E « Rage » (la case sans secondes visibles sur cette planche).
NON VÉRIFIÉ : captures non ouvertes en plein format ; les cinq guerriers ne sont pas clairement côte à côte (ils se tassent près de la Caserne) ; effet de teinte à confirmer de près ; un seul agent, aucune oreille.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 legions_warrior_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 3 min. Réponse attendue : `D`, `A`, `B`, `C` ou `A+C` (dans Q-legions-warrior-rage-visual.md), ou `BUG: ...`.
