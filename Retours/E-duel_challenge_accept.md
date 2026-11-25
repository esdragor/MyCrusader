# [À ESSAYER] E-duel_challenge_accept : défier le héros de l'IA (Entrée) et voir les deux héros se battre seuls dans l'anneau rouge, l'IA acceptant d'office

STATUT : VU À L'ÉCRAN (cadrage des deux héros ; geste interne, voir ci-dessous)
GESTE: interne      (Entrée = `waitkey` ; le défi part par la console `hol.Duel.Challenge 1`, l'acceptation par `hol.Duel.ForceAnswer 1 1`)
SON: non entendu   (lancé avec -nosound)
branches: duel-frame-1124sj

Pré-vol (2026-11-24 22:28, agent GLORFINDEL-1124sj, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main e503646) : `preflight.ps1 duel_challenge_accept -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -AllowUncommitted` PASS (74 s, 3 PNG).
PNG OUVERTS et vus :
- duel_heroes_face : héros bleu (bas, avec ses Paysans bleus) et héros de l'IA (haut, silhouette claire parmi les Paysans rouges) tous deux identifiables, sur de l'herbe dégagée ; un arbre masque en partie le bas mais pas les héros. Le panneau montre encore la note 1 (la note 2 n'est pas encore affichée au moment de la prise).
- duel_window : fenêtre « Défi envoyé au héros du joueur 2 / 6 s » en haut au centre, panneau caché, les deux héros côte à côte au centre.
- duel_started : anneau rouge, héros bleu (PV 374/450) et héros rouge de l'IA au corps à corps dans l'anneau ; un Paysan bleu isolé hors de l'anneau (un autre peut passer derrière un arbre).
NON VÉRIFIÉ : jeu réel avec fenêtre et souris (geste interne), son, tenue du cadrage sur une autre graine de carte (les arbres sont aléatoires ; y=1050 choisi sur la carte de ces 3 essais) ; Paysans de l'IA non vus pendant le duel.
Pre-vol ARWEN-1125ma 2026-11-25 10:01 (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, charge machine) : FAIL (178 s) : defi recu par le serveur (RpcDuel slot 0 -> 1) mais pas de ligne 'challenges slot 1 at step' (refus hors portee : les heros n'ont pas fini la marche), timeout 'waitlog made' ; duel_heroes_face OK, duel_window et duel_started absents. L'essai reste a confirmer sur une machine peu chargee ; correction proposee : attente d'arrivee au lieu de wait 3 fixes.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 duel_challenge_accept -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
