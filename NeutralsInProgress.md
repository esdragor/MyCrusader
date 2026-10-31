# NeutralsInProgress.md : unités neutres et monstres, camps neutres (GDD § 8.4, D32, D102, D315, D401)

Ouvert le 2026-10-31 (journée autonome, Arwen, branche `ai-ia1`). Règle D483 : le neuf en Angelscript (règles pures, tests), le C++ pour la simulation (unités neutres, état répliqué, XP du héros). Le chapitre voisin **Centres secondaires et relais neutres** (D384/D388) est mené par Elrond dans `RelaysInProgress.md` (worktree `HeroesOfLiberty-Elrond`, branche `tiers`, `Script/Relays/HOLRelayRules.as`) : **un relais est un lieu à tenir sans ennemi, un camp est un groupe d'unités à tuer**. Ici on ne refait rien de la prise par présence ; on s'en sert seulement pour le vocabulaire (adversaire, gel, rayon de zone).

## Ce que le GDD tranche déjà (donc pas de question)

- **D32** (§ 8.4) : présence légère et ciblée. **Quelques camps par carte, placés symétriquement**, qui **gardent quelque chose de précieux** : un gisement riche ou un passage stratégique. **Pas de réapparition** : un camp nettoyé l'est pour la partie. **XP modérée**, comptée dans la part « Exploration + Événements » (~15 %, D05). Pas de chasse aux monstres façon Warcraft 3 : combattre un neutre est une décision ponctuelle, pas une voie de progression.
- **D102** : l'équipement du héros est supprimé, donc plus de reliques uniques de carte en mode standard : la récompense d'un camp est **un gisement ou un passage** (reliques réservées au mode Reliques).
- **D401** : Neutre (posture diplomatique) = adversaire qui ne tire pas d'office ; seul le tir automatique change. Cela concerne la posture entre joueurs (D369), pas les monstres. Dans le code, un propriétaire hors des emplacements du lancement est « seul dans son camp » (`FHOLCampTable`, `HOLVictoryRules.h` : « any other owner (team 0, a closed slot, an owner beyond the slots) is alone »), donc hostile à tous : la voie est déjà ouverte pour des unités neutres.
- **D315** : l'interaction propre aux Enfants du Dragon avec les camps est **mise de côté** (rien de propre, *Le Cri qui chasse* ou *L'Antre du wyrm*, à reprendre avec les camps). Cercle de l'Ombre (diversion) et Légions Noires (cadavres) : points d'appui seulement, rien n'est tranché.
- **§ 12.2, événement « Invasion de monstres »** : des vagues neutres qui attaquent tout le monde près d'un site « s'appuie sur les camps neutres » : une unité neutre agressive existera donc aussi hors des camps, plus tard (chapitre Événements, V2 du volcan).
- **« Après le prototype »** : D32 est marqué ainsi dans le GDD. Ce chapitre est ouvert par consigne de la journée autonome (chapitres du GDD non commencés), pas parce que le prototype l'exige.

## Relevé de l'existant (2026-10-31)

- **Aucun camp neutre, aucune créature, aucun propriétaire neutre n'existe.** `Tools/Data/UnitTypes.csv` ne connaît que LightInfantry, Archer, Peasant, Spearman, ManAtArms, King, Hero, Crossbowman ; aucune occurrence de Monster, Creature, Wolf, Beast dans `Source/`, `Script/`, `Tools/Data/`.
- La trace de « Camp » dans la simulation est trompeuse : `FHOLCampTable` (`Match/HOLVictoryRules.h`) est la **table des équipes** (alliés = un camp) ; `FindCampSource` / `CampSource` (`HOLUnitSubsystem.cpp`, `HOLBuildProcessor`) est le **camp minier** (bâtiment de dépôt). Rien à voir avec les camps de monstres. Les « Neutral » du code sont des commentaires de D401 (`HOLForumPresence.cpp:82`, `HOLForumRules.h:37`) : un propriétaire hors des emplacements compte comme adversaire d'un forum.
- Le combat (`HOLCombatProcessor`) et l'évitement (`HOLSimulationProcessors`) comparent déjà les camps par propriétaire (`AreHostile`) : une unité d'un propriétaire neutre combattrait n'importe quel joueur sans changement, à condition que le propriétaire neutre soit représentable (octet de propriétaire) et que la vue (brouillard, `HOLVisionSubsystem`) sache le voir.
- Les ressources vivent dans `Source/` côté gisements (`SourceTypes.csv`) : un « gisement riche gardé » est un gisement ordinaire placé près du camp par la recette de carte ; **la récompense « gisement » n'a donc pas de code propre** (le camp bloque l'accès tant qu'il vit) ; **la récompense « passage »** est un camp posé sur un chemin étroit (rien à coder non plus : les unités du camp bloquent le passage).
- L'XP du héros : `Simulation/HOLHeroXPRules.h`, source `Exploration` (« exploration et territoire », D06 : compte pendant la mort du héros). Une XP de camp nettoyé s'y ajoutera (famille « Exploration + Événements »).
- Pas de recette de carte pour les camps : le placement vient avec le chantier cartes (`MapsInProgress.md`) et la décision V4-Q1 du volcan (recette Python, données, acteurs à la main) ; **la même décision vaut pour les camps** (symétrie contrôlée, D32).

