# Règles du projet HeroesOfLiberty

## Emplacement des sessions Claude

Les sessions (transcripts) de ce projet sont dans `C:\Users\baudo\ProjectPersoClaude\c--Users-baudo-Documents-MyCrusader\`.
Ne **jamais** les chercher ni écrire de notes à leur sujet dans `C:\Users\baudo\.claude\projects` : cet emplacement n'est
pas utilisé. Toute note de ce type va dans le projet (ce fichier, `EtapeDefini.md`, les `*InProgress.md`).

## Langage de développement : Angelscript par défaut, C++ par exception (règle permanente, D830, D836, D839)

Tout code NEUF s'écrit **directement en Angelscript** (règles, IA, interface, tests, outils de jeu) : plus de « C++ d'abord,
migration ensuite ». Le C++ reste l'exception et s'écrit seulement pour une raison dite dans la tâche : processeur Mass ou
code appelé par entité et par pas, réplication réseau, Slate / widget de base, mesure de performance, ou liaison à exposer au
script. Une règle n'a qu'une seule source (pas de jumeau C++ et script) ; sens de chaque règle existante : D836 du `GDD.md`.
Les tests de la simulation Mass restent en C++ (D838) ; tout autre test neuf s'écrit en script. Une tâche qui a besoin de C++
neuf le regroupe dans le lot C++ de la séance (un build) au lieu d'ouvrir un build par fichier.

## EtapeDefini.md à chaque avancée (règle absolue)

À chaque avancée (question tranchée, boucle ou sous-étape faite, commit même sur une branche de worktree, fusion dans
`main`, bug trouvé ou corrigé, étape ouverte ou close), mettre à jour
`C:\Users\baudo\Documents\MyCrusader\EtapeDefini.md` **en même temps** que le journal `*InProgress.md` du chantier.

- Chemin absolu, même depuis un worktree : c'est la copie vivante, lue par l'utilisateur.
- Titres seuls : où en est le chantier (fait / en cours / suivant, worktree, commit), le détail reste dans le journal.
- Relire le fichier juste avant d'éditer, ne toucher que ses lignes (plusieurs sessions écrivent en parallèle).
- Section « Fichiers de développement » à jour aussi (nouveau `*InProgress.md`, avancée d'un chantier).

## Nouveautés publiées dans toutes les langues (règle permanente, 2026-11-24)

Toute publication du jeu avec nouveautés (`Tools\Package\publish.ps1 -Notes x.txt`) fournit **obligatoirement** leurs
traductions à côté : `x.txt` (français), `x.en.txt`, `x.ja.txt`, toutes non vides ; noms du jeu repris de
`Content/Localization/Game/{en,ja}/Game.po`. `publish.ps1` (avant le paquet) et HOLPublish refusent (FAIL) sinon ; le
lanceur montre les nouveautés dans sa langue (`Manager/PROTOCOL.md`, `?lang=`).

## Placeholders.txt à jour (règle permanente)

`C:\Users\baudo\Documents\MyCrusader\Placeholders.txt` liste TOUT ce qui est provisoire dans le jeu (unités, bâtiments,
arbres, décor, icônes, logo, sons...), une ligne courte par chose. Chantier : `PlaceholdersInProgress.md`.

- Tout chantier qui **ajoute** un provisoire (forme du moteur, mannequin, son d'attente, icône absente...) ajoute sa ligne.
- Tout chantier qui **remplace ou supprime** un provisoire retire sa ligne et le note dans « Remplacés » de
  `PlaceholdersInProgress.md`.
- Chemin absolu, même depuis un worktree ; relire le fichier juste avant d'éditer, ne toucher que ses lignes.
- Ne pas clore un chantier sans avoir relu ce fichier.
