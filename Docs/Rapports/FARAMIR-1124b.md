# FARAMIR-1124b : S2-c Lumiere sacree appelle ClearMalusAround (zone-effect, sans C++)

Impact joueur : quand la Lumiere sacree est posee, les malus de moral des unites du camp du lanceur dans la zone sont retires (non verifie en jeu : non compile).

APPELS_DECLARES: 13
HEURE_DEBUT/HEURE_FIN: 10:58:38 / ~11:02
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 8b6fd50 (bouchon ClearMalusOrderOf qui rend des zeros + 2 tests), assertit l'effet : oui (ordre calcule par la regle : centre = pose tronquee, rayon = GetSacredLightRadiusCm, camp = lanceur) ; rouge NON VU a l'execution (pas de quick_check dans l'agent). TEST_ROUGE_SEPARE: oui
COMMITS: 3 (8b6fd50 rouge, 04b5db0 correctif, journal), dernier a ~11:02, merge_main_fait: oui (ec28263, premier geste)
TESTS: aucun lance ; commande : Tools\quick_check.ps1 -Parts tests DU worktree Faramir (attendu : 8b6fd50 rouge sur les 2 tests de HOLFactionPowerClear_Test.as ; 04b5db0 vert). TESTS_TAUTOLOGIQUES: 0 (limite : le rayon est compare au getter de la zone, la regle le lit au meme endroit ; centre et camp sont calcules). TESTS_SCRIPT_AJOUTES: 2 (SacredLightClearOrderIsCentredOnThePoseWithTheZoneRadius, SacredLightClearOrderFollowsTheCaster). BUILDS_DEMANDES: 0. TYPES_A_EXPOSER: aucun pour S2-c ; D900 reste : UHOLHeroLibrary::GetHeroUnitId (lot Glorfindel).
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- HOLFactionPowerServer.as : struct FHOLSacredLightClearOrder, ClearMalusOrderOf(Slot, Pose), et dans PutDown, apres StartSacredLight et avant le dessin : HOLMorale::ClearMalusAround(State, X, Y, Rayon, Camp). Un lancer refuse ou une zone non demarree ne vide pas les malus (l'appel est apres le succes de StartSacredLight).
- RESTE : pas de test de chaine de PutDown (il faut un AHOLGameState de test) ; la file d'ordres du moral n'est pas lisible depuis le script (NumPendingOrders n'est pas UFUNCTION) ; la pose effective de l'ordre est couverte cote C++ par HeroesOfLiberty.Morale.ClearMalusAround*. D900 (branchement TryCastPower) non touche.
NON VERIFIE: compilation script, nom d'espace HOLMorale::ClearMalusAround (deduit de HOLDuelLedger.as:118 qui appelle HOLMorale::ApplyEffectAround de la meme classe UHOLMoraleLibrary), rouge a l'execution, camp = Slot (OrderSlot = index de slot comme dans ApplyEffectAround), diagnostics AS (DIAGNOSTICS_AS: non lus).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir

ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE:  M Auto_Journey_File.md
 M Docs/SessionEnd.txt
 M EtapeDefini.md
 M Placeholders.txt
?? Docs/Rapports/ARWEN-1124a.md
?? Docs/Rapports/CELEBORN-1124a.md
?? Docs/Rapports/ELROND-1124a.md
?? Docs/Rapports/FARAMIR-1124a.md
?? Docs/Rapports/GALADRIEL-1124a.md
?? Docs/Rapports/GATE-1124-launch-galadriel.txt
?? Docs/Rapports/GLORFINDEL-1124a.md
?? Docs/Rapports/PLAN-GRILLE-FINE-1124.md
?? Docs/Rapports/SAURON-1124a.md
?? Retours/Q-grille-fine-structure.md
