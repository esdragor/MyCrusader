# [À RÉPONDRE] Rage nécrotique : le plafond du moral coupe les +40 % de dégâts

**Contexte.** Q2 = A de L4 fixe la rage à +40 % de dégâts et +25 % de cadence, posée comme un effet de moral (`Effects.csv`, ligne `NecroticRage`). Or `HOLMoraleRules.as` plafonne les bonus de moral à 30 % sur les dégâts portés et sur la cadence (50 % sur les dégâts reçus) : la rage seule donnerait donc +30 % de dégâts, pas +40 %. La cadence (+25 %) passe sous le plafond.

| Option | Ce qui change | Conséquence visible |
|---|---|---|
| **A (recommandée)** | plafond des dégâts portés relevé de 30 à 40 % (une constante de `HOLMoraleRules.as`) | la rage vaut vraiment +40 % ; un Guerrier enragé sous Triomphe (+10 %) reste à 40 %, jamais plus ; les autres effets de moral ne bougent pas aujourd'hui (aucun n'atteint 30 %) |
| B | la rage passe à +30 % de dégâts dans `Effects.csv` | plafond du moral intact (D704) ; le Guerrier enragé frappe moins fort que ce que tu as validé |
| C | la rage ne passe pas par le moral : un levier à elle, hors plafond | +40 % exact, mais un second système de bonus à tenir en même temps que le moral ; plus de code |
| Autre | tes chiffres | |

**Recommandation : A.** Le plafond de 30 % était « à régler à l'essai » (D704) ; aucun effet existant ne le touche, et la valeur de la rage est celle que tu as choisie.

**Valeur par défaut codée (en attendant).** Rien n'est changé : la ligne de `Effects.csv` porte les +40 % de ta réponse, le plafond du moral reste à 30 % (effet réel +30 %). Passer à A = une ligne dans `Script\Morale\HOLMoraleRules.as` (la fonction de plafond par levier, ligne 32 et 38) ; passer à B = le chiffre `40` de la ligne `NecroticRage` de `Tools\Data\Effects.csv`, puis `python Tools\Data\make_effects.py`.

Chantier : Légions L4, `LegionsInProgress.md`.

Reponse: A (plafond des degats portes releve de 30 a 40 %, HOLMoraleRules.as) - tout en recommande, utilisateur, 2026-11-11 soir
