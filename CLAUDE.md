# Règles du projet HeroesOfLiberty

## Emplacement des sessions Claude

Les sessions (transcripts) de ce projet sont dans `C:\Users\baudo\ProjectPersoClaude\c--Users-baudo-Documents-MyCrusader\`.
Ne **jamais** les chercher ni écrire de notes à leur sujet dans `C:\Users\baudo\.claude\projects` : cet emplacement n'est
pas utilisé. Toute note de ce type va dans le projet (ce fichier, `EtapeDefini.md`, les `*InProgress.md`).

## EtapeDefini.md à chaque avancée (règle absolue)

À chaque avancée (question tranchée, boucle ou sous-étape faite, commit même sur une branche de worktree, fusion dans
`main`, bug trouvé ou corrigé, étape ouverte ou close), mettre à jour
`C:\Users\baudo\Documents\MyCrusader\EtapeDefini.md` **en même temps** que le journal `*InProgress.md` du chantier.

- Chemin absolu, même depuis un worktree : c'est la copie vivante, lue par l'utilisateur.
- Titres seuls : où en est le chantier (fait / en cours / suivant, worktree, commit), le détail reste dans le journal.
- Relire le fichier juste avant d'éditer, ne toucher que ses lignes (plusieurs sessions écrivent en parallèle).
- Section « Fichiers de développement » à jour aussi (nouveau `*InProgress.md`, avancée d'un chantier).

## Placeholders.txt à jour (règle permanente)

`C:\Users\baudo\Documents\MyCrusader\Placeholders.txt` liste TOUT ce qui est provisoire dans le jeu (unités, bâtiments,
arbres, décor, icônes, logo, sons...), une ligne courte par chose. Chantier : `PlaceholdersInProgress.md`.

- Tout chantier qui **ajoute** un provisoire (forme du moteur, mannequin, son d'attente, icône absente...) ajoute sa ligne.
- Tout chantier qui **remplace ou supprime** un provisoire retire sa ligne et le note dans « Remplacés » de
  `PlaceholdersInProgress.md`.
- Chemin absolu, même depuis un worktree ; relire le fichier juste avant d'éditer, ne toucher que ses lignes.
- Ne pas clore un chantier sans avoir relu ce fichier.
