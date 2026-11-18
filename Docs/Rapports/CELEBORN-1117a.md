# CELEBORN-1117a : lot L17-03 (HOLTechnologyTableTests.cpp -> technology_table_test.py)

Branche hero-j8-close (= main 111d6b6 au depart), sans C++, aucun build, `Source\` non touche.

- FAIT : `Tools\Tests\technology_table_test.py` (nouveau) branche dans la partie values de `Tools\Tests\quick_check.ps1` (3 lignes ajoutees via edit_line.ps1, BOM UTF-8 conserve). Controles : grille GDD 11.1 (26 lignes : tier, batiment, faction Dawn/Common), par ligne (nom affiche, tier 1-2, niveau de heros 0, batiment connu ou Academy prevue, cout > 0 sans negatif, temps > 0, stats connues, pourcent > 0), Implemented = stats toutes codees (CodedStats), chaine II -> I (tier plus bas, meme batiment, memes stats, plus fort), Playable = batiment existant ET Implemented : exactement les 17 attendus, 4 "Soon" non jouables, file de 5 du batiment de chaque jouable ; plus auto-controle (ligne manquante, mauvais tier, mauvais drapeau Implemented refuses).
- ROUGE VU : copie temporaire de Technologies.csv sans PloughII -> FAIL (5 problemes, exit 1) ; PercentI de MinerPicksI a 0 -> FAIL (1 probleme, exit 1) ; vraie table -> PASS 0 probleme.
- `quick_check.ps1 -Parts values` (une fois) : values PASS, quick_check PASS.

## Reste en C++ (lu par UDataTable / Mass / code C++, pas un controle de table)
`HeroesOfLiberty.Technologies.Rules` (FHOLTechnologyRulesTest) en entier : HOLTechnology::Check et l'ordre des raisons, DescribeBlock, MakeCost, TryEnqueue / Cancel / Advance (file de production), IsQueued, GetBonusPercent (total de ligne, additions), ScaleByPercent, GetAttackStat, GetArmorStat, ArmorCoversAttack, FHOLTechnologyState. Aussi l'import lui-meme par UDataTable::CreateTableFromCSVString (analyse du CSV par le moteur : les `Problems` du helper Import) : le python lit le CSV avec son propre parseur.

## Cas C++ retirables (test C++ NON supprime, a faire par la session principale apres le vert)
- Technologies.Table `as many rows as the grid 11.1 has lines` -> python `as many rows as the grid 11.1 has lines`
- Table `%s of the grid 11.1 is in the table` / `tier of the grid` / `building of the grid` / `faction` -> python grille (is not in the table / tier / building / faction)
- Table `has a name to show`, `tier 1 or 2`, `hero level 0`, `building ... is a type ... or planned` -> python controles par ligne du meme nom
- Table `costs something`, `costs no negative amount`, `takes time`, `changes a statistic`, `statistic ... is one the effects code knows`, `changes by a positive percent` -> python (Cost / ResearchTime / Stats / Percent)
- Table `Implemented tells whether its statistics are applied by code` -> python Implemented vs CodedStats
- Table chaine (`asks for`, `required technology in the table`, `lower tier`, `same building`, `stronger`, `same statistics`) -> python chaine
- Technologies.Playable en entier (17 jouables, 4 Soon, attendus / inattendus, file de 5) -> python section 5 (IsPlayable ecrit en python : batiment existe ET Implemented, la regle de HOLTechnologyRules.h)
Limite : IsPlayable (HOLTechnology::IsPlayable) est recopie en python (2 criteres) : si la regle C++ change, le test python ne le voit pas ; un test d'accord n'a pas ete ajoute (regle du brief si deux copies : a decider si le Playable C++ est retire, la regle C++ reste testee par Rules ? NON : Rules ne teste pas IsPlayable, seul Playable le fait). Recommandation : garder le test C++ Playable tant qu'IsPlayable n'est pas script.

## Champs
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 ()
TESTS_SCRIPT_AJOUTES: 1 (technology_table_test.py, test python quick_check values)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~310 (Table + Playable de HOLTechnologyTableTests.cpp, hors Rules)
TYPES_A_EXPOSER: aucun
PROCESSUS_DETACHES: aucun lance
EDITS_REFUSES: 0
Placeholders.txt : relu en esprit, aucun provisoire ajoute ni retire. Merge main non fait.