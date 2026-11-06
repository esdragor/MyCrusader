# [À RÉPONDRE] Q-unit-stats : format des statistiques de l'unité sélectionnée (REGLAGE forge)

Contexte : le bloc de sélection d'une unité seule ne montrait que ses points de vie ; l'attaque (12 -> 13,2 avec Lames affûtées I) n'était visible nulle part. Il montre maintenant, sous les points de vie, l'attaque avec le bonus des technologies, l'armure (mêlée / distance), la portée et la vitesse. Le format n'est pas tranché au GDD (§ 7.1 liste seulement : points de vie, attaque, défense, vitesse, portée).

Options :
- A (défaut codé, hol.HUD.UnitStats 1) : une seule ligne : « Attaque 13,2 (+1,2)   ·   Armure 0 (mêlée) 4 (distance)   ·   Portée mêlée   ·   Vitesse 7 m/s ».
- B (hol.HUD.UnitStats 2) : une ligne par statistique, en colonne sous les points de vie (bloc plus haut).
- C : dans une infobulle au survol du nom de l'unité (bloc de sélection inchangé).
- Autre : ...

Conséquences visibles : A garde le bloc bas et lit d'un coup d'oeil mais s'étale sur 2 lignes dans la colonne étroite ; B est lisible mais agrandit le bloc de sélection de 4 lignes ; C n'ajoute rien à l'écran mais cache l'information derrière un survol (le problème posé).
Recommandation : A. Libellés (Attaque, Armure, Portée, Vitesse) tirés du vocabulaire du GDD (§ 7.1, technologies de la Forge) ; à confirmer. L'armure ne porte pas encore de bonus de technologie (Maille, Barde ne sont pas codées).
Réglage : `hol.HUD.UnitStats` (0 caché, 1 = A, 2 = B), une ligne à changer. Comparaison : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 forge_stats_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` (Entrée = option suivante).


Reponse: (utilisateur, via la session principale, 2026-11-06 11:53) « comme A, avec Atk, Armure CaC, Armure Dist, Armure Heros, vitesse ; et au survol on a le reste des stats ». Applique par Galadriel : ligne unique (hol.HUD.UnitStats 1) Attaque / Armure melee / Armure distance / Armure heros / Vitesse ; infobulle au survol du bloc de stats : Points de vie, Attaque de base, bonus des technologies, Portee, Armure siege. Armure heros : colonne HeroArmor de UnitTypes (0 pour la plupart des unites, 2 pour l'Homme d'armes, D345) : affichee toujours, meme a 0.
