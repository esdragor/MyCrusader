# [À RÉPONDRE] Signaux sur la carte (§ 15, D386) : durée de vie et coéquipier éliminé

Chantier `SignalsInProgress.md` (worktree Haldir, branche `maps`, HALDIR-1112e, 2026-11-12). Règles pures en script faites
(`Script/Signals/HOLSignalRules.as`) ; rien n'est encore visible en jeu (réseau, icônes et sons viennent après).

## Q1 : combien de temps un signal reste-t-il sur la carte ?

Contexte : D386 donne les 5 types, « alliés seulement » et la limite (3 toutes les 5 s), mais pas la durée d'affichage.
Elle décide combien de signaux d'un même allié on peut voir à la fois.

- **A. 5 s** (égale à la fenêtre de la limite) : jamais plus de 3 signaux d'un allié à l'écran ; carte propre, mais un signal posé
  pendant qu'on regarde ailleurs peut être manqué (la ligne du fil reste).
- **B. 10 s** : jusqu'à 6 signaux d'un allié en même temps ; plus facile à voir en retard, plus encombré en plein combat.
- **C. Jusqu'au 4e signal du même joueur** (les 3 derniers restent) : un « Regroupement » posé reste visible tant qu'on ne pose rien
  d'autre ; risque de vieux signaux trompeurs sur la carte.
- **Autre** : à préciser.

**Recommandation : A** (lisible, cohérent avec la limite ; le fil garde la trace). Codé par défaut : `HOLSignal::LifetimeMs = 5000`
(une ligne, registre `comm.signal.lifetime_ms`).

## Q2 : un joueur éliminé en partie à équipes peut-il encore poser des signaux pour son équipe ?

Contexte : D357 lui laisse la vision de son équipe et la discussion avec ses coéquipiers (il conseille) ; en chacun pour soi et en
*Diplomatie*, D376 le réserve aux spectateurs (là, aucun signal : déjà réglé).

- **A. Non** : il voit les signaux de son équipe mais n'en pose plus ; il conseille seulement par écrit.
- **B. Oui, comme sa discussion** : ses signaux arrivent chez ses coéquipiers vivants (même limite de 3 toutes les 5 s) ; il peut
  montrer du doigt ce que l'équipe voit déjà.
- **C. Oui, mais seulement le signal neutre** (Alt + clic) : il pointe un endroit sans donner d'ordre typé.
- **Autre** : à préciser.

**Recommandation : B** (même règle que sa discussion, D357 ; il n'apprend rien à l'équipe qu'elle ne voie). Codé par défaut :
`HOLSignal::DefeatedTeammateSignals = true` (une ligne).
