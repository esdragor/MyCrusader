# DIAG-1119b : preuve MarchMemberDetourSteps 0 vs 8 (quick_check -Parts formation), 2026-11-19

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` (branche maps). Aucun build, aucun commit, aucun fichier édité (le paramètre existait déjà).

## Moyen
`quick_check.ps1 -Parts formation -GameExec "hol.Units.MarchMemberDetourSteps 0"` : paramètre `$GameExec` (quick_check.ps1:120, inséré dans le `-ExecCmds` du jeu à :2746). Rien à retirer après.
Preuve que la variable valait 0 : `Saved_Tests\Logs\quick_game.log` ligne 32/463 (`-ExecCmds="hol.Combat.Enabled 0,hol.Test.SkipWaitRendering 1,hol.Units.MarchMemberDetourSteps 0,hol.Test.Run ..."`) et ligne 1997 `hol.Units.MarchMemberDetourSteps = "0"` (10:30 heure locale).

## Résultat avec 0
```
march, held, file gap, files formed after (steps) per block march: 102/0/44/7, 109/0/39/7, 65/0/39/12, 80/0/32/1, 94/0/35/4, 103/0/40/44, 111/0/38/5, 105/0/38/6, 110/0/39/7, 116/0/36/7, 121/0/39/10, 83/0/38/5
arrival backsteps (steps/cm) per block march: 0/0, 0/0, 1/2, 0/0, 0/0, 3/16, 0/0, 0/0, 0/0, 0/0, 2/8, 0/0
backsteps : PASS
quick_check : PASS in 66 s (game launch done at 64 s)
```
Aucune ligne `[KO]`. File gaps : 44, 39, 39, 32, 35, 40, 38, 38, 39, 36, 39, 38 cm (tous sous 60, retour à la fourchette 32-40 de MV.2).
Référence avec 8 (défaut actuel, mesure de DIAG-1119) : 174, 39, 138, 32, 37, 40, 128, 55, 39, 36, 82, 61 cm, `backsteps : FAIL`, `[KO] files: file gap 60 cm at most`.

## Verdict
HYPOTHÈSE PROUVÉE : seul `MarchMemberDetourSteps` 8 -> 0 fait passer la partie formation du rouge au vert (mêmes graines, simulation déterministe). Seconde piste (`SlotDetourMaxSteps`) : non jouée, inutile.

## Ce que la valeur 8 apportait (Docs/Archive/Retours_2026-11-15/Q-march-member-detour.md)
- Défaut corrigé : le tremblement de la marche en bloc (deux soldats du même bloc qui convergent vers leur place, cap qui alterne à chaque pas, scénario neutrals_trace / nt_run3). Test C++ `MarchPairNoTremble` (HOLAvoidTests.cpp:124, mirroir script HOLAvoidRules_Test.as) passe avec 8.
- Mesure x16 du 2026-11-15 (CELEBORN-1115c) : tremblement vu 3 fois sur 24 passes avec 0 (~1 passe sur 10), jamais avec 8. Les « spins » restants (boucles de seconde ligne en mêlée, D641) sont identiques dans les deux cas.
- Réponse de l'utilisateur : `Reponse (2026-11-15) : A : garder 8 pas`, reportée dans le GDD en D815. Le risque annoncé à l'époque était « écart de la file (MV.2), borné à 8 pas (~25 cm) » ; la mesure d'aujourd'hui montre jusqu'à 174 cm, donc la borne annoncée était fausse et la question mérite d'être reposée avec ce chiffre.
- Conséquence visible à présenter : garder 8 = pas de tremblement de paire (rare, ~10 % des passes) mais des soldats d'un bloc jusqu'à 1,7 m hors de leur file en marche + 5 des 12 marches de la partie formation en rouge ; revenir à 0 = files tenues (32-44 cm) mais le tremblement de paire peut revenir, rarement.
- Options de compromis (non essayées) : valeur 1-2 (mémoire plus courte), ou mémoire du détour de membre seulement hors marche en bloc (arrivée / mise en place). À rejouer avec `MarchPairNoTremble` ET `-Parts formation`.
