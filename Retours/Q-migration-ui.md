# [À RÉPONDRE] Migration : que faire des écrans et du HUD (étape 4) ?

**Contexte.** Toute la logique du jeu qui pouvait passer en script y est passée ce soir (règles, ponts du serveur, tests). Les écrans déjà
passés (menu de partie, fin de partie, boîte de confirmation) l'ont été sans changer leur aspect, mais **aucun test automatique ne juge
l'aspect** : seul un essai à l'écran, par vous, peut dire si un écran a gardé sa mise en page. Ce qui reste en C++ hors simulation est
surtout le HUD et les écrans (environ 5 000 à 6 000 lignes migrables), le contrôleur du joueur et la grille de commandes. Le reste du C++
(simulation, réseau, tests de simulation) reste en C++ par décision D838.

**Options.**
- **A. On s'arrête là pour l'interface.** Les écrans et le HUD restent en C++, la logique est déjà en script. Aucun risque visuel ; le C++
  restant continue d'exiger un build pour chaque changement d'écran.
- **B. On migre écran par écran, avec un essai de 3 minutes par vous à chaque fois.** Les écrans simples d'abord (titre, salon, options),
  un `play.ps1 <nom>` à jouer pour chacun, vous répondez OK ou BUG. Le HUD de jeu en dernier, par panneaux. Sûr, mais demande votre temps.
- **C. On migre tout sans contrôle visuel**, en comptant sur les tests de structure. Rapide, mais une régression de mise en page peut passer
  inaperçue jusqu'à votre prochain essai.

**Recommandation : B**, en commençant par 3 écrans.

**Défaut si vous ne répondez pas : A** (rien de plus n'est migré côté interface ; le reste de la migration continue : ports de tests et
suppression de doublons).

*Fichiers : Docs\Rapports\PLAN-1117a.md (inventaire des écrans par taille), Docs\Rapports\ANALYSE-1116a.md.*

Reponse:
