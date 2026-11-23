# SPEC-VOIES-1123 : effet des 3 voies de specialisation des Templiers (releve lecture seule, GALADRIEL-1123a)

Heure du releve : 2026-11-23 ~09:03. Worktree : HeroesOfLiberty-Galadriel, branche socle-ai (pas specialization-trial).

## Ce que le GDD fixe
- D07 : tronc commun + choix definitif d'une specialisation parmi 2 par palier ; Templiers : la specialisation = le choix d'une commanderie (GDD.md:649).
- D90 / D131 / D132 : 3 commanderies par palier, une par voie : Fer (melee, choc), Foi (soutien, moral), Pierre (tir, siege) ; 9 au total ; voies cumulables (on peut les melanger) (GDD.md:1763, D132 l.2467).
- D133 / D683 (GDD.md:1764) : les 9 commanderies sont des TECHNOLOGIES : le choix (interface, gratuit, applique apres le delai de D684, definitif, D332) debloque l'unite dans son batiment existant (D334) ET ajoute son contingent a la croisade. Grille : P1 Sergent du Temple / Frere infirmier / Arbaletrier a pavois ; P2 Sergent a cheval / Precheur de croisade / Frere macon ; P3 Marechal du Temple (unique) / Senechal (unique, hors croisade) / Beffroi.
- Effets par unite : GDD.md:1772-1790 (Ouvrir les rangs, Seconde vague, Escadron, Sermon, Mini-croisade, Planter le pavois...). Croisade : contingent ~10 croises par commanderie (D139/D140) ; gloire D90.
- D823 (UI : bouton permanent de faction, cases D/F en raccourci) et D868 (les 3 voies sur les cases 5, 6, 7 du Forum) : UNIQUEMENT l'interface, aucun effet.

## Ce que le code fait aujourd'hui
- `HOLSpecialization::ChosenOption(State, Tier)` (`Script\Specialization\HOLSpecializationRules.as:109`) est lu par la regle elle-meme (l.124, 132, 159) et par le tableau (`HOLSpecializationBoardScript.as:162`, `View.Chosen`) ; aucun autre lecteur de jeu (grep Source\ et Script\ : seulement tests). Choisir une voie ne change donc rien en partie : « Bientot » cote effet.

## Ce qui manque (trou)
1. Aucun lien ChosenOption -> deblocage de l'unite de commanderie (qui la recrute, quel batiment, quelle table de disponibilite). NON VERIFIE : existence des 9 unites dans `Tools\Data` (Sergent du Temple... Beffroi).
2. Aucun lien vers le contingent de croisade (croisade Templiers : NON VERIFIE si codee ; D88 a D90, D139).
3. Gloire par contingent (D90) : sans objet tant que 1 et 2 manquent.
4. Ordre d'attaque du chantier : le GDD fixe l'effet complet (unite + contingent) mais pas l'ordre ; il faut un premier lot minimal.

## Question posee
`Retours\Q-spec-voies.md` (une seule) : premier lot d'effet a coder.
