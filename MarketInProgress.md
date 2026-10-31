# Marché in progress

Chantier **Marché** (GDD § 5.5, D70, D84, D372 ; § 13.5 pour le marché noir du Cercle, après le prototype).
Ouvert le 2026-10-30 par le test autonome (Galadriel, branche `galadriel-tab`). D483 : le neuf en Angelscript sauf Mass / simulation / réseau / rendu.

## Ce que dit le GDD (tranché)

- Bâtiment économique standard, **commun à toutes les factions**, au palier 1 (Essor) : § 11.1, D129.
- Achat et vente de **nourriture, bois, pierre contre de l'or** (D84). Un seul **cours par ressource, partagé par tous les joueurs** (alliés comme ennemis). Plus une ressource est vendue, moins elle rapporte ; plus elle est achetée, plus elle coûte ; les cours **reviennent lentement à l'équilibre**.
- Cercle de l'Ombre : meilleur taux (marché noir, D69, après le prototype) : la règle doit donc prendre le taux de l'échange en paramètre.
- Tributs (D372) : le Marché permet de donner des ressources à n'importe quel joueur, taxe ~30 % (à régler), annoncé à tous : c'est un autre étage (M5), pas l'échange.
- Bâtiment à protéger et à cibler ; l'IA doit savoir s'en servir (M6, après la commande).

## Pas tranché (voir Auto_Journey_Retours.md, section 2, « Marché »)

Prix de base, taxe (écart achat / vente), pas du cours par lot, bornes, vitesse de retour à l'équilibre. M1 les met dans **un seul réglage** (`FHOLMarketSettings`) avec deux jeux de valeurs (options A et B de MK-Q1), pour que la réponse ne coûte qu'un changement de table. **Tranché le 2026-10-30 : MK-Q1 = C** (prix de base 80 / 80 / 120 / 100, le reste comme A ; D582, voir Suivi).

## Relevé de l'existant (M0, 2026-10-30)

