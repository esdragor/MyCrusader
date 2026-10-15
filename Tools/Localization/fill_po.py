"""Fills the English and Japanese translations of the Game localization target (D245, front-end step 5).

Provisional translations written by the assistant, keyed by "Namespace,Key" (the msgctxt of the .po files). The .po
files stay the reference a translator works in; this table only fills the msgstr of the keys it knows, and lists the
keys of the .po files it does not know (texts added since), so that none is forgotten.
Run, then gather again (Config/Localization/Game.ini) to import the .po files and compile the .locres:
  python Tools/Localization/fill_po.py
"""
import io
import os
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "Content", "Localization", "Game")

# "Namespace,Key": (English, Japanese)
TRANSLATIONS = {
    "HOLPlayerColors,Blue": ("Blue", "青"),
    "HOLPlayerColors,Red": ("Red", "赤"),
    "HOLPlayerColors,Green": ("Green", "緑"),
    "HOLPlayerColors,Yellow": ("Yellow", "黄"),
    "HOLPlayerColors,Teal": ("Teal", "ターコイズ"),
    "HOLPlayerColors,Purple": ("Purple", "紫"),
    "HOLPlayerColors,Orange": ("Orange", "オレンジ"),
    "HOLPlayerColors,Pink": ("Pink", "ピンク"),

    "HOLConfirmDialog,Yes": ("Yes", "はい"),
    "HOLConfirmDialog,No": ("No", "いいえ"),

    "HOLTitleScreen,Play": ("Play", "プレイ"),
    "HOLTitleScreen,Settings": ("Settings", "設定"),
    "HOLTitleScreen,Quit": ("Quit", "終了"),
    "HOLTitleScreen,QuitConfirm": ("Quit", "終了"),
    "HOLTitleScreen,QuitQuestion": ("Quit the game?", "ゲームを終了しますか？"),

    "HOLScreen,ColumnHint": ("LB / RB: change column", "LB / RB：列を切り替え"),

    "HOLNewGameScreen,Heading": ("New Game", "新しいゲーム"),
    "HOLNewGameScreen,Map_Plain": ("Plain", "平原"),
    "HOLNewGameScreen,Map": ("Map", "マップ"),
    "HOLNewGameScreen,Player": ("Player", "プレイヤー"),
    "HOLNewGameScreen,Slot": ("Slot", "スロット"),
    "HOLNewGameScreen,Faction": ("Faction", "勢力"),
    "HOLNewGameScreen,Colour": ("Color", "色"),
    "HOLNewGameScreen,Team": ("Team", "チーム"),
    "HOLNewGameScreen,PlayerN": ("Player {0}", "プレイヤー{0}"),
    "HOLNewGameScreen,You": ("You", "あなた"),
    "HOLNewGameScreen,Mode": ("Mode", "モード"),
    "HOLNewGameScreen,Standard": ("Standard", "スタンダード"),
    "HOLNewGameScreen,Nomad": ("Nomad", "ノマド"),
    "HOLNewGameScreen,Regicide": ("Regicide", "レジサイド"),
    "HOLNewGameScreen,Catastrophe": ("Catastrophe", "カタストロフィ"),
    "HOLNewGameScreen,Greyed": ("Greyed out: not in the game yet (AI, factions, teams, mode and options).",
                                "グレー表示：まだゲームにありません（AI、勢力、チーム、モード、オプション）。"),
    "HOLNewGameScreen,Back": ("Back", "戻る"),
    "HOLNewGameScreen,Launch": ("Start", "開始"),
    "HOLNewGameScreen,AI": ("AI", "AI"),
    "HOLNewGameScreen,Closed": ("Closed", "クローズ"),
    "HOLNewGameScreen,Human": ("Human", "人間"),
    "HOLNewGameScreen,DawnOrder": ("Order of the Dawn", "暁の騎士団"),
    "HOLNewGameScreen,BlackLegions": ("Black Legions", "黒の軍団"),
    "HOLNewGameScreen,DragonChildren": ("Children of the Dragon", "竜の子ら"),
    "HOLNewGameScreen,ShadowCircle": ("Circle of Shadow", "影の結社"),
    "HOLNewGameScreen,FireHeirs": ("Heirs of Fire", "炎の継承者"),
    "HOLNewGameScreen,Templars": ("Templars", "テンプル騎士団"),
    "HOLNewGameScreen,NoTeam": ("None", "なし"),
    "HOLNewGameScreen,Off": ("Off", "オフ"),
    "HOLNewGameScreen,On": ("On", "オン"),

    "HOLSettingsScreen,Heading": ("Settings", "設定"),
    "HOLSettingsScreen,Video": ("Video", "グラフィック"),
    "HOLSettingsScreen,Sound": ("Sound", "サウンド"),
    "HOLSettingsScreen,Controls": ("Controls", "操作"),
    "HOLSettingsScreen,Language": ("Language", "言語"),
    "HOLSettingsScreen,Back": ("Back", "戻る"),
    "HOLSettingsScreen,Resolution": ("Resolution", "解像度"),
    "HOLSettingsScreen,WindowMode": ("Display Mode", "表示モード"),
    "HOLSettingsScreen,Fullscreen": ("Fullscreen", "フルスクリーン"),
    "HOLSettingsScreen,WindowedFullscreen": ("Borderless Window", "ボーダーレス"),
    "HOLSettingsScreen,Windowed": ("Windowed", "ウィンドウ"),
    "HOLSettingsScreen,VSync": ("Vertical Sync", "垂直同期"),
    "HOLSettingsScreen,FrameRate": ("Frame Rate Limit", "フレームレート上限"),
    "HOLSettingsScreen,Unlimited": ("Unlimited", "無制限"),
    "HOLSettingsScreen,RenderScale": ("Render Scale", "レンダースケール"),
    "HOLSettingsScreen,Quality": ("Quality", "品質"),
    "HOLSettingsScreen,Custom": ("Custom", "カスタム"),
    "HOLSettingsScreen,Low": ("Low", "低"),
    "HOLSettingsScreen,Medium": ("Medium", "中"),
    "HOLSettingsScreen,High": ("High", "高"),
    "HOLSettingsScreen,Epic": ("Epic", "最高"),
    "HOLSettingsScreen,Cinematic": ("Cinematic", "シネマティック"),
    "HOLSettingsScreen,ViewDistance": ("View Distance", "描画距離"),
    "HOLSettingsScreen,AntiAliasing": ("Anti-Aliasing", "アンチエイリアス"),
    "HOLSettingsScreen,GlobalIllumination": ("Global Illumination", "グローバルイルミネーション"),
    "HOLSettingsScreen,Texture": ("Textures", "テクスチャ"),
    "HOLSettingsScreen,VisualEffect": ("Effects", "エフェクト"),
    "HOLSettingsScreen,Foliage": ("Foliage", "植生"),
    "HOLSettingsScreen,Shading": ("Shading", "シェーディング"),
    "HOLSettingsScreen,Landscape": ("Terrain", "地形"),
    "HOLSettingsScreen,MasterVolume": ("Master Volume", "マスター音量"),
    "HOLSettingsScreen,MusicVolume": ("Music", "音楽"),
    "HOLSettingsScreen,EffectsVolume": ("Effects", "効果音"),
    "HOLSettingsScreen,VoiceVolume": ("Voice", "ボイス"),
    "HOLSettingsScreen,SoundNote": ("Music, effects and voice will apply once the game has sounds.",
                                    "音楽・効果音・ボイスは、ゲームにサウンドが入ってから反映されます。"),
    "HOLSettingsScreen,Remap": ("Rebind Keys", "キー割り当て"),
    "HOLSettingsScreen,CameraSpeed": ("Camera Speed", "カメラ速度"),
    "HOLSettingsScreen,Slow": ("Slow", "遅い"),
    "HOLSettingsScreen,Normal": ("Normal", "普通"),
    "HOLSettingsScreen,Fast": ("Fast", "速い"),
    "HOLSettingsScreen,EdgeScroll": ("Edge Scrolling", "画面端スクロール"),
    "HOLSettingsScreen,RemapNote": ("Rebind keys: not in the game yet.", "キー割り当て：まだゲームにありません。"),
    "HOLSettingsScreen,LanguageLabel": ("Language", "言語"),
    "HOLSettingsScreen,Off": ("Off", "オフ"),
    "HOLSettingsScreen,On": ("On", "オン"),

    "HOLGameScreen,Food": ("Food", "食料"),
    "HOLGameScreen,Wood": ("Wood", "木材"),
    "HOLGameScreen,Stone": ("Stone", "石材"),
    "HOLGameScreen,Gold": ("Gold", "金"),
    "HOLGameScreen,Population": ("Population", "人口"),
    "HOLGameScreen,Health": ("Health", "耐久力"),
    "HOLGameScreen,You": ("You", "あなた"),
    "HOLGameScreen,PlayerN": ("Player {0}", "プレイヤー{0}"),
    "HOLGameScreen,PopulationFull": ("Population full", "人口上限"),
    "HOLGameScreen,CancelHint": ("Click: cancel", "クリック：キャンセル"),
    "HOLGameScreen,UnitCount": ("{0} × {1}", "{0} × {1}"),
    "HOLGameScreen,ActivityGathering": ("Gathering", "採集中"),
    "HOLGameScreen,ActivityMoving": ("Walking", "移動中"),
    "HOLGameScreen,ActivityIdle": ("Idle", "待機中"),
    "HOLGameScreen,ActivityWithLoad": ("{0} — {1} {2}/{3}", "{0} — {1} {2}/{3}"),

    "HOLGameNames,Peasant": ("Peasant", "農民"),
    "HOLGameNames,MainCentre": ("Main Center", "本拠地"),
}

CULTURES = {"en": 0, "ja": 1}


def po_escape(text):
    return text.replace("\\", "\\\\").replace("\"", "\\\"")


def fill(culture, column):
    path = os.path.join(ROOT, culture, "Game.po")
    with io.open(path, encoding="utf-8-sig") as po:
        lines = po.read().split("\n")
    unknown = []
    filled = 0
    context = None
    for index, line in enumerate(lines):
        if line.startswith("msgctxt "):
            context = line[len("msgctxt \""):-1]
        elif line.startswith("msgstr ") and context is not None:
            if context in TRANSLATIONS:
                lines[index] = "msgstr \"%s\"" % po_escape(TRANSLATIONS[context][column])
                filled += 1
            else:
                unknown.append(context)
            context = None
    with io.open(path, "w", encoding="utf-8-sig", newline="\n") as po:
        po.write("\n".join(lines))
    print("%s: %d texts filled, %d unknown keys%s" % (culture, filled, len(unknown),
          (": " + ", ".join(unknown)) if unknown else ""))
    return not unknown


ok = all([fill(culture, column) for culture, column in CULTURES.items()])
sys.exit(0 if ok else 1)
