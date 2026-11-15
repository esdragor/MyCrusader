# [À RÉPONDRE] Q-chat-ui : écrire et lire la discussion en partie (A4, D385)

Contexte : le transport du chat est fait (le serveur seul choisit qui lit un message Alliés / Tous / privé, D385, D376, D792 ; commande de développement `hol.Chat.Say`). Reste la saisie (le GDD dit « Entrée pour écrire, un raccourci pour basculer ») et l'apparence de la ligne dans le fil (D446) : ni la touche de bascule ni la marque du canal ne sont tranchées.

Question : comment choisit-on le canal en écrivant, et comment la ligne montre-t-elle le canal ?

- **A (recommandée)** : Entrée ouvre un champ sur le canal **Alliés** (Tous en chacun pour soi sans allié) ; **Tab** bascule Alliés → Tous → (privé au joueur choisi dans le panneau Diplomatie, D387) ; la ligne reçue dit « Bob : (Alliés) texte », « Bob : (Privé) texte », « Bob : texte » pour Tous. Conséquence visible : le canal se lit en toutes lettres, la ligne est un peu plus longue.
- **B** : Entrée = Alliés, **Maj + Entrée** = Tous (pas de bascule dans le champ) ; même marque « (Alliés) » dans la ligne. Conséquence : plus rapide, mais deux touches à apprendre, et le privé ne passe que par le panneau Diplomatie.
- **C** : comme A, mais le canal se montre par la **couleur** de la ligne (Alliés vert, privé violet, Tous blanc) sans mot. Conséquence : ligne courte, mais lecture moins sûre (daltonisme, couleurs des joueurs déjà dans les noms).
- **Autre** : à préciser.

Valeur par défaut codée : la marque de A/B dans la ligne (« (Alliés) », « (Privé) », rien pour Tous), dans `HOLForumMessages::GetChatMessage` (une ligne à changer). La saisie (touche, champ) n'est pas codée tant que la réponse n'est pas là.

Chantier : `ChatInProgress.md` (worktree HeroesOfLiberty-Glorfindel, branche net-playtest), GLORFINDEL-1115e, 2026-11-15.
