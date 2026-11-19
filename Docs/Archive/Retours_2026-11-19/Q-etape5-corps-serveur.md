# [RÉPONDU] Étape 5 : faut-il migrer les corps serveur des ordres (`Do<X>`) alors que le gain est presque nul ?

Contexte : D851 dit de faire tout le plan d'origine de l'étape 5. Le relevé du 2026-11-19 (`Docs\Rapports\C3-PLAN-1119.md`) montre que les lots C3, C4, C5 (les 43 corps `Do<X>` du contrôleur, ce que le serveur exécute quand un ordre ou une triche arrive par le réseau) rapportent peu : chaque corps migré remplace un pont C++ par une liaison C++ de taille voisine. Pour le joueur rien ne change si tout va bien ; si un lot se trompe, un ordre donné en multijoueur ne part plus (invisible en solo).

| Lot | Contenu | C++ qui part (contrôleur + pont) | Liaison C++ à ajouter | Avis du relevé | Risque joueur |
|---|---|---|---|---|---|
| C3a | 8 ordres (déplacement, attaque, arrêt, déploiement, récolte…) | ~80 + ~35 | ~70 | gain faible (net ~0 à -40 l.) : des corps de 7 à 9 lignes qui ne font déjà qu'appeler la règle | élevé (ordres en réseau) |
| C3b | 8 ordres (construction, dépôt, garnison, pose, annulation, soin, ralliement) | ~140 + ~40 | ~90 | douteux pour le soin et le ralliement (logique C++ à déplacer telle quelle, pas à réécrire) | élevé |
| C4a | 7 triches simples de développement | ~135 + ~45 | ~60 | bon rapport | nul (outils de test) |
| C4b | 7 triches d'état (blessure, bâtiment, défaite, forum…) | ~140 + ~45 | ~110 | gain faible, surtout de la cohérence | nul |
| C5a | économie + héros (production, tribut, marché, retour, rage, capacité, talent) | ~110 + ~45 | ~90 | à faire, bon gain | moyen |
| C5b | spécialisation, signal, chat | ~120 + ~30 | ~110 | ne vaut pas son risque (net ~0, aucune preuve réseau dédiée) | moyen |

Total estimé par le relevé si tout est fait : ~720 lignes quittent le contrôleur et ~240 lignes de pont, ~520 lignes de liaison s'ajoutent, soit ~-440 lignes nettes sur 6 lots et 3 builds. Ces chiffres sont des estimations (le relevé les marque « déduit » et ses sous-totaux ne se recoupent pas tous) : l'ordre de grandeur est sûr, pas la dizaine.

- A : ne faire que ce qui rapporte : C4 (triches, risque nul) et C5a (économie + héros). C4b en option. C3a, C3b et C5b sont abandonnés ; leurs corps restent en C++ comme « enveloppe réseau » (ils ne contiennent plus de règle : les ordres passent déjà par la planche script depuis aujourd'hui). 2 ou 3 lots, aucun ordre réseau touché.
- B : tout faire, comme le plan d'origine. ~-440 lignes, 6 lots, les ordres en réseau touchés deux fois.
- C : rien de tout cela : les 43 corps restent en C++ ; on passe directement à la suite (grille, production, pilotes de test, portage des tests).
- Autre.

Recommandation : **A**. Les règles des ordres sont déjà en script (lots P1 et P2) ; ce qui reste dans `Do<X>` n'est que du transport. Valeur par défaut tant que tu n'as pas répondu : rien n'est lancé sur C3, C4, C5 ; les autres lots continuent.

Reponse: C, et l'étape 5 est close (2026-11-19 ~19:23, interface de choix, « Clore maintenant » ; **D857** du `GDD.md`) : les 43 corps `Do<X>` restent en C++ comme enveloppe réseau ; lots C3, C4, C5 abandonnés.
