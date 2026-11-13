# Q-sites-nomade-distance [À RÉPONDRE]

Contexte : avec Sites sacrés + Nomade, D702 dit « aucun groupe de départ dans la zone d'un site : distance minimale », sans chiffre. La zone d'un site fait 12 m de rayon ; un Paysan voit à 32 m ; un site n'est visible qu'une fois exploré (D701) et rapporte 30 or / min à qui le prend (D748).

Question : à quelle distance au moins du centre d'un site un groupe Nomade peut-il démarrer ?

- **A. Juste hors de la zone (12 m)** : presque toute la carte reste disponible pour les groupes ; un groupe peut démarrer à quelques pas d'un site, le voit tout de suite et a une avance dans la course aux sites (le Paysan ne prend pas, mais il le révèle à son héros).
- **B. La zone plus la vue d'un Paysan (12 + 32 = 44 m)** : aucun groupe ne voit un site au départ, la course démarre à égalité pour tous ; un peu moins de places de départ sur les petites cartes (2 sites, disques de 44 m interdits).
- **C. La même distance qu'entre deux groupes ennemis (40 m du centre, D786)** : une seule distance à retenir pour Nomade ; un groupe peut parfois voir le bord d'un site (40 m < 44 m).
- **Autre** : une autre distance (dis-la en cases, 1 case = 1 m).

Recommandation : **B** (personne ne voit un site au départ : course équitable, conforme à D701).

Ce qui est fait (branche `net-playtest`, worktree Glorfindel, sans C++) : la règle pure `Script/Sites/HOLSiteOption.as` (case décochée par défaut, sites actifs seulement case cochée sur une carte qui en pose, case grisée sans site, groupe Nomade tenu à la zone + la marge) et ses 5 tests verts. Le réglage est une ligne : `NomadMarginCm = 3200` (B ; A = 0, C = 2800 pour 40 m du centre avec une zone de 12 m) ; ligne `site.nomad_margin_cm` de `Tools/Data/ValueSheet.csv`. Ce qui reste (C++) : la case du salon, la transmission au serveur, l'appel de la règle à la génération des groupes Nomade. Valeur par défaut si rien n'est répondu : **B**.

Réponse : B (zone + vue d'un Paysan, 44 m = `NomadMarginCm 3200`, le défaut codé). (réponse donnée dans la conversation, 2026-11-13 ~19:12)
