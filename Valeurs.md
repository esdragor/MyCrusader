# Fiche de valeurs du prototype

Générée par `Tools/Data/make_value_sheet.py` depuis `Tools/Data/ValueSheet.csv` (le registre) : **les valeurs sont lues dans les tables et le code, jamais recopiées**. Ne pas éditer ce fichier : changer la valeur (ou le registre) puis relancer `python Tools/Data/make_value_sheet.py`. Réglage fait à l'essai (`REGLAGE:`) : `python Tools/Data/make_value_sheet.py --accept <id> --note "..."`. Les statistiques des unités sont dans `UnitStats.md` (outil voisin).

États : *pas codé* (le GDD la nomme, aucun code), *provisoire* (codée, jamais jouée), *réglé à l'essai* (changée après un essai), *validé* (l'utilisateur l'a confirmée).

**196 réglages** : 12 pas codé, 183 provisoire, 0 réglé à l'essai, 1 validé.

## Question 1 : Le héros est-il important sans être obligatoire ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `hero.aura.radius` | Aura de l'Aube : rayon (cm) | 1200 | provisoire | D546 | HOLHeroAbilityRules.h |  |
| `hero.aura.reduction` | Aura de l'Aube : dégâts reçus en moins (%) | 15 | provisoire | D546 | HOLHeroAbilityRules.h |  |
| `hero.banner.duration` | Bannière de l'Aube : durée (ms) | 45000 | provisoire | D548 | HOLHeroAbilityRules.h |  |
| `hero.banner.range` | Bannière de l'Aube : portée de pose (cm) | 1600 | provisoire | D548 | HOLHeroAbilityRules.h |  |
| `hero.cry.duration` | Serrez les rangs : durée (ms) | 15000 | provisoire | D547 | HOLHeroAbilityRules.h |  |
| `hero.cry.reduction` | Serrez les rangs : dégâts reçus en moins (%) | 25 | provisoire | D547 | HOLHeroAbilityRules.h |  |
| `hero.dawnstep.speed` | Pas de l'aube : vitesse (+%) | 50 | provisoire | D546 | HOLHeroAbilityRules.h |  |
| `hero.morale.cap` | Moral : plafond de réduction (%) | 50 | provisoire | D74 | HOLHeroAbilityRules.h |  |
| `hero.stat.percent_per_level` | Statistiques du héros par niveau (+%) | 8 | provisoire | D544 | HOLHeroXPRules.cpp |  |
| `hero.troop_damage_factor` | Dégâts du héros contre les troupes (facteur) | 0.3 | provisoire | D17, D345 | HOLCombatRules.h |  |
| `hero.unit.attack` | Héros : attaque (niveau 1) | 30 | provisoire | D534 | UnitTypes.csv Hero:Attack |  |
| `hero.unit.health` | Héros : points de vie (niveau 1) | 450 | provisoire | D534 | UnitTypes.csv Hero:MaxHealth |  |
| `hero.unit.speed` | Héros : vitesse (cm/s) | 697 | provisoire | D534, D563 | UnitTypes.csv Hero:Speed |  |
| `std.unit.attack` | Homme d'armes (unité standard) : attaque | 12 | provisoire | D345 | UnitTypes.csv ManAtArms:Attack | Référence des « unités standard » de la lecture du rapport |
| `std.unit.health` | Homme d'armes (unité standard) : points de vie | 155 | provisoire | D345 | UnitTypes.csv ManAtArms:MaxHealth |  |

## Question 2 : La mort est-elle suffisamment punitive ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `hero.return.base_delay` | Retour du héros : délai de base (s) | 30 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.base_price` | Retour : prix de base (or) | 100 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.delay_per_level` | Retour : délai par niveau (s) | 8 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.price_drop` | Retour : le prix tombe à 0 en (s) | 60 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.price_per_level` | Retour : prix par niveau (or) | 50 | provisoire | D539, D25 | HOLHeroReturn.as |  |

## Question 3 : Le délai de résurrection ouvre-t-il une fenêtre intéressante ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `hero.return.base_delay` | Retour du héros : délai de base (s) | 30 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.base_price` | Retour : prix de base (or) | 100 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.delay_per_level` | Retour : délai par niveau (s) | 8 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.price_drop` | Retour : le prix tombe à 0 en (s) | 60 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.price_per_level` | Retour : prix par niveau (or) | 50 | provisoire | D539, D25 | HOLHeroReturn.as |  |
| `hero.return.time_percent` | Échelle des délais de retour (hol.Hero.ReturnTimePercent, %) | 100 | provisoire | D539 | HOLHeroLibrary.cpp | Réglage d'essai, 100 = délais du GDD |

