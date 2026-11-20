# GALADRIEL-1120f : pouvoirs de faction cliquables, regle de clic en script, visee = petit lot C++ (2026-11-20)

Joueur : aujourd'hui les pouvoirs de l'Aube sont des lignes de texte non cliquables ; la regle de clic est prete et testee en script, mais la visee au sol demande un petit morceau C++ (ci-dessous) avant qu'un clic puisse lancer le Sceau.

APPELS_DECLARES: 18 (arret a 25 ; arret dur 10:58 non atteint)
HEURE_DEBUT/HEURE_FIN: 10:37 / 10:4x (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: a3e4e80 (stub `ClickPower` qui ne fait rien + 5 tests) ; assertit l'effet : oui (aim, cast, cancel, refuse calcules par la regle sur une vue construite par `HOLFactionSlot::Make` avec le vrai livre) ; rouge NON VU a l'execution (aucun quick_check dans l'agent)
COMMITS: 2 (a3e4e80 test + stub, 44b8294 regle), dernier a 10:39, merge_main_fait: oui (premier geste, main = 37bf476)
TESTS: aucun lance (NON COMPILE) ; TESTS_SCRIPT_AJOUTES: 5 (Test_FactionSlotClickReadyBoxThenGroundCasts, Test_FactionSlotClickEscapeCancels, Test_FactionSlotClickSameBoxCancelsOtherBoxSwitches, Test_FactionSlotClickRefusesBoxesThatAreNotReady, Test_FactionSlotClickGroundAfterPowerCooledDoesNotCast ; fichier Script/Tests/HOLFactionSlotClick_Test.as) ; TESTS_TAUTOLOGIQUES: 0 (l'etat prêt/refus de chaque boite vient de HOLPowerBook::StateOf via Make, l'action est calculee par ClickPower)
ESSAIS: aucun
QUESTIONS: aucune creee (Q-sceau-cout.md deja ouverte ; voir NON VERIFIE sur bEffectCoded)
BUILDS_DEMANDES: 1 (le lot C++ de la seance, voir ci-dessous) ; TYPES_A_EXPOSER: SetFactionAim / GetFactionAim / ConsumeFactionAimClick (controleur), et la ligne `GetCursorGroundPoint` deja en C++ (non UFUNCTION)

## (2) Comment une capacite visee au sol se vise aujourd'hui : verdict = PAS faisable en script seul

La banniere du heros (le seul modele) est tout en C++ dans `HOLPlayerController` :
- Armer : `PressGridKey` -> `case EHOLGridCommand::HeroAbility` (HOLPlayerController.cpp:2625-2650) pose `bBannerArmed` (h:903, accesseur h:440).
- Clic gauche : branche `if (bBannerArmed && GetCursorFraction(Cursor)) { bSelectPressed = false; PlantBannerAt(Cursor); return; }` (cpp:4764-4770), AVANT la selection : c'est ce qui empeche le clic de selectionner/deselectionner des unites. Un script ne voit ni ce clic ni `bSelectPressed`.
- Projection au sol : `PlantBannerAt` (cpp:2986-3000) = `DeprojectScreenPositionToWorld` + `RTSCamera->GetGroundPoint` ; `GetCursorGroundPoint(FVector&)` (h:446) existe mais n'est PAS une UFUNCTION (aucun appel depuis `Script\`).
- Annulation : Echap remet `bBannerArmed = false` a 6 endroits (cpp:736, 1628, 2648, 4745, 4869, 4978).
- Cercle de portee + cercle sous le curseur (rouge hors portee) : `AHOLSelectionMarkers::UpdateAuraRings` (HOLSelectionMarkers.cpp:782-796) ; curseur : `HOLCommandGrid::GetMouseCursor(bAttackMoveArmed, bPlacing)`.
- Envoi : `RequestHeroAbility` -> `HOLRpcScriptRoute_Haldir1117n::RouteHeroAbility` -> RPC script `ServerHeroAbility`.
`hol.Select.OrderGround` / `HOLGroundAbilityRules.as` ne visent pas : le premier est une commande de test (clic droit simule), le second une regle pure (portee, recharge, zone), deja reprise par `HOLFactionPowerCast::Decide`.

Une visee « tout script » (un voile plein ecran qui avale le clic + `GetHitResultUnderCursor`) est theoriquement possible mais : le clic gauche touche AUSSI la selection par le chemin d'entree du controleur (sauf si le voile est un widget visible qui le consomme, a verifier a l'ecran), pas de cercle de portee ni de curseur sans C++, Echap non vu par le script. Non retenu : plus fragile que 20 lignes de C++ sur le modele de la banniere.

## (4) Le plus petit morceau C++ pour le lot de la seance (NON ECRIT)

`Source\HeroesOfLiberty\Player\HOLPlayerController.h/.cpp` seulement (pas de nouvelle route RPC : le script envoie lui-meme le RPC) :
1. Membres : `int32 FactionAimIndex = -1;` `bool bFactionAimClicked = false;` `FVector FactionAimPoint;`.
2. Trois UFUNCTION(BlueprintCallable) : `void SetFactionAim(int32 Index)` (arme / desarme : -1 = rien ; desarme aussi `bBannerArmed`, `bAttackMoveArmed`, placement), `int32 GetFactionAim() const`, `bool ConsumeFactionAimClick(FVector& OutPoint, int32& OutIndex)` (rend le clic en attente et le vide).
3. Branche du clic gauche juste apres celle de la banniere (cpp:4770) : `if (FactionAimIndex >= 0 && GetCursorGroundPoint(Point)) { bSelectPressed = false; FactionAimPoint = Point; bFactionAimClicked = true; return; }` (l'index reste jusqu'a `Consume`).
4. Echap : ajouter `FactionAimIndex = -1;` aux 6 sites de `bBannerArmed = false` ; armer la banniere ou l'attaque-deplacement remet aussi l'index a -1.
5. Facultatif (visuel, meme lot si peu cher) : dans `UpdateAuraRings` (HOLSelectionMarkers.cpp:782) une branche miroir `GetFactionAim() >= 0` : cercle de portee autour du heros (rayon 800, `HOLFactionPowerCast::SealRangeCm`, a passer par un setter de rayon car la constante est en script) et cercle du Sceau sous le curseur ; `GetMouseCursor` -> Crosshairs quand `GetFactionAim() >= 0`.

Cote script ensuite (sans C++, apres ce lot ; je ne l'ai pas ecrit : il ne se teste pas sans le C++ et edite le gros `HOLGameScreenScript.as` a l'aveugle) :
- `BuildFactionSlot` : une `UButton` par boite (le panneau passe de `HitTestInvisible` a `Visible` pour la zone des boutons seulement), `OnClicked` -> `HOLFactionSlot::ClickPower(FactionView, Controller.GetFactionAim(), EHOLFactionClickKind::Box, Index)` puis `SetFactionAim(Result.AimIndex)` pour Aim / Cancel ; refus -> son `UiBack`.
- `RefreshFactionSlot` (chaque tick) : `if (Controller.ConsumeFactionAimClick(Point, Index))` -> `ClickPower(..., Ground, ...)` ; sur `Cast` : `Rpc.ServerCastFactionPower(Slot, CastIndex, Point)` ; `SetFactionAim(-1)`.
- Echap : deja gere par le C++ (remet -1) ; `EHOLFactionClickKind::Escape` sert au test et a un eventuel raccourci script.

## Fait / reste
- FAIT (sans C++) : `HOLFactionSlot::ClickPower(View, AimIndex, Kind, BoxIndex)` + enums `EHOLFactionClickKind` (Box, Ground, Escape), `EHOLFactionClickAction` (Nothing, Aim, Cast, Cancel, Refuse), struct `FHOLFactionClickResult` (Action, AimIndex, CastIndex, Describe) dans `Script\UI\HOLFactionSlot.as` ; 5 tests. Regles : boite prete -> Aim ; la meme boite -> Cancel ; une autre prete -> Aim bascule ; boite en recharge / manque de jauge / palier / sans effet code (Soon) -> Refuse, visee inchangee, mot du livre dans `Describe` ; clic sol : pret -> Cast et fin de visee, plus pret -> Cancel sans cast, sans visee -> Nothing ; Echap : visee -> Cancel, sinon Nothing ; index hors livre ou emplacement vide -> Nothing.
- RESTE : lot C++ ci-dessus (un build) ; boutons + sondage cote `HOLGameScreenScript.as` ; scenario avec geste reel (touche puis clic) apres le build.

## NON VERIFIE
- Rien compile ni execute : syntaxe Angelscript des `enum` a virgule finale, `f"cast {AimIndex}"`, `const EHOLPowerState State` ; la session principale doit lancer `quick_check -Parts tests` apres build des scripts et VOIR le rouge sur a3e4e80 (5 tests rouges contre le stub) puis le vert sur 44b8294. Le fichier de test est en LF (Write), le reste du depot en CRLF : `eol_check.ps1 -Fix` si Git avertit.
- Piege de donnees : `HOLDawnPowers::Rows()` laisse `bEffectCoded = false` (HOLDawnPowers.as:3 et :14, le Sceau n'etait pas code a l'ecriture). Avec la vraie ligne le livre dit donc `Soon` et `ClickPower` REFUSE : tant que cette ligne n'est pas passee a `true` (et son cout / `Q-sceau-cout.md` tranches par l'utilisateur) aucun clic ne peut lancer le Sceau, meme apres le lot C++. Je n'y ai pas touche (decision de cout et d'etat du livre). Verifier aussi `HOLDawnPowers_Test.as` qui assertit sans doute « Soon ».
- Recharge livre 20 s contre 40 s du Sceau (ecart deja signale par FARAMIR-1120c).
- Aucune boite n'est un widget cliquable aujourd'hui (le panneau est `HitTestInvisible`, texte seul).

PROCESSUS_DETACHES: voir derniere ligne de la reponse finale
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchange hors docs voulus : oui (seuls AubeInProgress.md, EtapeDefini.md, Auto_Journey_File.md, ce rapport)
