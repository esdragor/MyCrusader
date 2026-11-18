# GALADRIEL-1118a rapport (lot R, T outils + S script, sans C++, sans build)
- worktree: Documents\HeroesOfLiberty-Galadriel, branche galadriel-tab, base 0bff683
- commits: S = e5f3d8b (HOLRpcPlayer.as cvars + commentaires, HOLRpcOrderPilot.as supprime, commentaire ProofSpawn) ; T = 5d8b148 (gate L2 + order_latency_run/order_order_run supprimes)
- TEST_ROUGE 2026-11-18 13:43:35 : Get-NoCppRpcLeft sur le .h de ce worktree = 16 noms (les 16 Server...). Vert attendu apres fusion avec la branche C++.
- FAIT: T1, T3, S1 (aucun lecteur des cvars dans le script : seules les 2 declarations), S2.
- RESTE: T2 (rpc_orders_p1p2_run.ps1 l.2,8,10,16,74,101,178,186,195,211 ; rpc_orders_p3p4_run.ps1 l.9,11,19,34,64,104,115,131,159 : retirer OrderScript/OrderTrace des ExecCmds, omettre -ExecCmds serveur vide, retirer colonne OrderRecv). Non fait faute d appels (limite 25). Ces scenarios mettent encore des cvars inexistantes : sans effet mais les assertions OrderRecv echoueront apres le retrait C++.
- NON VERIFIE: Angelscript non compile ici ; diff relu (suppressions de lignes seules + commentaires).
- Controle final Grep: reste uniquement dans p1p2/p3p4 (T2).
- Anomalie: IN_USE.txt non suivi par git. Get-NoCppRpcLeft exige le chemin du .h en argument (HeaderPath).
- Champs migration: BUILDS_DEMANDES: 0, TESTS_CPP_RETIRES: 0, TESTS_SCRIPT_AJOUTES: 0, LIGNES_CPP_SUPPRIMEES: 0, LIGNES_MIGREES_PAR_LOT: 0, TYPES_A_EXPOSER: aucun
