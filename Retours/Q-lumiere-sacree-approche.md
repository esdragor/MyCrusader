# [À RÉPONDRE] Lumière sacrée hors de portée : moment du paiement et annulation (D900, GLORFINDEL-1124e)

Contexte : le héros marche jusqu'à (portée - 50 cm) du point visé puis lance (D900, réponse C). Deux points restent à fixer ; le jeu applique les défauts ci-dessous sans attendre.

1. Moment du paiement de l'Honneur et de la recharge.
   - A (défaut codé, `hol.Faction.ApproachPays 0`) : à l'arrivée ; rien n'est dépensé si le héros meurt ou si l'ordre est annulé en chemin. Risque : l'Honneur peut manquer à l'arrivée (refus muet à ce moment).
   - B (`hol.Faction.ApproachPays 1`) : à l'ordre ; l'Honneur est garanti, mais perdu si le héros meurt en chemin ou si le lancer est annulé.
   - Autre : à l'arrivée, avec contrôle de l'Honneur dès l'ordre (refus immédiat s'il manque).
   Recommandation : A (rien de perdu sans effet).
2. Annulation du lancer en attente.
   - A (défaut) : tout nouvel ordre du héros annule le lancer ; en script seul, le lancer expire au bout de 60 s et s'abandonne si le héros meurt ; le crochet « nouvel ordre -> CancelPending » est C++ (TYPES_A_EXPOSER).
   - B : seul un nouvel ordre de déplacement l'annule (attaquer ou garder la zone ne l'annule pas).
   Recommandation : A.