# [A REPONDRE] Lumiere sacree de l'IA : d'ou lance-t-elle, et a quelle portee ? (GLORFINDEL-1123h)

Contexte : l'IA de l'Aube ne plante jamais de banniere, donc son origine de lancer valait (0, 0) alors que son armee est a ~14700 cm : 255 decisions cast=false. Defaut code : sans banniere plantee, l'origine est le CENTRE DE SON ARMEE (HOLAIPowerCall::OriginOf) ; la portee reste celle du Sceau de Lumiere (800 cm, UnitAbilities.csv LightSeal) faute de portee propre a la Lumiere sacree.

Q1. Origine de lancer de l'IA sans banniere ?
- A (defaut code) : centre de son armee. Lance sur ses blesses proches du gros de la troupe.
- B : sa position de forum / de roi. Plus stable, mais les blesses au front sont souvent hors portee.
- C : l'IA plante une banniere comme le joueur (comportement neuf a ecrire).
- Autre.
Recommandation : A.

Q2. Portee de la Lumiere sacree (aujourd'hui 800 cm = celle du Sceau) ?
- A (defaut) : 800 cm. B : 1500 cm (pouvoir de zone, comme une aura de banniere). C : valeur propre dans la table des pouvoirs.
Recommandation : C, une ligne a part, avec ta valeur ; en attendant A.

Reglage : `HOLAIPowerCall::OriginOf` (une ligne) et `HOLFactionPowerCast::SealRangeCm()`.
