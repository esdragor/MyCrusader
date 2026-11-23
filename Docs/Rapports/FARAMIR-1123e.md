# FARAMIR-1123e (S1-e soin, zone-effect)
BUILD: PASS (1er build 10:10, 33 s ; 2e build apres CreditHonor). Rouge vu a 10:14:54 : HeroesOfLiberty.Zone.HealHonor Result=Fail (bouchon).
Commits : rouge 12dd617 (test reflexion + bouchon), vert 6b90b54 Honor from healing: CreditHonor credits the Dawn gauge (reflection on the private accessors), script installer, Aube journal. Pas de push, pas de merge main.
Cause du build rouge : AHOLGameState::GetFactionResource/SetFactionResource et FactionResource sont dans la section private: du .h (L429-607), le commentaire "public" ment. Visibilite inchangee ; lecture/ecriture par reflexion (FindFunction + ProcessEvent), comme le script, dans le test (ReadFactionGauge) et dans CreditHonor.
Tests : Zone.*/Heal.* 4 tests, 0 rouge ; tous : 2099 tests, 0 rouge ; erreurs Angelscript : 0. TESTS_TAUTOLOGIQUES: 0 (Honor attendu calcule depuis les PV soignes).
NON VERIFIE : RELECTURE_CPP (L20-2) non faite faute d'appels ; Honneur a la mort non fait ; pas d'essai en jeu.
Anomalie : HOLFactionResourceLibrary absent du worktree (le commit 1f7e15b le modifie mais ls-files ne le montre pas) ; dossier de reflexion a remplacer par un accesseur public si le lot C++ le permet.