## Question 4 : Le duel est-il une vraie décision ou une obligation ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `duel.ArenaRadiusCm` | Rayon de l'arène (cm) | 600 | provisoire | § 10.2 | HOLDuelRules.as |  |
| `duel.ChallengeCooldownMs` | Délai entre deux défis (ms) | 150000 | provisoire | D16, D80 | HOLDuelRules.as |  |
| `duel.ForumZoneRadiusCm` | Zone sans duel autour du forum (cm) | 2500 | provisoire | D403, D592 | HOLDuelRules.as |  |
| `duel.HesitationMs` | Hésitation de celui qui refuse (ms) | 25000 | provisoire | D16 | HOLDuelRules.as |  |
| `duel.MaxDurationMs` | Durée maximale d'un duel (ms) | 90000 | provisoire | D18 | HOLDuelRules.as |  |
| `duel.ResponseMs` | Fenêtre de réponse à un défi (ms) | 6500 | provisoire | D80 | HOLDuelRules.as |  |
| `duel.TriumphMs` | Triomphe / Démoralisé après un duel (ms) | 40000 | provisoire | D35 | HOLDuelRules.as |  |
| `duel.WindowUrgentMs` | Fenêtre de défi : clignote dans ses derniers (ms) | 2000 | provisoire | D117 | HOLDuelRules.as |  |
| `duel.kit.BalancedParryWindowMs` | Posture équilibrée : fenêtre de parade (ms) | 450 | provisoire | D14-D18 | HOLDuelKit.as |  |
| `duel.kit.DefensiveDamagePercent` | Posture défensive : dégâts (%) | 75 | provisoire | D14-D18 | HOLDuelKit.as |  |
| `duel.kit.DefensiveParryWindowMs` | Posture défensive : fenêtre de parade (ms) | 600 | provisoire | D14-D18 | HOLDuelKit.as |  |
| `duel.kit.OffensiveDamagePercent` | Posture offensive : dégâts (%) | 125 | provisoire | D14-D18 | HOLDuelKit.as |  |
| `duel.kit.OffensiveParryWindowMs` | Posture offensive : fenêtre de parade (ms) | 300 | provisoire | D14-D18 | HOLDuelKit.as |  |
| `duel.kit.ParryCooldownMs` | Parade : recharge (ms) | 4000 | provisoire | D14-D18 | HOLDuelKit.as |  |
| `duel.kit.ParryReducePercent` | Parade : dégâts réduits de (%) | 70 | provisoire | D14-D18 | HOLDuelKit.as |  |
| `ai.duel.BaseAcceptPercent` | IA : chance de base d'accepter un défi (%) | 60 | provisoire | D421-D427 | HOLAIDuel.as |  |
| `ai.duel.ChallengeAcceptPercent` | IA : seuil pour lancer un défi (%) | 65 | provisoire | D421-D427 | HOLAIDuel.as |  |
| `ai.duel.MinAcceptPercent` | IA : chance minimale d'accepter (%) | 35 | provisoire | D421-D427 | HOLAIDuel.as |  |
| `ai.duel.ProtectionFloorPercent` | IA : plancher de protection (%) | 30 | provisoire | D421-D427 | HOLAIDuel.as |  |

## Question 5 : La progression marche-t-elle sans système d'âges ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `hero.stat.percent_per_level` | Statistiques du héros par niveau (+%) | 8 | provisoire | D544 | HOLHeroXPRules.cpp |  |
| `hero.xp.command_radius` | Rayon de l'XP de commandement (cm) | 1200 | provisoire | D543 | HOLHeroXPRules.h |  |
| `hero.xp.kill_hero` | XP d'un héros tué | 300 | provisoire | D543 | HOLHeroXPRules.h |  |
| `hero.xp.levels` | XP totale pour atteindre les niveaux 1 à 6 | 0, 200, 600, 1300, 2200, 3300 | provisoire | D543 | HOLHeroXPRules.cpp | Courbe d'XP, reste à régler (Q04) |
| `hero.xp.max_level` | Niveaux du prototype | 6 | provisoire | D04, D543 | HOLHeroXPRules.h | Q04 : valeurs indicatives, à régler en test |
| `hero.xp.weight.combat` | Poids de l'XP Combat (%) | 100 | provisoire | D543, D05 | HOLHeroXPRules.cpp | Le code a 4 sources, D05 en nomme 4 autres (Territoire, Événements sans porteur) |
| `hero.xp.weight.command` | Poids de l'XP Commandement (%) | 75 | provisoire | D543, D05 | HOLHeroXPRules.cpp | Le code a 4 sources, D05 en nomme 4 autres (Territoire, Événements sans porteur) |
| `hero.xp.weight.development` | Poids de l'XP Développement (%) | 50 | provisoire | D543, D05 | HOLHeroXPRules.cpp | Le code a 4 sources, D05 en nomme 4 autres (Territoire, Événements sans porteur) |
| `hero.xp.weight.exploration` | Poids de l'XP Exploration (%) | 25 | provisoire | D543, D05 | HOLHeroXPRules.cpp | Le code a 4 sources, D05 en nomme 4 autres (Territoire, Événements sans porteur) |
| `camp.reward_xp` | Camp neutre vaincu : XP | 100 | provisoire | D32 | HOLCampRules.as |  |
| `gdd.tier3` | Tronc commun du palier 3 : grille d'options (valeurs « à régler en test ») | — | pas codé | D179, D180 | GDD | Chantier Palier 3 : Palier3InProgress.md |
| `relay.centre_xp_per_minute` | Centre secondaire tenu : XP par minute | 10 | provisoire | D587 | HOLRelayRules.as |  |
| `relay.decay_steps` | Relais : recul du compte (pas par pas) | 2 | provisoire | D388, V3-Q3 | HOLRelayRules.as |  |
| `relay.hold_seconds` | Relais : secondes à tenir seul | 60 | provisoire | D388, D348 | HOLRelayRules.as |  |
| `relay.sim.hold_seconds` | Relais (simulation C++) : secondes à tenir | 60 | provisoire | D388 | HOLRelayRules.h | Doit rester égal au script (le test de script compare) |
| `relay.xp_per_minute` | Relais tenu : XP par minute | 10 | provisoire | D587 | HOLRelayRules.as | Décidé (RQ1 = A) ; reste à voir en partie |
| `relay.zone_radius` | Relais : rayon de la zone (cm) | 1200 | provisoire | D388 | HOLRelayLayout.h |  |

