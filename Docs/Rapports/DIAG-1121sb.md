# DIAG-1121sb : integ-43, erreurs de compilation Angelscript (worktree HeroesOfLiberty-Sauron)

Heure 19:43 -> 19:51. Correctif : commit 0b46f93 sur integ-43 (6 fichiers .as, aucun C++, aucun build).
Journal lu : Saved_Tests\Logs\quick_tests.log (le journal de l'editeur de quick_check ; Saved\Logs ne contient que build.log).

## Erreurs (le compilateur les revele par vagues : 3 passes)
| # | fichier:ligne | branche d'origine (git log -1 -- fichier) | erreur | correctif |
|---|---|---|---|---|
| 1 | Script/Fog/HOLStealthFilter_Test.as:8 | 1f1c86a (Stealth script filter) | StealthOneZone() est une fonction globale de HOLStealthRules_Test.as, invisible d'un autre fichier (piege 2) | zone construite localement (FHOLStealthZone Center 0,0 / HalfExtent 400) |
| 2 | Script/Orders/HOLFollowOrders.as:63 | 1637398 (Glorfindel d7cccc2) | HOLUnitScript::GetUnitLocation(UObject, int, FVector&) : la liaison script n'a pas le WorldContext (piege 7, la signature script est (int, FVector&)) | appel sans le contexte ; la signature de ReadUnit(UObject WorldContext, ...) est gardee (appelants inchanges) |
| 3 | Script/Orders/HOLFollowOrders_Test.as:93 | 1637398 | idem pour IsUnitAlive(nullptr, id) | IsUnitAlive(987654) |
| 4 | Script/UI/HOLTopBar_Test.as:87,88,91,94 | c945861 (Galadriel c6060a0) | UKismetInternationalizationLibrary n'est pas liee au script (GetCurrentLanguage / SetCurrentLanguage) | HOLForumMessagesRead::SetCultureName (liaison C++ existante, renvoie la culture precedente) |
| 5 | Script/Fog/HOLStealthFilter.as:15 et HOLStealthRules_Test.as:143 | 6e80c33 / 9049921-537c6ec (Arwen 684c947) | « Use name IsVisibleToCamp instead to override C++ event BP_IsVisibleToCamp ... stripped prefix » : l'override script d'un BP_X porte le nom sans prefixe BP_ | `bool BP_IsVisibleToCamp(` -> `bool IsVisibleToCamp(` (les deux classes) |

Note : la ligne « unknown super type UHOLStealthFilter » du log d'integration ne se reproduit plus avec le binaire a jour (le log Integrate etait plus ancien que le binaire : LaunchCheck signalait la dll plus vieille que HOLUnitScriptLibrary.h).

## Resultat
quick_check -Parts tests : **2025/2026**. Le module compile (0/0 resolu).

## Rouge restant (1)
Angelscript.UnitTests.UI.HOLTopBar_Test.Test_TopBarIncomeTipIsSetAgainOnALanguageSwitchWithTheIncomeUnchanged (Galadriel c6060a0 / c945861 ; jamais execute sur sa branche, qui ne compilait pas).
Echecs : `KeyEn != KeyJa` et `TextEn != TextJa` (« Expected true, got false »). Essaye avec ja puis fr : meme resultat. Cause probable : dans l'editeur / test automatise, la localisation du jeu (Content/Localization/Game/<culture>/Game.locres, qui contient bien IncomeTip et TributeWood en en/fr/ja) n'est pas appliquee par SetCurrentCulture (editeur : apercu de localisation du jeu a activer, FTextLocalizationManager::EnableGameLocalizationPreview). Correctif possible : C++ (une liaison qui active l'apercu de la localisation du jeu pour la culture donnee, ex. dans UHOLForumMessagesReadLibrary::SetCultureName) ; NON FAIT (C++ : pas de build). Alternative sans C++ : declarer ce test en rouge connu (known_reds.txt) ou l'ecarter, a decider par la session principale.

## Champs agent_rules
APPELS_DECLARES: ~24 | HEURE_DEBUT/HEURE_FIN: 19:43 / 19:51 | PREMIER_BUILD: sans C++ ; BUILD: NON COMPILÉ (sans C++) ; LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 0/0 vu au depart (log Integrate), 2025/2026 apres ; assertit l'effet : sans objet (diagnostic)
COMMITS: 1 (0b46f93, 19:49), merge_main_fait: non (non demande) | TESTS: quick_check -Parts tests 2025/2026 ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun | QUESTIONS: aucune | NON VÉRIFIÉ: le correctif du rouge restant ; DIAGNOSTICS_AS serveur de langage non consulte (journal de l'editeur utilise)
PROCESSUS_DETACHES: aucun lance | ANOMALIES_PROCESSUS: aucune | GIT_STATUS_PRINCIPALE: non touche
