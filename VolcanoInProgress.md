# VolcanoInProgress : le volcan et l'option Catastrophe (GDD § 12, D205, D208, D377 à D380, D398 à D400, D427)

Ouvert le 2026-10-30 (Arwen, branche `ai-ia1`, test autonome de la veille de l'Auto_Journey). Ce fichier est le journal du chantier ; `EtapeDefini.md` n'en garde que les titres.

## Ce qui est décidé (rien à retrancher, voir GDD § 12.1 et § 12.2 bis)

- Cycle d'un site : endormi -> fenêtre de déclenchement -> alerte 60 à 90 s (tracés dessinés au sol) -> éruption (la lave avance le long d'un tracé à la vitesse d'un fantassin lent, détruit unités, bâtiments, héros hors duel) -> refroidissement 2 à 3 min (infranchissable) -> basalte praticable + gisement neuf -> repos 6 à 8 min -> nouvelle fenêtre (D400).
- Sans l'option Catastrophe, jamais d'éruption (D398) ; volcan endormi = gisements riches, pas d'autel (D399). Avec Catastrophe : 3 à 4 éruptions par partie, fenêtres plus précoces, préavis inchangé (D208).
- Direction tirée parmi plusieurs tracés du site, jamais choisie ; tirage à équilibrer entre les côtés (D379) ; autel tenu ~60 s comme la ruine (D348) réveille plus tôt, alerte inchangée (D380).
- IA opportuniste (D427) : évacue dès l'alerte, s'installe sur les pentes quand la fenêtre est loin, court au gisement neuf, conteste l'autel.

## Répartition du code (D483 : le neuf en Angelscript, sauf Mass / simulation / réseau / rendu)

| Pièce | Langage | Pourquoi |
|---|---|---|
| Machine d'états du site (`FHOLVolcanoRules`, pure, fonction du temps et d'une graine) ; avance de la lave ; dégâts ; obstacle de lave ; autel (réutilise la règle de la ruine) | C++ (`Simulation/`) | simulation déterministe, doit tourner à l'identique chez tous (réseau) |
| Tracés au sol, fumée, lave (rendu), grille des cases bloquées | C++ (`Presentation/`) | rendu |
| Texte d'alerte, bandeau « éruption dans N s », ligne du rapport HUD | Angelscript (`Script/UI`) | neuf, interface |
| Comportement de l'IA (évacuer, pentes, gisement neuf, autel) | Angelscript (`Script/AI`) | neuf, IA existante en script |
| Tests de la machine d'états, de la lave, de l'autel | C++ (`Tests/HOLVolcanoTests.cpp`) | le code testé est en C++ ; les tests IA en `*_Test.as` |

## Plan en étapes de 15 min (une compilation, un passage de test par étape)