## Question 6 : L'économie offre-t-elle assez de décisions ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `buildings.table` | Table des bâtiments | 14 lignes, 21 colonnes | provisoire | D260, D345 | BuildingTypes.csv * |  |
| `ai.opening.QueueLength` | IA : file de Paysans en attente | 2 | provisoire | D527-D531 | HOLAIOpening.as |  |
| `ai.opening.WeightFood` | IA : poids de la nourriture (ouverture) | 3 | provisoire | D527-D531 | HOLAIOpening.as |  |
| `ai.opening.WeightWood` | IA : poids du bois (ouverture) | 2 | provisoire | D527-D531 | HOLAIOpening.as |  |
| `ai.tech.GiveUpSeconds` | IA technologies : abandonne après (s) | 150 | provisoire | D597-D617 | HOLAITech.as |  |
| `ai.tech.MinPeasants` | IA technologies : Paysans avant de chercher | 20 | provisoire | D597-D617 | HOLAITech.as |  |
| `ai.tech.SaveSeconds` | IA technologies : économise pendant (s) | 45 | provisoire | D597-D617 | HOLAITech.as |  |
| `eco.peasant_target` | IA : Paysans visés par le Forum | 80 | provisoire | D527-D531 | HOLAIOpening.as |  |
| `tech.table` | Table des technologies (coûts, durées, pourcentages) | 26 lignes, 13 colonnes | provisoire | D597-D617 | Technologies.csv * |  |
| `eco.barracks.buildtime` | Caserne : temps de construction (s) | 20 | provisoire | D283 | BuildingTypes.csv Barracks:BuildTime |  |
| `eco.barracks.cost` | Caserne : coût | (Food=0,Wood=150,Stone=0,Gold=0) | provisoire | D283 | BuildingTypes.csv Barracks:Cost |  |
| `eco.berry.amount` | Buisson de baies : nourriture | 300 | provisoire | D297 | SourceTypes.csv BerryBush:Amount |  |
| `eco.farm.cost` | Ferme : coût | (Food=0,Wood=75,Stone=0,Gold=0) | provisoire | D527 | BuildingTypes.csv Farm:Cost |  |
| `eco.gold.amount` | Veine d'or : or par veine | 3000 | provisoire | D297 | SourceTypes.csv GoldVein:Amount |  |
| `eco.house.cost` | Maison : coût | (Food=0,Wood=50,Stone=0,Gold=0) | provisoire | D283 | BuildingTypes.csv House:Cost |  |
| `eco.house.pop` | Maison : population ajoutée | 10 | provisoire | D283 | BuildingTypes.csv House:PopulationProvided |  |
| `eco.mainfc.health` | Forum principal : points de vie | 2400 | provisoire | D345 | BuildingTypes.csv MainCentre:MaxHealth |  |
| `eco.market.fee` | Marché : taxe (%) | 30 | provisoire | D70 | Market.csv Standard:FeePercent |  |
| `eco.market.maxprice` | Marché : prix maximal | 300 | provisoire | D70 | Market.csv Standard:MaxPrice |  |
| `eco.market.minprice` | Marché : prix minimal | 20 | provisoire | D70 | Market.csv Standard:MinPrice |  |
| `eco.market.recover` | Marché : retour des prix toutes les (s) | 10 | provisoire | D70 | Market.csv Standard:RecoverEverySeconds |  |
| `eco.market.step` | Marché : pas de prix | 3 | provisoire | D70 | Market.csv Standard:Step |  |
| `eco.market.tax_cpp` | Tribut (C++) : taxe (%) | 30 | provisoire | D372 | HOLMarketRules.h | Doit rester égale au script |
| `eco.mill.cost` | Moulin : coût | (Food=0,Wood=100,Stone=0,Gold=0) | provisoire | D527 | BuildingTypes.csv Mill:Cost |  |
| `eco.peasant.buildtime` | Paysan : temps de production (s) | 20 | provisoire | D296 | UnitTypes.csv Peasant:BuildTime |  |
| `eco.peasant.carry` | Paysan : capacité de transport | 10 | provisoire | D296 | UnitTypes.csv Peasant:CarryCapacity |  |
| `eco.peasant.cost` | Paysan : coût | (Food=50,Wood=0,Stone=0,Gold=0) | provisoire | D296 | UnitTypes.csv Peasant:Cost |  |
| `eco.peasant.gather` | Paysan : vitesse de récolte | (Food=0.7,Wood=0.7,Stone=0.7,Gold=0.7) | provisoire | D296 | UnitTypes.csv Peasant:GatherRates |  |
| `eco.sanctuary.farm_percent` | Sanctuaire : nourriture de la ferme en plus (%) | 15 | provisoire | D602 | HOLGatherRules.h |  |
| `eco.sanctuary.radius` | Sanctuaire : rayon du bonus de ferme (cm) | 3000.0 | provisoire | D602 | HOLGatherRules.h |  |
| `eco.start.peasants` | Paysans de départ | 5 | provisoire | D528 | HOLMatchSettings.h |  |
| `eco.start.resources` | Stock de départ de chaque joueur (nourriture, bois, pierre, or) | 200, 200, 200, 200 | provisoire | D283 | HOLMatchSettings.h |  |
| `eco.tree.amount` | Arbre : bois par arbre | 100 | provisoire | D297 | SourceTypes.csv Tree:Amount |  |
| `eco.tribute.tax` | Tribut : taxe (%) | 30 | provisoire | D372 | HOLTributeRules.as |  |
| `market.table` | Table du Marché | 1 lignes, 11 colonnes | provisoire | D70 | Market.csv * |  |
| `sources.table` | Table des sources (arbres, veines, buissons) | 11 lignes, 12 colonnes | provisoire | D297 | SourceTypes.csv * |  |

