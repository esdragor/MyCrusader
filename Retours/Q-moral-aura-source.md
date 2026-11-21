# [À RÉPONDRE] Qui pose l'Aura de l'Aube : tout héros, ou seulement le héros de l'Aube ?

Contexte : `Effects.csv` donne à DawnAura la source « Hero », 12 m, 15 % de dégâts reçus en moins, au propriétaire seul (D705) ; le GDD ne dit pas si l'aura est celle de TOUT héros ou propre à la faction de l'Aube. Le processeur de moral (`HOLMoraleProcessor.cpp`, 0498e99) pose l'aura depuis tout héros vivant (`HOLIsHero`), sur les unités du même camp dans 12 m, hors Paysans ; le héros se couvre lui-même.

- A : tout héros vivant pose l'aura (défaut codé).
- B : seulement les héros de l'Aube (un test de faction à ajouter ; les autres héros ne donnent rien).
- C : tout héros, mais le héros ne se couvre pas lui-même.
- Autre.

Conséquence visible : A = un camp de Légions avec son héros reçoit aussi 15 % de dégâts en moins près de lui ; B = seul le camp de l'Aube.
Recommandation : B si l'aura est un trait de l'Aube (la section Aube du GDD la lie au « Paladin-Commandant »), sinon A.
Valeur par défaut codée : A (une ligne à changer dans `UHOLMoraleProcessor::Execute`, le test `bHero` de la première passe).