- **V0** Ouvrir le chantier (ce fichier, relevé des chapitres, questions). FAIT 2026-10-30.
- **V1** Machine d'états pure du site : `Dormant / Armed / Warning / Erupting / Cooling / Basalt+Rest`, fonction `Advance(Now)`. **Premier test rouge** (`HOLVolcanoTests`, cas « standard » et « catastrophe ») : (a) sans Catastrophe, sur 60 min simulées, l'état reste `Dormant` (D398) ; (b) avec Catastrophe, l'état passe à `Warning` dans la fenêtre, puis à `Erupting` entre 60 et 90 s plus tard (D28) ; (c) après l'éruption : `Cooling` 120-180 s puis `Rest` puis `Armed` à nouveau (D400). Valeurs lues dans une table de réglages (pas dans le test).
- **V2** Lave sur un tracé : position du front à l'instant t = distance parcourue le long d'une polyligne à la vitesse réglée ; test : le front atteint le bout au bon instant, ne recule pas.
- **V3** Tirage de la direction (graine du match, tirage équilibré entre les côtés : voir V-Q2 avant de coder) ; test : sur N éruptions, aucun côté visé deux fois de suite.
- **V4** Branchement dans la partie : sites lus dans la recette de carte (`MapsInProgress.md`), horloge de simulation, drapeau `bCatastrophe` de `FHOLMatchConfig` ; réplication de l'état du site.
- **V5** Dégâts : unités et bâtiments touchés par le front détruits, héros aussi sauf en duel (§ 9.8, § 10.2), gisements épargnés (D379).
- **V6** Obstacle : cases de lave infranchissables pendant le refroidissement, puis praticables (basalte) ; navigation et bâtiments.
- **V7** Gisement neuf après le refroidissement ; gisements riches des pentes (c'est du level design : sur la carte, pas ici).
- **V8** Autel ancien : tenu 60 s sans ennemi (règle de la ruine, D348) réveille le site plus tôt ; rallumage au réarmement.
- **V9** Rendu : tracés au sol dès l'alerte, fumée, lave qui avance, basalte ; sons d'alerte.
- **V10** Interface en script : bandeau et ligne du rapport HUD, minuteur par site.
- **V11** IA (script) : évacuation, pentes, gisement neuf, autel (D427) ; niveaux Facile / Difficile.
- **V12** Scénario `Tools/Scenarios/volcano.txt` (site, alerte, éruption, fuite), mesures de fin de chapitre, essai à l'utilisateur.

## Décisions manquantes (à trancher par l'utilisateur ; l'agent n'en tranche aucune)

Reprises dans `Auto_Journey.md`, « Questions restées sans réponse ».

- **V-Q1 Réglages de Catastrophe : fenêtres et durées.** Le GDD dit « plus précoces » et « 3 à 4 éruptions par partie » (D208) mais ne donne aucun chiffre ; la fenêtre standard (14e-18e minute) n'existe plus en standard (D398). A : première fenêtre entre la 6e et la 9e minute, repos 6-8 min (D400), 3 à 4 éruptions sur une partie de 35 min. B : première fenêtre entre la 8e et la 12e minute. Recommandation : A, à régler en test (table de réglages, pas de constante dans le code).
- **V-Q2 Équilibrer le tirage entre les côtés (D379).** A : tirage seedé avec mémoire, la direction qui vise le côté visé à l'éruption précédente est exclue tant qu'une autre existe. B : tirage libre, chaque tracé a la même chance, l'équité vient du nombre de tracés symétriques du site. Recommandation : A (aucune chance qu'un joueur soit visé deux fois de suite).
- **V-Q3 Qui voit les tracés avec le brouillard (D381) ?** A : les tracés sont visibles de tous dès l'alerte, même sous le brouillard (cohérent avec « visibles de tous », D377). B : seulement les cases vues ou explorées. Recommandation : A (sinon l'alerte ne prévient pas, contre le § 12.2).
- **V-Q4 Vitesse de la lave.** « Fantassin lent, indicatif ». A : celle de l'Homme d'armes moins 30 % (~490 cm/s, le Roi et l'Homme d'armes marchent à 697 cm/s, D563). B : celle du Paysan. Recommandation : A, réglable dans la table.

## Suivi

- 2026-10-30 16:45 : V0 faite (relevé des chapitres dans `Auto_Journey.md`, ce journal, ligne dans `EtapeDefini.md`). Rien de codé : V1 attend le temps restant de la séance de test (arrêt dur 17:00).
- 2026-10-30 16:50 : **V1 faite**. Test rouge d'abord (stub toujours Dormant : CatastropheCycle rouge, StandardStaysDormant vert par construction), puis implémentation (Simulation/HOLVolcanoRules.h/.cpp, Tests/HOLVolcanoTests.cpp), build 24 s, quick_check -Parts tests 486/486 (484 + 2), commit b8bfb9e sur ai-ia1 (pas dans main). Valeurs de FHOLVolcanoSettings = option A de V-Q1 (provisoire). Suivant V2 (lave sur un tracé).

