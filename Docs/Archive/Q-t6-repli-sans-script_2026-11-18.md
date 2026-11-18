# [À RÉPONDRE] Q-t6-repli-sans-script : que fait `Request<X>` quand la route script échoue, une fois la RPC C++ retirée ?

Contexte : aujourd'hui, si la cvar vaut 0, si l'acteur script n'existe pas ou si la fonction est introuvable, `Request<X>` retombe sur la RPC C++. Après retrait du repli, il n'y a plus de chemin de secours (client sans script, script non compilé, serveur dédié sans `Script\Net`, spectateur sans acteur). Voir `Docs\Rapports\PLAN-1118a.md` §4.

Q1 : comportement quand la route script échoue ?
- A (défaut codé) : `UE_LOG(Error)` une ligne claire + l'action n'a pas lieu ; une porte `quick_check` vérifie que l'acteur script existe avant les scénarios.
- B : même chose, mais en plus une assertion bruyante (`ensure`) en build de développement.
- C : garder la RPC C++ comme repli pour les actions critiques (Abandon, Pause) seulement.
- Autre.
Recommandation : A.

Reponse: A (2026-11-18, utilisateur, interface de choix) -> D845. Retrait des RPC sures autorise (lots A a C du PLAN-1118a).
