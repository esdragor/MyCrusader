# [RÉPONDU 2026-11-12] Infobulle de revenu (+N/min) : quel calcul pour les Paysans ?

Contexte : ta réponse à Q-sites-gold-line (AUTRE) est codée dans Glorfindel (branche net-playtest) : au survol d'une ressource de la barre du haut (montant ou « (3) »), une infobulle « +N nourriture/min », « +N or/min »... = tous les revenus de cette ressource (Paysans dessus + sites sacrés tenus pour l'or). Reste à choisir comment compter les Paysans.

Options :
- **A (codé par défaut)** : chaque Paysan compté à son taux À LA SOURCE (taux du Paysan x taux de la source, ferme 0,55, x technologies et Sanctuaire), que la source soit atteinte ou qu'il marche vers le dépôt. Chiffre stable, réagit tout de suite à un ordre, mais AU-DESSUS du réel (les allers-retours au dépôt ne sont pas comptés : environ 1,5 à 2 fois trop si le dépôt est loin).
- B : le revenu MESURÉ : ce qui a vraiment été déposé dans les 60 dernières secondes (façon compteur). Exact, compte les trajets, mais en retard d'une minute après un changement et à 0 en début de partie.
- C : A corrigé des trajets (estimation par la distance à la source du dépôt le plus proche, comme l'IA, `HOLAINeeds::NetRate`). Proche du réel et immédiat, mais une estimation (un dépôt bloqué ou une foule faussent le chiffre).
- Autre.

Conséquences visibles : avec A, 10 Paysans au bois loin du camp affichent par exemple +600 bois/min alors que le stock monte d'environ +350/min ; B et C affichent un chiffre proche des +350.

Recommandation : C pour le prototype (immédiat et honnête) ; B si tu veux un chiffre exact, quitte au retard.

Valeur par défaut : A (`UHOLGatherProcessor::CountGatherer` additionne `GetGatherRate` par Paysan compté ; une fonction à changer). Textes : « +{0} {1}/min » ({1} = les mots du tribut : nourriture, bois, pierre, or ; anglais et japonais PROVISOIRES dans fill_po.py).

Reponse: C (taux corrigé des trajets, estimation par la distance au dépôt, comme HOLAINeeds::NetRate ; à écrire dans UHOLGatherProcessor) - 2026-11-12
