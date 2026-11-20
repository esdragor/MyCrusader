# [À RÉPONDRE] Moteur de moral : cinq choix groupés (M3 à M7)

Contexte : le moteur de moral n'a aucun C++ (`MoraleInProgress.md` M3-M5, `Docs\Rapports\PLAN-MORAL-1120.md`). Le GDD tranche les effets et les plafonds (D704-D709) mais pas ces cinq points ; le premier bloque l'écriture de M3. Réponse attendue : une lettre par ligne (ou « défaut partout »). Valeur par défaut codée = la recommandation.

**Q1 Source unique du moteur (bloque M3).**
- A (recommandé) : le C++ est la seule source (la règle est appelée par unité et par pas, D836 (1)/(5)) ; `HOLMoraleRules.as` devient un habillage du pont, ses 15 tests restent. En jeu : rien de visible ; une seule règle à maintenir.
- B : copie C++ + test d'accord comme D708 prévoyait avant D839 ; deux sources à tenir, un test d'accord de plus.
- Autre.

**Q2 Lieu du duel (Triomphe, Démoralisé ; D709 dit « lieu du duel »).**
- A (recommandé) : le centre de l'arène du duel ; *Hésitation* : autour du héros qui refuse.
- B : le milieu des deux héros au moment de la fin.
- C : la position du héros vainqueur (Triomphe) et du héros vaincu (Démoralisé), chacun autour de lui.
En jeu : A et B donnent le même cercle de 12 m aux deux camps ; C fait deux cercles, plus dispersés.

**Q3 Levier « dégâts infligés ».** A (recommandé) : additif avec le pourcentage d'attaque des technologies et de la rage (comme `RagePercent`). B : multiplicatif après l'armure. En jeu : A donne +10 % simple ; B composerait avec les technologies.

**Q4 Rage nécrotique (+40 % dégâts, +25 % cadence, 10 s, déjà dans `Effects.csv`).** A (recommandé) : ses leviers passent au moteur, coût en PV / recharge / arrêt à 20 PV restent à part. B : elle reste entièrement hors du moteur (la ligne de la table ne sert à rien). En jeu : aucun changement visible pour A ; B laisse deux chemins pour la cadence.

**Q5 Cadence.** A (recommandé) : intervalle × 100 / (100 + bonus) en entiers (comme `GetRageIntervalMs`). B : intervalle × (100 − bonus) / 100 (un bonus de 50 % ferait une attaque instantanée à 100 %, plafonné à 30 % par le plafond actuel).
