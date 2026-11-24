using System.Globalization;

namespace HOLLauncher;

/** A display mode of the settings list: Mode null is "the game's own" (no argument). */
public sealed record DisplayChoice(string? Mode, string Text)
{
    public override string ToString() => Text;
}

/** A resolution of the settings list: Size null with Custom false is "the game's own"; Custom: the two typed sizes. */
public sealed record ResolutionChoice((int Width, int Height)? Size, string Text, bool Custom = false)
{
    public override string ToString() => Text;
}

/** A theme of the settings list (Themes.Choices). */
public sealed record ThemeChoice(string Choice, string Text)
{
    public override string ToString() => Text;
}

/** A language of the settings list, named in its own language. */
public sealed record LanguageChoice(string Code)
{
    public override string ToString() => Texts.LanguageName(Code);
}

/** An automatic update mode of the settings list (L23). */
public sealed record AutoUpdateChoice(AutoUpdateMode Mode, string Text)
{
    public override string ToString() => Text;
}

/**
 * What the settings window shows for the stored settings: the selected index of each list and the field texts. L23's
 * background fields have defaults (the stored defaults) so that older callers keep building them.
 */
public sealed record SettingsFields(
    int LanguageIndex,
    int ThemeIndex,
    string GameDir,
    int DisplayIndex,
    int ResolutionIndex,
    string Width,
    string Height,
    string ExtraArguments,
    bool CloseOnPlay,
    bool MinimizeToTray = false,
    int AutoUpdateIndex = 1,
    string CheckEveryMinutes = "30",
    bool StartWithWindows = false);

/** "Save": the settings to apply (Result), or why not (Problem, in the player's language). */
public sealed record SettingsOutcome(LauncherSettings? Result, string? Problem);

/**
 * The settings window's logic (L15, L19, L22, launcher 5; pure, tested), apart from the WPF window that only shows it:
 * the lists, what they show for the stored settings (Fields), and what "Save" makes of the fields (Build): the old
 * settings with these fields changed (later fields kept), or the problem that keeps the window open: a game folder that
 * is not a full path, an unclosed quote or extra arguments the launch would leave out (LaunchArguments.Rejected), a
 * custom size out of LaunchArguments' bounds. An unknown mode or an invalid stored size shows "the game's own", as the
 * launch treats them. CloseOnPlay is stored true or not at all (false is the default).
 * Background (L23): MinimizeToTray and StartWithWindows likewise true or not at all, AutoUpdate only when not "notify",
 * CheckEveryMinutes only when not 30 (a whole number of minutes within BackgroundUpdatePolicy's bounds, else the
 * problem); a stored interval out of bounds shows its bounded value.
 */
public static class SettingsModel
{
    /** The size shown in the custom fields when none is stored. */
    public static readonly (int Width, int Height) DefaultSize = (1920, 1080);

    public static IReadOnlyList<LanguageChoice> Languages() => Texts.Languages.Select(code => new LanguageChoice(code)).ToList();

    public static IReadOnlyList<ThemeChoice> ThemeChoices(string language)
        => Themes.Choices.Select(c => new ThemeChoice(c, Themes.ChoiceText(c, language))).ToList();

    public static IReadOnlyList<DisplayChoice> DisplayChoices(string language) =>
    [
        new(null, Texts.Get("SettingsGameOwn", language)),
        new(LaunchArguments.Windowed, Texts.Get("SettingsWindowed", language)),
        new(LaunchArguments.Fullscreen, Texts.Get("SettingsFullscreen", language)),
    ];

    /** "The game's own", the usual sizes, then "Custom" (last). */
    public static IReadOnlyList<ResolutionChoice> ResolutionChoices(string language)
    {
        var list = new List<ResolutionChoice> { new(null, Texts.Get("SettingsGameOwn", language)) };
        list.AddRange(LaunchArguments.CommonResolutions.Select(r => new ResolutionChoice(r, $"{r.Width} × {r.Height}")));
        list.Add(new ResolutionChoice(null, Texts.Get("SettingsResolutionCustom", language), Custom: true));
        return list;
    }

    /** The fields for the stored settings; currentGameDir: the folder in use (--game-dir may differ from the setting). */
    public static SettingsFields Fields(LauncherSettings settings, string currentGameDir, string language)
    {
        var languageIndex = Math.Max(0, Texts.Languages.ToList().IndexOf(language));
        var themeIndex = Math.Max(0, Themes.Choices.ToList().IndexOf(Themes.ChoiceOf(settings.Theme)));
        var displayIndex = Math.Max(0, DisplayChoices(language).ToList().FindIndex(
            c => c.Mode is not null && string.Equals(c.Mode, settings.DisplayMode, StringComparison.OrdinalIgnoreCase)));
        (int Width, int Height)? saved = settings is { ResolutionWidth: { } w, ResolutionHeight: { } h }
            && LaunchArguments.IsValidResolution(w, h) ? (w, h) : null;
        var resolutions = ResolutionChoices(language);
        var common = resolutions.ToList().FindIndex(c => !c.Custom && saved is not null && c.Size == saved);
        var resolutionIndex = saved is null ? 0 : common >= 0 ? common : resolutions.Count - 1;
        var (shownWidth, shownHeight) = saved ?? DefaultSize;
        var autoUpdateIndex = Math.Max(0, AutoUpdateModes.ToList().IndexOf(BackgroundUpdatePolicy.ModeOf(settings.AutoUpdate)));
        var minutes = (int)BackgroundUpdatePolicy.Interval(settings.CheckEveryMinutes).TotalMinutes;
        return new SettingsFields(languageIndex, themeIndex, currentGameDir, displayIndex, resolutionIndex,
            shownWidth.ToString(CultureInfo.InvariantCulture), shownHeight.ToString(CultureInfo.InvariantCulture),
            settings.ExtraArguments ?? "", settings.CloseOnPlay == true, settings.MinimizeToTray == true, autoUpdateIndex,
            minutes.ToString(CultureInfo.InvariantCulture), settings.StartWithWindows == true);
    }

