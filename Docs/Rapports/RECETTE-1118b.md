# RECETTE-1118b : écran Nouvelle partie, AVANT (C++) contre APRÈS (script)

Agent indépendant, lecture seule (aucun lancement). Les 10 captures ont été ouvertes et comparées.

## Dates des fichiers (vérifiées, PowerShell, 21:43)
- AVANT (Sauron) : les 5 fichiers datent du 18/11/2026 20:44:24 à 20:44:26. Cohérent avec le pré-vol de 20:44.
- APRÈS (Arwen) : les 5 fichiers datent du 18/11/2026 21:41:51 à 21:41:53. Cohérent avec le pré-vol de 21:41.
- Aucune date ne détonne.

## Point préalable : la résolution des captures diffère (pas l'écran)
- AVANT : 1066 x 600. APRÈS : 1280 x 720. Le facteur est exactement 1,2.
- J'ai donc comparé en proportion : positions et largeurs des AVANT multipliées par 1,2, contre les APRÈS.
- Repères mesurés (AVANT x 1,2 contre APRÈS) :
  - Liste Carte : 179-461 donne 215-553, APRÈS 215-553.
  - Liste Brouillard : 601-723 donne 721-868, APRÈS 721-868.
  - Bouton Mode Standard : 111 px de large donne 133, APRÈS 133.
  - Ligne 1 de la grille : y 205 donne 246, APRÈS 245.
  - Pas entre lignes : 29 donne 34,8, APRÈS 35.
  - Mode : y 445 donne 534, APRÈS 535.
  - Retour/Lancer : y 513 donne 616, APRÈS 617.
  - Titre centré dans les deux.
