# SAURON-1121a (2026-11-21 10:50-10:58, worktree HeroesOfLiberty-Sauron, tool-known-reds)
APPELS_DECLARES: 22 (vus par moi)
HEURE_DEBUT/HEURE_FIN: 10:50:39 / 10:58
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: pas vu (la garde hol.Select.Hold n'a jamais casse en 4 rejeux) ; assertit l'effet : sans objet
COMMITS: 1 (4387164, 10:56), merge_main_fait: oui (branche a main bce2d69, rien a fusionner)
TESTS: preflight virtuous_knight x4 : R1 PASS, R2 et R3 check PASS (31/34 s) mais preflight FAIL(1) car virtuous_knight.txt modifie non commite, R4 PASS apres commit ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: virtuous_knight : preflight PASS (R4), PNG hp ouvert (Chevalier Vertueux 151/155, anneau 8 m, allie dedans), geste interne (hol.*), son non entendu ; LOGIQUE/VU A L'ECRAN inchange ; E-virtuous_knight.md non touche (consigne)
QUESTIONS: aucune
FAIT: RECETTE-1120sb n'avait pas la cause au journal -> rejeux. Garde tenue 4/4. Cause : redirection. Journal : '#15 Spearman hits #17 ... 74.4 HP left' (80-5.6), '#16 hits #15 ... 68.8 HP left' (74.4-5.6) : l'allie garde 70 %, le Chevalier (#14) recoit 30 % = 2.4/coup ; 2 coups = 150.2 -> 151/155 ; 3 coups (recette) = 147.8 -> 148. 0 ligne 'hits #14'. Scenario : hol.Units.Where 0 avant la capture hp (journal : '#14 hp 151/155', '#15 hp 69/80') + note 3 chiffree.
RESTE: C++ non permis ici : une ligne UE_LOG par part dans HOLCombatProcessor.cpp:752 ApplyDamageRedirect (ex. 'Combat: redirect #<allie> -> #<chevalier> <part>') pour qu'un waitlog la voie ; a mettre au lot C++ groupe. Puis neutrals_clairiere non pris (temps).
NON VERIFIE: 148 de la recette non rejoue exactement (ici 151, 2 coups au moment du rapport, meme arithmetique) ; vrais gestes souris/clavier ; son ; la valeur 30 % / 8 m lue de la table pas relue.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: preflight donne FAIL(1) sur un scenario modifie non commite alors que le 'check' est PASS (comportement de la garde 'commite', pas un bug). 3 UnrealEditor visibles pendant R1 (autres worktrees dont celui de l'utilisateur, non touches).
DIAGNOSTICS_AS: sans objet (aucun .as modifie)
GIT_STATUS_PRINCIPALE: M AlertsMinorInProgress.md
 M AubeInProgress.md
 M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M Auto_Journey_Retours.md
 M ChatInProgress.md
R  Retours/E-virtuous_knight.md -> Docs/Archive/Retours_2026-11-21/E-virtuous_knight.md
 M EtapeDefini.md
 M GDD.md
 M MoraleInProgress.md
 M Palier3InProgress.md
 M Retours/ESSAIS-A-JOUER.md
?? Docs/Archive/Retours_2026-11-21/Auto_Journey_Retours_item_virtuous_knight_1121.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/CELEBORN-1121a.md
?? Docs/Rapports/ELROND-1121a.md
?? Docs/Rapports/GALADRIEL-1121a.md
?? Docs/Rapports/GALADRIEL-1121b.md
?? Docs/Rapports/GLORFINDEL-1121a.md
?? Docs/Rapports/HALDIR-1121a.md
?? Docs/Rapports/HALDIR-1121b.md ; inchange hors docs voulus : oui