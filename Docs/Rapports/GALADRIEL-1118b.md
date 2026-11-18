# GALADRIEL-1118b : lot R T2 (2026-11-18)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, branche galadriel-tab, commit f186c68 (amend d'un premier commit 181d4e5 : 2 lignes inserees en LF remises en CRLF). Pas de C++, pas de build, rien lance.

## Fait
- rpc_orders_p1p2_run.ps1 : `hol.Rpc.OrderScript 1` / `hol.Rpc.OrderTrace 1` retires des commentaires et du client vert (`-ExecCmds="hol.Rpc.ProofRoute $bits"`). Serveur : `$svParts` (Deployable, ProofSpawn) joint en `$svCmds`, `$svExec` = `-ExecCmds="..."` seulement si non vide ; `-abslog=...$svExec" -PassThru` (guillemet ferme avant -PassThru). Colonne « Do » et `OrderRecv` retirees (champ `Kind` du tableau $orders, `$do`, `Do =` du resultat, en-tete du tableau, ligne de sortie, condition du VERDICT : `Recv -and Effect`).
- rpc_orders_p3p4_run.ps1 : plus aucun `-ExecCmds` serveur (il ne restait que OrderTrace ; pas de ProofSpawn), client vert `-ExecCmds="hol.Rpc.ProofRoute $bits"`. Champ `Kind` retire des 8 ordres, `$r.Recv` = ligne « RpcOrder: <nom> received by the server » seule, filtres d'affichage sans `OrderRecv` / `OrderSend`, commentaires renumerotes (reception = ligne 1, effet = ligne 2).
- Verdict d'un ordre = ligne RpcOrder du script + EFFET en jeu ; temoin rouge « client sans appel » (RedLines / NotSent) inchange.
- Encodage d'origine (UTF-8 sans BOM, CRLF) conserve.

## Controle final
- Grep `OrderScript|OrderTrace|OrderRecv|OrderSend|OrderLatency|OrderOrder` dans Tools\ du worktree : 0 occurrence.
- ParseFile : rpc_orders_p1p2_run.ps1 = 0 erreur ; rpc_orders_p3p4_run.ps1 = 0 erreur.
- Lignes Start-Process relues (p1p2 l.182/185/188, p3p4 l.106/110/114) : chaque ArgumentList est ferme avant -PassThru.

## Reste / NON VERIFIE
- Aucun des deux scripts n'a ete execute (consigne : aucun jeu). A rejouer apres le lot R : `rpc_orders_p1p2_run.ps1 -Root <worktree>` et `rpc_orders_p3p4_run.ps1 -Root <worktree>`.
- Le comportement serveur sans `-ExecCmds` (p3p4 toujours, p1p2 pour les ordres sans Spawn) n'est pas vu a l'execution.

## Anomalies
- Un premier essai a echoue (alias PowerShell `R` = Invoke-History) : fichiers reecrits sans changement, puis refait avec `Rep` ; sans consequence.
- IN_USE.txt mis a `OCCUPE` puis `libre` (non suivi par git). EtapeDefini.md et Auto_Journey_File.md non touches.
