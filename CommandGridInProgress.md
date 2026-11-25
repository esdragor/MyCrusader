# CommandGridInProgress.md : choix de l'action d'une case de la grille en Angelscript (ENONCES-1125m section 1)

Chantier : le script decide, le C++ execute (D839). Lignes G-1 a G-6 : `Docs\Rapports\ENONCES-1125m.md` 1.3.

## Etat (GLORFINDEL-1125ma, 2026-11-25 10:0x, worktree Glorfindel, branche duel-frame-1124sj)
- G-1 / G-2 en cours : `Script\Player\HOLCommandGridAction.as` (enum `EHOLGridActionKind`, structs `FHOLGridActionContext` {bMonkUnit, MonkUnitId} et `FHOLGridActionView` {Kind, AimMode, RangeCm, UnitId, Report}, `HOLCommandGridAction::Decide(Context, Command)`, modes `GetAimNone/AttackMove/Banner/Seal` = -1/0/1/2) et `Script\Tests\HOLCommandGridAction_Test.as` (3 tests : LightSeal, HeroAbility, OrdersAndSweep).
- Commit 1 (3421414) = bouchon + tests, ROUGE VU 10:05 : quick_check -Parts tests = 2234/2237, les 3 rouges sont Test_CGALightSeal, Test_CGAHeroAbility, Test_CGAOrdersAndSweep ; commit 2 (c3765bd) = correctif Decide (G-1, G-2, modes d'armement 100+ selon ELROND-1125ma), VERT NON RELANCE. G-3 : a faire apres integration de G-4.
- G-3 (OnGroundClick, liaisons Request*) : a faire, depend des liaisons exposees par le lot C++ G-4 (ELROND-1125ma).
- Signature donnee a l'agent C++ (G-4) : `Decide(const FHOLGridActionContext&, const FHOLGridCommandView&) -> FHOLGridActionView` ; le contexte porte `bMonkUnit` et `MonkUnitId` (le crochet C++ remplit l'id du premier Moine selectionne).