- 2026-10-31 : **réponses de l'utilisateur** (Auto_Journey_Retours.md § 2 ; GDD D573-D575) : **V-Q2 = B** (tirage libre, équité par tracés symétriques, contre la recommandation ; donc V3 n'a pas de mémoire à coder : tirage uniforme parmi les tracés du site, la symétrie est du level design) ; **V-Q3 = A** (tracés visibles de tous dès l'alerte, même sous le brouillard) ; **V-Q4 = A** (lave à Homme d'armes -30 %, ~490 cm/s : 697 x 0,7 = 488) ; **V-Q1 = A TRANCHER** (l'utilisateur veut en discuter ; ne rien écrire comme décidé, les valeurs de l'option A de `FHOLVolcanoSettings` restent provisoires). V2 (lave sur un tracé) est débloquée.
- 2026-10-31 : **V2 faite** (Arwen, `ai-ia1`, commit ae476b5, pas dans `main`). Test rouge d'abord (stubs, `Volcano.FlowFront` et `EruptionFollowsTheFlow` rouges ; `LavaSpeed` vert par construction), puis vert, tests **493/493**. `HOLVolcano::FFlow` (polyligne au sol, Z ignoré) : `Length`, `EruptionDuration(Flow, Speed)`, `FrontDistance(Flow, Now, EruptAt, Speed)` (0 avant l'éruption, vitesse x temps, bornée à la fin, jamais en arrière), `PointAt(Flow, Distance)` ; `MakeCycle` et `GetPhase` prennent la durée de l'éruption du tracé tirée (sinon le placeholder de 90 s de V1). **Vitesse (D575)** : `FHOLVolcanoSettings::LavaSpeed(vitesseHommeDArmes)` = vitesse lue dans la table x `LavaSpeedFactor` 0,7 (488 cm/s avec 697) ; le test lit la vraie table. Suivant V3 : tirage de la direction **libre** (V-Q2 = B : pas de mémoire, chaque tracé du site a la même chance, tirage seedé par éruption comme les dates du cycle ; l'équité est du level design, tracés symétriques dans la recette de carte).

- 2026-10-30 19:10 (réponse de l'utilisateur, V-Q1 -> D580) : **A**, valeurs provisoires (première fenêtre 6e-9e min, repos 6-8 min, 3-4 éruptions) ; « on verra lors de vraies parties si c'est trop ou pas assez ». Valeurs de table, déjà en place (FHOLVolcanoSettings).

- 2026-10-30 20:16 : **V3 faite** (Arwen, ai-ia1, commit d7e78d0, pas dans main). Test rouge d'abord (stub INDEX_NONE : Volcano.DrawFlow rouge, 530/531), puis vert, 531/531. HOLVolcano::DrawFlow(Seed, Index, FlowCount) : tirage uniforme et libre (V-Q2 = B, D573), sans memoire, flux de hasard a part (les dates du cycle ne bougent pas), INDEX_NONE sans trace. Piege trouve : FRandomStream est lineaire, des graines voisines (cycle 0, cycle 1) tiraient pareil (2381 repetitions sur 3000 paires) ; graine passee par MurmurFinalize32/HashCombineFast. Le test mesure la repartition (850-1150 sur 4000) et le taux de repetition (15-35 %). Suivant V4 (branchement dans la partie : recette de carte, horloge, bCatastrophe, replication) : touche la carte et le reseau, a faire quand les cartes (Haldir) sont dans main.

- **V4a FAITE (2026-10-31, Arwen ai-ia1)** : l'etat complet d'un site a un instant, fonction pure des options, de la graine du match, du site et du temps, donc **rien a repliquer que l'horloge** (et la graine) : `HOLVolcano::FSite` (les tracés ; le cratere = premier point), `FSiteState` (phase, cycle, tracé tiré, dates, compte a rebours de l'alerte, distance et point du front), `SiteSeed(MatchSeed, SiteIndex)` (hache : deux sites d'un match n'eruptent jamais pareil), `SiteStateAt(...)` : chaque cycle tire son tracé (`DrawFlow`), dure le temps de sa lave (`EruptionDuration` a la vitesse `LavaSpeed` de l'Homme d'armes lu dans la table), cycles enchaines comme `GetPhase`. Rouge d'abord (stub : 579/580), vert 580/580 : test `Volcano.SiteState` sur 12 graines, 3 tracés (4000, 7000, 9000 cm), vitesse lue de la vraie table. Sans Catastrophe : dormant une heure. **Reste de V4 (b : sites lus dans la carte, c : graine et horloge dans la partie) : questions V4-Q1 et V4-Q2 posees dans Auto_Journey_Retours.md § 2** ; rien n'est branche dans la partie.
- 2026-11-01 (Auto_Journey, agent de docs B) : **V4-Q1 = A et V4-Q2 = C, réponses de l'utilisateur** (enregistrées au GDD, D607 et D608). **V4-Q1 = A** : le site du volcan (cratère et tracés de lave) est écrit dans la recette de carte (script Python de `Tools/Editor/maps/`), tracés symétriques par construction, contrôle automatique d'équité (D573). **V4-Q2 = C** : la graine est tirée par le salon et peut être imposée pour les essais et scénarios (champ `seed` de la config du match, publiée dans l'état du match avec l'heure de début). Reste de V4 : V4b (sites lus dans la carte) et V4c (graine et horloge dans la partie), lignes `[ ]` en tête de la section Arwen.
