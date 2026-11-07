[À RÉPONDRE] Q-audio-buildings : un son par type de bâtiment, pour quels moments ?

Contexte : tu as voulu muets, le 2026-11-06, le son générique de sélection d'un bâtiment (`Select.Building`) et celui de fin de construction (`Alert.BuildingDone`), et tu as eu l'idée d'un son propre à chaque type de bâtiment. Il y a 14 types (Centre principal, Maison, Camp de bûcherons, Camp de mineurs, Moulin, Ferme, Caserne, Champ de tir, Écurie, Atelier de siège, Chapelle, Sanctuaire, Forge, Marché). Quatre moments possibles : sélection (S), construction finie (C), production finie (P : unité formée, technologie, commerce), destruction (D). Détail dans `HOLAudioBuildings` et `AudioBuildingsInProgress.md` (worktree Arwen, branche `heal-sleep`).

Rien ne change à ton oreille tant que tu ne réponds pas : tout reste muet comme aujourd'hui, sauf la fin d'entraînement et le commerce qui sonnent déjà.

Options :

- **A. Sélection seulement, un son par famille de bâtiments** (recommandée). Tu cliques un bâtiment et tu entends un bref son de son matériau : bois (Maison, camps, Moulin, Ferme), pierre (Centre principal, Caserne, Champ de tir, Écurie, Atelier de siège), sacré (Chapelle, Sanctuaire), métal (Forge), pièces (Marché) : environ 5 sons. Construction finie, production et destruction restent muettes comme tu l'as voulu. Peu de sons à choisir (méthode des dix échantillons par famille), facile à étendre ensuite à un son par type.
- **B. Sélection par type + construction finie par famille.** 14 sons de sélection, plus un son de « bâtiment terminé » par famille (environ 5). Destruction toujours muette. Tu entends chaque bâtiment se distinguer, et tu sais quand l'ouvrier a fini ; mais tu rouvres ce que tu avais mis muet (`Alert.BuildingDone`), donc à confirmer.
- **C. Tout par type : S, C, P, D pour les 14 types** (une cinquantaine de sons). Le plus riche, mais il rouvre trois sons que tu avais coupés, ajoute une destruction qui n'existe pas encore (événement à créer côté jeu, C++), et fait beaucoup de choix d'oreille.
- **D. Autre** : dis lesquels (par exemple « seulement le Forum et la Forge », « seulement la destruction »).

Recommandation : **A**. C'est ton idée de départ (la sélection), sans rouvrir un son que tu as coupé, et la plus petite à valider ; on étend après écoute.

Valeur par défaut codée : **tout muet comme aujourd'hui** (sélection et construction générique muettes, fin d'entraînement = `Alert.TrainingDone`, commerce = `Alert.Trade`, destruction sans événement). Une ligne par bâtiment à changer dans `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Script\Audio\HOLAudioBuildings.as` (fonction `Build`).

Réponse :
