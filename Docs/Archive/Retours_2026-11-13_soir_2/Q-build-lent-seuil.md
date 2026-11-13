# [À RÉPONDRE] « BUILD LENT » : sur quoi comparer la durée d'un build ? (L91, Faramir, 2026-11-13)

Contexte : `Tools\build.ps1` (branche `fog`, commit f4ace98) écrit maintenant « BUILD LENT » quand un build dépasse 3 fois la médiane des builds PASS du même type (`normal` / `merge`) de `Docs\BuildQueue.csv`. Le CSV ne compte pas les actions UBT, donc un vrai build C++ (beaucoup de fichiers) est comparé à des builds presque vides : sur le CSV actuel, 67 builds `normal` sur 326 (20 %) auraient été marqués (médiane 90 s, seuil 270 s), 6 `merge` sur 44.

- **A. Garder la version minimale** (3 x médiane par type) : en place, rien à faire ; un build sur cinq crie « BUILD LENT », le veilleur prévient souvent pour rien.
- **B. Compter les actions UBT** (lire `[n/N]` dans `Saved\Logs\build.log`, l'écrire dans la colonne `load`, comparer aux builds d'un nombre d'actions proche) : l'alerte ne vise que les builds vraiment lents pour leur taille ; ~30 min d'outil, sans C++.
- **C. Seuil fixe plus haut** (par exemple 3 x le 3e quartile, ~650 s pour `normal`) : une ligne à changer, peu d'alertes, mais un petit build bloqué 10 min passe inaperçu.
- **Autre** : à préciser.

Recommandation : **B** (c'est ce que dit la ligne L91 : « pour le même nombre d'actions ») ; défaut codé = A (médiane du même type), le facteur 3 est une ligne dans `Get-SlowVerdict` de `Tools\build.ps1`.

Reponse: B (compter les actions UBT `[n/N]` dans la colonne `load` et comparer aux builds d'un nombre d'actions proche). (réponse donnée dans la conversation, 2026-11-13 ~17:35)
