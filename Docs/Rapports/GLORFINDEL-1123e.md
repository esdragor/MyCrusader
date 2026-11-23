APPELS_DECLARES: 19
HEURE_DEBUT/HEURE_FIN: 09:41 / 09:55
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (script seul)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 738ed02 (Test_AIBrainCastsSacredLightWhenHonorAllows) assertit l'effet : non (decision de Plan sur le vrai livre Aube + regles reelles ; l'effet monde n'est pas rejoue) ; vu a 09:47 : tests 2102/2105   TEST_ROUGE_SEPARE: oui
COMMITS: 3 (738ed02 rouge, vert 9f43ed8, docs), dernier 09:55, merge_main_fait: non (consigne : fusionner seulement a915a1f)
TESTS: quick_check -Parts tests 2103/2105 (rouges attendus Test_CGTables, Test_WallTypesListsPalisade) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune ; valeurs par defaut : reserves 0/25/50/75, seuil blesse 70 % (HOLAIPowerCall::DefaultData / WoundedPercent)
FAIT: ThinkPowers dans HOLAIBrain.as (appel apres ThinkRage), HOLAIPowerCall::Plan, HOLFactionPowerServer::TryCastPower (RPC ServerCastFactionPower l'appelle apres son controle d'emetteur : pas de copie)
RESTE: aiserver/aiduel non rejoues ; Grep quick_check parties aiduel/aiserver (L22-3) non fait ; DIAGNOSTICS_AS non relus
NON VERIFIE: effet reel en partie (soin applique par l'IA), origine = banniere du heros (GetHeroBannerLocation) pour un slot IA, Health du FHOLAIUnit en meme unite que Figures.Health ; avertissement : le test ne passe pas par le monde
PROCESSUS_DETACHES: voir stray_processes
ANOMALIES_PROCESSUS: Cast est un mot reserve AS (1re version TryCast nommee Cast : echec compile, corrige) ; 1er edit au 8e appel ; sleep PowerShell utilise pour attendre quick_check
GIT_STATUS_PRINCIPALE: non verifie
