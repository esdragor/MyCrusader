# CELEBORN-1119n1 : recompenses du duel D7, regle pure

APPELS_DECLARES: 12 (arret 25)
HEURE_DEBUT/HEURE_FIN: 21:22 / 21:24
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build demande)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: fe8fbce (bouchon qui compile, liste vide, + 5 tests) ; assertit l'effet : oui (ordres rendus par une vraie chaine Challenge / Respond / Advance -> Effects). Attendu rouge sur fe8fbce : 3 sur 5 (Winner, RefusalHesitation, RadiusIsTheDuelsOwn) ; les 2 autres (DoubleKnockOutGivesNothing, NothingWhileNotOver) passent sur le bouchon par construction (liste vide attendue), ils gardent la regle contre une regression. Vert attendu sur a9df9f0 : 5/5. NON VU a l'execution.
COMMITS: 2 (fe8fbce rouge, a9df9f0 vert), dernier a 21:24, merge_main_fait: non (worktree a jour avec main au depart, pas de merge)
TESTS: aucun lance (consigne). A lancer : quick_check.ps1 -Parts tests sur fe8fbce puis a9df9f0 (HOLDuelRewards_Test.as, + non-regression HOLDuelRules_Test.as, HOLMoraleTable_Test.as) ; TESTS_TAUTOLOGIQUES: 0 ; TESTS_SCRIPT_AJOUTES: 5 (Test_DuelRewardsWinnerTriumphLoserDemoralised, Test_DuelRewardsRefusalHesitationOnRefuserOnly, Test_DuelRewardsDoubleKnockOutGivesNothing, Test_DuelRewardsNothingWhileNotOver, Test_DuelRewardsRadiusIsTheDuelsOwn)
BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun
ESSAIS: aucun (regle pure)
QUESTIONS: aucune creee ; rayon non tranche (parametre d'entree, pas de valeur par defaut codee)
FAIT / RESTE:
- Fait : Script\Duel\HOLDuelRewards.as (struct FHOLDuelMoraleOrder { Slot, EffectId, DurationMs, RadiusCm } + HOLDuelRewards::Orders(S, Duel, RadiusCm)) ; s'appuie sur HOLDuel::Effects (pas de jumeau de la regle de fin) ; durees = HOLMoraleTable::Row(id).DurationMs ; aucun fichier existant modifie.
- Ecart au plan T1 du rapport ETAPE0-1119n-B : signature (S, Duel, RadiusCm) au lieu de (S, E), et pas de constante DefaultRadiusCm (l'enonce dit : rayon d'entree, pas une constante du fichier). "Rayon du duel" = valeur passee par l'appelant (FHOLDuelSettings n'a que ArenaRadiusCm = 600, le cercle d'arene, pas un rayon de moral).
- Reste (branchement) : HOLDuelLedger.as:194-199 (boucle Ended) appelle Orders puis applique ; l'application aux unites = pont moral cote simulation, C++ (M7 : HOLMoraleLibrary + HOLDuelSubsystem), a mettre dans le lot C++ groupe ; choix du rayon (12 m provisoire au journal du moral) a trancher par l'utilisateur ; XP : pas de source Duel dans EHOLXPSource (C++), montant sans nombre au GDD ; effet de faction (bFactionEffect, Honneur de l'Aube) : autre chantier.
- Placeholders.txt : rien d'ajoute (aucun provisoire visible par le joueur).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune. Anomalie mineure : un appel perdu (alias R de PowerShell = Invoke-History). IN_USE.txt du worktree modifie, non commite.
GIT_STATUS_PRINCIPALE:
M AubeInProgress.md
 M Auto_Journey_Jour.md
 M DuelInProgress.md
 M LegionsInProgress.md
 M SpecializationInProgress.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ETAPE0-1119n-A.md
?? Docs/Rapports/ETAPE0-1119n-B.md
?? Docs/Rapports/GALADRIEL-1119n1.md
?? Docs/Rapports/GLORFINDEL-1119n1.md
?? Docs/Rapports/HALDIR-1119n1.md
(DuelInProgress.md modifie voulu : ligne D7 ajoutee ; le reste est hors de mon fait : oui/inchange par moi)
