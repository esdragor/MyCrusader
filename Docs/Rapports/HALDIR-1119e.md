# HALDIR-1119e : HUD H-6, barre des groupes, ARRETE A LA FRONTIERE DU COMMIT 1 (budget de 25 appels atteint a 01:28)

Impact joueur : aucun (commit 1 = liaison seule). NON COMPILE, NON LANCE. Worktree Documents\HeroesOfLiberty-Haldir, branche maps.

## Fait
- Commit 1 fd9b5ff (LIAISON) : dans HOLScriptScreen.h/.cpp les 8 membres de la section (a) de HALDIR-1119d (GetGroupsPanel, GetNumControlGroups statique, GetControlGroupsAvailable, GetControlGroupNow, GetControlGroupTypes, GetLastSelectedGroup, GetLastSelectedGroupAge, SelectControlGroup), signatures identiques a ce que HOLGroupBar.as appelle (aucune correction du .as). HOLGameScreen.h/.cpp : GetGroupsPanel() inline et UHOLGameScreen::GetGroupTypes (non const : FindUnitType / LocalFactionKey ; types dans l'ordre rencontre, noms localises, comptes) ; copie fidele du bloc d'arbre de types de RefreshControlGroups, sans le tri (le tri est dans le script).

## RESTE (commits 2, 3 et scenario : NON FAITS), plan exact, numeros de ce worktree (cherche par nom)
Commit 2 BASCULE :
- HOLGameScreenScript.as : membre UHOLGroupBarPanel GroupBar apres PlayerTable (l.124) ; OnInitialized apres PlayerTable.Build(this, GetPlayersPanel()) (l.262) : GroupBar = Cast<UHOLGroupBarPanel>(NewObject(this, UHOLGroupBarPanel)); GroupBar.Build(this, GetGroupsPanel()); Refresh() (l.1369) apres PlayerTable.Refresh() (l.1383) : if (GroupBar != nullptr) GroupBar.Refresh(); ClickWidget (l.2202) apres TopBar.ClickName (l.2239) : if (GroupBar != nullptr && GroupBar.ClickName(WidgetName)) return true;
- Groupe allume : le C++ le testait A CHAQUE IMAGE (RefreshControlGroups, avant le garde 0,25 s). Il existe deja un crochet par image dans le script : FrameTick(float32) (HOLGameScreenScript.as l.1427, BlueprintOverride). Plan : extraire dans UHOLGroupBarPanel une methode RefreshLit() (le bloc NewLit/Lit de Refresh, l.248-259 de HOLGroupBar.as), la faire appeler au debut de Refresh() ET depuis FrameTick (if (GroupBar != nullptr) GroupBar.RefreshLit();). A VERIFIER : que UHOLScriptScreen::NativeTick (HOLScriptScreen.cpp l.465) appelle bien FrameTick a chaque image (non lu faute d'appels).
- HOLGameScreen.cpp dans le MEME commit : retirer l'appel RefreshControlGroups(FrameSeconds) (tick, apres RefreshIdleWorkers(), ~l.627), la boucle de liaison des puces (OnInitialized, l.419-427) et ShownGroups.Init (l.439), la branche GroupSlots de ClickWidget (~l.2622-2634), dans BuildLayout le bloc GroupsRow + boucle ControlGroupBox (l.3010-3023 ; GARDER l.3004-3009 : GroupsPanel, padding, Collapsed, slot BottomLeft).
- Journal : MenuLog deja dans HOLGroupBar.as (lignes HUD groups: et HUD: group N clicked). NB : la ligne lue par quick_check est un UE_LOG (RefreshControlGroups), PAS une ligne de Describe : rien a copier cote Describe.
Commit 3 RETRAIT (grep chaque symbole dans Source\ et Script\ d'abord) : RefreshControlGroups, HandleGroupSlotClicked, GroupSlotName (cpp ~1816-1934) ; constantes GroupSlotWidth, GroupLightSeconds (cpp l.284-285 ; GARDER GroupRefreshInterval : encore utilisee l.540, 596, 612) ; au .h : RefreshControlGroups, HandleGroupSlotClicked, GroupSlotName, GroupSlots, ShownGroups, ShownGroupsLine, SinceGroupsRefresh, LitGroup, LitGroupTime (h l.312-321, 391-400). Ne PAS retirer GetGroupTypes ni GetGroupsPanel. Lignes retirees attendues : ~150.
Scenario ecran_hud_groupes : voir HALDIR-1119d.md section (g) ; non commence ; verifier les commandes hol.Select.Group et hol.UI.Key par grep avant usage.

## Table outils -> lignes (inchangee, voir HALDIR-1119d.md (d))
quick_check.ps1 lit HUD groups: 1=9 / 1=3 2=n, hol.HUD.Click ControlGroup2, Group 2: selected, HUD: group 2 clicked : toutes ecrites par HOLGroupBar.as via MenuLog apres le commit 2 ; aujourd'hui (commit 1 seul) encore par le C++.

## Suspects de compilation
- UFUNCTION avec TArray<FString>& / TArray<int32>& et int32& / bool& en sortie sur une methode const (GetControlGroupTypes, GetControlGroupNow) : GetTypedOuter<UHOLGameScreen>() const renvoie un pointeur non const (utilise pour appeler GetGroupTypes non const). Si le plugin refuse le TArray en sortie : renvoyer une struct a deux tableaux.
- HOLScriptScreen.cpp : AHOLBuilding doit etre une declaration complete ou au moins avancee via HOLPlayerController.h (TArray<AHOLBuilding*> seulement ; Buildings.Num()). static GetNumControlGroups dans un UCLASS(Abstract) comme GetResourceColor.
- HOLGameScreen.cpp GetGroupTypes : lambda AddType capturant Order/PerType ; HOLUnitView, HOLGameNames, UHOLBuildingSettings, FHOLUnitView deja inclus par le meme fichier (copie du bloc d'origine).
- Les recettes (e) et (f) de HALDIR-1119d (Insert, rouges a poser) s'appliquent inchangees.

## Commandes de validation (session principale, apres commits 2 et 3)
Tools\build.ps1 -LiveCoding=0 -Wait dans le worktree ; quick_check -Parts groups (+ tests script HOLGroupBar_Test : 10) ; poser les 2 rouges (HALDIR-1119d (f)) ; preflight.ps1 ecran_hud_groupes puis play.ps1 AVANT / APRES.

BUILDS_DEMANDES: 1 (apres commits 2-3) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 (commit 1 : +~120 lignes C++ de liaison) ; LIGNES_MIGREES_PAR_LOT: 0 ; TEST_ROUGE: non pose.