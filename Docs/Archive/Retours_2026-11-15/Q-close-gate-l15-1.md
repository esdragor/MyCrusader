# [À RÉPONDRE] Gate de fin de séance L15-1 : deux cas qui feront FAIL sans qu'aucun build ne manque

Contexte : `session_gate.ps1 -Phase Close` contrôle maintenant chaque worktree de `## Agents du <date>` (L15-1, FARAMIR-1115b, branche fog). Lancé pour de vrai le 2026-11-15 à 10:1x, il montre deux cas à trancher (en plus des retards normaux en milieu de séance).

## Question 1 : un commit Source fait APRÈS le build de son propre worktree
Faramir : « 1 commit(s) Source plus recents que sa DLL du 11-15 09:50 » (53c106f commité après le build qui l'a compilé ; les fichiers, eux, sont plus anciens que la DLL : `Get-StaleBinaryProblem` est vide). Même cas pour Elrond. Le geste normal d'un agent (build, tests, puis commit) fera donc FAIL à la clôture, et `sync_worktrees.ps1` ne rebuild pas ce worktree (rien n'a changé dans ses fichiers).
- A : FAIL quand même (règle à la lettre) : il faut rebuilder ce worktree pour rien.
- B : le critère « commits Source plus récents que la DLL » ne fait FAIL que si `Get-StaleBinaryProblem` est aussi non vide ; sinon WARN.
- C : WARN toujours pour ce critère (seuls « pas à main » et `Get-StaleBinaryProblem` font FAIL).
- Autre : ...
Recommandation : **B** (le binaire correspond aux fichiers ; seul un fichier plus récent que la DLL prouve un build manquant).
Défaut codé : **A** (la règle telle qu'écrite) ; reglage NON code (budget d appels) : une ligne a changer dans le bloc L15-1 de `session_gate.ps1`

## Question 2 : la copie principale citée au tableau (agents de nettoyage, docs seulement)
La copie principale figure au tableau (« copie principale », NETTOYAGE-0a) ; sa DLL date du 2026-11-11 (138 commits Source de retard). Aucun agent n'y compile (docs seulement) : elle fera FAIL à chaque clôture.
- A : la contrôler comme un worktree (à main + DLL) : il faut la compiler à chaque fin de séance.
- B : pour la copie principale, seulement « à main » (+0/-0), pas la DLL.
- C : ne la contrôler que si une ligne du tableau à son nom porte `C++: oui` (colonne C++).
- Autre : ...
Recommandation : **B** (personne ne joue depuis la copie principale ; ses essais passent par les worktrees).
Défaut codé : **A** (la règle telle qu'écrite) ; reglage NON code (budget d appels) : une ligne a changer dans le bloc L15-1 de `session_gate.ps1`

Reponse (2026-11-15) : Q1 B : un commit Source plus récent que la DLL ne fait FAIL que si Get-StaleBinaryProblem est aussi non vide, sinon WARN. Q2 B : la copie principale n'est contrôlée que sur « à main » (+0/-0), pas sur la DLL. (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D808) et le journal du chantier.
