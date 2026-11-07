# [À RÉPONDRE] Q-disconnect-left : le texte « Bob a quitté la partie » (défaite par déconnexion)

Contexte : D722 veut un texte dédié pour la défaite `Disconnected`, pas « vaincu ». L'événement de forum (`FHOLForumEvent`) ne porte pas la raison de défaite. Le code (Glorfindel-10, branche net-playtest) a pris un NOUVEAU TYPE `EHOLForumEventKind::Left`, envoyé par `DefeatSlot` quand la raison est `Disconnected`. C'est une décision de design à confirmer (je n'ai rien tranché : c'est le défaut codé).
Le tableau des joueurs garde le nom avec une pastille (« Connexion perdue » puis « IA ») : sans objet pour cette question.

Options
- A (défaut codé) : un type d'événement `Left` distinct. Les lecteurs d'événements (messages du forum, journal, statistiques « Defeated ») voient deux types : « vaincu » et « a quitté la partie ». Conséquence visible : la statistique « Defeated » de la chronologie ne compte pas le départ, à moins d'ajouter `Left` à la ligne de la chronologie.
- B : garder `Defeated` et ajouter un champ `Reason` à l'événement. Un seul type, le texte choisi par la raison ; les statistiques restent identiques (le départ est une défaite). Coût : un champ répliqué de plus et la sérialisation de l'événement.
- C : aucun texte dédié, « Bob est vaincu » pour tous (retour de D722, le plus simple, le joueur ne sait pas que l'adversaire a quitté).
- Autre : (à préciser)

Recommandation : B si les statistiques doivent traiter le départ comme une défaite (une seule ligne par siège, D508) ; A sinon. A est celui qui est codé et testé.

Valeur par défaut codée : A (type `Left`), derrière `HOLForumMessages` (une ligne à changer pour revenir à `Defeated`).

Reponse: B (utilisateur 10:36)
