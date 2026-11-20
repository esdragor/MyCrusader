# [À ESSAYER] E-faction_slot : la barre d'Honneur de l'Aube dans le HUD (coin bas gauche, au-dessus du bloc du héros)

STATUT : VU À L'ÉCRAN
GESTE: interne
SON: sans objet
branches: aucune

Pré-vol (2026-11-20 12:56, session principale, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main 991a8bc) : `preflight.ps1 faction_slot -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (34 s, 3 PNG).
PNG OUVERTS et vus :
- faction_slot_30 : barre « 30 / 100 » au-dessus du bloc du héros, ligne « SacredLight cost 25 soon » dessous.
- faction_slot_100 : barre pleine « 100 / 100 ».
- faction_slot_0 : barre vide « 0 / 100 ».
NON VÉRIFIÉ : provisoire visible (Placeholders.txt) : texte brut de la ligne de pouvoir (« SacredLight cost 25 soon »), sans icône, position à 150 px ; aucun pouvoir n'est cliquable (boutons non branchés) ; l'Honneur est posé par la triche `hol.Cheat.Honor`, pas gagné en duel dans l'essai.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 faction_slot -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Gestes : regarder la barre passer à 30, 100 puis 0 (trois étapes du panneau du scénario).
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...` (place, taille, texte).
Reponse:
