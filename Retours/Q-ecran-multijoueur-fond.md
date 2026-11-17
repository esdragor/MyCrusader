# [RÉPONDU 2026-11-17, D843] Q-ecran-multijoueur-fond : fond de l'écran Multijoueur et état de « Créer un salon » sans gestionnaire

Reponse: Q1 A (Créer un salon reste actif sans gestionnaire : on corrige le .look et le .txt en « actif »), Q2 B (voile sombre derrière l'écran Multijoueur seulement). Texte complet : D843 du GDD.

Contexte : le PNG de ecran_multijoueur montre (1) « Créer un salon » ACTIF et (2) aucun fond (le décor de la partie passe derrière la ligne rouge). Le .look disait « grisé ». Le C++ (HOLManagerErrorText.cpp, CanCreateLobbyAfter, testé par HOLManagerErrorTextTests) ne grise Créer que sur `bad_key` ; gestionnaire injoignable (HTTP 0, code vide) le laisse actif, et le .txt (étape 2) demande de cliquer Créer sur un nom vide, donc actif. L'ancien écran C++ n'avait pas de fond non plus (MakeCenteredColumn seul, comme Titre et Réglages).

Q1 : l'état voulu de « Créer un salon » quand le gestionnaire ne répond pas ?
- A (défaut codé, comportement actuel) : actif ; on corrige le .look (et le .txt ligne 4) en « actif ». Conséquence : clic sur Créer sans gestionnaire = Nouvelle partie en mode salon qui échoue ensuite.
- B : grisé aussi sur « injoignable » ; il faut changer CanCreateLobbyAfter (C++ + son test, donc un build) et l'étape 2 du .txt.
- Autre.
Recommandation : A (aucune régression, le .look était faux).

Q2 : un fond (voile sombre, HOLUIStyle::Veil) derrière l'écran Multijoueur pour lire la ligne rouge ?
- A (défaut codé, comportement actuel) : pas de fond, comme l'ancien écran et les autres menus ; depuis le titre le décor est celui de la carte du titre.
- B : voile sombre sous la colonne, dans MakeCenteredColumn (C++, touche tous les écrans qui l'utilisent) ou en Angelscript pour ce seul écran (exige d'exposer la racine du WidgetTree au script, donc C++).
- Autre.
Recommandation : B pour ce seul écran, après avoir joué l'écran depuis le titre (le scénario actuel l'ouvre par hol.UI.Open au-dessus d'une partie, ce qui exagère le défaut).
