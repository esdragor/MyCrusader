[FAITS, pas une question] Celeborn-14, 2026-11-07 13:20, hero-j8-close, binaire b125064 + 39d5ea3
Voir NeutralsInProgress.md (entree Celeborn-14) du worktree HeroesOfLiberty-Celeborn ; sorties brutes : C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\c14\.
- tests 1092/1092, combat PASS ; walk UNANSWERED 0/8 (Q5+FinalLeg), 0/4 (FinalLeg seul), 1/4 (Q5 seul), 4/4 rouge (tout coupe).
- ROUGE restant : Sanity spins dans diplomacy_qground_watch (Q5+FinalLeg 2/8, #12 24 reversals ; FinalLeg seul 1/4 ; tout coupe 3/4) et melee_stuck 1/4 (#12). Le spin existe sans correctif : il n est pas cause par Q5/FinalLeg, mais bloque le vert. WIP non retire, non valide.
- NoStrike : 0 regression.

## 2026-11-10 10:05-10:3x, Celeborn-27 (hero-j8-close, SANS C++, binaire du 2026-11-09 17:24 avec ShouldKeepTargetBothFull) : mesure du spin de melee, A/B N=8
Commande : `repeat_scenario.ps1 -Name <sc> -Runs 8 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, les deux scenarios en parallele (copies de binaires distinctes). Chiffres lus dans la sortie des outils, journaux `Saved_Tests\Logs\scenario_<sc>_run1..8.log` du worktree.
- **melee_stuck (cvar hol.Combat.RearKeepBothFull = 0, base) : PASS 5/8** (79 / 89 / 140 s). Echecs : #12 spin 551 deg en 3 s + #11 shake 24 reversals ; #6 spin 944 deg en 3 s ; #8 spin 8 s 28,7 rad.
- **melee_stuck_on_bf (cvar a 1) : PASS 7/8** (70 / 86 / 133 s). Seul echec : #11 slot 1 spin 8 s 13,7 rad, 2 reversals.
- Base connue 3/8 le 2026-11-09 ; 5/8 ce jour (bruit de n=8). Seuil de la ligne (>= 6/8 sur on_bf) atteint, mais l ecart avec la base (2 passes sur 8) n est pas significatif.
- Defaut du cvar NON change (0), aucun C++. Proposition notee en tete de NeutralsInProgress.md (worktree Celeborn) : passer RearKeepBothFull a 1, apres un A/B N=16 et `quick_check -Parts tests,combat`.

### Ligne 41 d Auto_Journey_File.md (D641) : playtest_hero_army et neutrals rejoues, meme binaire, cvar RearKeepBothFull 0 (defaut), `repeat_scenario.ps1 -Name <sc> -Runs 4 -Root Celeborn` (check.ps1 = meme audit que le pre-vol ; pas de `preflight.ps1` lance)
- **playtest_hero_army : PASS 0/4** (118 / 118 / 123 s), ROUGE a chaque passe, toujours le Sanity « spins » : #28 slot 1 727 deg en 3 s (passes 1 et 3, meme point (-15261, 944)), #14 slot 0 575 deg en 3 s (passe 2), #22 slot 1 746 deg en 3 s (passe 4) ; la passe 1 et 3 listent aussi #18 slot 0. Les deux camps tournent, pas seulement le slot 0 comme le 2026-11-05 (BUG-playtest_hero_army-spin.md).
- **neutrals : PASS 2/4** (60 / 60 / 69 s). Echecs : passe 1 #12 slot 0 spin 8 s, path 2749 cm, net 1261 cm, 37,8 rad, 8 reversals ; passe 3 #9 slot 0 547 deg en 3 s.
- Non teste avec RearKeepBothFull 1 (aucune variante `_on_bf` de ces deux scenarios n existe ; en creer une est faisable sans C++ : copier le .txt, le .json et le .look, ajouter la ligne `hol.Combat.RearKeepBothFull 1` apres `hol.Sanity.Watch`). Les fichiers BUG-melee-spin-sanity.md et BUG-playtest_hero_army-spin.md n ont PAS ete modifies (lecture seule dans l enonce) : les chiffres ci-dessus sont a y reporter par la session principale.

