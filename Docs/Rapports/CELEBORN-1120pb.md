APPELS_DECLARES: 17 (arret a 25 vus ; 16:01 non atteint)
HEURE_DEBUT/HEURE_FIN: 15:46 / 15:48
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 95cf44a (Test_DuelRewardsOrdersCarryTheirPlace ; stub OrdersAt = Orders sans lieu) ; assertit l'effet : oui (lieu de l'ordre = centre calcule par MakeArena / position donnee) ; rouge NON VU a l'execution
COMMITS: 4 (95cf44a test, 7d0511b correctif, 1aaab39 if au lieu de ternaire, journal 7cc5697), dernier 15:48, merge_main_fait: oui (deja a jour)
TESTS: aucun lance (sans build ni quick_check) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune (D866 Q2 = A deja tranchee). A signaler : le ledger n'a pas la position du heros refuseur (AdvanceDuels ne recoit que la sante) : le pont C++ devra la fournir a OrdersAt.
FAIT / RESTE: D866 exige du C++ (source unique) : pris la sous-etape 6a du plan (sans C++). FHOLDuelMoraleOrder + bHasCenter/CenterXCm/CenterYCm ; PlacedOrders/OrdersAt/Orders dans Script\Duel\HOLDuelRewards.as ; test dans HOLDuelRewards_Test.as. Reste le lot C++ M3-M5 + pont ApplyEffectAround (sous-etapes 1-10 du plan), HOLMoraleRules.as en habillage.
NON VERIFIE: syntaxe Angelscript non compilee (struct par valeur modifie dans PlacedAt, OrdersAt non appelee ailleurs) ; rouge et vert jamais executes ; DIAGNOSTICS_AS non releves (outil de diagnostics non charge) ; appelant de OrdersAt avec la position du refuseur inexistant.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune ; accents du journal : le texte a ete passe par PowerShell, HasCenter ecrit HasCenter (backtick-b) dans MoraleInProgress.md l.167
GIT_STATUS_PRINCIPALE: M AubeInProgress.md
 M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M ChatInProgress.md
 M EtapeDefini.md
 M Palier3InProgress.md
 M SpecializationInProgress.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/CELEBORN-1120pa.md
?? Docs/Rapports/ELROND-1120pa.md
?? Docs/Rapports/ETAPE0-1120pm.md
?? Docs/Rapports/FARAMIR-1120pa.md
?? Docs/Rapports/GALADRIEL-1120pa.md
?? Docs/Rapports/GALADRIEL-1120pb.md
?? Docs/Rapports/GLORFINDEL-1120pa.md
?? Docs/Rapports/HALDIR-1120pa.md
?? Docs/Rapports/SAURON-1120pa.md
?? Retours/Q-chevalier-ligne.md
?? Retours/Q-deploy-visible.md (docs voulus : EtapeDefini.md, Auto_Journey_File.md ; le reste preexistant)