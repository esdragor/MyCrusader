# [À RÉPONDRE] Séance d'essai du prototype (PT7) : combien de scénarios par séance ?

**Contexte.** PT7 a deux scénarios neufs (`playtest_hero_army` : le même combat avec puis sans le héros, questions 1 et 7 ; `playtest_hero_death` : mort et fenêtre de résurrection au vrai délai, questions 2 et 3). Les questions 4 (duel), 5 (progression) et 8 (volcan) réutilisent `duel_challenge`, `hero_xp` et `volcano` ; 6 et 9 se jugent en partie réelle. La page `Docs/Seance_Essai_Prototype.md` donne l'ordre.

**Question.** Combien d'essais dans une séance de scénarios avant les 3 parties ?
- **A : les 5 scénarios d'affilée (~15 min), puis les 3 parties (défaut écrit dans la page)**
- B : un scénario par question à la demande de l'utilisateur, sans ordre
- C : seulement `playtest_hero_army` et `playtest_hero_death` avant les parties, le duel, le volcan et la progression se jugent en partie

**Autre** : ton ordre. **Recommandation : A** (une seule chaîne, un seul questionnaire à remplir à la fin, rien d'oublié). Conséquence visible : la page de séance et l'ordre des commandes `play.ps1` à te donner.
**Défaut codé :** A, dans la page `Docs/Seance_Essai_Prototype.md` de la branche `ai-ia1` (une ligne à changer).

Reponse: A (5 scenarios d'affilee ~15 min, puis les 3 parties). 2026-11-05 20:38, utilisateur.
