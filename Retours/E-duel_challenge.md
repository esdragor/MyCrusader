# [À ESSAYER après correction du build] E-duel_challenge : Entrée, ton héros défie celui de l'IA ; la fenêtre « Défi envoyé » apparaît avec ses secondes, l'IA refuse, les héros se battent

STATUT : VU À L'ÉCRAN (les trois PNG ouverts et vus par l'agent ; binaire Editor du 2026-11-11 14:26, voir NON VÉRIFIÉ)
GESTE: interne   (waitkey Entrée puis commande `hol.Duel.Challenge 1`, le même appel que fera le bouton ; pas de clic sur un bouton)
SON: sans objet   (lancé sans son)
branches: aucune   (visual = main 521adcb)

Pré-vol (2026-11-11 15:19, agent Sauron-F, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main 521adcb) : `preflight.ps1 duel_challenge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (53 s, 25,6 s d'attente explicite, 1 waitkey, 3 PNG, aucun WARN ; Sanity passé, aucun panneau ne en couvre un autre).
PNG OUVERTS et vus (dossier C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- duel_heroes_face.png : ton héros (bleu) avec ses 5 Paysans au centre, le héros rouge de l'IA juste en face, cercle clair au sol, les arbres de remplissage autour ; panneau du scénario « 1. Ton héros et celui de l'IA marchent vers le centre... » visible ; bloc du héros « Niveau 1 PV 450/450 ». Aucun Paysan de l'IA visible (repartis ou hors champ : annoncé comme normal).
- duel_window.png : la fenêtre « Défi envoyé au héros du joueur 2 » en haut au centre, barre dorée presque pleine et « 6 s » ; panneau du scénario caché ; les deux héros au contact.
- duel_answer.png : la fenêtre a disparu ; les héros se battent (PV du héros 399/450 dans le bloc) ; le panneau du scénario n'est pas revenu à l'écran au moment de la capture.
Écart à signaler (pas jugé) : la note dit que le panneau « revient après la réponse de l'IA » ; en duel_answer il n'est pas encore revenu (peut-être un délai). Les arbres sont les provisoires du moteur (lollipops verts).
NON VÉRIFIÉ : binaire PÉRIMÉ (le build de main 521adcb échoue : HOLCombatRules.h(403) C4459 « Margin ») ; captures du binaire du 14:26. Le cas « l'IA accepte » est le scénario duel_challenge_accept (non rejoué ici). Le texte de la fenêtre et le chronomètre ne sont pas jugés. Le retour du panneau après la réponse n'est pas confirmé.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 duel_challenge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Hidden`
Durée : environ 1 min (marche de 25 s, puis Entrée). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
