# [A ESSAYER] E-forest_stretch_compare : les forets peintes etirees sur les cinq tailles de carte (Petite a Immense) ; dis aime / pas aime par taille

STATUT : VU À L'ÉCRAN (2 PNG ouverts a plein format ; aucun gout juge)
GESTE: interne      (commandes du scenario ; la touche Entree est le seul geste de l'utilisateur, passee en auto par preflight)
SON: sans objet
branches: fog

Options : 1 Petite, 2 Moyenne, 3 Normale, 4 Grande, 5 Immense ; trois vues par taille : ma base (forets peintes etirees autour), le milieu de la carte, la plus grande foret (15 PNG fstretch<n>_<taille>_{base,centre,forest}).

Faramir-5 2026-11-11 16:2x (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog tip 3fd83e2 : `notereset` + etat « pret » des la 1re waitkey ; scenario converti) : `preflight.ps1 forest_stretch_compare -Root <worktree>` PASS (2 WARN : pas de ligne « HUD overlap », pas de ligne « Sanity »). PNG OUVERTS et vus : fstretch1_small_base (Petite : le centre-ville au milieu, forets peintes autour en plaques vert clair, minimap a gauche verte avec taches claires ; panneau masque, aucun bandeau) et fstretch5_huge_forest (Immense : grande masse de forets en pins serres sur plusieurs centaines d'arbres, bien lisibles ; panneau masque). Les 13 autres non rouverts.
NON VERIFIE : les 13 autres PNG ; l'aspect etire (comparaison entre tailles) non juge ; pas de bandeau sur ces PNG (panneau masque) ; pas vu en partie complete.

Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 forest_stretch_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir  (sans -Hidden = fenetre, reservee a l'utilisateur ; un agent ajoute -Hidden)
Reponse attendue : par taille `aime` / `pas aime` ou `REGLAGE: ...`. Pas encore liste dans ESSAIS-A-JOUER.
