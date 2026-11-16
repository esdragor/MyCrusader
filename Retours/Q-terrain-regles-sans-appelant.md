# [À RÉPONDRE] Q-terrain-regles-sans-appelant : que faire des règles de forêt, de gué et d'arrêt de charge sans appelant

Contexte : `Script\Terrain\HOLTerrainRules.as` garde `ForestSpeedPercent` (F4), `FordDamageTaken` (F5) et `StopsCharge` (charge arrêtée en marais), testées seules, mais aucun code du jeu (Script ni Source) ne les appelle : D810 (branchement) n'est pas fait. La vitesse marais/route est, elle, la règle C++ seule (D836), appelée par le pas Mass.

Options :
- A : brancher en D810 (le mouvement et le combat lisent les règles ; en jeu : cavalerie 80 % et engins 70 % en forêt, 120 % de dégâts reçus sur un gué, charge stoppée en marais). Ce branchement touche le pas Mass (C++) et le combat : une tâche à part, avec build.
- B : retirer les trois règles et leurs 4 tests (le GDD 8.1 / D33 resterait sans effet en jeu : forêt et gué sans conséquence sur les combats ; la règle se réécrirait au moment voulu).

Recommandation : A (D810 est déjà tranché Q1 A + Q2 A : valeurs et sens décidés, il ne manque que le branchement ; retirer perdrait du code testé, rouge vu).

Défaut codé : rien changé (les règles et leurs tests restent, sans appelant).