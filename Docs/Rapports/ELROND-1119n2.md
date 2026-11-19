# ELROND-1119n2 (2026-11-19 21:30-21:35)
Branche ai-power-spend (worktree HeroesOfLiberty-Elrond, depuis c7067cf). ROUGE 97a29d8 (bouchon : Choose retourne -1), VERT 7928566.
Fichiers : Script/AI/HOLAIPowerSpend.as (namespace HOLAIPowerSpend, Choose), Script/Tests/HOLAIPowerSpend_Test.as. Aucun fichier existant modifie.
Regle : D688/L12/A12 ne donnent que des exemples (pas de regle ecrite) : socle minimal. Choose(Rows, CooldownLeftMs, Wanted, Stock, Tier, Reserve) -> index ou -1 ; premier pouvoir du livre qui est Ready (HOLPowerBook::StateOf, refus non recopies), voulu par l'appelant (Wanted : declencheur de situation, a calculer par le cerveau), et dont Stock - Cost >= Reserve. Ordre = ordre du livre (rien d'ecrit au GDD). Tableaux de tailles differentes = -1.
Non tranche : valeurs des reserves par niveau, seuils de declenchement (Wanted), ordre de preference.
TESTS_SCRIPT_AJOUTES: 4 (Test_AIPowerSpendNothingWhenNoPowerReady, ...KeepsTheReserve, ...PicksAReadyPower, ...NeverCheats). Tous les 4 doivent etre ROUGES sur 97a29d8 (chacun a un controle positif), verts sur 7928566.
BUILDS_DEMANDES: 0. TYPES_A_EXPOSER: aucun. TEST_ROUGE: non vu (a lancer par la session principale). TESTS_TAUTOLOGIQUES: 0.
NON VERIFIE : compilation Angelscript (aucun build) ; en particulier TArray<...>() temporaires passes en const ref, const TArray indexe, helpers globaux de test (noms PowerSpend*). Commande : la partie tests de quick_check (Angelscript.UnitTests), filtre AIPowerSpend.
Note : le test est dans Script/Tests/ comme demande (les tests IA existants sont dans Script/AI/). Un warning LF/CRLF git, sans effet.
Suivi : une ligne ajoutee en fin de LegionsInProgress.md (voir ci-dessus pour sa position). EtapeDefini/Auto_Journey_File : session principale.
