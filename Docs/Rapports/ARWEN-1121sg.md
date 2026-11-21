NON COMPILE (C++ ajoute : Presentation/HOLUnitOpacity.h/.cpp, aucun build lance, consigne du brief)

APPELS_DECLARES: ~25 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 20:19 / 20:22 (Get-Date PowerShell)
PREMIER_BUILD: aucun   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non fait (aucun build ; brief)
TEST_ROUGE: 358a948 (stub C++ qui ne stocke rien + 2 tests) ; assertit l'effet : oui (la valeur est calculee par HOLStealth::OwnUnitOpacity sur une vraie zone, puis passee par le setter, relue par GetStealthOpacity). Rouge NON VU.
  ROUGES ATTENDUS sur 358a948 : Test_UnitOpacitySetterCarriesTheStealthRuleValue (2 assertions : Drawn == RuleOpacity, Drawn < 0.99).
  Passe deja sur le stub : Test_UnitOpacityUnsetUnitIsOpaqueAndOthersAreUntouched.
  Attendu vert sur 8f03b93 : les trois + Test_StealthApplyOwnUnitsSetsOpacityFromTheRule (rouge sur 358a948/81aae44 impossible : HOLStealthApply n'existe qu'en 8f03b93).
COMMITS: 3 (358a948 stub+tests, 81aae44 vrai setter, 8f03b93 HOLStealthApply+test), dernier 20:21, merge_main_fait: non (integ-37 = main 204f769 au depart)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : HOLUnitOpacity.cpp)   TESTS_SCRIPT_AJOUTES: 3 (Test_UnitOpacitySetterCarriesTheStealthRuleValue, Test_UnitOpacityUnsetUnitIsOpaqueAndOthersAreUntouched, Test_StealthApplyOwnUnitsSetsOpacityFromTheRule)
CLASSE C++ : la vue d'unite n'est pas une classe : les unites sont des instances Mass dessinees par UHOLInterpolatedInstancesProcessor (Presentation/HOLInterpolatedInstancesProcessor.cpp, AddBatchedCustomData ~l.582, custom data = 4 floats Couleur+Prop, M_UnitPlayerColor). NON TOUCHEE (pas de materiau a entree d'opacite : aucun moyen de la rendre sans toucher au materiau). Les fichiers C++ pris : nouvelle paire HOLUnitOpacity.h/.cpp (UHOLUnitOpacityLibrary : SetStealthOpacity(int32,float) / GetStealthOpacity(int32) / ClearStealthOpacities(), carte par unite verrouillee, locale a la machine, sans WorldContext donc pas de piege 7 ; float par valeur).
FAIT: setter + lecteur exposes au script, regle appliquee par HOLStealthApply::ApplyOwnUnit(s) (Script/Fog/HOLStealthOpacityApply.as, namespace ; test sans global croise : zone reconstruite localement, piege 2).
RESTE (a lier, lot C++ suivant) : (1) dessin : le processeur lit GetStealthOpacity(State.UnitId) a ~l.582 et l'ecrit en 5e float de custom data + M_UnitPlayerColor avec entree d'opacite (translucide) ; aussi les corps (~l.656) ; (2) point d'appel : UHOLStealthInstaller.Tick (HOLStealthFilter.as) appellera HOLStealthApply::ApplyOwnUnits(Zones, ids, positions) pour le joueur local : il manque un binding C++ listant ids + positions des unites du joueur local.
TYPES_A_EXPOSER: GetPlayerUnitIds(WorldContext, PlayerIndex) + positions (script), indice du joueur local.
NON VERIFIE: compilation C++ (FScopeLock/Misc/ScopeLock.h, UHT int32/float), compilation Angelscript (UHOLUnitOpacityLibrary::SetStealthOpacity(7, RuleOpacity) : double->float32 par valeur ; TArray<int>), DIAGNOSTICS_AS non releves, rouge/vert a l'execution, aucun rendu translucide visible. Edit de .as par printf (fins de ligne LF dans HOLUnitOpacity_Test.as : lancer eol_check -Fix).
COMMANDE: Tools\build.ps1 -LiveCoding=0 -Wait dans C:\Users\baudo\Documents\HeroesOfLiberty-Arwen puis quick_check -Parts tests (sur 358a948 : 1 rouge ; sur 8f03b93 : vert attendu).
Placeholders.txt (relu avant) : ligne 205 ajoutee (50 % furtivite, stockee non dessinee). StealthInProgress.md et EtapeDefini.md (ligne 861) a jour.
PROCESSUS_DETACHES: aucun processus detache
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: hors docs voulus inchange (EtapeDefini.md, Placeholders.txt par moi) : oui
