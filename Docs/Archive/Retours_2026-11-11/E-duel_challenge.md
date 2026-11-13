# [À ESSAYER] E-duel_challenge : Entrée, ton héros défie celui de l'IA ; la fenêtre « Défi envoyé » apparaît avec ses secondes, l'IA refuse, les héros se battent

STATUT : VU À L'ÉCRAN (les trois PNG ouverts et vus par l'agent sur le binaire de main 39ba5ec, build PASS 461 s, 2026-11-11)
GESTE: interne   (waitkey Entrée puis commande `hol.Duel.Challenge 1`, le même appel que fera le bouton ; pas de clic sur un bouton)
SON: sans objet   (lancé sans son)
branches: aucune   (visual contient main 39ba5ec)

Pré-vol (2026-11-11 15:54, agent Sauron-R3, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron) : `preflight.ps1 duel_challenge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (25,6 s d'attente explicite, 1 waitkey, 3 PNG, aucun WARN ; Sanity passé, aucun panneau n'en couvre un autre).
Correction de scénario (écart de la fiche précédente, « le panneau n'est pas revenu à la capture ») : `panel show` puis `frames 30` AVANT `capture duel_answer` (avant, la capture précédait le retour du panneau).
PNG OUVERTS et vus (dossier C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- duel_heroes_face.png : ton héros (bleu) avec ses 5 Paysans au centre, le héros rouge de l'IA juste en face, cercle clair au sol, arbres provisoires (cercles verts) autour ; panneau du scénario « 1. Ton héros et celui de l'IA marchent vers le centre... » ; bloc du héros « Niveau 1 PV 450/450 ».
- duel_window.png : la fenêtre « Défi envoyé au héros du joueur 2 » en haut au centre, barre dorée presque pleine et « 6 s » ; panneau du scénario caché ; les deux héros au contact.
- duel_answer.png : la fenêtre a disparu ; le panneau du scénario est REVENU (notes 1 et 2) ; les héros se battent (PV du héros 399/450).
Constat (pas jugé) : le panneau est revenu à la capture, l'écart est levé ; les Paysans de l'IA ne sont pas visibles (repartis, annoncé comme normal) ; arbres = provisoires du moteur.
NON VÉRIFIÉ : le cas « l'IA accepte » est le scénario duel_challenge_accept (non rejoué ici) ; le texte de la fenêtre et le chronomètre ne sont pas jugés ; le vrai bouton de défi n'est pas cliqué (commande interne).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 duel_challenge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Hidden`
Durée : environ 1 min (marche de 25 s, puis Entrée). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.