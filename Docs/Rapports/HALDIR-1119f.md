# HALDIR-1119f : HUD H-6, barre des groupes, commit 2 (BASCULE) fait ; commit 3 (retrait) et scenario non faits

Impact joueur : la barre des groupes (1..0) est maintenant dessinee et cliquee par le script ; meme aspect attendu. NON COMPILE, NON LANCE. Worktree Documents\HeroesOfLiberty-Haldir, branche maps, commit 1119f94 (sur fd9b5ff).

## Fait (commit 1119f94)
- HOLGameScreenScript.as : membre GroupBar, Build apres PlayerTable.Build (GetGroupsPanel()), Refresh, ClickWidget (apres TopBar), et un nouvel override GroupLitChanged().
- HOLGroupBar.as : RefreshLit() extrait (bloc NewLit/Lit), appele au debut de Refresh() et par GroupLitChanged.
- CHOIX groupe allume : NativeTick (HOLScriptScreen.cpp) n'appelle FrameTick que tant qu'un message vit (bMessageLive), donc PAS a chaque image. J'ai garde la DETECTION cote C++ : HOLScriptScreen.h/.cpp ajoutent l'evenement BlueprintImplementableEvent GroupLitChanged() et le membre LitGroupSeen ; NativeTick calcule chaque image lit = (GetLastSelectedGroup != NONE && GetLastSelectedGroupAge() < 0.5) ? groupe : NONE (0.5 = LightSeconds de HOLGroupBar.as, a garder d'accord) et n'appelle le script que quand cette valeur change (rappel, fin des 0,5 s). Le groupe s'allume donc a l'image meme, comme en C++, sans appel script par image.
- HOLGameScreen.cpp : retires l'appel RefreshControlGroups(FrameSeconds), la boucle de liaison des puces (GroupSlots.Add / OnClicked), ShownGroups.Init, la branche GroupSlots de ClickWidget, le bloc GroupsRow + ControlGroupBox de BuildLayout (GroupsPanel, padding, Collapsed, slot gardes). Le journal "HUD groups:" que lit quick_check est ecrit par HOLGroupBar.as via MenuLog (inchange depuis le commit 1).

## RESTE (commit 3 + scenario)
- Retrait du C++ devenu sans appelant, apres grep Source\ et Script\ : dans HOLGameScreen.cpp RefreshControlGroups, HandleGroupSlotClicked, GroupSlotName (~l.1856-1980), constantes GroupSlotWidth (l.284) et GroupLightSeconds (l.285 ; GARDER GroupRefreshInterval, SinceGroupRefresh l.583 etc.) ; dans HOLGameScreen.h : declarations RefreshControlGroups, HandleGroupSlotClicked, GroupSlotName (l.326-330), membres GroupSlots, ShownGroups, ShownGroupsLine, SinceGroupsRefresh, LitGroup, LitGroupTime (l.402-409). Ne PAS retirer GetGroupTypes ni GetGroupsPanel. Attention : verifier que SinceGroupRefresh (l.583, sans « s ») n'est pas lie aux groupes avant retrait. Le code mort compile tel quel (il traite GroupSlots vide : RefreshControlGroups sort a GroupSlots.Num() != NumControlGroups).
- Scenario ecran_hud_groupes.txt/.look/.json : non commence (voir HALDIR-1119d section g ; verifier hol.Select.Group / hol.UI.Key par grep).

## Suspects de compilation
- Ceux de HALDIR-1119e (TArray en sortie sur UFUNCTION const, etc.).
- GroupLitChanged : BlueprintImplementableEvent sans parametre ; le script l'override avec UFUNCTION(BlueprintOverride) void GroupLitChanged() (meme forme que FrameTick). UHOLScriptScreen::GetLastSelectedGroup/Age sont const UFUNCTION appelees depuis C++ : OK.
- NativeTick tourne pour tous les ecrans script : le cast du controleur est null-safe (GetLastSelectedGroup renvoie INDEX_NONE sans AHOLPlayerController) ; GroupLitChanged appele sur un ecran sans override = no-op.
- Script : la reference a UHOLGroupBarPanel dans HOLGameScreenScript.as suppose que HOLGroupBar.as est dans le meme module (comme HOLPlayerTable.as).

## Commandes de validation (session principale)
Tools\build.ps1 -LiveCoding=0 -Wait dans le worktree ; quick_check -Parts groups (lignes "HUD groups:", hol.HUD.Click ControlGroup2, "Group 2: selected", "HUD: group 2 clicked") + tests script HOLGroupBar_Test (10) ; puis les 2 rouges de HALDIR-1119d (f), puis commit 3 et scenario.

## Table outils -> lignes
Inchangee (HALDIR-1119d (d)) ; ecrite maintenant par HOLGroupBar.as via MenuLog.

BUILDS_DEMANDES: 1 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: ~38 (commit 2 : tick call 1, boucle de liaison 8, Init 1, branche clic 13, layout 14 + 1 commentaire ; +14 lignes de detection/evenement) ; LIGNES_MIGREES_PAR_LOT: 0 ; TEST_ROUGE: non pose ; PROCESSUS_DETACHES: aucun lance.
