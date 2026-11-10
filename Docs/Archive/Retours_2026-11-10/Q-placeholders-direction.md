[À RÉPONDRE] Placeholders : direction artistique, logo et script de contrôle (PQ-2 + PQ-3 + PQ-4)

Contexte : la direction « stylisé et franc » a été décidée pour les bâtiments (2026-11-03) mais pas pour les arbres, rochers, ressources et unités ; aucun logo ni titre définitif n'existe dans `Content/` ; la liste `Placeholders.txt` ne tient que si chaque chantier pense à la mettre à jour.

**PQ-2 : direction artistique des ressources, du décor et des unités**
- A = « stylisé et franc » partout ; unités : garder le mannequin avec des accessoires de couleur jusqu'au prototype. **Recommandation.**
- B = « stylisé et franc » pour les ressources et le décor ; viser des corps d'unités propres dès maintenant.
- C = autre direction pour les ressources et le décor (à décrire).
- Autre : à préciser.
- Conséquence visible : A garde les unités telles qu'elles sont et unifie le décor ; B ouvre un gros chantier d'unités avant le prototype.
- Défaut appliqué sans réponse : A (réglable : seul l'ordre des lots dans `PlaceholdersInProgress.md` change).

**PQ-3 : le logo**
- A = pas de logo avant le nom définitif ; ligne `[rien]` gardée. **Recommandation.**
- B = nom de travail « Heroes of Liberty » et dix logos d'essai (méthode des échantillons).
- C = tu fournis l'idée ou un croquis.
- Autre : à préciser.
- Conséquence visible : B donne un titre dans le menu principal ; A laisse le menu tel quel.
- Défaut appliqué sans réponse : A.

**PQ-4 : script de contrôle `Tools/check_placeholders.ps1`**
- A = oui : il échoue si un type à `Mesh` vide ou à mannequin n'a pas sa ligne dans `Placeholders.txt` (et inversement), appelé par `quick_check`, test rouge d'abord. **Recommandation** (les règles en prose ne tiennent pas, `lessons-must-be-mechanical`).
- B = non, garder la règle en prose dans `CLAUDE.md`.
- C = oui mais en simple avertissement (WARN), sans bloquer un commit.
- Autre : à préciser.
- Conséquence visible : A peut faire échouer un commit qui ajoute un type sans ligne ; C le signale seulement.
- Défaut appliqué sans réponse : B (rien codé tant que tu n'as pas répondu).

Réponds par « PQ-2 = X, PQ-3 = Y, PQ-4 = Z » ou « Autre : ... ».

Reponse (utilisateur, 2026-11-09) : comme recommande : PQ-2 = A, PQ-3 = A, PQ-4 = A (script Tools/check_placeholders.ps1 qui echoue, appele par quick_check, test rouge d'abord).
