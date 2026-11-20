[RÉPONDU : A, GDD D873] Ligne VirtuousKnight de UnitTypes.csv (D864 + D865, SAURON-1120pa)

Contexte : les colonnes `DamageRedirectPercent` / `DamageRedirectRadius` existent maintenant (table, fragment, CSV) mais aucune unité ne les porte : `Tools\Data\UnitTypes.csv` n'a pas de ligne Chevalier Vertueux (17 lignes, toutes à 0 / 0). Sans ligne, la passive est inactive en jeu. Les stats du Chevalier (PV, attaque, armures, coût, rayon, vision, tier, bâtiment) n'ont jamais été tranchées.

Options :
- A : ligne `VirtuousKnight` calquée sur `ManAtArms` (Caserne, palier 1, faction Dawn) avec 30 / 800 ; visible : un Chevalier achetable qui joue comme un Homme d'armes et redirige 30 % à 8 m.
- B : stats propres (infanterie lourde, tank) à fournir par l'utilisateur ; rien de visible tant que non fournies.
- C : pas de ligne pour l'instant, colonnes seules (état actuel).
- Autre : à préciser.

Recommandation : A (provisoire, réglable ensuite par la table, D345), à lister dans `Placeholders.txt` à l'ajout.

Valeur par défaut codée : C (aucune ligne ; colonnes à 0 sur toutes les unités = aucune redirection). Une ligne à changer dans le CSV (`30,800` sur la ligne du Chevalier) pour l'activer.

Reponse: A (utilisateur, 2026-11-20 ~15:5x, interface de choix) : ligne `VirtuousKnight` provisoire calquée sur `ManAtArms` (Caserne, palier 1, faction Dawn), redirection 30 % à 800 cm, listée dans `Placeholders.txt`. Enregistré : GDD D873 ; ligne `[ ]` en tête de « Sauron » dans `Auto_Journey_File.md`.