## Question 7 : Les combats d'armée sont-ils intéressants sans le héros ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `ai.army.AllyHelpMinRatio` | IA : rapport de force pour aider un équipier | 1 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AssemblyRadius` | IA : rayon du point de rassemblement (cm) | 1000 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackCooldown` | IA : pause entre deux vagues (s) | 90 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackLaunchRatio` | IA : rapport de force pour lancer une vague | 1.5 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackLossShare` | IA : part de pertes qui fait reculer la vague | 0.6 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackMinSeconds` | IA : pas de vague avant (s, ennemis vus) | 420 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackMinSecondsUnseen` | IA : pas de vague avant (s, aucun ennemi vu) | 300 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackMinSoldiers` | IA : soldats minimum pour une vague | 8 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackRecallCount` | IA : ennemis près de la base qui rappellent la vague | 3 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.AttackRetreatRatio` | IA : rapport de force sous lequel la vague recule | 0.6 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.DefenceRadius` | IA : rayon de défense de la base (cm) | 3000 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.army.RegroupEvery` | IA : rappel d'un soldat au rassemblement (s) | 10 | provisoire | D552-D556, D571 | HOLAIArmy.as |  |
| `ai.cavalry.EasyMax` | IA cavalerie : Facile, maximum | 2 | provisoire | D672 | HOLAICavalry.as |  |
| `ai.cavalry.HardMax` | IA cavalerie : Difficile, maximum | 6 | provisoire | D672 | HOLAICavalry.as |  |
| `ai.cavalry.MinSoldiers` | IA et cavalerie : soldats avant l'Écurie | 6 | provisoire | D672 | HOLAICavalry.as |  |
| `ai.cavalry.NormalMax` | IA cavalerie : Normal, maximum | 4 | provisoire | D672 | HOLAICavalry.as |  |
| `ai.lives.FeintAroundSeconds` | IA Normal : contournement de la feinte (s) | 10 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.FeintBackSeconds` | IA Normal : recul de la feinte (s) | 6 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.HardCrushRatio` | IA Difficile : rapport qui écrase le camp adverse | 1.5 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.HardMaxLoss` | IA Difficile : perte maximale acceptée | 0.5 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.NormalLoseRatio` | IA Normal : rapport de force qui déclenche la feinte | 0.3 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.siege.MaxMangonels` | IA siège : Mangonneaux au plus | 1 | provisoire | S6, D614 | HOLAISiege.as |  |
| `ai.siege.MaxRams` | IA siège : Béliers au plus | 2 | provisoire | S6, D614 | HOLAISiege.as |  |
| `ai.siege.MinSoldiers` | IA et siège : soldats avant l'Atelier de siège | 8 | provisoire | S6, D614 | HOLAISiege.as |  |
| `morale.attenuation_cap` | Moral : atténuation maximale cumulée (%) | 75 | provisoire | D404, D706 | HOLMoraleRules.as | Jamais 100 % : seule la Lumière sacrée retire (D404) |
| `morale.bonus_cap_other` | Moral : plafond du bonus sur les dégâts infligés et la cadence (%) | 30 | provisoire | D704 | HOLMoraleRules.as | À régler en test |
| `morale.bonus_cap_taken` | Moral : plafond du bonus sur les dégâts subis (%) | 50 | provisoire | D704 | HOLMoraleRules.as | À régler en test |
| `morale.malus_cap_other` | Moral : plafond du malus sur les dégâts infligés et la cadence (%) | 30 | provisoire | D704 | HOLMoraleRules.as | À régler en test |
| `morale.malus_cap_taken` | Moral : plafond du malus sur les dégâts subis (%) | 50 | provisoire | D704 | HOLMoraleRules.as | Symétrique du bonus ; décidé par l'utilisateur le 2026-11-05 |
| `units.table` | Table des unités (voir UnitStats.md) | 16 lignes, 57 colonnes | provisoire | D227, D345 | UnitTypes.csv * | Détail dans UnitStats.md (make_unit_stats.py) |
| `eco.mainfc.health` | Forum principal : points de vie | 2400 | provisoire | D345 | BuildingTypes.csv MainCentre:MaxHealth |  |

## Question 8 : Le volcan est-il une opportunité plutôt qu'une frustration ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `ai.altar.PauseAfterRecallSeconds` | IA et autel : pause après rappel (s) | 30 | provisoire | D639 | HOLAIAltar.as |  |
| `ai.altar.ReissueSeconds` | IA et autel : ordres renouvelés toutes les (s) | 5 | provisoire | D639 | HOLAIAltar.as |  |
| `volcano.AltarDecayRate` | Autel : recul du compte (s par s) | 2 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.AltarHoldSeconds` | Autel : durée à tenir (s) | 60 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.CoolingMax` | Refroidissement, maximum (s) | 180 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.CoolingMin` | Refroidissement, minimum (s) | 120 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.EruptionLength` | Éruption : durée (s) | 90 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.FreshDepositBigShare` | Veine neuve : part de grande veine | 0.5 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.FreshDepositGoldShare` | Veine neuve : part d'or | 0.5 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.LavaHalfWidth` | Demi-largeur de la coulée (cm) | 800 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.LavaSpeedFactor` | Vitesse de la lave (facteur) | 0.7 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.RestMax` | Repos entre éruptions, maximum (s) | 480 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.RestMin` | Repos entre éruptions, minimum (s) | 360 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.WarningMax` | Annonce avant l'éruption, maximum (s) | 90 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.WarningMin` | Annonce avant l'éruption, minimum (s) | 60 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.WindowLength` | Fenêtre d'éruption : durée (s) | 180 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.WindowOpensAfter` | Fenêtre d'éruption : s'ouvre après (s) | 360 | provisoire | D205, D208, D379 | HOLVolcanoRules.h |  |
| `volcano.ai.AiAltarHoldShare` | IA et autel : part de l'armée qui tient l'autel | 0.25 | provisoire | D427, D639, D640 | HOLVolcanoRules.h |  |
| `volcano.ai.AiAltarHoldShareEasy` | IA et autel : part en Facile | 0.125 | provisoire | D427, D639, D640 | HOLVolcanoRules.h |  |
| `volcano.ai.AiAltarMargin` | IA et autel : marge | 1.5 | provisoire | D427, D639, D640 | HOLVolcanoRules.h |  |
| `volcano.ai.AiAltarReserveShare` | IA et autel : part de réserve | 0.4 | provisoire | D427, D639, D640 | HOLVolcanoRules.h |  |
| `volcano.ai.AiAltarSafeDistance` | IA et autel : distance de sûreté (cm) | 4000 | provisoire | D427, D639, D640 | HOLVolcanoRules.h |  |
| `volcano.ai.AiAltarWorkerWeight` | IA et autel : poids d'un ouvrier | 0.25 | provisoire | D427, D639, D640 | HOLVolcanoRules.h |  |
| `volcano.ai.distance_mode` | IA et autel : où mesure-t-elle les 40 m (cvar) | 0 | provisoire | D640 | HOLVolcanoLibrary.cpp | 0 = ses bâtiments (défaut) |

