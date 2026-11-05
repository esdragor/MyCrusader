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

Reponse: (2026-11-05 14:3x) Scenario diplomacy : au debut allies et melanges (rien ne se passe, normal), Entree = passage a Neutre (les DEUX cotes ou un seul : non precise, l'autre cote suit le scenario). Q+clic sur le SOL derriere les hommes neutres : les hommes se deplacent seulement, ne les tapent pas (meme apres les avoir eloignes puis re-Q+clic derriere eux). Q+clic directement SUR un homme neutre : tous l'attaquent. => le clic direct marche (comme le test d'Elrond) ; le defaut est l'attack-move sur le sol (Q+clic sans cible) : il n'engage pas les neutres a portee. Design a confirmer : l'attack-move doit-il engager les neutres rencontres ? (question a poser, non tranchee).

Reponse (2026-11-05 14:4x) a la question de design : OUI, l'attack-move (Q + clic sur le sol) doit engager les neutres rencontres en chemin, comme des ennemis (decision de l'utilisateur ; posture diplomatique jamais modifiee par l'attaque, comme avant). A coder : l'acquisition de cible de l'attack-move compte les Neutres (hors camps neutres de la carte : non precise, a confirmer). Test rouge : Q+clic sur le sol derriere des hommes neutres -> PV des neutres qui baissent.

Reponse (2026-11-05 14:5x) : les camps neutres de la carte (N4) sont AUSSI concernes : l'attack-move engage aussi leurs unites rencontrees en chemin (D733 complete).
