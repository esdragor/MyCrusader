# RECETTE-1119e : bloc de sélection du HUD, AVANT (C++) / APRÈS (script), 2026-11-19

Méthode : 15 paires 1920x1080 (toutes de même résolution). Zone comparée : x 0..767, y 669..1079 (0-40 % x 62-100 % y), seuil 30 par canal.
Images côte à côte (x2, AVANT à gauche, APRÈS à droite) : `...\scratchpad\compare2\<scénario>\<capture>.png`. Tableau brut : `...\scratchpad\compare2\selection_pixels.txt`.

## Pixels différents et boîtes englobantes (coordonnées écran)

| Scénario | Capture | Pixels | Boîte (x min, x max, y min, y max) | Verdict |
|---|---|---|---|---|
| unite | lancier | 3 | 708,725,999,1048 | IDENTIQUE À L'ŒIL |
| unite | lancier_survol | 3 | 724,735,999,1079 | IDENTIQUE À L'ŒIL (pas d'infobulle visible, comme AVANT) |
| unite | paysan_recolte | 3 | 0,2,870,873 | IDENTIQUE À L'ŒIL |
| unite | groupe_paysans | 8 | 57,431,763,793 | IDENTIQUE À L'ŒIL |
| unite | unite_ennemie | 6 | 569,699,681,701 | IDENTIQUE À L'ŒIL |
| batiment | arbre | 8 | 0,385,866,944 | IDENTIQUE (pixels non vus, hors seuil visuel ; non ouverte, < 20 px) |
| batiment | chantier | 205 | 92,456,674,934 | DIFFÉRENT seulement par la progression (voir ci-dessous) |
| batiment | fondation | 0 | - | IDENTIQUE (0 pixel) |
| batiment | forum_ami | 5 | 157,743,672,695 | IDENTIQUE (< 20 px, non ouverte) |
| batiment | forum_ennemi | 1 | 742,742,692,692 | IDENTIQUE (< 20 px, non ouverte) |
| puces | archers | 4 | 708,750,977,1048 | IDENTIQUE (< 20 px, non ouverte) |
| puces | batiments_sous | 6 | 321,348,674,778 | IDENTIQUE À L'ŒIL |
| puces | batiments_tous | 5 | 321,348,674,778 | IDENTIQUE (< 20 px, non ouverte) |
| puces | hommes_darmes | 5 | 708,745,977,1048 | IDENTIQUE À L'ŒIL |
| puces | tous | 3 | 708,745,977,1048 | IDENTIQUE (< 20 px, non ouverte) |

Les boîtes à x >= 700 sont dans le décor 3D (queue de la zone, hors bloc ; le bloc va jusqu'à x ~355). Les 3 à 8 pixels des autres boîtes sont des pixels isolés (décor / bord de pavé), jamais du texte : un texte déplacé d'1 px ferait des centaines de pixels.

`hud_sel_chantier` (205 px) : la barre avance entre deux passages (AVANT 191 / 600, 31 % ; APRÈS 196 / 600, 32 % ; barre jaune un peu plus longue). Défaut connu du scénario, pas du panneau. Le reste (titre « Chantier : Maison », pastille bleue « Vous », « Bâtisseurs : 2 », case V « Annuler ») est identique.

## Lecture ligne par ligne (images ouvertes : 5 unite, chantier, hommes_darmes, batiments_sous)

- Nom (gras), pastille de propriétaire + nom (« Vous » bleu, « IA Normal 2 » rouge) : identiques en police, taille, position.
- « Points de vie » petit + valeur « 80 / 80 » : identiques.
- Puces de stats (« Attaque 8 · Armure mêlée 0 · / Armure distance 0 · / Armure héros 0 · / Vitesse 7,8 m/s ») : même texte, mêmes valeurs, même retour à la ligne, mêmes séparateurs « · » (y compris le « · » final en bout de ligne, présent des deux côtés), même ligne `Armure héros 0 · Vitesse 8 m/s` pour le Paysan.
- Activité : « Récolte — Bois 2/10 » et « Récolte — Nourriture 0/10 » identiques.
- Groupe de paysans : puce « Paysan (5) » encadrée jaune (active), « 1 en récolte · 4 inactifs » identiques.
- Puces de sous-groupes : « Tous (8) », « Archer (3) », « Homme d'arm. (2) » (active, cadre jaune), « Paysan (3) », puis « Tous (4) », « Centre prin. » (active), « Maison (3) » : libellés, comptes, puce active allumée et largeurs identiques ; « 3 inactifs » identique.
- Infobulle de survol (`lancier_survol`) : aucune visible ni AVANT ni APRÈS ; images identiques entre elles.
- Cartes de commandes (grille A-V, Attaque / Arrêt / Tenir, Cloche d'alarme améliorée tronquée) : identiques.

## Point 3 : décalage des séparateurs « · » du Lancier

Balayage des colonnes dont min(R,G,B) > 110 sur les 4 lignes de stats de `hud_sel_lancier` (et de `hud_sel_unite_ennemie`), x en pixels d'écran, plages de colonnes séparées par un trou > 6 px :

| Ligne | AVANT | APRÈS |
|---|---|---|
| L1 (y 915-936) : « Attaque 8 », « · », « Armure mêlée 0 », « · » | 28-100 / 108-117 / 135-137 / 155-221 / 229-234 | 28-100 / 108-117 / 135-137 / 155-221 / 229-234 |
| L2 (y 940-956) : « Armure distance 0 », « · » | 29-94 / 102-180 / 188-197 / 215-217 | idem |
| L3 (y 960-976) : « Armure héros 0 », « · » | 31-94 / 102-152 / 160-169 / 187-189 | idem |
| L4 (y 978-995) : « Vitesse 7,8 m/s » | 28-94 / 102-169 | idem |

Ennemi (Paysan) : L3 « · » 187-189 puis « Vitesse… » 207-234 ; L4 (récolte) 179-181 identiques des deux côtés.

Conclusion : écart NUL (0 px) sur tous les débuts de puces et tous les séparateurs, donc ni constant ni croissant. Le décalage de 1-2 px signalé n'existe pas à ce seuil : l'œil à l'échelle 1 ne voit rien. La très faible différence perçue dans l'image compare2 vient seulement du rééchantillonnage d'affichage de l'image (2000 px de large pour 3088), pas des captures. Limite : seuil 110 sur le min des canaux ; un décalage de sous-pixel (anticrénelage) ne serait pas distingué, mais il ne produit aucun pixel au-dessus de 30 par canal dans le tableau ci-dessus.

## Notes (hors défaut du panneau)

- `unite_ennemie` montre un Paysan de l'IA au lieu d'un Lancier : défaut connu du scénario, identique des deux côtés.
- Barre de chantier : avance entre deux passages (31 % -> 32 %).
- Anomalies : aucune autre ; aucune capture non appariée.