## Question 9 : Le snowball est-il puissant sans être irréversible ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `ai.lives.FeintAroundSeconds` | IA Normal : contournement de la feinte (s) | 10 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.FeintBackSeconds` | IA Normal : recul de la feinte (s) | 6 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.HardCrushRatio` | IA Difficile : rapport qui écrase le camp adverse | 1.5 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.HardMaxLoss` | IA Difficile : perte maximale acceptée | 0.5 | provisoire | D579, D329 | HOLAILives.as |  |
| `ai.lives.NormalLoseRatio` | IA Normal : rapport de force qui déclenche la feinte | 0.3 | provisoire | D579, D329 | HOLAILives.as |  |

## Question 10 : Chaque faction a-t-elle une façon différente de jouer ?

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `gdd.aube` | Aube : pouvoirs, capacités, grilles de talents (valeurs « à régler en test ») | — | pas codé | D85, D36, D111, D195 | GDD | Chantier Aube : AubeInProgress.md (D685-D690) |
| `gdd.ombre` | Cercle de l'Ombre : ordres, contrôle, talents (valeurs « à régler en test ») | — | pas codé | D165, D167, D199 | GDD | Après le prototype |
| `gdd.dragon` | Enfants du Dragon : Nids, Aura draconique, talents (valeurs « à régler en test ») | — | pas codé | D157, D198 | GDD | Après le prototype |
| `gdd.heritiers` | Héritiers du Feu : Ascendant, Jugement, palier 3 (valeurs « à régler en test ») | — | pas codé | D170, D172, D177, D200, D337 | GDD | Après le prototype |
| `gdd.legions` | Légions Noires : économie, ultime, talents, serviteurs (valeurs « à régler en test ») | — | pas codé | D53, D55, D66, D116, D121, D196 | GDD | Chantier Légions : LegionsInProgress.md (L0) |
| `gdd.templiers` | Templiers : croisade, commanderies, talents (valeurs « à régler en test ») | — | pas codé | D96, D133, D136, D139, D147, D187, D197 | GDD | Après le prototype |

## Autres valeurs, par chantier

