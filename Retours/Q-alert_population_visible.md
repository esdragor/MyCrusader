# [À RÉPONDRE] Q-alert_population_visible : la ligne « Population pleine » est entrée au fil sans que tu la voies : faut-il la rendre plus visible ?

Contexte : pendant ton essai de 16:05, ton journal (HeroesOfLiberty-Galadriel, Saved\Logs\HeroesOfLiberty.log) montre `HUD messages: [Population pleine]` 6 s après ta touche A : la ligne est bien entrée au fil de messages (en haut à droite, sous « Vous 2 vies »), muette (D449), et elle n'y reste qu'environ 8 s (D446). Tu dis ne l'avoir jamais vue.
Depuis D829 (SAURON-1116e), la ligne entre aussi au moment où une unité reste bloquée à 100 % faute de place, en même temps que la note « Population pleine » de la file et le message du centre « Population maximale — construisez des maisons ».

Options :
- A. Rien de plus : la ligne reste discrète (8 s, muette). En jeu : tu la rates si tu regardes ailleurs ; la file et le message du centre restent tes repères.
- B. La ligne reste plus longtemps (15 s) et joue le son court prévu par D804 B. En jeu : tu l'entends même si tu regardes ailleurs ; le fil est un peu plus chargé.
- C. La ligne clignote ou est en couleur d'alerte pendant 2 s à son entrée. En jeu : l'œil y va sans son ; plus visible que les autres lignes du fil.
- Autre : ta réponse.

Recommandation : B (le son court est déjà décidé par D804 B ; c'est le moyen le plus sûr de remarquer la ligne sans la rendre criarde).
Valeur par défaut codée : A (rien n'a changé : durée D446, ligne muette) ; aucune réglage ajouté à ce stade.

Réponse :
