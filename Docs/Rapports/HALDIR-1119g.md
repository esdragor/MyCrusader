# HALDIR-1119g : H-6 retrait du C++ mort, build, tests

Impact joueur : aucun changement visible attendu ; la barre des groupes (script) est la seule source. Worktree Documents\HeroesOfLiberty-Haldir, branche maps, commit 865964b (sur 1119f94). Pas de push, pas de merge.

## Fait
- Grep de chaque symbole (Source, Script, Tools) : seules occurrences = les fonctions mortes elles-memes et des commentaires de HOLGroupBar.as / _Test.as. Retire de HOLGameScreen.cpp : GroupSlotName, RefreshControlGroups, HandleGroupSlotClicked (120 lignes), constantes GroupSlotWidth et GroupLightSeconds + leur commentaire (3 lignes) ; de HOLGameScreen.h : 3 declarations (10 lignes avec commentaires) et 6 membres GroupSlots, ShownGroups, ShownGroupsLine, SinceGroupsRefresh, LitGroup, LitGroupTime (11 lignes). Total 144 lignes.
- RESTE volontairement : GroupRefreshInterval et SinceGroupRefresh (sans s : lus par le panneau de groupe selectionne, l.530/583-604), GetGroupsPanel, GetGroupTypes, GroupsPanel.
- Build : PASS (52 s) a la premiere compilation, aucune erreur.
- Scripts : compilent du premier coup ; quick_check -Parts tests : 1590/1590 PASS (1580 + 10), 111 s.
- Rouge vu a 01:41 : les deux rouges de HALDIR-1119d (f) ensemble (HUD groups : 1=9 dans le test du journal ; Position % Count dans GroupAt) -> 1588/1590, exactement Test_GroupBarJournalLinesAreTheCppOnesWordForWord et Test_GroupBarOrderIsTheKeysOneToNineThenZero ; remis par edition inverse, vert a 01:44 (1590/1590, arbre propre).

## Erreurs rencontrees
- Edit tool refuse "not read" sur les 2 fichiers .as (jamais lus dans cette session) : remplacement PowerShell sur chaine exacte unique (aucune autre consequence).
- Aucune erreur de compilation C++ ni Angelscript.

## Reste / NON VERIFIE
- Scenario ecran_hud_groupes (txt/look/json) non fait ; comparaison avant/apres a l'oeil (capture) non faite ; parties quick_check groups non lancees (consigne : seulement -Parts tests) : le journal "HUD groups:", le clic ControlGroup2 et le rappel allume en jeu reel ne sont pas verifies a l'execution.
- Hors tests : plus aucune verification de la vue (puces avec libelle "1 · 9").

BUILDS_DEMANDES: 1 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 144 ; LIGNES_MIGREES_PAR_LOT: 0 ; TEST_ROUGE: vu a 01:41 : 1588/1590 ; TESTS_TAUTOLOGIQUES: 0 ; PROCESSUS_DETACHES: voir stray_processes.