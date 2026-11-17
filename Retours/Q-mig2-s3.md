# [À RÉPONDRE] MIG2-S3 : où doivent vivre les RPC du joueur maintenant que le script sait en écrire ?

**Contexte.** La preuve est faite (2026-11-17) : une RPC « client demande, serveur répond » écrite en script marche sur le serveur dédié. Les 57 RPC du contrôleur
du joueur (ordres, marché, diplomatie, signaux, chat, triches...) sont aujourd'hui déclarées en C++ : en ajouter une demande une compilation (ou la changer). Détail : `Docs\Rapports\PLAN-MIG2-S3-1117.md`.
Réponds par deux lettres, par exemple « 1A 2B ».

## Question 1 : qui déclare les RPC ?

- **A. Un acteur script par joueur déclare les RPC** (comme la sonde). Ajouter ou changer une RPC ne demande plus de compilation ; le contrôleur C++ perd ~3 000 lignes de déclarations et d'enveloppe. Mais chaque appelant de ces RPC (interface, IA, console) change, et une RPC migrée peut devenir muette si l'acteur n'est pas créé (spectateur, reconnexion) : chaque RPC migrée demande un essai à 2 clients.
- **B. Le contrôleur C++ garde les déclarations** et chacune appelle le script en une ligne. Aucun appelant ne change, le risque est faible ; mais ajouter une nouvelle demande demande toujours une compilation (seul le corps change sans compilation).
- **C. Une paire de RPC génériques** (« demande du client », « réponse du serveur ») que le script étend. Plus aucune compilation pour une nouvelle demande, mais la validation du serveur (qui a le droit de faire quoi) se déplace entièrement dans le script, avec un seul canal à surveiller pour la triche.

**Recommandation : A, en commençant par UNE seule RPC (la diplomatie, 12 lignes) pour mesurer, puis décider du reste.** **Valeur par défaut codée : A pour le pilote seulement**, aucune autre RPC ne bouge sans réponse.

## Question 2 : quelles RPC migrer, dans quel ordre ?

Les 57 se classent en 5 lots (détail dans le plan) : 18 triviales (diplomatie, duel, partie, héros, signaux, chat), 16 ordres du joueur (un clic = une RPC, fréquence élevée), 4 économie, 14 triches (outils de test), 5 à grosse charge (statistiques de partie, mémoire du brouillard, bulles de dépôt ; structures invisibles du script).

- **A. Les 18 triviales d'abord, puis les ordres après une mesure de latence, les triches en remplissage, les 5 grosses charges restent en C++.** Le plus sûr ; gain visible tout de suite sur les règles déjà en script.
- **B. Tout migrer, y compris les 5 grosses charges** (3 structures à rendre visibles au script, un lot C++). Plus complet, plus long, gain faible sur ces 5.
- **C. S'arrêter aux 18 triviales** : le reste des 57 reste en C++, pas de mesure de latence à faire.

**Recommandation : A.** **Valeur par défaut codée : C** (on ne fait que les 18, le reste attend ta réponse).
