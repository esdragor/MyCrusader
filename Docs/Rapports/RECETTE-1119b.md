# RECETTE-1119b : panneau de triche, AVANT (C++) / APRES (script), 2026-11-19

Verdict global : DIFFERENT. Le panneau script n'est pas identique au panneau C++ (marges et couleur des poignees de curseurs).

## Resolutions
Les 10 captures (5 AVANT, 5 APRES) font 1920x1080 : la comparaison est valable.

## Images cote a cote (zone 0,0 - 620,560, echelle 1, AVANT a gauche x 0-619, bande magenta 10 px, APRES a droite x 630-1249)
Dossier : C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\compare\ecran_hud_triche\
triche_pliee.png, triche_ouverte.png, triche_nourriture.png, triche_construction_rapide.png, triche_vitesse.png

## Verdict par paire
| Paire | Verdict | Difference |
|---|---|---|
| triche_pliee | DIFFERENT (leger) | Cadre du bouton « Triches + » 8 px plus large : AVANT x 16-222 environ, APRES x 16-230 environ ; bouton interne decale de 4 px vers la droite (marge gauche 12 px au lieu de 8). Texte, hauteur et position verticale identiques. |
| triche_ouverte | DIFFERENT | Voir ci-dessous (largeur du panneau, decalage des boutons, curseurs). |
| triche_nourriture | DIFFERENT | Memes ecarts que triche_ouverte. « Nourriture 6200 » dans la barre du haut : identique des deux cotes (barre du haut : 1 pixel different sur 38656). |
| triche_construction_rapide | DIFFERENT | Memes ecarts. Libelle « Construction rapide : oui » identique des deux cotes ; sur la ligne la plus large le texte touche les bords des deux cotes (192 px de bouton, meme largeur). |
| triche_vitesse | DIFFERENT | Memes ecarts ; libelle « Vitesse des unites : x2.0 » identique ; poignee a x environ 89 (AVANT) contre 93 (APRES) : meme decalage de 4 px, pas de difference de valeur. |

## Differences precises (mesurees sur les pixels de triche_ouverte)
1. Largeur du panneau : AVANT x 16-224 (208 px), APRES x 16-232 (216 px), soit +8 px a droite. Hauteur identique (y 70-440). Position par rapport a la barre du haut (y 64) et au bord gauche (x 16) : identique.
2. Boutons : largeur identique (192 px). AVANT x 24-216 (marge 8 px de chaque cote), APRES x 28-220 (marge 12 px de chaque cote) : decalage de 4 px vers la droite. Positions verticales des 8 boutons (y 78, 113, 145, 177, 209, 241, 273, 305) identiques. Couleur du fond du panneau (58,67,64) et des boutons (75,75,85) identiques ; libelles, ordre, taille et couleur des textes identiques a l'oeil.
3. Curseurs : piste AVANT x 28-212, APRES x 32-216 (decalage de 4 px). Libelles « Vitesse des unites : x1.0 » et « Vitesse de collecte : x1.0 » identiques mais decales de 4 px dans APRES.
4. Poignees des curseurs : AVANT dore/ocre (181,161,103) ; APRES blanc/gris clair (255,255,255 au centre, 195,195,195 sur les bords) : DIFFERENCE de couleur visible. Forme et taille a peu pres equivalentes.
5. Rien en plus ni en moins : pas de ligne « niveau de l'IA » ni d'autre element d'un seul cote.

## Indice de pixels (triche_ouverte)
- Zone entiere 620x560 : 183 673 pixels differents sur 347 200 (decor 3D, ignore).
- Rectangle du panneau seul (x 16-224, y 70-440, soit 76 960 pixels) : 17 728 differents au seuil 0 ; 12 548 au seuil de 12 (somme des ecarts RGB) ; ils sont dus au decalage de 4 px des boutons et textes, a la colonne de 8 px a droite, et aux poignees.
- Barre du haut (y 0-64) : 1 pixel different au seuil 12.
