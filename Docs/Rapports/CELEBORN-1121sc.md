# CELEBORN-1121sc : moral, sous-etape 4, tranche 2 (inscription du processeur + ClearAuras)

NON COMPILE (aucun build, aucun quick_check, consigne). Impact joueur : le processeur de moral tourne enfin en partie (inscrit) ; les effets temporaires s'epuisent au pas, les auras posees a la main disparaissent (rien ne les repose encore).

APPELS_DECLARES: 20 (arret a 25 vus ; premier Edit au 7e appel)
HEURE_DEBUT/HEURE_FIN: 19:22:31 / ~19:25 (arret dur 19:39 respecte)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de compilation par l'agent)
TEST_ROUGE: 6e98882 (test seul, pas de stub necessaire : Execute ne faisait que Tick) ; rouge NON VU a l'execution ; assertit l'effet : oui (fragment de l'entite nee par RequestUnits/ServeBirthRequests relu apres UE::Mass::Executor::Run ; attendu CALCULE par HOLMorale::ClearAuras + Tick sur une copie)
COMMITS: 3 (456c1dc ini, 6e98882 test rouge, 0895ac7 correctif), dernier a 19:24, merge_main_fait: non
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 0 de plus (le lot C++ de la session principale, avec 1121sa et 1121sb)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun

FAIT / RESTE:
- FAIT T1 : Config\DefaultGame.ini : +SimulationProcessors=/Script/HeroesOfLiberty.HOLMoraleProcessor apres le soin (456c1dc). L16-3 / L18-d : Grep de SimulationProcessors, Processors.Num, ProcessorCount, GetDefault<UHOLSimulationSettings> dans Source\ et des fichiers Tools : AUCUN test ne compte les processeurs inscrits ni ne lit la liste (HOLAIBrainTests.cpp:236 la sauve et la vide puis la remet, sans compter ; HOLSimulationSubsystem.cpp:246 la lit pour instancier). Rien d'autre a mettre a jour.
- FAIT T2 : HOLMoraleProcessor.cpp Execute = ClearAuras puis Tick sur chaque fragment ; test HeroesOfLiberty.Morale.ProcessorClearsAurasEachStep (6e98882, rouge attendu) ; correctif 0895ac7 met aussi a jour HeroesOfLiberty.Morale.ProcessorTicksTheFragment (l'aura posee a la main disparait : plus d'assertion « aura reste », a la place « aura partie » + IsEmpty).
- ROUGES ATTENDUS : au tip 6e98882 : 1 rouge, HeroesOfLiberty.Morale.ProcessorClearsAurasEachStep (assertions « one step: the aura nobody laid again has ended » et « the dealt lever no longer reads the aura » ; la 3e passe). Au tip 0895ac7 : 0 rouge parmi les 9 tests HeroesOfLiberty.Morale.*.
- RESTE : auras reposees dans la portee (sources + destinataires D705), lecture des leviers par le combat (HOLCombatProcessor.cpp, interdit ici), mesure a 2000 unites (D190). Tant que la repose manque, ClearAuras efface toute aura : aucune n'est posee en production aujourd'hui.
RELECTURE_CPP (L20-2) : nouveau test : includes identiques a ProcessorTicksTheFragment (meme fichier, meme espace de noms) : oui ; ClearAuras(FHOLMoraleState&) et IsEmpty(const FHOLMoraleState&) verifies dans HOLMoraleRules.h:117/123 ; accolades du test relues : oui ; aucun switch ; pas de nouveau include dans le .cpp du processeur (HOLMoraleRules.h deja inclus).
NON VERIFIE: tout (compilation, rouge puis vert) ; que Apply laisse IdA et IdB dans les emplacements attendus ; que RemoveAt de ClearAuras ne laisse pas un emplacement dont Total depend (la regle est testee par TickClearRemove) ; la suppression de l'assertion d'aura du vieux test ne masque rien d'autre ; que l'ordre ClearAuras-puis-Tick soit celui voulu (sans effet aujourd'hui : Tick ne touche jamais une aura).
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune ; IN_USE.txt mis a jour (libre, CPP: non) ; avertissement git LF -> CRLF sur HOLMoraleProcessor.cpp (normal).
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md et MoraleInProgress.md par moi, le reste = autres sessions) : oui.