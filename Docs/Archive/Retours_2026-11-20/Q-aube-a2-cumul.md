# [À RÉPONDRE] Défi du Chevalier (passif) : que font deux Chevaliers proches du même allié ?

Contexte : D19 redirige une part des dégâts d'un allié vers le Chevalier Vertueux. Le GDD ne dit rien du cas de plusieurs Chevaliers (AubeInProgress.md A2 : « règle à proposer : le plus proche, ou plafond de cumul, à demander »). La règle pure `HOLDamageRedirect::Redirect` (SAURON-1120f, branche tool-known-reds) accepte les deux modes.

- **A. Le plus proche seul** (`ECumulMode::Nearest`) : seul le Chevalier le plus proche de l'allié prend la part (égalité : le plus petit identifiant). En jeu : deux Chevaliers ne protègent pas mieux qu'un ; en revanche l'allié passe d'un Chevalier à l'autre en se déplaçant, et le joueur n'a pas intérêt à empiler les Chevaliers pour la redirection. Lisible.
- **B. Cumul plafonné** (`ECumulMode::CappedSum`) : chaque Chevalier à portée ajoute sa part, la somme est plafonnée (par exemple 50 %), partagée à égalité. En jeu : deux ou trois Chevaliers soulagent nettement l'infanterie, jusqu'au plafond ; plus fort pour l'Aube, plus de contre-jeu nécessaire (dégâts de zone, perforants). Il faut en plus choisir le plafond.
- **C. Cumul sans plafond** : chaque Chevalier redirige sa part (non codé ; possible à 100 % cumulé, donc un allié intouchable) : déconseillé.
- **Autre** : à dire.

**Recommandation : A** (lisible, pas d'empilement, cohérent avec « le héros reste quasi-intuable évité » de D19).
**Valeur par défaut codée** : `HOLDamageRedirect::DefaultCumulMode = ECumulMode::Nearest` (`Source\HeroesOfLiberty\Simulation\HOLDamageRedirectRules.h`, une ligne ; le plafond `CapPercent` est un paramètre de l'appel, 50 dans les tests).

Reponse: (2026-11-20 ~12:30, interface de choix) A : le Chevalier le plus proche seul (`ECumulMode::Nearest`, défaut codé déjà). Voir GDD D864.