- **Aucun code Marché** : ni bâtiment, ni ligne de `Tools/Data/BuildingTypes.csv`, ni commande, ni règle (recherche `market|marché|trade|exchange` dans `Source/`, `Script/`, `Config/`, `Tools/` : seulement des `std::atomic::exchange`).
- Ressources : `EHOLResource` (Food, Wood, Stone, Gold) et `FHOLResources` (`Source/HeroesOfLiberty/Match/HOLResources.h`) ; le stock du joueur est dans le sous-système de match ; les règles pures déjà en script suivent le modèle de `Script/AI/HOLAIEconomy.as` + `HOLAIEconomy_Test.as` (fonctions pures d'un espace de noms, test `Test_*(FUnitTest& T)`, lancé par `quick_check -Parts tests`).
- Bâtiments : `FHOLBuildingTypeRow` (`Source/HeroesOfLiberty/Buildings/HOLBuildingTypes.h`), table `Tools/Data/BuildingTypes.csv` (8 types : MainCentre, House, LumberCamp, MiningCamp, Mill, Barracks, Farm, ArcheryRange) ; cases de la grille des Paysans `BuildGridSlot` 0-5 et 8 (0 A, 1 Z, 2 E, 3 R, 4 Q, 5 S, 8 W) ; **âge** (`Age` 0-3) : le Marché est d'âge 1 (palier 1, D569 : niveau 3 du héros) ; catégorie `Economic` (F2). Forme provisoire : `BlockAndTower` ; il n'y a pas encore de `Walkable`/`Source` à lui donner.
- Grille de commandes : `Source/HeroesOfLiberty/Player/HOLCommandGrid.{h,cpp}` ; un bâtiment qui n'a pas de production n'a pas de page ; la page du Marché (acheter / vendre 100, trois ressources) sera à y ajouter.
- Réseau : le stock est répliqué à son propriétaire ; un échange est une **commande client vers serveur** validée par le serveur (comme la pose d'un bâtiment) ; le **cours** est un état de partie commun, à répliquer à tous (M4).

## Plan (étapes de 15 min)

- **M0** relevé de l'existant : **fait** (ci-dessus).
- **M1** règle pure de l'échange en script (`Script/Economy/HOLMarketRules.as`) : taux, bornes, rendement, retour à l'équilibre, taux en paramètre (marché noir) ; test d'abord rouge puis vert (`Script/Economy/HOLMarketRules_Test.as`, `quick_check -Parts tests`).
- **M2** table CSV `Tools/Data/Market.csv` (prix de base, taxe, pas, bornes, retour) lue au démarrage, test C++ de cohérence, seulement si MK-Q1 est répondue (sinon les valeurs restent dans le script).
- **M3** type de bâtiment `Market` (ligne de `BuildingTypes.csv`, âge 1, case de la grille des Paysans, forme provisoire, vue) et test de la table ; refus de pose avant le palier 1 déjà fourni par D569.
- **M4** état du cours dans la partie (une instance par partie, serveur autoritaire, répliquée à tous), retour à l'équilibre au pas fixe, commande « échanger » client vers serveur (stock, Marché fini du joueur) ; test C++ de la chaîne commande -> stock -> cours.
- **M5** interface : page de commandes du Marché (acheter / vendre 100 de chaque ressource, Shift = 500), cours affichés, message de refus ; puis les tributs (D372) depuis le panneau Diplomatie.
- **M6** IA : vendre le surplus quand un besoin manque (se branche sur `HOLAINeeds.as`) ; scénario d'essai `market` ; GDD (§ 5.5, nouvelles décisions), clôture.

## Suivi

- 2026-10-30 20:30 : chantier ouvert, M0 faite (relevé), question MK-Q1 posée dans `Auto_Journey_Retours.md`.
- 2026-10-30 20:36 : **M1 faite**. `Script/Economy/HOLMarketRules.as` : `FHOLMarketSettings` (prix de base, pas, taxe, bornes, retour), `FHOLMarketBook` (le cours commun), `FHOLMarketTrade` ; `HOLMarket::Preset(0/1)` (options A / B de MK-Q1), `NewBook`, `Sell`, `Buy` (lot de 100, chaque lot au cours laissé par le précédent, taxe en paramètre pour le marché noir, raisons de refus `lots` / `resource` / `stock` / `gold`), `Recover` (vers la base sans la dépasser). Entiers seulement (accord serveur / clients). Test `Script/Economy/HOLMarketRules_Test.as` (11 cas) : **rouge d'abord** avec une coquille vide (530/541, 11 échecs) puis **vert** (541/541) ; après fusion de `main` (Talents + cartes) et recompilation : 554/554. Commit 7be2ba9 sur `galadriel-tab`, fusionné dans `main` (7293e52). Pas de test réseau ni de partie : M1 est une règle pure (la chaîne commande -> stock -> cours vient avec M4). Prochaine étape : M2 (table CSV, après MK-Q1) ou M3 (type de bâtiment `Market` : ligne de `BuildingTypes.csv`, âge 1, case de grille, test de table).
- 2026-10-30 21:4x (réponse de l'utilisateur, enregistrée par Auto_Journey, **rien de codé**) : **MK-Q1 = C** (contre la recommandation A) : prix de base différents, **nourriture 80, bois 80, pierre 120, or 100** ; le reste comme A : taxe 30 %, le cours bouge de 3 points par lot de 100, retour à l'équilibre d'1 point toutes les 10 s, bornes 20 à 300. Écrit au GDD sous **D582** et au § 5.5. Ligne `[ ]` en tête de la file de Galadriel : ajouter le jeu C à `HOLMarket::Preset` (test rouge d'abord : les prix de base et un aller-retour chiffré), puis M2 (table CSV `Market.csv`).

- 2026-10-31 07:5x (Auto_Journey, Galadriel) : **jeu C fait et M2 faite**. Jeu C ajouté à `HOLMarket::Preset` (option 2, `DefaultOption`, `DefaultSettings()`) : tout autre numéro donne C ; A = 0 et B = 1 restent. Tests rouge d'abord : `Test_MarketPresetCNumbers`, `Test_MarketDefaultIsC`, `Test_MarketRoundTripC` (80 vend 56, prix 77, rachat 101 ; pierre 120 vend 84, rachat à 117 coûte 153), `Test_MarketRecoverC` (3 lots, 71, 9 intervalles pour revenir à 80) : 576/580 rouge puis 580/580 vert ; commit 668b774. M2 : `Tools/Data/Market.csv` (ligne `Standard`), `FHOLMarketRow` (`Source/HeroesOfLiberty/Match/HOLMarketTypes.h`), `Tools/Editor/make_market.py` (non lancé, comme make_technologies), test `HeroesOfLiberty.Market.Table` (rouge 580/581 sans CSV, vert 581/581) ; commit 49fe15a sur `galadriel-tab`. Suivant : M3 (type de bâtiment `Market`).