NON COMPILE (aucun build, aucun quick_check, aucun editeur : C++ jamais compile)

APPELS_DECLARES: 14 (arret a 25 vus ; 16:15 non atteint)
HEURE_DEBUT/HEURE_FIN: 15:59 / 16:03 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build demande)
TEST_ROUGE: 5328bbb (Tests\HOLMoraleRulesTests.cpp, 5 tests + stub HOLMoraleRules.cpp qui compile et ne fait rien) ; rouge NON VU a l'execution (pas de build) ; assertit l'effet : oui (Total / Multiplier / Has / State.Num lus apres Apply / Tick / RemoveMalus, valeurs de table lues de HOLMoraleTableRows::Find, pas recopiees)
COMMITS: 3 sur duel-rewards (5328bbb test rouge + stub, 4836719 corps, af320c4 journal) + merge main 27261f5 (reel, sans conflit), dernier 16:03, merge_main_fait: oui
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (les totaux sont calcules par la regle, comparees aux plafonds de la spec et aux lignes de la table)
ESSAIS: aucun
QUESTIONS: aucune (voir choix de forme dans NON VERIFIE)
FAIT / RESTE:
 - FAIT : Source\HeroesOfLiberty\Simulation\HOLMoraleRules.h/.cpp (namespace HOLMorale, regle pure, FHOLMoraleState a 4 emplacements fixes + Num + ImmunityMs ; BonusCap/MalusCap 50/40/30 et 50/30/30, MakeEffect, MakeEffectFromRow, Scale, WithIntensity, Attenuated, CombineAttenuation, Apply, Total, Multiplier, Tick, ClearAuras, RemoveMalus, GrantImmunity, SacredLightImmunityMs=5000) ; copie fidele de Script\Morale\HOLMoraleRules.as, inclut HOLMoraleTableRows.h.
 - FAIT : Source\HeroesOfLiberty\Tests\HOLMoraleRulesTests.cpp : HeroesOfLiberty.Morale.CapsAndOffset, TableRowsThroughTheRule, ApplyNotCumulated, AttenuationAndImmunity, TickClearRemove.
 - Aucun fragment, aucun processeur, aucun en-tete central touche. Etape 3 NON commencee : ecrite comme prochaine etape dans MoraleInProgress.md (l.170).
 - RESTE de la sous-etape 2 : UHOLMoraleLibrary (HOLMoraleLibrary.h/.cpp, pont UFUNCTION, ApplyEffectAround, test script Test_MoraleTableCppRowsAgreeWithCsv, HOLMoraleRules.as en habillage) ; puis sous-etape 3 (FHOLMoraleFragment dans HOLSimulationFragments.h, empreinte HOLSimulationSubsystem.cpp ~:153).
 - Commande de test (session principale, apres build du lot) : Tools\build.ps1 -LiveCoding=0 -Wait puis les tests HeroesOfLiberty.Morale.* ; rouges attendus sur le stub (5328bbb) : les 5 tests ci-dessus.
NON VERIFIE: rien compile. Points a surveiller au build : (1) HOLMoraleTableRows.h jamais compile (constexpr inline struct aggregat avec const char*) ; (2) TestEqual sur int32 vs int (Row->Dealt est int, meme type que int32 sous MSVC) ; (3) FString::Printf %hs avec const char* Row->Key ; (4) CHOIX DE FORME a confirmer : ApplyNoRoom=4 (nouvelle valeur, absente du script) quand 4 emplacements sont pris et qu'un 5e effet distinct arrive (un effet deja present se rafraichit toujours) ; MakeEffectFromRow d'une ligne Gives (Banner) donne les leviers de DawnAura sous l'id DawnAura (banniere et aura du heros = un seul effet, non cumules), duree et aura de la ligne donnee ; CombineAttenuation prend (const int32*, const int32*, Count) a la place des TArray du script (le pont UHOLMoraleLibrary convertira) ; (5) tous les parametres de lever hors 0..2 : LeverValue = 0, caps = ceux de la cadence (comme le script).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: edit_line -After refuse 2 fois (ancres inversees entre EtapeDefini.md et Auto_Journey_File.md), corrige a la 3e ; Substring de ma propre commande de verification en erreur (affichage seul, edits faits). Fins de ligne : git avertit LF -> CRLF sur les 3 fichiers neufs (sans effet).
GIT_STATUS_PRINCIPALE: EtapeDefini.md et Auto_Journey_File.md modifies par moi (ligne CELEBORN-1120pd seulement) ; reste preexistant : oui
