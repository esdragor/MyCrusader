# HALDIR-1119n1 : nombre d'options du choix de specialisation par faction (Templiers 3, autres 2), 2026-11-19 21:2x

Branche `templar-options` (worktree HeroesOfLiberty-Haldir), branche prete, pas de merge dans main.
FAIT: `Script\Specialization\HOLSpecializationRules.as` : `OptionCountOf(EHOLFaction)` (Templars = 3 via `TemplarOptionCount`, sinon `DefaultOptionCount` = 2), `NewStateFor(EHOLFaction)`, `IsValidOptionFor(EHOLFaction, int)`. Variantes ajoutees, aucune signature existante changee (appelants : seul `HOLSpecializationBoardScript.as:13` appelle `NewState(DefaultOptionCount)`, inchange). `Start` valide deja via `State.OptionCount` : un etat `NewStateFor` suffit pour que la 3e option soit acceptee (Templiers) ou rendue `BadOption` (autres).
COMMIT_ROUGE: 5d2709f (stub qui compile : 2 pour tous, 3e option refusee + 3 tests)
COMMIT_VERT: bd5c239 (vraie regle)
TESTS_SCRIPT_AJOUTES: 3 (Test_SpecializationOptionCountPerFaction, Test_SpecializationTemplarsCanPickTheThirdOption, Test_SpecializationDawnRefusesTheThirdOption) dans `Script\Tests\HOLSpecializationRules_Test.as`
ROUGES_ATTENDUS_SUR_5d2709f: 2 sur 3 (OptionCountPerFaction : 3 attendu, 2 rendu ; TemplarsCanPickTheThirdOption : Start rend BadOption). Le test Dawn passe sur le stub (le stub refuse deja la 3e option) : verrou de non-regression, pas de rouge.
BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun
APPELS_DECLARES: ~12
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 5d2709f + assertit l'effet : oui (non lance, rouge NON VU a l'execution)
COMMITS: 2, dernier a 21:23, merge_main_fait: non (worktree a jour avec main au depart)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (les 2 tests de comportement passent par Start/Advance reels)
ESSAIS: aucun
QUESTIONS: aucune
A LANCER (session principale) : `quick_check -Parts tests` sur -Root le worktree, d'abord sur 5d2709f (2 rouges attendus), puis sur bd5c239 (vert). Syntaxe AS relue contre le code voisin, NON COMPILEE.
BRANCHEMENT RESTANT (non fait, non tranche) : `HOLSpecializationBoardScript.as:13` doit construire l'etat de chaque siege avec `NewStateFor(faction du siege)` (la faction du siege doit etre connue du tableau) ; 3e case de la grille du Forum (cases D/F aujourd'hui, Voie A/B) et disposition de la 3e voie (Q-specialization-ui, D823) ; libelles des commanderies (GDD 13, provisoires).
Suivi : une ligne ajoutee en fin de SpecializationInProgress.md (ligne 117). EtapeDefini.md et Auto_Journey_File.md non touches.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE (hors docs voulus : voir sortie) :
 M AubeInProgress.md
 M Auto_Journey_Jour.md
 M LegionsInProgress.md
 M SpecializationInProgress.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ETAPE0-1119n-A.md
?? Docs/Rapports/ETAPE0-1119n-B.md
?? Docs/Rapports/GALADRIEL-1119n1.md
?? Docs/Rapports/GLORFINDEL-1119n1.md
