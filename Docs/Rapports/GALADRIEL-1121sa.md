APPELS_DECLARES: 25 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 19:12 / 19:23
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build, scripts seuls)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun test neuf (défaut vu au scénario, correctif sans test rouge : non appliqué) ; assertit l'effet : non
COMMITS: 2 sur specialization-trial (59334c6 scénario, 116bf29 HOLTopBar.as), dernier 19:21, merge_main_fait: non (DLL non vérifiée)
TESTS: preflight income_tooltip PASS (34 s) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: income_tooltip : preflight PASS, 4 PNG ouverts, VU À L'ÉCRAN, play.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
QUESTIONS: aucune
FAIT: (1) json sans mapSize : "mapSize": "Small" ajouté. (2) les waitlog de texte échouaient : le HUD-panel est un widget d'infobulle, "tip text" (UHOLUIMouse GetToolTipText) est vide ; remplacé par 'tooltip shown', le texte se lit au PNG. (3) DEFAUT REEL VU au PNG JA : l'infobulle restait "+377 bois/min" après passage en ja : SetIncomeTips ne se rejouait que si le revenu changeait ; ajout de TipLanguageKey + ShownTipKey dans HOLTopBar.as (116bf29), JA affiche "+377 木材/分". Police lisible ~14 px, fond sombre, bord doré.
RESTE: test script rouge pour TipLanguageKey (non fait) ; run Hidden du scénario flaky une fois (hover JA sur SBorder avant 'wait 1').
TYPES_A_EXPOSER: HOLUIMouse.cpp HoverVerb : lire le texte du widget d'infobulle (GetToolTipText vide pour un ToolTipWidget) ; UWidget.SetToolTip est lié (infobulle montrée).
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0
NON VÉRIFIÉ: texte de l'infobulle non asserté par log (PNG seulement) ; chemin exact de play.ps1 ; DIAGNOSTICS_AS non relus (le script compile : check PASS)
EDITS_REFUSES: 0PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relu
