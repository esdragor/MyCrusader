# [À RÉPONDRE] Nomade : comment l'IA fonde son premier forum (N3 IA)

Agent HALDIR-1115c, 2026-11-15. Journal : `NomadInProgress.md` (worktree Haldir, branche `maps`).

**Contexte.** En Nomade (D207, D402, D786), chaque joueur part sans forum avec 5 Paysans seuls, dispersés sur toute la carte, et le prix du forum dans son stock. Le GDD dit ce que fait le joueur (il pose son forum où et quand il veut, D402) mais pas ce que fait **l'IA**.

## Q1 : où l'IA pose-t-elle son premier forum, et quand ?

- **A (recommandé, codé par défaut)** : **au groupe de Paysans le plus nombreux, dès que le stock couvre le prix**. Avec 1 Paysan par groupe (D786), tous les groupes sont à égalité : c'est le **premier groupe** (celui du Roi en Régicide) qui l'emporte. En partie : l'IA pose sa fondation **dès la première seconde**, là où se tient l'un de ses Paysans ; les autres Paysans la rejoignent pendant la construction. Rapide, prévisible, mais l'endroit peut être pauvre (pas de bois ni d'or proches).
- **B** : **au point le plus riche en ressources** (bois, pierre et or proches) **parmi les endroits de ses groupes**. En partie : l'IA prend quelques secondes de plus, mais son forum est mieux placé (moins de marche pour récolter, comme un bon joueur d'AoE2) ; le choix dépend de ce qu'elle a exploré (carte noire, D381).
- **C** : **au milieu de ses groupes** (le point le plus proche de tous ses Paysans). En partie : moins de marche pour tous les Paysans, mais l'endroit peut tomber n'importe où (forêt, près d'un ennemi) et rien ne garantit des ressources.
- **Autre** : à préciser.

## Q2 : l'IA rassemble-t-elle ses groupes avant de poser ?

- **A (recommandé, codé par défaut)** : **non** : elle pose tout de suite et les autres Paysans marchent vers la fondation pour la bâtir ensemble. En partie : forum plus tôt ; Paysans isolés exposés en chemin.
- **B** : **oui, en partie** : elle attend d'avoir au moins 3 Paysans réunis au point choisi avant de poser. En partie : forum un peu plus tard, Paysans moins exposés pendant la marche, fondation moins souvent détruite.
- **C** : **non, et les Paysans éloignés récoltent sur place** (ils ne viennent pas bâtir, seuls ceux du groupe choisi bâtissent). En partie : économie qui démarre partout à la fois, forum plus lent à finir.
- **Autre** : à préciser.

**Réglage codé** (une ligne à changer, `Script\AI\HOLAINomadRules.as`) : `hol.AI.NomadForumRule` = 1 (Q1 A ; 0 = l'IA ne fonde jamais d'elle-même) ; `hol.AI.NomadGroupRadius` = 700 cm (deux Paysans à moins de 7 m forment un groupe). Q2 A n'est codé qu'en partie (la règle pose sans attendre de rassemblement ; l'envoi des autres Paysans vers la fondation reste à coder) ; Q1 B, Q1 C, Q2 B et Q2 C ne sont pas codés : rien n'est encore branché au jeu (l'ordre de construction de l'IA, en C++, reste à faire).

Reponse (2026-11-15) : Q1 A : l'IA pose son premier forum au groupe de Paysans le plus nombreux dès que le stock couvre le prix (défaut codé). Q2 A : elle pose tout de suite, les autres Paysans marchent vers la fondation pour la bâtir ensemble. (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D817) et le journal du chantier.
