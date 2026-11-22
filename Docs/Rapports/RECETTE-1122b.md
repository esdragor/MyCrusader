# RECETTE-1122b (recette independante, integ-50 = 7053828, worktree HeroesOfLiberty-Haldir)

Debut 17:25 (Get-Date), fin ~17:31. Aucun code, scenario, build ni commit. `git status` initial : seuls Docs\Integrate_integ-49/50 .log/.txt non suivis (conforme).

## Resultats (un preflight chacun, une seule fois)

| essai | preflight | duree | FAIL | WARN | erreurs Angelscript | requires WorldContext | PNG ouverts | verdict |
|---|---|---|---|---|---|---|---|---|
| income_tooltip | PASS | 43 s (48 s total) | 0 | 2 (HUD overlap, Sanity absents : preexistants) | 0 | 0 | 5/5 | PASS VU A L'ECRAN (geste Mouse.Hover interne) |
| order_lines_blocks | PASS | 28 s | 0 | 2 (idem) | 0 | 0 | 1/1 | PASS VU A L'ECRAN (geste interne) |
| chat_input | PASS | 34 s | 0 | 0 | 0 | 0 | 4/4 | PASS VU A L'ECRAN |
| secondary_centre | PASS | 40 s | 0 | 0 | 0 | 0 | 5/5 | PASS VU A L'ECRAN |

Comptes d'erreurs : grep `Angelscript.*rror` et `requires WorldContext` sur Saved_Tests\Logs\scenario_<nom>.log. Le bruit « engine smoke tests: 20 Condition failed » (UnifiedErrorTests, -culture=fr) est signale par l'outil comme hors scenario.

## Points vus (PNG ouverts avec Read, 1066x600 pour income_tooltip et order_lines_blocks, 1920x1080 pour les autres)

income_tooltip (modifie aujourd'hui) :
- income_tip_fr : « +377 bois/min » sous le montant Bois, en haut a gauche. VU.
- income_tip_gold_fr : « +0 or/min » ABAISSE sous le bandeau Scenario (y ~66-88), sans le recouvrir (le bandeau est a y 40-65). VU. Corrige le reste cosmetique note par 1121sd (l'infobulle or recouvrait le bandeau).
- income_tip_en : « +377 wood/min ». VU. Libelles EN du haut d'ecran traduits (Food/Wood/Stone/Gold, Idle: 0, You / AI Normal 2).
- income_tip_ja : « +377 木材/分 » (et non /min). VU. Haut d'ecran JA traduit (待機中 : 0).
- income_tip_gold_nobanner : sans bandeau, « +0 or/min » juste sous la barre (y ~30-48), place naturelle, pas abaisse. VU.
- Douteux : l'infobulle est decalee de ~35 a 60 px vers la droite par rapport au montant survole (centree vers x 406 pour un Or a x ~370 ; x 239 pour un Bois a x ~178). Lisible, associee a la bonne case, mais pas centree dessous. Probablement la position de la souris du scenario ; non tranche.
- Douteux : +377 bois/min pour 5 Paysans est une valeur elevee (« trajets compris ») ; identique a 1121sb/1121sd (373-377), donc pas une regression de ce jour. Non verifie contre le GDD.
- Geste : hol.Mouse.Hover (interne) ; son non entendu : pour le geste, LOGIQUE SEULEMENT ; pour le rendu, VU.

order_lines_blocks : 12 unites en 2 blocs (Tous 12 / Homme d'arm. 6 / Lancier 6 dans le panneau), DEUX lignes jaunes, une par bloc (gauche ~x400, droite ~x660), convergeant vers le point d'ordre central (~x532,y307). PAS de ligne unique du milieu. VU. Geste interne (hol.Select.OrderGround).

chat_input :
- chat_input_typing : champ « (Allies) : » vide avec curseur, au-dessus de la barre du bas a gauche, Diplomatie ouvert a droite avec la ligne Joueur 2. VU.
- chat_input_shown : champ ferme, fil en haut a droite : une seule ligne « Vous : (Allies) salut les allies ». VU.
- chat_input_target : champ « Joueur 2 : » (nom du destinataire) apres Tab Tab, ligne Joueur 2 surlignee dans Diplomatie, fil avec « (Prive) salut » deja present. VU, MAIS le .look attend la marque « (Prive) » dans le champ : le champ montre le NOM. Conforme au design (D807 : « (Prive) » seulement sur la ligne recue, voir la fiche, recette 1121sa) ; le .look est a corriger, pas le jeu.
- chat_input_typed : texte « salut les allies » DANS la boite, marque (Allies). VU. Geste reel (frappe hol.UI.Type + Entree).

secondary_centre :
- sc_grille_age2 : age II, case Z « Centre secondaire » avec badge Z allumee. VU.
- sc_grille_infobulle : infobulle « Centre secondaire », 150 bois, 400 pierre, +10 population, Depot toutes ressources, Produit : Paysan. VU.
- sc_pendant : chantier (fondation, 4 tourelles), 5 Paysans au pied, « 5 en construction », Bois 5050 / Pierre 4800, Population 5/10. VU.
- sc_apres : centre fini seul selectionne (Centre secondaire, Vous, 2400/2400), grille A Paysan, Population 5/20, « Batiment acheve ». VU (un arbre au premier plan masque le bas du batiment mais l'objet reste lisible).
- sc_forum : Centre principal, Vies 3, grille A Paysan + D/F Voie A/B du palier 1 + W Cloche. VU.
- Reserve cosmetique : en W, texte « Cloche d'alarme amelioree / Bientot » coupe en bas de case (« Bientot » tronque).

## Fiches mises a jour (une ligne `recette:` ajoutee, rien d'autre touche)
E-income_tooltip.md (ligne 9), E-order_lines_blocks.md (8), E-chat_input.md (8), E-secondary_centre.md (9), via edit_line.ps1 (-Append -After, relu par l'outil).

## Non fait / non verifie
- Aucun essai non joue (les 4 demandes sont faites). Aucun son (-nosound). Geste reel pour income_tooltip, order_lines_blocks, secondary_centre : non (commandes hol.*).
- Planches de contact : non utilisees, PNG ouverts individuellement.
- Pas de check manuel de la valeur +377 bois/min contre le GDD.
- Non lance : stray_processes.ps1 (aucun editeur laisse par moi : chaque preflight a rendu la main ; non verifie par liste de processus).

## Appels
Environ 20 appels d'outils (limite 25), ~6 min.