## Plan (étapes de 15 min)

- **N0 relevé** : FAIT (ce fichier).
- **N1 règles pures en script** : `Script/Neutrals/HOLCampRules.as` + `HOLCampRules_Test.as`. État d'un camp (garde, combat, retour, nettoyé), déclenchement (à vue ou quand on l'attaque), laisse, retour, nettoyage définitif (pas de réapparition), partage de la récompense (dernier coup, plus de dégâts, au prorata). Test rouge d'abord. **Les deux choix de design (NQ1, NQ2) sont des réglages**, pas des décisions.
- **N2 données** : où les cartes posent leurs camps (type de camp, position, composition, ce qu'il garde) ; symétrie par construction ; attend V4-Q1 et le chantier cartes.
- **N3 simulation (C++)** : propriétaire neutre (un numéro hors des emplacements), apparition des unités du camp à la création de la partie, état du camp par pas (doublon C++ de `HOLCamp::Step` avec test d'accord, comme les autres règles pures), état répliqué (public, D203 style).
- **N4 IA des neutres** : ordres des membres du camp (attaquer l'intrus, revenir au poste) ; tir automatique des neutres selon la posture (D369/D401) ; compatibilité avec le brouillard (un neutre est vu comme un ennemi).
- **N5 récompenses** : XP au héros du preneur (source Exploration), accès au gisement ou au passage, journal de partie.
- **N6 IA du joueur** : l'IA évalue un camp (rapport de force `HOLAIArmy`, D553) avant d'envoyer une vague (chapitre IA, après IA4).
- **N7 affichage et vérification** : indicateur de camp (vivant / nettoyé), scénario d'essai, `quick_check`, essai de l'utilisateur.

## Questions (réponses dans `Auto_Journey_Retours.md`, section 2)

- **NQ1** : un camp neutre attaque-t-il ceux qui passent près de lui (**à vue**), ou seulement ceux qui l'attaquent (**quand on l'attaque**) ? Réglage `Aggro` des règles (les deux sont codés).
- **NQ2** : à qui va l'XP d'un camp nettoyé : au joueur qui porte **le dernier coup**, à celui qui a fait **le plus de dégâts**, ou **partagée au prorata des dégâts** ? Réglage `Credit` des règles (les trois sont codés).
- Valeurs provisoires codées (rayons de déclenchement et de laisse, XP) : réglables en test, pas des questions.

## Suivi

- 2026-10-31 (Arwen, `ai-ia1`) : **chapitre ouvert, N0 fait** (relevé ci-dessus).
- 2026-10-31 (Arwen, `ai-ia1`) : **N1 faite.** `Script/Neutrals/HOLCampRules.as` + `HOLCampRules_Test.as` (7 tests, rouge d'abord : 6 sur 7 rouges avec des règles vides, 592/598, puis 598/598). Règles : état `Guarding` / `Fighting` / `Returning` / `Cleared` (`HOLCamp::Step`) ; déclenchement à vue (rayon inclus) ou par un coup reçu d'un intrus à portée de laisse ; laisse (un membre qui poursuit au-delà du rayon de laisse est renvoyé, plus aucun intrus à portée = tout le camp rentre) ; retour au poste (tous à moins du rayon d'arrivée) ; **nettoyé pour toujours** (plus de membre = `Cleared`, `bJustCleared` une seule fois, des membres listés après ne le ressuscitent pas, D32) ; récompense par règle de crédit (`HOLCamp::Rewards` : dernier coup, plus de dégâts avec le plus petit propriétaire en cas d'égalité, partage au prorata avec les points perdus à l'arrondi au plus gros ; la somme fait toujours `RewardXP`). NQ1 (`Aggro`) et NQ2 (`Credit`) sont des réglages, les deux branches sont testées. Valeurs provisoires (`FHOLCampSettings` : déclenchement 1500 cm, laisse 3000 cm, arrivée 400 cm, 100 XP). **Limites** : une règle seule, rien dans le jeu (pas de propriétaire neutre, pas d'unités de camp, pas de carte : N2-N3) ; la guérison des membres au retour n'est pas décidée (la règle ne soigne pas) ; les équipes ne sont pas connues (un partage au prorata se fait par propriétaire, pas par camp d'alliés) ; un attaquant à distance hors de la laisse n'est pas riposté (voulu : `bReachable`).
