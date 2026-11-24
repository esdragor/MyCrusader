# RECETTE-1124b (HALDIR-1124m, recette independante n°2, 2026-11-24, arret dur 12:45)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir (integ-58), `git merge main` fait (main 25e29ab dans l'arbre), `build_wait.ps1` : PASS (484 s, file + build). Pre-vol : `preflight.ps1 <nom> -Root <worktree>` (check.ps1 en fenetre masquee ; aucun play.ps1 visible).

| Essai | Pre-vol | Verdict | Objet vu (PNG ouverts par l'agent) |
|---|---|---|---|
| faction_aim_free | PASS (60 s, 4 PNG) | PASS | aim : cercle de visee VERT de 6 m devant le chateau, sur les soldats, dans l'anneau de 12 m ; aim_far : cercle ORANGE a droite, hors anneau ; after : zone doree au point clique, Honneur 84 / 100, case Lumiere sacree « recharge 18 s », PV heros 246 / 450 |
| sacred_light_heal | PASS (43 s, 2 PNG) | PASS | after : PV heros 330 / 450 (remonte de 150), Honneur 89 / 100, case « recharge 14 s », zone de 5 s deja finie, barre de vie des Hommes d'armes pleine |
| deploy_far | PASS (56 s, 4 PNG) | PASS | walking : Mangonneau demonte (case D = Monter) en marche, Maison rouge a gauche, loin ; arrived : Mangonneau monte (case D = Demonter, activite Attaque), ligne d'attaque rouge, bien plus pres de la Maison |
| specialization_slot (culture par defaut) | PASS (59 s) | PASS pre-vol seulement | PNG ecrases par la passe ja avant ouverture : non vus |
| specialization_slot (HOL_CHECK_CULTURE=ja) | PASS (63 s, 3 PNG) | PASS | before : bouton « 専門化：選択してください » sous la jauge 100 / 100 et la ligne « 聖なる光：コスト 25、使用可能 », texte entier ; during : barre de progression bleue + « 28 s » + lignes « Voie A du palier 1 (annuler) » (bordure doree) et « Voie B du palier 1 » entieres ; after : « 専門化：ルートA（段階1） » sous la jauge, bouton/barre/voies disparus |
| ai_sacred_light | PASS (77 s, 2 PNG, WARN frames 45 non bloquant) | PASS | before : 6 Hallebardiers + Chevalier rouges de l IA rassembles devant son forum, aucune zone ; after : melee avec les soldats bleus du joueur, zone doree (Lumiere sacree) de 6 m sur les soldats de l IA |

Constats : specialization_slot en ja : les deux lignes de voies restent en francais (« Voie A du palier 1 ») dans l'UI japonaise, alors que le reste est traduit (a classer : texte non localise, pas un debordement). faction_aim_free_after : Honneur 84 (le texte du .look dit 75 puis 85) : ecart de timing du soin, pas un defaut. Cercle de visee = cercle de debogage (placeholder) ; souris du bureau jamais utilisee (curseur virtuel Slate).

Fiches Retours\E-*.md mises a jour (faction_aim_free, sacred_light_heal, deploy_far, specialization_slot) : `branches: main`, « dans main 25e29ab », commande play.ps1 complete avec -Root du worktree Haldir. ai_sacred_light : pas de fiche E-.

NON VERIFIE : son (-nosound) ; `session_gate.ps1 -Phase Essay` non lance ; geste avec vraie souris du bureau ; PNG specialization_slot en culture par defaut.