- Les notes jaunes du scénario ne sont pas mises à l'échelle (texte de debug). Elles se superposent donc différemment à l'écran sans que ce soit une différence de l'écran.
- Conséquence : je ne peux pas juger les écarts de 1 à 2 px (arrondis d'échelle). Les écarts que je signale sont ceux qui dépassent cet arrondi, ou qui sont de nature différente (couleur, état, texte).

## Paire 1 : ecran_np_ouverture
Verdict : identique (à l'échelle 1,2 près).
- Titre « Nouvelle partie », ligne du haut : Carte = Plaine ; Taille = Petite (2 joueurs) ; Brouillard = Sans brouillard ; Équipes ensemble = Oui. Identiques.
- En-têtes Joueur / Niveau / Faction / Couleur / Équipe : même place, même italique gris.
- Place 1 : « Vous », pas de liste de niveau, Ordre de l'Aube, pastille bleue + « Bleu », équipe 1. Identique.
- Place 2 : « IA Normal 2 » actif, niveau « Normal » grisé, Ordre de l'Aube, pastille rouge + « Rouge », équipe 2. Identique.
- Places 3 à 8 : « Fermé » grisé (état seul, ni niveau ni faction ni couleur ni équipe). Identique.
- Ligne du bas : Mode Standard actif ; Nomade, Régicide, Catastrophe grisés sur « Non » ; Sites sacrés « Non » actif. Identique.
- Note « Grisé : pas encore dans le jeu (factions, mode et options). », centrée sous la ligne du bas. Identique.
- Retour et Lancer présents et actifs ; cadre doré de focus sur Lancer dans les deux.
- Aucun texte coupé, aucune erreur affichée.

## Paire 2 : ecran_np_ia_niveau
Verdict : identique.
- Taille = Moyenne (4 joueurs) dans les deux.
- Place 2 « IA Normal 2 » et place 3 « IA Normal 3 » : états actifs, listes de niveau « Normal » grisées, Ordre de l'Aube, pastilles Bleu / Rouge / Vert, équipes 1 / 2 / « Aucune ». Identique.
- Place 4 : « Fermé » actif (non grisé, chevron visible), sans niveau ni faction ni couleur ni équipe. Identique.
- Places 5 à 8 : « Fermé » grisé. Identique.
- Ligne du bas, note, boutons : identiques.
- Écart de nature des notes jaunes : la note 1 est plus longue dans l'APRÈS. Ce n'est pas une différence d'écran, c'est la largeur de la capture (1280 contre 1066).

## Paire 3 : ecran_np_couleur
Verdict : identique.
- Places 2, 3, 4 sur IA, chacune avec son niveau « Normal » grisé. Pastilles et noms : Bleu, Rouge, Jaune, Turquoise. Les 4 sont différentes dans les deux séries.
- La place 4 est bien passée à Turquoise (couleur libre suivante) dans les deux séries.
- Équipes : 1, 2, « Aucune », « Aucune ». Identique.
- Focus sur la liste de couleur de la place 4 : fond brun-doré et cadre clair dans les deux.
- Places 5 à 8 « Fermé » grisé. Ligne du bas et boutons identiques.
- Teintes des pastilles : les 4 teintes me paraissent identiques à l'oeil (bleu franc, rouge, jaune, turquoise). Je n'ai pas de mesure en pixels.
- Non jugeable dans l'AVANT : la ligne du haut (Carte, Brouillard) est masquée par les notes jaunes (la capture est plus petite, les notes tombent sur cette ligne). Elle est visible dans l'APRÈS (Plaine / Sans brouillard) et dans la paire 1.

## Paire 4 : ecran_np_taille_liste
Verdict : identique sur le contenu et le grisé, un écart probable de couleur de cadre à vérifier (mineur).
- Entrées de la liste déroulée, dans les deux séries : « — », « Petite (2 joueurs) », « Moyenne (4 joueurs) », « Normale (6 joueurs) », « Grande (8 joueurs) », « Immense (8 joueurs) ».
- Grisés : seule « Petite (2 joueurs) » est grisée (texte gris). « — » est en gris clair, comme dans l'autre série. Moyenne, Normale, Grande, Immense sont en clair. Identique.
- Géométrie : même largeur que la liste Taille, collée en dessous, pas entre entrées 28 px donne 33,6 contre 33,5 mesurés. Identique.
- La liste cache les mêmes cellules (colonnes Joueur / Niveau / Faction des places 1 à 4) dans les deux séries.
- Le focus a quitté Lancer (pas de cadre doré) dans les deux. Le champ Taille est en brun-doré dans les deux.
- Écart probable (couleur de cadre, faible confiance, à confirmer sur les pixels) :
  - AVANT : le cadre extérieur de la liste déroulée paraît doré, et l'entrée « Moyenne » (valeur courante) paraît cernée d'un trait blanc/clair.
  - APRÈS : le cadre extérieur paraît gris clair, et l'entrée « Moyenne » paraît cernée d'un trait doré.
  - Autrement dit, les couleurs du cadre de liste et du cadre d'entrée courante semblent inversées. Le rendu PNG est trop fin pour l'affirmer à coup sûr.
- Non jugeable : la ligne du haut (Carte, Brouillard) est masquée par les notes jaunes dans l'AVANT, et partiellement dans l'APRÈS.

## Paire 5 : ecran_np_retour (écran titre après Retour)
Verdict : identique pour ce qui est le but de la capture (retour au titre réussi).
- Les deux montrent l'écran titre : « Heroes of Liberty », Jouer (cadre doré de focus), Multijoueur, Réglages, Quitter. Textes et ordre identiques.
- Écarts d'échelle, probablement des arrondis : largeur des boutons 313 px donne 376 contre 384 mesurés (environ 2 %). Positions verticales identiques à 1 px.
- Étiquette « dev » en bas à droite : droite dans l'AVANT, en italique dans l'APRÈS. Écart de style de police à confirmer. C'est probablement du code de l'écran titre ou du HUD de debug, pas de l'écran Nouvelle partie.
- Les notes jaunes sont plus longues dans l'APRÈS (largeur de capture). L'entrée cyan « Entrée : j'ai manipulé la grille » est identique.

## Verdict global : IDENTIQUE
- Aucune différence de valeur, d'état, de grisé, de texte ou de présence d'élément sur les 5 paires.
- Seules différences notées, toutes mineures ou de contexte :
  1. Résolution de capture différente (1066x600 contre 1280x720), ce qui rend impossible la comparaison au pixel.
  2. ecran_np_taille_liste : couleur du cadre de la liste déroulée et de l'entrée courante qui paraissent inversées (doré / clair), faible confiance.
  3. ecran_np_retour : étiquette « dev » droite contre italique, et largeur des boutons du titre 2 % plus large dans l'APRÈS. Probablement hors écran Nouvelle partie.
- Si l'utilisateur veut une conclusion stricte, ces points 2 et 3 relèvent de ÉCARTS MINEURS (mise en page). Je les laisse en réserve et conclus IDENTIQUE sur le comportement.

## Ce que je n'ai pas pu juger
- Comparaison au pixel : impossible avec deux résolutions différentes. Les écarts de 1 à 2 px sont indiscernables d'un arrondi de mise à l'échelle.
- Teintes exactes des pastilles et des fonds : jugées à l'oeil, sans mesure.
- Ligne du haut (Carte, Brouillard) dans ecran_np_couleur et ecran_np_taille_liste de l'AVANT : masquée par les notes jaunes.
- Texte fantôme sombre « ... Messages » pour supprimer » sous les notes : présent dans les deux séries, c'est un reste de l'outil de capture, hors écran.
- Cadre de liste et d'entrée courante (paire 4) : trop fin pour être tranché.
- Comportement de la liste Niveau grisée (focus, navigation clavier) : une capture fixe ne le montre pas.
