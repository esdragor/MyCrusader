# ELROND-1117g : P1d, pourquoi la sonde RPC script ne naît pas sur le serveur dédié (diagnostic par lecture, 2026-11-17)

Aucun build, aucune exécution, aucun commit. Lecture du plugin Angelscript (C:\UE\UE_5.8_HOL\Engine\Plugins\Angelscript\Source) et du journal du serveur de la 2e passe (%TEMP%\rpc_probe\rpc2_server.log, 09:42-09:45).

## Cause la plus probable : le dossier `Script\Dev\` n'est pas chargé hors éditeur interactif (hypothèse (a), confirmée par le code)

Le fichier de la sonde est dans un dossier nommé `Dev` : `Script\Dev\HOLScriptRpcProbe.as` (seul fichier de `Script\Dev`, seul dossier `Dev`/`Editor`/`Examples`/`Developers` de `Script\`).

Preuve dans le plugin :
- `AngelscriptManager.cpp:855-856` : `bSkipDevelopmentScripts = !FAngelscriptManager::bUseEditorScripts; bSkipEditorScripts = bSkipDevelopmentScripts;`
- `AngelscriptManager.cpp:825-835` (`FindScriptFiles`) : si `bSkipDevelopmentScripts`, tout sous-dossier nommé `Examples`, `Dev` ou `Developers` est ignoré ; si `bSkipEditorScripts`, `Editor` aussi. Le fichier n'est donc jamais lu, jamais compilé, aucune classe n'est créée : cela explique à la fois « classe introuvable par nom » ET « introuvable par IsChildOf d'une base C++ » ET « aucune erreur, aucun chargement dans le journal ».
- `AngelscriptManager.cpp:321-323` : `bUseEditorScripts = WITH_EDITOR && ((GIsEditor && !IsRunningCommandlet()) || bForcePreprocessEditorCode) && !bSimulateCooked`. Le serveur est lancé par `UnrealEditor.exe ... -server` (rpc_probe_run.ps1:12) : WITH_EDITOR vrai mais `GIsEditor` FAUX (et les clients `-game` aussi) donc `bUseEditorScripts` = faux, le dossier `Dev` est sauté. L'éditeur interactif (GIsEditor vrai) le charge, d'où « le script compile côté éditeur ».

Preuve dans le journal du serveur : `rpc2_server.log:1371` `Angelscript root path: ...HeroesOfLiberty-Elrond/Script` puis compilation complète sans erreur (lignes 1372-1625) mais aucune ligne ne cite `Script/Dev/` ; 1827 et 1845 : `RpcProbe: no script class derived from AHOLScriptRpcProbeActor on this server` (le pont s'exécute, `FindScriptClass` renvoie nullptr). Le fichier existait (mtime 09:41, serveur 09:42:36) et la DLL C++ avec la base `AHOLScriptRpcProbeActor` datait de 09:42.

## Hypothèses écartées (par lecture)
- (b) ordre d'initialisation : la compilation et le « class generator reload » finissent à 08:42:53 (UTC) ; le monde monte à 08:42:56 et PostLogin à 08:43:11, donc le module script est prêt bien avant PostLogin. `FindScriptClass` (UClass par IsChildOf) est correcte ; la base est `Abstract` mais le drapeau n'est pas hérité par le script (`AngelscriptClassGenerator.cpp:3182`, seul `CLASS_ScriptInherit` l'est ; `3258` ne le pose que si le script déclare `abstract`).
- (c) chemin de roots : bon (journal 1371, root = `Script` du worktree Elrond, 188 ms de compilation de tous les autres fichiers).
- Précompilation / cooked : `bUsePrecompiledData` est faux (`AngelscriptManager.cpp:362-363`, WITH_EDITOR vrai), le serveur compile bien depuis les sources.

## 2 expériences à faire ensuite (pour la session principale, sans build : script seul)

1. Sans rien déplacer, forcer le dossier `Dev` : relancer le lanceur avec `-as-force-preprocess-editor-code` ajouté aux 3 lignes de commande (serveur et 2 clients) de `Tools\Scenarios\rpc_probe_run.ps1` (le paramètre rend `bUseEditorScripts` vrai, `AngelscriptManager.cpp:310,322`) :
   `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\rpc_probe_run.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
   (après avoir ajouté le drapeau). Attendu : `server spawned: True` ; confirme la cause. À noter : le drapeau compile aussi le code `#if EDITOR`, donc ce n'est qu'un test, pas le correctif.
2. Déplacer le fichier hors du dossier exclu, DANS le worktree Elrond, puis rejouer sans drapeau :
   `git -C C:\Users\baudo\Documents\HeroesOfLiberty-Elrond mv Script/Dev/HOLScriptRpcProbe.as Script/Net/HOLScriptRpcProbe.as` (puis la même commande rpc_probe_run.ps1). Attendu : `server up`, `server spawned: True`, puis le rouge (`RED CHECK ... = False`) et les deux verts (`green server received`, `green client pong`). Vérifier d'abord le chemin d'aucun autre fichier dépendant de `Script\Dev` (aucun autre fichier dedans).

## Correctif proposé (aucun design tranché)
Ne plus mettre de code qui doit tourner sur le serveur ou le client dans un dossier nommé `Dev`, `Developers`, `Examples` ou `Editor` de `Script\` : déplacer la sonde (nom de dossier au choix de la session principale, par exemple `Script\Net\`). Si la sonde doit rester « jetable », la règle utile à noter dans CLAUDE.md/AngelscriptInProgress.md est que ces quatre noms de dossier sont réservés à du code de l'éditeur interactif seulement (`-game` et `-server` de `UnrealEditor.exe` ne les chargent pas). Le pont C++ et la base C++ n'ont pas besoin de changer ; le commentaire de HOLScriptRpcProbeActor.h (« looking up a script class by name found nothing ») est à corriger après la preuve.

## Hors sujet utile
Même piège possible pour tout futur `Script\*\Dev` ou `Script\Editor` ; tout test script déjà sous `Script\Tests\` n'est pas concerné.

NON VERIFIE : rien n'a été exécuté ; que l'acteur script se réplique avec Iris et que l'Owner donné à la création permette le Server RPC (c'est ce que la sonde doit prouver une fois chargée).
