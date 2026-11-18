# SAURON-1118q : scénario d'essai de l'écran Nouvelle partie (2026-11-18)

Impact joueur : l'essai « ecran_nouvelle_partie » se joue enfin jusqu'au bout, avec un vrai changement visible à chaque étape.

## Cause du FAIL (20:38)
Le scénario faisait `hol.UI.Choose SlotState1 2` en croyant la place 2 fermée. Or le `.json` du scénario lance d'abord une partie à 2 joueurs
(Humain + IA, Plain_Small) ; l'écran Nouvelle partie reprend cette config (`HOLNewGameScreen.cpp:457`, `GetConfigOrDefault`) : place 2 DÉJÀ en IA
(« IA Normal 2 », couleur 1), taille Petite. `UHOLDropdown::Choose` (`HOLDropdown.cpp:187`) ne fait rien et ne journalise rien quand `Index == SelectedIndex` :
pas de ligne « value 2 chosen », donc `waitlog` en timeout 60 s. Les indices d'état (Fermé 0, Humain 1, IA 2) et `hol.UI.Choose` étaient corrects.

## Correction (scénario seul, aucun code de jeu)
`Tools\Scenarios\ecran_nouvelle_partie.txt` : Taille -> Moyenne (4 places) ; place 3 (`SlotState2`) Fermée -> IA (son niveau apparaît) ; place 3 prend la couleur 3
(`SlotColor2`, libre car place 4 fermée) puis place 4 (`SlotState3`) s'ouvre : sa couleur 3 est prise, elle saute à la 4 (`FindFreeColor`, `:1337`) ; liste des tailles avec 4 joueurs
(Petite grisée) ; waitkey ; Retour. Chaque `waitlog` attend une ligne qui existe. `.look` mis à jour (captures décrites telles qu'on les voit).
Commits sur `visual` : 80b7b08 (scénario), 1628174 (.look).

## Pré-vol
2 passages sur 3. 1er : scénario PASS (22 s) mais pré-vol FAIL « fichiers non commités » ; 2e (20:44) : `preflight ecran_nouvelle_partie : PASS`, 0 warning/erreur de journal (2 WARN : pas de `HUD overlap:` ni `Sanity:`).

## Ce que montre chaque PNG
- `ecran_np_ouverture` : écran Nouvelle partie, Taille Petite (2 joueurs), « Vous » place 1, « IA Normal 2 » (niveau Normal grisé) place 2, 6 places Fermées grisées, colonnes alignées, Retour / Lancer visibles, rien de coupé.
- `ecran_np_ia_niveau` : Taille Moyenne (4 joueurs), places 2 et 3 en « IA Normal 2 / 3 », chacune avec sa liste Niveau (Normal, grisée) ; place 3 Verte.
- `ecran_np_couleur` : places 2, 3, 4 en IA avec Rouge, Jaune, Turquoise (la place 4 a sauté de Jaune à Turquoise), trois pastilles distinctes.
- `ecran_np_taille_liste` : liste Taille ouverte sous le champ : « Petite (2 joueurs) » grisée, Moyenne (surlignée), Normale, Grande, Immense en clair.
- `ecran_np_retour` : écran titre (Heroes of Liberty, Jouer, Multijoueur, Réglages, Quitter).

## Reste / anomalies
- Les notes jaunes du scénario (overlay du panneau scénario) s'empilent en haut des captures et masquent partiellement les listes Carte / Brouillard (visible surtout sur couleur, taille_liste, retour) ; les captures prouvent la grille, mais un test de « rien de coupé » en haut serait faussé par ces notes : à voir côté outil (notes après capture ou overlay masqué).
- Le niveau des IA apparaît grisé (« Grisé : pas encore dans le jeu ») : à confirmer comme voulu par la session principale.
- Les deux WARN du pré-vol (`hol.HUD.Report`, `hol.Test.Sanity`) ne sont pas appelés par ce scénario.
- Non vérifié : le rejeu avec un vrai joueur (waitkey), la comparaison avec l'écran script (étape 4 : pas encore migré).
- Non touchés : EtapeDefini.md, Auto_Journey_File.md, *InProgress.md, fiches Retours.
