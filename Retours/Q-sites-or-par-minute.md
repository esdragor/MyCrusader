[À RÉPONDRE] Sites sacrés S6 : combien d'or par minute et par site ?

Contexte : un site tenu verse de l'or à son propriétaire seul (D699), avec report des fractions ; premier revenu passif du jeu. Le taux n'est écrit nulle part au GDD (D345 : c'est l'utilisateur qui le choisit). Valeur codée : 30 or / min / site, réglage `hol.Site.GoldPerMinute` (`HOLSite::DefaultGoldPerMinute`, une ligne à changer).

Options :
- A. 30 or / min, le même pour tous les sites (défaut codé). Deux sites = 60 / min, 4 sites = 120 / min.
- B. 60 or / min, le même pour tous (revenu qui compte dès deux sites, la prise devient un vrai enjeu économique).
- C. 15 or / min (anecdotique, la prise vaut surtout pour la victoire par le compte).
- D. Un taux propre à chaque site (centre plus riche, bord plus pauvre) : demande une colonne par site dans les données de carte.
- Autre.

Conséquences visibles : le taux se lit sur le bloc du site (ligne « +N or / min », pas encore écrite) et dans l'or du joueur qui tient un site ; il pèse sur l'économie de la partie (un Paysan rapporte quelques ressources par minute).

Recommandation : A pour l'essai, puis ajuster après avoir joué une partie avec le site tenu 10 minutes.
