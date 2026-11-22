# [À RÉPONDRE] Furtivité : comment rendre « ses propres unités à demi transparentes » (D824 Q3 B)

Contexte : la règle pose déjà 0.5 (en zone) / 1.0 (hors zone) dans le 5e float de donnée d'instance de chaque unité (index 4, prouvé par le scénario `stealth_zones`, ARWEN-1122e). Aucun matériau ne lit cet index : en jeu, rien ne change à l'écran. D824 dit « à demi transparentes » mais pas COMMENT, et les trois techniques coûtent très différemment (ARWEN-1122f, estimations NON MESURÉES).

Les matériaux à modifier sont trois (pas un) : `M_UnitPlayerColorProps` (paysan), `M_UnitPlayerColorWeapons` (archer, arquebusier, arbalétrier, lancier, homme d'armes, héros, roi) et `M_UnitPlayerColor` (mannequins d'essai seulement). Le mélange est aujourd'hui Opaque (jamais changé dans les scripts générateurs).

- A. Masked + tramage (dither) sur l'opacité. En jeu : l'unité en zone est « criblée » (un pixel sur deux), lisible, avec ses ombres ; les couleurs de joueur (index 0 à 3) ne changent pas ; grain visible si l'anticrénelage temporel est coupé. Coût : TOUTES les unités passent en Masked, même à 1.0 (le mode est du matériau, pas de l'instance) : perte du test de profondeur précoce et ombres avec test de pixel ; surcoût modéré en foule (unités petites à la caméra RTS), à mesurer avec le test de charge à 2000 unités avant de garder.
- B. Translucent. En jeu : vraie transparence lisse, mais UN tri par composant instancié (pas par unité) : membres et accessoires se superposent de façon instable ; les unités n'écrivent plus la profondeur (le brouillard post-process et l'occlusion ne les voient plus comme avant) ; éclairage et ombres reçues dégradés, couleurs de joueur modifiées par l'éclairage translucide. Coût : le plus lourd (2000 unités en passe translucide, surdessin maximal), et toutes les unités y passent, même à 1.0. Déconseillé en foule.
- C. Teinte / désaturation sans transparence (mélange vers un gris, ou assombrissement, selon l'opacité). En jeu : l'unité en zone paraît « pâlie », pas transparente : ce n'est PAS la lettre de D824 Q3 B ; le décor derrière ne se voit pas à travers. Coût : quasi nul (une lecture de donnée et un mélange par pixel, pas de changement de passe). Les couleurs 0 à 3 gardent leur teinte si la désaturation est partielle (ex. 50 %).
- Autre : (à préciser).

Recommandation : A (seule option qui tient « à demi transparentes » sans le tri ni le surdessin de B), à condition que la mesure de charge à 2000 unités soit acceptable ; sinon C.

Valeur par défaut si pas de réponse : rien de rendu (matériaux inchangés ; la règle et l'index 4 restent en place, sans effet visible).
