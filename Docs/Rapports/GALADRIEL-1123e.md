# GALADRIEL-1123e
APPELS_DECLARES: 15 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 09:56 (Bash) 09:58 (Get-Date PS) / 10:02
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: suppose (rouge non vu : binaire perime, aucun test lance) ; assertit l'effet : oui   TEST_ROUGE_SEPARE: non (pas de binaire pour voir le rouge ; stub + test + correctif dans UN commit 04a233f)
COMMITS: 1 (04a233f, ~10:01), merge_main_fait: non (DLL perimee, pas de merge)
TESTS: aucun lance (NON JOUES) ; 4 ajoutes dans Script\Tests\HOLMoraleView_Test.as : Test_MoraleViewEmptyUntilMask, Test_MoraleViewPastilleAppearsWhenInjected, Test_MoraleViewLabelsAndMalusColour, Test_MoraleViewPeasantNoneAndImmunity ; TESTS_TAUTOLOGIQUES: 0 (texte calcule par Present depuis un etat pose par HOLMorale::Apply de la vraie table)
ESSAIS: aucun (morale_look reste a faire apres le masque C++)
QUESTIONS: aucune
FAIT / RESTE: FAIT Script\UI\HOLMoraleView.as (namespace HOLMoraleView : Read = seul point de lecture, vue vide jusqu'a la sous-etape 9 ; Present pure sur HOLMoraleBlock::Badge/Lines ; Refresh = pastille UCommonTextBlock "MoraleBadge" dans SelectionStatsHost), UN crochet HOLGameScreenScript.as (SelectionFrameTick, apres SendSelectionStats : HOLMoraleView::Refresh(this, SelectionBlock.StatsHost)), libelles via HOLMoraleTable::Name (Triomphe, Demoralise, Hesitation), couleurs vert/rouge/or. RESTE : masque C++ reseau (9), corps de Read, scenario morale_look ; le bloc de liste complet de la selection (Lines en plusieurs lignes) n'est pas dessine : la pastille tient en une ligne de texte.
NON VERIFIE: compilation Angelscript (aucun diagnostic lu), API supposees : UVerticalBox.GetChildrenCount/GetChildAt, UWidget.GetName()==n"MoraleBadge", UCommonTextBlock.SetColorAndOpacity(FLinearColor) (vu ligne 404 de HOLGameScreenScript sur DuelTitleText), FString.Contains/Find, "\u25CF" dans une chaine ; Peasant deduit de TypeWorkers[0] (pas de champ Peasant dans FHOLUnitSelView) ; le crochet en SelectionFrameTick ne tourne que quand une seule unite est selectionnee ; la pastille n'est pas rendue a cote de la barre de vie (C++ HOLSelectionMarkers, sous-etape 10 partie C++). DIAGNOSTICS_AS: non lus.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel

ANOMALIES_PROCESSUS: aucune ; Bash a ete modifie par un hook (rtk) 2 fois, relance telle quelle
GIT_STATUS_PRINCIPALE:  M EtapeDefini.md
?? Docs/Rapports/FARAMIR-1123b-fix.ps1
 (inchange hors docs voulus : oui)
