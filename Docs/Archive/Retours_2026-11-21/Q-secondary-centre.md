[À RÉPONDRE] Centre secondaire : ligne SecondaryCentre de BuildingTypes.csv (SC2)

Contexte : le GDD (5.4, D384, D783, 11.1, D523) donne le coût (150 bois + 400 pierre), la population (10), le palier (1) et la vue (16 cases) ; il ne donne pas la durée de construction, les PV, l'emplacement dans la grille du Paysan ni la liste des unités produites. Valeurs posées (derrière la ligne du CSV, une ligne à changer, la table se régénère par `Tools\AutoJourney\regen_tables.ps1`) : durée 150 s (comme le Centre principal, D816), PV 2400 et emprise 16 m x 16 m comme le forum, case de grille 1 de l'onglet II (Essor), Paysan seulement (pas de Zombie).

Question unique : durée de construction et PV du centre secondaire.
- A (défaut codé) : comme le forum, 150 s pour un bâtisseur, 2400 PV. Conséquence : le boom coûte cher en temps et il faut défendre un chantier long ; cohérent avec « un vrai forum de plus » (D384).
- B : plus court, 100 s, 2400 PV. Conséquence : expansion plus rapide, le rush punit moins un centre posé loin.
- C : plus court et plus fragile, 100 s, 1800 PV. Conséquence : expansion rapide mais un centre isolé tombe vite.
- Autre : ta valeur.

Recommandation : A (aucun chiffre de design neuf, le centre secondaire se comporte comme le forum ; on ajuste après un essai de boom).

Note sans question : la case de grille (1, onglet II) et « Paysan seulement » sont provisoires et se changent dans le CSV ; le Zombie reste réservé au Centre principal tant que tu ne dis pas autre chose.
Reponse: A (2026-11-21 14:09, interface de choix)
