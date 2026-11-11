# [À RÉPONDRE] Sites sacrés S6 : où et comment afficher « +N or/min » ?

Contexte : les sites versent déjà l'or (30 par minute et par site, D748). Le plan de S6 dit « ligne +N or/min au bloc du site », mais aucun site n'est sélectionnable (pas de bloc de site : S9 non fait). Glorfindel-31 a donc codé la vue pure (script, `HOLSiteBanner::GoldLine(sites tenus, taux)`), pas encore branchée à l'écran.

Options :
- **A (défaut proposé, pas encore codé à l'écran)** : une ligne dans le panneau du haut à gauche, sous le bandeau « Victoire sacrée » / volcan, visible seulement quand le joueur tient au moins un site : « +60 or/min » (total de ses sites). Rien à sélectionner, visible tout le temps.
- B : la ligne près des ressources (barre du haut, à côté de l'or), façon revenu d'AoE4. Plus lisible, mais touche la barre du haut (chantier interface).
- C : seulement au bloc de sélection d'un site, quand S9 le fera (un clic sur le site montre « +30 or/min »). Pas d'information tant qu'on ne clique pas.
- Autre.

Conséquences visibles : A et B montrent le revenu total ; C le montre par site. Le texte `+{0} or/min` est PROVISOIRE (français ; anglais et japonais à remplir).

Recommandation : A pour l'essai, B quand la barre des ressources est reprise.

Valeur par défaut : le total des sites tenus par le joueur local (propriétaire seul, jamais ses alliés) fois `hol.Site.GoldPerMinute` ; une ligne à changer.