### Brouillard

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `gdd.fog_grace` | Brouillard : délai de grâce d'environ 0,5 s | — | pas codé | D526 | GDD | Chantier brouillard : FogInProgress.md |
| `vision.building_sight` | Rayon de vue d'un bâtiment (forum principal, cases) | 6400 | provisoire | D523 | BuildingTypes.csv MainCentre:SightRadius |  |
| `vision.forest_trees` | Vue : arbres qui font une case de forêt | 2 | provisoire | D383, D523 | HOLVisionRules.h |  |
| `vision.height_bonus_per_cm` | Vue : bonus de hauteur (cm de vue par cm) | 1 | provisoire | D383, D523 | HOLVisionRules.h |  |
| `vision.max_height_bonus` | Vue : bonus de hauteur maximal (cm) | 800 | provisoire | D383, D523 | HOLVisionRules.h |  |

### Caméra

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `camera.min_height` | Caméra : hauteur minimale (cm) | 1500 | provisoire | D254 | HOLRTSCamera.h |  |
| `camera.pitch` | Caméra : inclinaison (degrés) | 55 | provisoire | D254 | HOLRTSCamera.h |  |

### Cartes

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `biome.decor.density_first` | Biomes : densité de décor de la première case (par hectare) | 60.0 | provisoire | D564-D575 | HOLBiomeApplier.as | Première des 8 cases de ReferenceDensities (60 30 20 12 8 6 4 3) : milieu de ce que demande le décor cuit des cartes |
| `biome.detail_scale` | Biomes : échelle du détail de sol (cm) | 900 | provisoire | D564-D575 | HOLBiomeRules.as |  |
| `biome.detail_strength` | Biomes : force du détail de sol (%) | 70 | provisoire | D564-D575 | HOLBiomeRules.as |  |
| `maps.table` | Cartes et leur budget de ressources | 8 lignes, 4 colonnes | provisoire | D498, D564 | Maps.csv * |  |
| `mapsizes.table` | Tailles de carte | 5 lignes, 4 colonnes | provisoire | D490-D496 | MapSizes.csv * |  |

### Cercle de l'Ombre

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `gdd.convert` | Conversion de la Voix : cibles et durées | — | pas codé | D63 | GDD |  |

### Communication

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `comm.signal.lifetime_ms` | Signaux : durée sur la carte (ms) | 5000 | provisoire |  | HOLSignalRules.as | Q-signals Q1 (pas dans le GDD) |
| `comm.signal.max_per_window` | Signaux : nombre par joueur dans la fenêtre | 3 | provisoire | D386 | HOLSignalRules.as | indicatif : 3 toutes les 5 s |
| `comm.signal.window_ms` | Signaux : fenêtre de la limite (ms) | 5000 | provisoire | D386 | HOLSignalRules.as | indicatif : 3 toutes les 5 s |

### Diplomatie

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `diplomacy.default_stance` | Diplomatie : posture au lancement | Enemy | provisoire | D369-D376, DQ-1 | HOLDiplomacyRules.h | Ennemie, tous contre tous |

### GDD

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `gdd.review_notes` | Notes de reprise de la review du GDD (renvoient à cette fiche) | — | pas codé | D130, D210, D428-D433 | GDD | Les lignes du § 22 « à régler en test » annoncent cette fiche |

### IA

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `ai.hold.base_radius` | IA : rayon de la zone tenue autour du centre (cm) | 3000 | provisoire |  | HOLAIHold.as | Chiffre provisoire ; Retours\Q-hold-scope.md |

### Légions

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `unit.rage.cooldown_ms` | Rage nécrotique : recharge (ms) | 40000 | provisoire | Q-legions-l4 Q2 = A | HOLUnitAbilityRules.h | Copie de UnitAbilities.csv |
| `unit.rage.damage_percent` | Rage nécrotique : dégâts en plus (%) | 40 | provisoire | Q-legions-l4 Q2 = A | HOLUnitAbilityRules.h | Copie de Effects.csv (test Combat.DamnedWarrior.RageConstants) |
| `unit.rage.duration_ms` | Rage nécrotique : durée (ms) | 10000 | provisoire | Q-legions-l4 Q2 = A | HOLUnitAbilityRules.h | Copie de UnitAbilities.csv |
| `unit.rage.hp_per_hit` | Rage nécrotique : PV perdus par coup porté | 3 | provisoire | Q-legions-l4 Q2 = A | HOLUnitAbilityRules.h | Copie de UnitAbilities.csv |
| `unit.rage.min_hp` | Rage nécrotique : refusée sous (PV) | 30 | provisoire | Q-legions-l4 Q2 = A | HOLUnitAbilityRules.h | Copie de UnitAbilities.csv |
| `unit.rage.rate_percent` | Rage nécrotique : cadence en plus (%) | 25 | provisoire | Q-legions-l4 Q2 = A | HOLUnitAbilityRules.h | Copie de Effects.csv |
| `unit.rage.stop_hp` | Rage nécrotique : s'arrête à (PV) | 20 | provisoire | Q-legions-l4 Q2 = A | HOLUnitAbilityRules.h | Copie de UnitAbilities.csv |

### Neutres

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `camp.aggro_radius` | Camp neutre : rayon d'agression (cm) | 1500 | provisoire | D32 | HOLCampRules.as |  |
| `camp.leash_radius` | Camp neutre : rayon de laisse (cm) | 3000 | provisoire | D32 | HOLCampRules.as |  |
| `camps.table` | Table des camps neutres | 1 lignes, 3 colonnes | provisoire | D32 | Camps.csv * |  |