    /** The automatic update modes in the settings list's order (L23). */
    public static readonly IReadOnlyList<AutoUpdateMode> AutoUpdateModes = [AutoUpdateMode.Off, AutoUpdateMode.Notify, AutoUpdateMode.Install];

    public static IReadOnlyList<AutoUpdateChoice> AutoUpdateChoices(string language) => AutoUpdateModes
        .Select(mode => new AutoUpdateChoice(mode, Texts.Get(mode switch
        {
            AutoUpdateMode.Off => "SettingsAutoUpdateOff",
            AutoUpdateMode.Install => "SettingsAutoUpdateInstall",
            _ => "SettingsAutoUpdateNotify",
        }, language)))
        .ToList();

    /** A typed interval in whole minutes within BackgroundUpdatePolicy's bounds, else null. */
    public static int? Minutes(string text)
        => int.TryParse(text.Trim(), NumberStyles.None, CultureInfo.InvariantCulture, out var value)
            && value >= BackgroundUpdatePolicy.MinMinutes && value <= BackgroundUpdatePolicy.MaxMinutes ? value : null;

    /** "Save" with these fields, the window's texts in language. */
    public static SettingsOutcome Build(LauncherSettings old, SettingsFields fields, string launcherDir, string language)
    {
        string? dir;
        try
        {
            dir = LauncherSettings.GameDirFor(fields.GameDir, launcherDir);
        }
        catch (ArgumentException e)
        {
            return new SettingsOutcome(null, Texts.FormatIn(language, "SettingsBadFolder", e.Message));
        }
        var extra = fields.ExtraArguments.Trim();
        if (LaunchArguments.HasUnclosedQuote(extra))
        {
            return new SettingsOutcome(null, Texts.Get("SettingsUnclosedQuote", language));
        }
        var resolution = ResolutionChoices(language)[Math.Clamp(fields.ResolutionIndex, 0, ResolutionChoices(language).Count - 1)];
        int? width = null, height = null;
        if (resolution.Custom)
        {
            if (Size(fields.Width) is not { } w || Size(fields.Height) is not { } h)
            {
                return new SettingsOutcome(null,
                    Texts.FormatIn(language, "SettingsBadSize", LaunchArguments.MinSize, LaunchArguments.MaxSize));
            }
            (width, height) = (w, h);
        }
        else if (resolution.Size is var (cw, ch))
        {
            (width, height) = (cw, ch);
        }
        if (Minutes(fields.CheckEveryMinutes) is not { } minutes)
        {
            return new SettingsOutcome(null, Texts.FormatIn(language, "SettingsBadInterval",
                BackgroundUpdatePolicy.MinMinutes, BackgroundUpdatePolicy.MaxMinutes));
        }
        var mode = AutoUpdateModes[Math.Clamp(fields.AutoUpdateIndex, 0, AutoUpdateModes.Count - 1)];
        var displays = DisplayChoices(language);
        var chosen = old with
        {
            Language = Texts.Languages[Math.Clamp(fields.LanguageIndex, 0, Texts.Languages.Count - 1)],
            GameDir = dir,
            DisplayMode = displays[Math.Clamp(fields.DisplayIndex, 0, displays.Count - 1)].Mode,
            ResolutionWidth = width,
            ResolutionHeight = height,
            ExtraArguments = extra == "" ? null : extra,
            Theme = Themes.SettingFor(Themes.Choices[Math.Clamp(fields.ThemeIndex, 0, Themes.Choices.Count - 1)]),
            CloseOnPlay = fields.CloseOnPlay ? true : null,
            MinimizeToTray = fields.MinimizeToTray ? true : null,
            AutoUpdate = BackgroundUpdatePolicy.SettingFor(mode),
            CheckEveryMinutes = minutes == BackgroundUpdatePolicy.DefaultMinutes ? null : minutes,
            StartWithWindows = fields.StartWithWindows ? true : null,
        };
        if (LaunchArguments.Rejected(chosen) is { Count: > 0 } rejected)
        {
            return new SettingsOutcome(null, Texts.FormatIn(language, "SettingsReservedArguments", string.Join(" ", rejected)));
        }
        return new SettingsOutcome(chosen, null);
    }

    /** A typed size within LaunchArguments' bounds, else null. */
    public static int? Size(string text)
        => int.TryParse(text.Trim(), NumberStyles.None, CultureInfo.InvariantCulture, out var value)
            && value >= LaunchArguments.MinSize && value <= LaunchArguments.MaxSize ? value : null;
}
