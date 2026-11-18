# SAURON-1118r : diagnostic du rouge `menu fr` (TC-Q12)

Verdict : (a) le CONTROLE etait perime. L'ecran n'a pas de defaut.

## Ce que le controle attend
`Tools\Tests\quick_check.ps1:3598-3600` (section journal "menu sizes", marque posee a la ligne 698) exige trois lignes :
- `Shown SlotTeam0: yes` (fin de ligne)
- `Shown SlotFaction0: yes, greyed`  <- perime
- `Shown TeamsTogetherDropdown: yes` (fin de ligne)
Le controle n'existe que dans la branche `$Culture -eq "fr"` (la partie "menu sizes" n'est jouee qu'en fr, lignes 695-701), d'ou un rouge fr seul.

## Ce que le jeu a ecrit (quick_game.log, section "menu sizes", rejouee 21:01)
```
Shown SizeDropdown: yes
Shown TeamsTogetherDropdown: yes
Shown SlotTeam0: yes
Shown SlotFaction0: yes          <- pas ", greyed"
```
Seule la 2e condition differait.

## Cause
`HOLNewGameScreen.cpp:393-395` et `:884` : la liste de factions est ouverte en solo (`SetIsEnabled(bOccupied)`, ligne occupee donc active). Le commentaire ligne 393 dit "FactionChoiceInProgress.md (lifts the grey of D240 and D495)". Commit : `ec41678` (2026-11-13, "Faction choice: list rules, session cheat, ... faction lists in solo and lobby"). Le controle a ete ecrit par `07a8da6` (2026-10-27, TC1-TC4, quand les factions etaient grisees) et n'a pas ete mis a jour par ec41678 (qui ne touche pas quick_check.ps1). Le rouge date donc du 2026-11-13.

## Correction
`quick_check.ps1` (Edit, BOM EF BB BF verifie) : `Shown SlotFaction0: yes$`, libelle "teams and factions open in solo (TC-Q12)", commentaires mis a jour. Rejeu : `menu fr : PASS`, en/ja/ailaunch PASS (43 s). `known_reds.txt` ne contenait pas `menu fr` dans ce worktree (la partie sortait "REGRESSION") : rien a retirer ici ; sur `main`, `Tools\Tests\known_reds.txt` a une ligne `menu fr` (commit 3b4f139) a retirer a la fusion.

Commit : 87b6d4e (branche visual, pas de push, pas de fusion).
Aucun fichier Source\ ni Script\ touche.
