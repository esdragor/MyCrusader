# [À RÉPONDRE] Neutres N7 : l'indicateur de camp (vivant / nettoyé)

Contexte : le script lit maintenant l'état des camps neutres (portes `UHOLCampSubsystem` : `GetNumCamps`, `IsCampCleared`, `GetCampLivingMembers`, `GetCampPost`, branche `d751-root`). Les camps n'existent que là où la simulation tourne (solo, serveur) ; le GDD ne nomme ni le libellé ni la forme de l'indicateur, et aucune carte ne pose encore de camp (N2). Au plus 2 questions, valeur par défaut codée derrière un réglage, rien n'attend la réponse.

## Q1. Le client voit-il l'état des camps ?
Aujourd'hui un client (partie en réseau, hors hôte) lit 0 camp : l'indicateur ne s'afficherait que pour le solo et l'hôte.
- **A. Réplication de tous les camps** (état + position + vivants envoyés à chaque client). En jeu : tous les joueurs voient toujours si un camp est vivant ou nettoyé, même sous le brouillard (donne une information sur la carte non gagnée, et contredit D381-D383 si le camp n'a jamais été vu). Coût : un champ répliqué de plus.
- **B. Hôte et solo seulement, le client ne voit rien** (état actuel, rien à coder). En jeu : en réseau, l'indicateur manque chez les clients ; incohérent entre joueurs.
- **C. Réplication limitée aux camps déjà vus par le joueur** (le camp entre dans sa liste quand son brouillard l'a découvert, puis son état suit comme les bâtiments vus). En jeu : même règle que le reste de la carte (le brouillard ne fuit pas) ; coût le plus élevé (suivi de « vu par » par camp).
- **Autre** : à préciser.
Recommandation : **C** pour la partie finale, **B** gardé tant que le prototype n'a pas de camp en réseau (aucun camp ne sort des cartes avant N2).
Valeur par défaut codée : **B** (un client lit 0 camp ; une ligne à changer dans `UHOLCampSubsystem::OnWorldBeginPlay` le jour où C est choisi).

## Q2. Que montre l'indicateur et où ?
- **A. Une ligne de texte sous celle des sites** (écran de jeu, `HOLGameScreenScript.as`) : « Camp : vivant (N) » / « Camp : nettoyé » par camp connu. En jeu : lisible, sans toucher à la minicarte ; ne dit pas où est le camp.
- **B. Un point sur la minicarte** (rouge = vivant, gris = nettoyé) à la position du camp. En jeu : on repère les camps d'un coup d'oeil, comme les sites ; demande la règle de la minicarte (`HOLMinimapRules`) et un libellé de survol.
- **C. Les deux** (A et B). En jeu : le plus complet, le plus de code à régler et à essayer.
- **Autre** : à préciser.
Recommandation : **B** (cohérent avec les sites et le brouillard : un camp non découvert n'a pas de point), libellés de survol à fixer avec la réponse.
Valeur par défaut codée : aucun indicateur affiché tant que la réponse n'est pas donnée (les portes de lecture existent, la règle d'affichage `HOLCampBanner` n'est pas écrite : un libellé inventé serait à jeter).

Reponse: Q1 C (replication limitee aux camps deja vus), Q2 B (point sur la minicarte, rouge vivant / gris nettoye ; libelle de survol a fixer) - recommande, utilisateur, 2026-11-11 soir
