# [À ESSAYER] E-legions_warrior : tu cliques le Guerrier damné des Légions puis la case Rage (E) ; la rage est lancée, mais rien ne la montre à l'écran

STATUT : LOGIQUE SEULEMENT   (la rage elle-même n'est pas visible : le bouton et le Guerrier le sont ; jamais listé prêt tant que Q-legions-warrior-rage-visual n'est pas tranchée)
GESTE: interne   (la production passe par la touche E = hol.Select.Grid 2, la sélection par un vrai clic sur le corps = hol.Select.ClickUnit, la rage par hol.HUD.Click GridCell02 : le chemin de la souris sur la case, mais une commande hol.*, pas le vrai pointeur)
SON: non entendu   (-nosound)
branches: galadriel-tab

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-11 16:39, agent Galadriel, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, main 15d1cca + galadriel-tab f6cd658) : `preflight.ps1 legions_warrior -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (39 s, 3 PNG, 15 s de jeu).
PNG OUVERTS et vus :
- legions_warrior_button : VU À L'ÉCRAN. Le Guerrier damné (petit corps bleu sous la Caserne bleue) est sélectionné ; le bloc du bas dit « Guerrier damné », PV 130 / 130, Attaque 16 ; la grille montre une seule case allumée, E « Rage » (A, Z, R, Q... vides). Un Lancier rouge de l'Aube est juste à côté de la Caserne.
- legions_warrior_rage : LOGIQUE SEULEMENT. Soixante images après le clic sur « Rage » : le Guerrier bleu et le Lancier rouge sont au contact ; PV encore 130 / 130 ; aucune teinte, aura, texte ni icône d'état, la case E est identique à celle de la capture (a). Preuve de l'effet : le journal (« Grid: necrotic rage », « Rage order: slot 0 queued for 1 units », « #9 DamnedWarrior casts the necrotic rage (130 health) »).
- legions_warrior_after : VU À L'ÉCRAN pour la perte de PV, LOGIQUE SEULEMENT pour la fin de rage. Le Lancier rouge est couché (« killed by #9 DamnedWarrior (4 hits struck) »), le Guerrier est à 106 / 130 (2 coups reçus au moins et 3 PV par coup porté) ; la case E reste identique (pas grisée pendant la recharge) ; aucun signe de fin de rage à l'écran. Un second clic sur la case est accepté par l'ordre (« Rage order queued ») et refusé en silence (aucune seconde ligne « casts »).
Question posée : Retours\Q-legions-warrior-rage-visual.md (défaut codé : rien dessiné).
NON VÉRIFIÉ : la durée de 10 s et la fin de rage (aucun témoin à l'écran ni au journal) ; le fait que le clic soit un vrai clic de souris (commande hol.HUD.Click) ; le son ; le Guerrier produit par la touche E n'a pas été essayé en partie réelle avec l'IA (la règle de rage de l'IA n'a jamais tourné) ; la lecture par un humain du bouton « Rage » (libellé et info-bulle en en/ja = propositions).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 legions_warrior -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 1 min (15 s de jeu). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse: BUG: la rage ne fait rien (rien ne se voit) = Q-legions-warrior-rage-visual A+C (2026-11-11 soir)
