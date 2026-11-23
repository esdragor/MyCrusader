# DIAG-1123c (2026-11-23)
CAUSE HUD: attente perimee de quick_check.ps1 (ligne ~4271 'the cells write the name alone' et ~4273 'tooltips'): le jeu affiche maintenant 'I D: Palissade' (cellules) et 'I D: Palissade | W 5 missing' (infobulles) entre Champ de tir et Tour de guet ; la regex ne l'attendait pas. Le test de grille (D=Palisade) avait ete mis a jour par a31953b, pas ces deux textes.
CAUSE BARRACKS: meme attente perimee, ligne ~4513 (regex '; I Q: Caserne; I S: Champ de tir; I F: ...' sans '; I D: Palissade').
COMMIT FAUTIF (decision): S3-b, 8e6281d (Palisade table row, grid count 16) + 61d0fdc (WallTypes lists the Palisade) ; mise a jour partielle a31953b. Pas un defaut du jeu.
CORRECTIF: 9a8a4e8 sur integ-55 (3 lignes de Tools\Tests\quick_check.ps1, BOM garde, analyse OK).
BUILD: quick_check -Parts build PASS in 201 s (36 parties, hud et barracks PASS), 10:50.
C++: non. Rouge: sans objet (attente de test). Pas de merge main, pas de push.
ANOMALIES: la sortie PARTS ne gardait pas le detail des checks en echec ; cause lue dans Saved_Tests\Logs\quick_game.log (HUD cells/tooltips) et comparee aux regex.
