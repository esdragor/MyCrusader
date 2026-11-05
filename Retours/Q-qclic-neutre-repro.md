# [A REPONDRE] Q-qclic-neutre-repro : Q + clic sur un neutre, comment l'as-tu fait quand rien ne s'est passe ?

Contexte : le test (912e3ca, ordre d'attaque -> PV de la cible) est VERT et deux scenarios de jeu (diplomacy_qneutral, diplomacy_qneutral_building) montrent les coups portes sur un homme et sur une Maison neutres ; je n'ai pas pu reproduire « ne fait rien ». Il me manque ce que tu as fait exactement.

Comment as-tu mis le joueur en Neutre et sur quoi as-tu clique ?
- A. Les DEUX cotes sont Neutres (tu as pose Neutre et l'autre aussi, ou le scenario l'a force), clic sur un HOMME d'armes du joueur 2
- B. Pareil mais clic sur un BATIMENT du joueur 2
- C. Seul TOI es Neutre (l'autre est reste Ennemi ou Allie), clic sur un homme ou un batiment
- D. Le clic etait sur une unite d'un camp neutre de la carte (monstres, N4), pas sur un joueur
- Autre : (dis ce que tu as fait, quelle selection, et si les hommes ont bouge un peu ou pas du tout)

Recommandation : repondre A, B, C ou D avec la selection utilisee (hommes d'armes seuls ? heros inclus ?).
Valeur par defaut codee : aucune ; le comportement actuel est inchange (ordre d'attaque direct sur une unite ou un batiment neutre, posture jamais modifiee).
