# [À RÉPONDRE] Q-t6-temoin-cvar : que devient la cvar `hol.Rpc.Script<Nom>` après retrait de la RPC C++ ?

Contexte : les cvars servent aujourd'hui d'interrupteur ET de témoin rouge des preuves réseau (`rpc_routes_seats_run.ps1` : « Duel cvar 0 : rien » ; `quick_check -RpcScript` pose 22 lignes de cvars). Sans RPC C++, une cvar à 0 ne ferait plus que couper l'action. Voir `Docs\Rapports\PLAN-1118a.md` §3 lots A à C.

Q1 : sort des cvars des RPC retirées ?
- A (défaut codé) : supprimées ; le témoin rouge devient « client sans appel » (aucune ligne serveur) et `-RpcScript` perd les cvars retirées.
- B : gardées comme interrupteur de secours (défaut 1), retirées plus tard.
- Autre.
Recommandation : A (une règle, un chemin, D839).
