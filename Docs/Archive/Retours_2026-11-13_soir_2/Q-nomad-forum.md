# [À RÉPONDRE] Nomade : le prix du forum ajouté au stock, et la spécialisation tant qu'aucun forum n'est debout

Chantier : Nomade (`NomadInProgress.md`, worktree Haldir, branche `maps`). 2 questions, regroupées. Rien n'est bloqué : un défaut est codé
derrière un réglage d'une ligne, tu peux répondre plus tard.

## Q1. Quel est « le prix d'un forum » que le Nomade reçoit au départ (D402) ?

Contexte : D402 dit « le prix d'un forum est ajouté au stock de départ », mais le forum principal ne coûte rien dans les données
(`BuildingTypes.csv`, `MainCentre` : 0 partout, on ne le construit jamais). Le seul forum payant est le centre secondaire (D783 : 150 bois + 400 pierre).

- **A. Le prix du centre secondaire, 150 bois + 400 pierre** (D783). En jeu : le Nomade démarre avec 400 pierre de plus ; poser son
  premier forum vide presque toute la pierre ; un seul chiffre à régler pour les deux forums.
- **B. Un prix propre au premier forum du Nomade, plus doux, 300 bois + 100 pierre** (ordre de grandeur AoE2 : 275 bois + 100 pierre).
  En jeu : départ plus riche en bois, le forum se pose plus vite ; un deuxième chiffre à tenir.
- **C. Le prix du centre secondaire, mais le premier forum du Nomade ne coûte rien** (le stock reçu reste libre). En jeu : le joueur a
  550 ressources « gratuites » et peut tout dépenser ailleurs, au risque de D354.
- **Autre** : un autre prix, ou un prix par faction.

**Recommandation : A** (un forum est un forum, un seul chiffre ; le risque de D354 reste réel puisque le stock est juste le prix).
Défaut codé : A (`HOLNomad::StartingAmount` lira le coût du centre secondaire quand N2 sera codé ; une ligne à changer).

## Q2. Le choix de spécialisation d'un Nomade qui n'a encore aucun forum achevé

Contexte : le palier vient du niveau du héros (D569), donc un Nomade peut l'atteindre avant son premier forum ; or le déblocage
« se lance au Forum » (D684) et se met en pause quand le Forum est à terre (D785 Q2 = A).

- **A. Le bouton s'allume, le choix est pris, mais le compte à rebours reste en pause jusqu'au premier forum achevé** (même règle que
  « Forum à terre »). En jeu : le joueur choisit tôt, rien n'avance tant qu'il n'a pas bâti.
- **B. Le bouton reste éteint tant qu'aucun forum n'est achevé.** En jeu : le joueur ne voit pas le choix ; il s'allume à l'achèvement
  du forum (il peut l'oublier).
- **C. Le compte à rebours court sans forum (exception Nomade).** En jeu : le Nomade n'est pas pénalisé pour bâtir tard ; une exception
  de plus dans la règle.
- **Autre.**

**Recommandation : A** (une seule règle : pas de Forum debout = pause, déjà codée).
Défaut codé : A (aucun forum compte comme « Forum à terre » dans `HOLSpecialization::Advance` ; rien à ajouter).

Reponse: Q1 A (150 bois + 400 pierre, le prix du centre secondaire D783) ; Q2 B (le bouton de spécialisation reste ÉTEINT tant qu'aucun forum n'est achevé ; il s'allume à l'achèvement du forum : le défaut codé A est donc à changer). (réponse donnée dans la conversation, 2026-11-13 ~17:10)
