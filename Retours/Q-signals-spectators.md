# [À RÉPONDRE] Signaux : qui les voit après une élimination (chacun pour soi, Diplomatie) et chez les spectateurs

Chantier : signaux (`SignalsInProgress.md`, worktree Haldir, branche `maps`). 1 question. Rien n'est bloqué : le défaut est le
comportement actuel.

## Q1. Un joueur éliminé en chacun pour soi ou en *Diplomatie*, et un spectateur pur, voient-ils les signaux des joueurs ?

Contexte : en équipe, l'éliminé voit les signaux de son équipe sans en poser (D784). En chacun pour soi et en *Diplomatie*, l'éliminé
devient spectateur : il voit toute la carte mais ne parle qu'aux spectateurs (D376) ; rien ne dit s'il voit les signaux.

- **A. Il voit tous les signaux de tous les joueurs** (comme il voit toute la carte). En jeu : le spectateur suit les plans de chacun ;
  aucune fuite, puisqu'il ne parle qu'aux spectateurs.
- **B. Il ne voit aucun signal.** En jeu : le spectateur voit les armées bouger sans savoir ce que les alliés se disent ; plus simple.
- **C. Il ne voit que les signaux de ses anciens alliés** (ceux qu'il voyait avant d'être éliminé) ; un spectateur pur les voit tous.
  En jeu : continuité pour l'éliminé ; deux règles différentes selon le spectateur.
- **Autre.**

**Recommandation : A** (même logique que « voit toute la carte », et aucune information ne remonte aux joueurs).
Défaut codé : B, de fait (`HOLSignal::Sees` ne connaît que les alliances entre joueurs vivants ; aucun spectateur ne reçoit rien) ;
passer à A = un booléen au serveur quand SG2 enverra les signaux (envoi aux spectateurs).

Reponse: A (l'éliminé en chacun pour soi / Diplomatie et le spectateur pur voient tous les signaux de tous les joueurs). (réponse donnée dans la conversation, 2026-11-13 ~17:10)
