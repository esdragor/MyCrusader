# [À RÉPONDRE] Arquebusier : quel modèle et quelles poses (planche de dix échantillons, tour 1)

**Contexte.** L'Arquebusier porte aujourd'hui le mannequin et l'arbalète de l'Arbalétrier (Placeholders.txt). Planche hors jeu de dix candidats étiquetés A à J, chacun vu de face, de côté et d'en haut (vue RTS) : `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved\AnimPreview\arquebus_sheet.png` (outil `Tools\Editor\arquebus_sheet.py`, branche `visual` ; rien n'est dans le jeu).

**Ce que montre la planche** (les gestes sont ceux de l'Arbalétrier, réutilisés tels quels) :
- A arquebuse 110 cm au repos ; B mousquet 150 cm au repos ; C arquebuse + bandoulière de charges (« douze apôtres ») et poire à poudre ; D mousquet + bandoulière.
- E arquebuse en marche ; F mousquet + bandoulière en marche.
- G arquebuse au tir + petite fumée ; H mousquet au tir + grosse fumée.
- I rechargement = le rechargement à l'étrier de l'arbalète (l'arme pointe au sol : faux pour une arme à feu, montré pour le dire) ; J mort = la chute de l'Arbalétrier.

**Question 1 : dis « aime / pas aime » par lettre (A à J).**

**Question 2 : quelle voie pour la suite ?**
- **A** — Arquebuse courte (A ou C) sur les gestes de l'Arbalétrier pour le repos, la marche et la mort ; deux gestes NEUFS seulement : le tir (recul + fumée) et le rechargement à la baguette. En jeu : silhouette proche de l'Arbalétrier, arme pas plus longue que la lance du Lancier ; le moins de travail.
- **B** — Mousquet long (B ou D) sur les mêmes gestes, même travail que A. En jeu : se distingue mieux de l'Arbalétrier vu d'en haut, mais l'arme dépasse loin devant lui dans les rangs serrés.
- **C** — Tout neuf : arme sur l'épaule en marche, fourquine (fourche d'appui) au tir, baguette, chute qui lâche l'arme. En jeu : le plus lisible et le plus « XVIe siècle », trois fois plus de gestes à faire et à valider.
- **Autre** — à préciser (casque morion, couleur du joueur sur la bandoulière, etc.).

**Recommandation :** A avec la bandoulière (C) et la petite fumée (G) : lisible à la hauteur de la caméra RTS, peu de gestes neufs ; I montre que le rechargement de l'arbalète ne convient pas et doit être refait.

**Défaut en attendant :** rien ne change dans le jeu (l'Arquebusier garde le corps et les gestes de l'Arbalétrier, la ligne de Placeholders.txt reste).

Reponse: pas de choix encore : l'utilisateur demande un GIF animé (poses en mouvement : marche, tir, rechargement, mort ; plusieurs angles) pour juger les candidats ; Q1 et Q2 restent ouvertes jusqu'au GIF. (réponse donnée dans la conversation, 2026-11-13 ~17:15)
GIF : voie A (arquebuse + bandoulière C + petite fumée G) `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved\AnimPreview\arquebus_way_A.gif` ; voie B (mousquet + bandoulière D + grosse fumée H) `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved\AnimPreview\arquebus_way_B.gif` (repos, marche x2, visée, tir + recul + fumée, rechargement, mort ; face / profil / vue RTS / caméra du jeu de dos à 55° ; ralenti x2 ; le rechargement est celui de l'étrier d'arbalète, étiqueté en rouge « FAUX pour une arme à feu ») ; images clés : `arquebus_way_A_keys.png`, `arquebus_way_B_keys.png` (même dossier) ; voie C sans GIF (ses gestes n'existent pas encore). Outil `Tools\Editor\arquebus_gif.py` (branche `visual`).
Reponse (2026-11-14) : Q1 « il n'y a pas de lettres » (les GIF n'étiquettent aucun candidat A à J : impossible de répondre lettre par lettre) ; Q2 « les deux GIF montrent les animations de l'Arbalétrier, aucune nouvelle animation » (la voie A et la voie B du GIF réutilisent les gestes de l'Arbalétrier : rien de neuf à juger). AUCUN choix, Q1 et Q2 restent OUVERTES. (réponse donnée dans la conversation, 2026-11-14) Suite : ligne [ ] pour Sauron (vrais gestes neufs de tir et de rechargement, hors jeu, puis nouveaux GIF avec la lettre visible à l'écran).

Suite (2026-11-14, SAURON-1114c, gestes NEUFS hors jeu, voie A seulement) : GIF avec la LETTRE du candidat a l'ecran et ce qui est neuf (vert) ou garde de l'Arbaletrier (orange) : `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved\AnimPreview\arquebus_new_A.gif` (images cles : `arquebus_new_A_keys.png`, meme dossier). C = repos arquebuse sur l'epaule + bandouliere (NEUF), E = marche arme sur l'epaule (NEUF), G = tir + recul violent + fumee (NEUF ; la mise en joue reste celle de l'Arbaletrier), I' = rechargement a la baguette en six temps (NEUF, remplace le I de la planche), J = mort (chute de l'Arbaletrier, gardee). Pas de GIF pour la voie B (le mousquet de 150 cm demande une baguette tiree en deux prises) ni la voie C. Outils : `Tools\Editor\arquebus_poses.py`, `arquebus_new_gif.py` (branche `visual`, non commites).
**Question 1 (a reposer) : dis « aime / pas aime » pour C, E, G, I', J.**
**Question 2 (a reposer) : voie A (arquebuse + ces gestes), B, C ou autre ?**
Defaut en attendant : rien ne change dans le jeu.

Reponse (2026-11-14, apres le GIF `arquebus_new_A.gif`) : « ok, parfait, je valide » : VOIE A validee avec les gestes neufs du GIF (C repos arme sur l'epaule + bandouliere, E marche, G tir + recul + fumee, I' rechargement a la baguette) ; la mort J reste la chute de l'Arbaletrier (vue dans le GIF, pas de reserve). Q1 et Q2 REPONDUES (pas de lettre par lettre : validation en bloc). (reponse donnee dans la conversation, 2026-11-14) Suite : cuisson et jeu (modele propre de l'Arquebusier, baker, UnitTypes.csv, ligne Placeholders.txt a retirer a la fin) ; archivage de cette fiche et numero GDD (D797) par la session principale.
