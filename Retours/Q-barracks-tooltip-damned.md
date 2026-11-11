# [À RÉPONDRE] Infobulle de la Caserne : l'Âme damnée (DamnedWarrior) dans « Produit : »

Contexte : depuis 757b5ce (L4, 2026-11-11 14:05, dans main) la Caserne a DamnedWarrior dans `Produces` (BuildingTypes.csv). L'infobulle de la case Caserne d'une partie Aube lit donc « Produit : Lancier, Homme d'armes, DamnedWarrior » (id brut, non traduit, unité de l'autre faction). `quick_check -Full` : seule partie rouge, `build hud` (attente « Produit : Lancier, Homme d'armes »).

Options :
- A : l'infobulle filtre par faction (n'affiche que les unités de la faction du joueur) et utilise le nom localisé ; l'attente du test reste telle quelle (le correctif est dans le jeu).
- B : l'infobulle montre toutes les unités de la Caserne, avec nom localisé (Âme damnée) ; on met à jour l'attente du test.
- C : autre.

Conséquences visibles : A = un joueur Aube ne voit jamais d'unité Légions ; B = il la voit (grisée ou non).

Recommandation : A.

Valeur par défaut : rien changé dans le jeu ; `build hud` est listé dans `Tools\Tests\known_reds.txt` (une ligne à retirer quand tranché).
