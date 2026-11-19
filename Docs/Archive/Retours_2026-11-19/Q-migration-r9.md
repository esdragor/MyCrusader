# [RÉPONDU 2026-11-16 21:2x, D836] Migration : où doit vivre chaque règle écrite en double ?

Reponse: l'utilisateur a répondu en direct à 6 questions de la session principale (texte complet : D836 du GDD). Écarts avec les recommandations ci-dessous : Palier d'unité suit le groupe « script source » (comme Camps, Relais, Sites) ; Garnison et Marché : script source ; Diplomatie : l'interface lit le C++. À archiver au prochain nettoyage.

**Contexte.** Quinze règles du jeu existent aujourd'hui en deux exemplaires : un en C++ (changer = attendre une compilation) et un en
script (changer = rien à attendre). On n'en veut plus qu'un par règle. Le bon choix dépend de qui se sert de la règle.

Détail technique : `Docs\Rapports\PLAN-1116-E3.md` (parties a et b). Réponds par trois lettres, par exemple « 1A 2B 3A ».

## Groupe 1 : les règles que la simulation applique à chaque unité

Terrain (marais, route), déploiement des engins, rage nécrotique, technologies, siège, diplomatie, palier des unités, XP d'exploration.

- **A. Le C++ garde la règle, le script la lit.** On supprime l'exemplaire en script. Pour 5 des 8 règles, aucune compilation :
  c'est fait en une séance. Pour les 3 autres (siège, diplomatie, rage), il faut le lot de compilation prévu en début de prochaine
  séance. Ensuite, changer une de ces règles demande toujours une compilation. Risque : faible.
- **B. On garde les deux exemplaires**, surveillés par des tests qui vérifient qu'ils disent la même chose. Aucune compilation
  maintenant. Mais chaque changement s'écrit deux fois, et un écart peut passer (on en a trouvé un vrai ce soir sur le palier du
  Guerrier damné).

Le script seul n'est pas proposé ici : il serait trop lent pour des milliers d'unités à chaque instant.

**Recommandation : A.**

## Groupe 2 : les règles que le serveur applique en continu, mais pas unité par unité

Camps neutres, relais, sites à tenir.

- **A. Le script devient la règle.** Ce sont des règles qu'on règle souvent : après la bascule, on les change sans compilation.
  Coût : un petit lot de compilation par règle (trois en tout), à faire une règle à la fois avec une mesure de vitesse du serveur
  avant et après. Risque : moyen. Un script cassé arrête le serveur (les tests le voient avant) ; si la mesure est mauvaise, la règle
  revient en C++.
- **B. Le C++ garde la règle.** On supprime l'exemplaire en script tout de suite, sans compilation. Aucun risque. Mais chaque réglage
  futur des camps, des relais ou des sites demandera une compilation, donc des séances plus lentes sur ces sujets.

**Recommandation : A**, en commençant par les camps neutres (la règle la moins sollicitée), puis les sites, puis les relais.

## Groupe 3 : les règles déclenchées par un clic du joueur ou par l'affichage

Chat, menu de jeu et bannière de fin, garnison, marché.

- **A. Le script devient la règle.** Le chat est presque fini (une compilation déjà faite ce soir). Les trois autres passent avec
  le lot de compilation du début de la prochaine séance. Ensuite, changer ces règles ou ces écrans ne demande plus de compilation.
  Risque : faible à moyen (le marché touche 6 fichiers).
- **B. Le C++ garde la règle.** Il faut quand même une compilation (pour que l'IA et l'interface en script puissent lire la règle),
  et le travail du soir sur le chat et le menu est en partie jeté. Chaque changement futur demande une compilation.

**Recommandation : A.**

## Si tu ne réponds pas

La prochaine séance de migration applique les recommandations (**1A 2A 3A**), dans cet ordre de prudence : d'abord ce qui ne demande
aucune compilation (groupe 1), puis le lot de compilation du début de séance (groupe 3), et pour le groupe 2 seulement les camps
neutres, avec la mesure. Rien n'est poussé, et chaque exemplaire supprimé reste récupérable dans git.

## Ce que cela change pour les décisions écrites

Ta réponse complète la réécriture de D483 et D484 (texte proposé dans le rapport, partie d). Une mesure manque encore ce soir :
savoir si une demande réseau peut s'écrire en script (essai du pilote du chat).