### Performance

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `gdd.stress` | Seuils des stress tests (niveau exigeant, 144 i/s, 2 000 unités) | — | pas codé | D218, D236 | GDD | Seuils de performance : Docs/Mesures_*.md |

### Présentation

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `anim.walk_start` | Animation : seuil de départ en marche (part de la vitesse) | 0.15 | provisoire | D234 | HOLUnitAnimation.h |  |
| `anim.walk_stop` | Animation : seuil d'arrêt (part de la vitesse) | 0.10 | provisoire | D234 | HOLUnitAnimation.h |  |

### Sites sacrés

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `site.decay_steps` | Sites sacrés : recul du compte par pas | 2 | provisoire | D345, D699 | HOLSiteRules.as | Récession du relais (2 par pas), à régler en test |
| `site.gold_per_minute` | Sites sacrés : or par minute et par site tenu | 30 | provisoire | D699 | HOLSiteRules.h | Petit revenu passif par défaut ; Retours\Q-sites-or-par-minute.md ; cvar hol.Site.GoldPerMinute |
| `site.hold_seconds` | Sites sacrés : secondes à tenir seul | 60 | provisoire | D345, D699 | HOLSiteRules.as | Valeur du relais, à régler en test (D345 : l'utilisateur choisit) |
| `site.radius` | Sites sacrés : rayon de la zone par défaut (cm) | 1200 | provisoire | D429 | HOLSiteLayout.h | Celui du relais et de la ruine (12 m) ; la carte peut donner le sien (Tools/Editor/maps/common.py SITE_RADIUS) |
| `site.sim.decay_steps` | Sites sacrés (simulation C++) : recul du compte par pas | 2 | provisoire | D345, D699 | HOLSiteRules.h | Doit rester égal au script (le test de script compare) |
| `site.sim.hold_seconds` | Sites sacrés (simulation C++) : secondes à tenir | 60 | provisoire | D345, D699 | HOLSiteRules.h | Doit rester égal au script (le test de script compare) ; cvar hol.Site.HoldSeconds |
| `site.victory_seconds` | Sites sacrés : secondes à tout tenir pour gagner | 600 | validé | D699, D747 | HOLSiteRules.h | Choisi par l'utilisateur le 2026-11-08 ; le script (HOLSiteVictory.as) a la même valeur |

### Talents

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `hero.talents.values` | Talents : valeurs des grilles (toutes factions) | — | pas codé | D110, D111, D116, D147, D157, D167, D177, D194-D200 | GDD | Talents.csv n'a pas de valeurs numériques ; les grilles des 6 factions donnent des valeurs indicatives |
| `talents.table` | Table des talents (sans valeurs numériques) | 24 lignes, 7 colonnes | provisoire | D105-D110 | Talents.csv * | Effets en texte, sans valeurs numériques : les valeurs des grilles ne sont pas codées |

### Économie

| Id | Réglage | Valeur | État | Décision | Où | Note |
|---|---|---|---|---|---|---|
| `eco.secondary_centre.cost_stone` | Centre secondaire : coût en pierre | 300 | provisoire | D384 | HOLSecondaryCentreRules.as | Q-secondary-centers Q1 |
| `eco.secondary_centre.cost_wood` | Centre secondaire : coût en bois | 300 | provisoire | D384 | HOLSecondaryCentreRules.as | Q-secondary-centers Q1 |
| `eco.secondary_centre.population` | Centre secondaire : base de population | 10 | provisoire | D384 | HOLSecondaryCentreRules.as | Q-secondary-centers Q2 |

## Fichiers marqués « provisoire » sans valeur à régler

| Id | Fichier | Pourquoi |
|---|---|---|
| `note.Vision` | Source/HeroesOfLiberty/Map/HOLVisionRules.h | Valeurs de vue lues par les lignes vision.* |
| `note.AIAltar` | Script/AI/HOLAIAltar.as | Valeurs lues par les lignes ai.altar.* et volcano.ai.* |
| `note.AIBrain` | Script/AI/HOLAIBrain.as | Commentaires des étapes de l'IA : valeurs dans HOLAIArmy / Siege / Cavalry / Tech |
| `note.AITalent` | Script/Talents/HOLAITalent.as | Tirage du talent de l'IA, provisoire (TQ1) : règle, pas une valeur |
| `note.DiplomacyGameState` | Source/HeroesOfLiberty/Match/HOLGameState.h | Commentaire : renvoie à diplomacy.default_stance |
| `note.DiplomacyMatchConfig` | Source/HeroesOfLiberty/Match/HOLMatchConfig.h | Nom provisoire du mode Diplomatie, pas une valeur |
| `note.DiplomacyRulesAs` | Script/Diplomacy/HOLDiplomacyRules.as | Posture au lancement : voir diplomacy.default_stance |
| `note.ForumMessages` | Source/HeroesOfLiberty/Match/HOLForumMessages.h | Son d'alerte provisoire, présentation |
| `note.ForumRules` | Source/HeroesOfLiberty/Match/HOLForumRules.cpp | Neutre sans icône (provisoire), présentation |
| `note.GameScreen` | Source/HeroesOfLibertyUI/HUD/HOLGameScreen.cpp | Son d'alerte provisoire, présentation |
| `note.GameScreenScript` | Script/UI/HOLGameScreenScript.as | Textes du bloc du héros non traduits, présentation |
| `note.HOLAudioPlayer.as` | Script/Audio/HOLAudioPlayer.as | Sons choisis par nom de fichier sans écoute : l'utilisateur les règle à l'oreille (AudioInProgress.md) |
| `note.HOLBiomeDecorPlan.as` | Script/Biomes/HOLBiomeDecorPlan.as | Table d'aspect du décor générique (forme moteur |
| `note.HOLGameNames.h` | Source/HeroesOfLibertyUI/HUD/HOLGameNames.h | Noms d'unités des Légions (DisplayName de FactionOverrides.csv) provisoires jusqu'à L13 : un libellé et non une valeur |
| `note.HOLSiteBanner.as` | Script/UI/HOLSiteBanner.as | Libellés de la bannière des sites : formulation provisoire que le GDD ne nomme pas (à confirmer avec l'utilisateur) |
| `note.HOLSiteRulesLibrary.h` | Source/HeroesOfLiberty/Simulation/HOLSiteRulesLibrary.h | Expose à l'Angelscript les valeurs de HOLSiteRules.h (site.sim.*) pour que le test de script les compare : aucune valeur propre |
| `note.HOLSiteSubsystem.cpp` | Source/HeroesOfLiberty/Simulation/HOLSiteSubsystem.cpp | Les cvar hol.Site.* reprennent les valeurs de HOLSiteRules.h (lignes site.sim.*) : rien à régler ici |
| `note.HOLSiteSubsystem.h` | Source/HeroesOfLiberty/Simulation/HOLSiteSubsystem.h | Liste des unités saintes (Moine de l'Aube et Moine lumineux) en attendant la colonne de la table des factions : une liste provisoire (D702 ; à remplacer au chantier Légions L13) et non un réglage chiffré |
| `note.MakeUnitStats` | Tools/Data/make_unit_stats.py | Texte de UnitStats.md : « valeurs provisoires » ; les valeurs sont dans UnitTypes.csv |
| `note.MatchSettings` | Source/HeroesOfLiberty/Match/HOLMatchSettings.h | Valeurs lues par eco.start.* |
| `note.RelayLibrary` | Source/HeroesOfLiberty/Simulation/HOLRelayLibrary.h | Commentaire : renvoie aux valeurs du relais (lignes relay.*) |
| `note.RelaySubsystem` | Source/HeroesOfLiberty/Simulation/HOLRelaySubsystem.cpp | Cvar : valeur par défaut = celle du relais (relay.sim.hold_seconds) |
| `note.ScriptScreen` | Source/HeroesOfLibertyUI/HUD/HOLScriptScreen.cpp | Touche provisoire du panneau de Diplomatie, reconfigurable |
| `note.TalentRules` | Script/Talents/HOLTalentRules.as | Famille conseillée en cas d'égalité (TQ1) : règle, pas une valeur |
| `note.TechTypes` | Source/HeroesOfLiberty/Technologies/HOLTechnologyTypes.h | Commentaire : les valeurs sont dans Technologies.csv (ligne tech.table) |
| `note.UIStyles` | Source/HeroesOfLibertyUI/Style/HOLUIStyles.h | Styles d'interface provisoires, pas une valeur de jeu |
| `note.UnitTypes` | Source/HeroesOfLiberty/Simulation/HOLUnitTypes.h | Commentaires : les valeurs sont dans UnitTypes.csv (ligne units.table) |
| `note.VolcanoBanner` | Script/UI/HOLVolcanoBanner.as | Libellés provisoires de la bannière du volcan |
| `vis.HOLBuilding.cpp` | Source/HeroesOfLiberty/Buildings/HOLBuilding.cpp | Forme provisoire d'un modèle (D261, D297), pas une valeur de jeu |
| `vis.HOLBuilding.h` | Source/HeroesOfLiberty/Buildings/HOLBuilding.h | Forme provisoire d'un modèle (D261, D297), pas une valeur de jeu |
| `vis.HOLBuildingTypes.h` | Source/HeroesOfLiberty/Buildings/HOLBuildingTypes.h | Forme provisoire d'un modèle (D261, D297), pas une valeur de jeu |
| `vis.HOLSourceField.cpp` | Source/HeroesOfLiberty/Resources/HOLSourceField.cpp | Forme provisoire d'un modèle (D261, D297), pas une valeur de jeu |
| `vis.HOLSourceField.h` | Source/HeroesOfLiberty/Resources/HOLSourceField.h | Forme provisoire d'un modèle (D261, D297), pas une valeur de jeu |
| `vis.SourceTypes` | Source/HeroesOfLiberty/Resources/HOLSourceTypes.h | Commentaire : les valeurs sont dans SourceTypes.csv (ligne sources.table) |

## Contrôles du jour

- Valeurs introuvables : 0
- Dérives (réglé / validé) : 0
- Fichiers marqués absents du registre : 0
- Lignes du GDD « à régler en test » sans réglage au registre : 0
- Lignes « pas codé » dont le fichier existe maintenant : 0
