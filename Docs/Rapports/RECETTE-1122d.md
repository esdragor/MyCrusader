# RECETTE-1122d (recette independante de GALADRIEL-1122g, worktree Galadriel 7687af6)

Debut 18:46, fin 18:51 (Get-Date). Aucun fichier du worktree modifie (seules sorties : Saved_Tests, ignore par git ; crops dans le scratchpad).

## Lancements
- `preflight.ps1 tech_soon` : scenario PASS (check 19 s), "Angelscript ... Error" : 0, mais preflight FAIL (1) : `tech_soon.json` n'a pas de `mapSize` (D820, map grise de test). Sans rapport avec la case. 1 WARN (pas de hol.Test.Sanity).
- `preflight.ps1 secondary_centre` : PASS (43 s), Angelscript Error : 0, sanity OK. 5 PNG ouverts (sc_forum, sc_apres ouverts + agrandis ; les 3 autres non utiles a la case W).
- Langues : tech_soon n'appelle pas `hol.UI.Language` ; check.ps1 lance en `-culture=fr` fixe. Pour EN / JA j'ai lance le MEME jeu en script ad hoc (scratchpad, meme ligne de commande que check.ps1, `-culture=en` puis `-culture=ja`, -RenderOffScreen -nosound -unattended, fenetre cachee, processus arrete apres "ready"). `-culture` change bien toute l'interface. En 0 Angelscript Error, "ready" atteint. secondary_centre force `hol.UI.Language fr` : FR seulement.

## Ce qui a ete vu (PNG ouverts avec Read, agrandis x5 sur la grille)
tech_soon_barracks est capture en 1280x720 (le scenario n'a pas de `r.SetRes`) : la case y mesure ~51 px, pas 76. Cas plus severe que le jeu reel.

FR (tech_soon, 1280x720) : "Hallebardes trempees" (2 lignes) puis "Bientot" ENTIER sur sa propre ligne en bas, grise, dans la case. Le mot "Hallebardes" est coupe a gauche et a droite ("allebarde") : mot plus large que la case, meme defaut sur la case A SANS etat ("allebardie"), donc non cause par le statut.
FR (secondary_centre sc_forum, 1920x1080 = vraie case de 76 px) : W = "Cloche d'alarme" (2 lignes) + "Bientot" entier, bas de case, aucun debordement, cases voisines propres. C'est la capture la plus representative.
EN (1280x720) : "Tempered Halberds" 2 lignes + "Coming soon" sur sa ligne, MAIS "Coming soon" (~60 px) est plus large que la case de 51 px : il deborde a gauche sur le bord du panneau (x~1 px de la case) et a droite sur l'espace entre W et X (empiete d'~4 px sur le bord de X). Hauteur OK (entier en bas). A 76 px il tient tres probablement (60 < 76) mais NON VU.
JA (1280x720) : "焼き入れ戟" (2 lignes) + "近日公開" ENTIER, en bas, dans la case, pas de debordement.

Cases sans etat (A Hallebardier/Z/E, Paysan, Voie A/B) : alignees comme avant (une ligne centree sous le badge, deux lignes pour Voie A du palier 1), aucune bougee ; pas de capture "avant" pour comparer pixel a pixel (juge a l'oeil et au code : ligne d'etat vide ne reserve rien).
Cases a nom d'une ligne : "Paysan" (sc_apres, sc_forum) et Zone Z/E de la Caserne : meme place.

## Defaut commun aux 3 langues (a corriger)
Le badge de touche "W" CHEVAUCHE la premiere ligne du nom de la case a etat : "WCloche" (FR 1920), "Wallebarde" (FR 1280), "Tempered" sous le W (EN), "焼き入れ" sous le W (JA). La bande haute de 12 px ne suffit pas pour le badge ; le nom de 2 lignes + Bientot remplit toute la case sous le badge. Une case a nom court sans etat n'a pas ce chevauchement.

## VERDICT par langue
- FR : PASS VU A L'ECRAN pour "Bientot" (entier, en bas, 76 px confirme par sc_forum) ; reserve : chevauchement badge W / premiere ligne du nom.
- EN : FAIL sur la capture tech_soon (1280x720, case 51 px) : "Coming soon" deborde de la case ; non vu a 76 px.
- JA : PASS VU A L'ECRAN (近日公開 entier) ; meme reserve de chevauchement du badge W.

## A changer (je n'ai rien touche)
1. tech_soon.txt : ajouter `r.SetRes 1920x1080w` avant la capture (cases de 76 px reelles) + `mapSize Small` dans tech_soon.json (preflight FAIL) ; ajouter, ou ecrire en variantes tech_soon_en / tech_soon_ja, `hol.UI.Language en|ja` pour que les 3 langues passent par le scenario.
2. Code : bande haute de la zone du nom (NameTopBand = 12) a augmenter ou badge W a ne pas recouvrir ; verifier la largeur de "Coming soon" a 76 px (sinon police plus petite ou coupure a deux lignes).
3. tech_soon.look dit "Bientot entier visible" : a rejouer en 1920x1080 avant de le mettre `VU A L'ECRAN`.
