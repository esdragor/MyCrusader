# [À RÉPONDRE] Q-moral-attenuation : un effet de moral doit-il s'estomper peu à peu avant sa fin ?

Contexte : le GDD (D125, D706) ne connaît qu'une atténuation venant d'une SOURCE (le Moine Lumineux, Discipline de l'Aube) : elle raccourcit un effet temporaire ou affaiblit une aura, de 75 % au plus. Aucun texte ne dit qu'un effet pose à pleine force perde de la force tout seul avant sa fin ; aujourd'hui Triomphe, Démoralisé, Hésitation et le cri valent leur pleine force jusqu'à la dernière seconde, puis disparaissent d'un coup.

Options (effet en jeu) :
- **A. Pas d'estompe (GDD actuel).** Un Démoralisé de 40 s coûte -10 % pendant 40 s, puis rien. En jeu : lisible (la pastille dit la vérité jusqu'au bout), mais le coup de moral à la 39e seconde est aussi dur qu'à la première.
- **B. Estompe linéaire sur la dernière part de la durée** (par exemple les derniers 25 %). En jeu : la fin d'un malus se relâche en douceur, le joueur qui a tenu 30 s sent un répit ; mais la pastille doit montrer une force qui change, le pourcentage lu par le combat varie chaque pas, et les valeurs du GDD (-10, +10) ne sont plus celles qu'on voit à l'écran.

Recommandation : A (c'est le GDD ; l'atténuation par une source existe déjà dans la règle et couvre le besoin « le Moine rend le malus plus doux »).

Défaut codé : A. Codé dans ce lot : l'atténuation PAR SOURCE (règle `HOLMorale::Apply`, déjà là ; le processeur lui passe le pourcentage du camp, commande provisoire `hol.Morale.Attenuate <slot> <pourcent>` en attendant le Moine, A3). L'estompe B n'est pas codée.
