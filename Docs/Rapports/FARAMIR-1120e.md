NON COMPILE

# FARAMIR-1120e : visee des pouvoirs de faction sur le controleur (C++, 2026-11-20)

Joueur : le clic gauche en visee ne selectionne plus, il rend le point au sol au script, Echap l'annule, un anneau de portee (800 cm) entoure le heros ; rien n'est jouable avant le build et les boutons du script.

APPELS_DECLARES: 20 (arret a 25 ; arret dur 11:10)
HEURE_DEBUT/HEURE_FIN: 10:48 / 10:5x (Get-Date)
PREMIER_BUILD: sans build (tache C++ non compilee par consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans build
TEST_ROUGE: 05c63a6 (stub : SetFactionAim vide, GetFactionAim = -1) ; assertit l'effet : oui (etat de visee lu sur le controleur, remise a -1 par le vrai HandleDeselect et HandleOrder) ; rouge NON VU a l'execution
COMMITS: 2 (05c63a6 test + stub, dc1d928 correctif), merge_main_fait: oui (premier geste, main a jour)
TESTS: aucun lance ; TESTS_SCRIPT_AJOUTES: 0 (test C++ HeroesOfLiberty.Faction.Aim.StateAndEscape dans Source\HeroesOfLiberty\Tests\HOLFactionAimTests.cpp : tache C++ de controleur, pas la simulation Mass ; a deplacer en script si le controleur est testable en script) ; TESTS_TAUTOLOGIQUES: 0 (les -1 viennent des vrais chemins Escape / clic droit)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ de la seance)
TYPES_A_EXPOSER: aucun de neuf hors les trois UFUNCTION ci-dessous

## API livree (HOLPlayerController.h, public, pres de IsBannerArmed)
- `UFUNCTION(BlueprintCallable) void SetFactionAim(int32 Index, double RangeCm)` : -1 = aucune ; >= 0 arme la visee du pouvoir Index, coupe placement / age choisi / attaque-deplacement / banniere ; RangeCm <= 0 garde la portee precedente (defaut 800).
- `UFUNCTION(BlueprintCallable) int32 GetFactionAim() const`.
- `UFUNCTION(BlueprintCallable) bool ConsumeFactionAimClick(FVector& OutPoint, int32& OutIndex)` : vrai une fois par clic, l'index reste arme (le script decide Cast / Cancel puis `SetFactionAim(-1, 0)`).
- `double GetFactionAimRangeCm() const` (C++ seulement, pour les marques) ; `ScriptEscape()` / `ScriptRightClick()` : crochets de test (comme `ScriptLeftClick`).
- Script : `Controller.SetFactionAim(Index, HOLFactionPowerCast::SealRangeCm())` (le script passe 800 cm), puis a chaque tick `FVector P; int I; if (Controller.ConsumeFactionAimClick(P, I)) ...` ; `FVector&` / `int32&` en sortie en script : a verifier au premier build des scripts.

## Sites touches (HOLPlayerController.cpp)
- Clic gauche (HandleSelectStarted) : branche apres celle de la banniere : en visee, `bSelectPressed = false`, le point au sol (`GetCursorGroundPoint`) est garde, `return` (le clic est avale meme si le point au sol manque).
- Remise a -1 : Echap (HandleDeselect, condition de `NativeGetEscapeAction` + branche CancelMode), clic droit (HandleOrder), changement de sous-groupe, armement de la banniere et de l'attaque-deplacement, spectateur (D357).
- HOLSelectionMarkers.cpp `UpdateAuraRings` : `else if (Hero && GetFactionAim() >= 0)` : anneau de portee autour du heros (ReachRing), `faction reach N` dans la description.

## NON VERIFIE / RESTE
- Rien compile : `SetChosenAge(INDEX_NONE)` dans `SetFactionAim` (controleur de test sans etat de jeu), `UFUNCTION` avec `double` + `FVector&` vers Angelscript, tout le test.
- Le clic de sol n'est pas teste en C++ (le controleur de test n'a pas de `RTSCamera`, `GetCursorGroundPoint` renvoie faux) : a prouver par scenario avec geste reel apres le build.
- Non fait : curseur `Crosshairs` (`HOLCommandGrid::GetMouseCursor`) ; anneau sous le curseur (rayon d'effet du Sceau non cable) ; anneau centre sur la banniere (le heros seul).
- Le pouvoir reste refuse par `ClickPower` tant que `HOLDawnPowers` a `bEffectCoded = false` (voir GALADRIEL-1120f).
- Le fichier de test est en LF (Write) : `eol_check.ps1 -Fix` si Git avertit.

PROCESSUS_DETACHES: voir derniere ligne de la reponse finale
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchange hors docs voulus : oui (Auto_Journey_File.md, EtapeDefini.md, AubeInProgress.md, ce rapport)
