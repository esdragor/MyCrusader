# FARAMIR-1121sd

APPELS_DECLARES: 7 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 19:33 / ~19:36 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (tache bloquee : fichier interdit) ; assertit l'effet : sans objet
COMMITS: 0, merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (pas de correctif, preflight non relance), pas de E-talents_heal.md
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun
FAIT / RESTE:
- BLOQUE : HeroPanel ET FactionPanel sont tous deux crees et places dans Script\UI\HOLGameScreenScript.as, fichier INTERDIT par l'enonce (aucun autre .as de Script\UI ne les cite). Rien code, rien commite.
- Cause localisee (script, pas du Slate C++) : HOLGameScreenScript.as:47 `const float FactionSlotRise = 150.0;` ; :2005-2010 FactionSlotPlace() = GetBottomLeftPlace() avec Bottom += 150 (decalage FIXE). Le HeroPanel (l.1170-1178, ancre bas-gauche, padding GetBottomLeftPlace()) grandit vers le haut quand ShowTalents (l.2061) ajoute la rangee d'icones de talents + « Talents pris (N) » ; au-dela de 150 px de haut il recouvre la jauge SacredLight du FactionPanel (l.1989-1996, meme ancre bas-gauche).
- Modele deja present dans le meme fichier : l.814-823 (panneau diplomatie) lit la hauteur reelle du HeroPanel via GetWidgetBox(HeroPanel, HeroLow, HeroHigh) et se cale sur HeroLow.Y. Le HUD-rect check est l.2667 (message « HUD overlap »).
- Correctif propose (a faire par qui a le droit de toucher ce fichier) : dans FactionSlotPlace(), `FVector2D HeroLow, HeroHigh; if (GetWidgetBox(HeroPanel, HeroLow, HeroHigh)) Place.Bottom = Math::Max(Place.Bottom + FactionSlotRise, Room.Y - HeroLow.Y + 8.0);` (Room = taille ecran comme l.816), et rappeler RefreshFactionSlot apres ShowTalents / quand la hauteur du HeroPanel change (le geometry n'est valide qu'apres un tick). Test rouge propose : un test script qui prend N=2..4 talents sur le heros, lit GetWidgetBox des deux panneaux et assertit que les rectangles ne se recouvrent pas (le preflight talents_heal le voit deja : FAIL 2 « HUD overlap HeroPanel x FactionPanel », log + HUD rects).
- Regle GDD sur le HUD : non trouvee par grep (HeroPanel / FactionPanel absents du GDD.md) ; a citer par la session principale si elle existe sous un autre nom.
- RESTE : reaffecter la tache avec HOLGameScreenScript.as autorise, puis relancer preflight talents_heal et creer Retours\E-talents_heal.md.
NON VERIFIE: le correctif propose (non compile, non joue) ; que GetWidgetBox renvoie une geometrie valide a l'instant de RefreshFactionSlot ; talents_heal_base / _boosted non ouverts.
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune ; DIAGNOSTICS_AS: sans objet (aucun .as touche)
GIT_STATUS_PRINCIPALE: inchange par moi : oui
