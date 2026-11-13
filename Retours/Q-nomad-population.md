# [À RÉPONDRE] Nomade : quel plafond de population avant le premier bâtiment ?

Contexte : au départ Nomade (D204, D207), le joueur n'a aucun bâtiment ; le plafond de population vient seulement des
bâtiments finis (forum, maisons, D312), donc il vaut 0. Corrigé ce soir (Haldir, 2bde715) : les 5 Paysans de départ sont
maintenant comptés, la barre du haut affiche « Population 5 / 0 » au lieu de « 0 / 0 ». Le GDD ne dit rien du plafond du Nomade.

- **A. Plafond 0 jusqu'au premier forum ou maison** (règle actuelle, façon AoE2 Nomade) : la barre affiche « 5 / 0 » (au-dessus
  du plafond) ; rien n'est bloqué en pratique, puisque sans bâtiment rien ne se produit ; construire ne coûte pas de population.
- **B. Plafond de départ égal à la population de départ** (5) tant que le joueur n'a aucun bâtiment fini : la barre affiche
  « 5 / 5 », plus lisible ; disparaît au premier bâtiment fini (le plafond redevient celui des bâtiments).
- **C. Plafond de départ fixe du Nomade** (par exemple 10, le forum en donne normalement) ajouté au plafond des bâtiments
  pendant toute la partie : avantage permanent qui compense l'absence de forum au départ.
- **Autre** : à préciser.

Recommandation : **A** (aucune règle de plus, le chiffre « 5 / 0 » dit justement « construis un forum ») ; B si « 5 / 0 »
gêne à l'œil (peut sembler une erreur).

Valeur par défaut codée : A (comportement actuel, aucun réglage ajouté ce soir faute de temps : NON FAIT, B ou C demandent
un réglage dans `UHOLMatchSettings` lu par `UHOLProductionSubsystem::AdvanceQueues`).

Reponse:
