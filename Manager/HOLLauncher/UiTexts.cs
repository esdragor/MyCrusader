using System.ComponentModel;
using System.Windows.Data;
using System.Windows.Markup;

namespace HOLLauncher;

/**
 * The launcher's texts for XAML (launcher 5): {local:Tr Name} binds to Texts.Get(Name) in the language in use, and
 * Refresh (after a language change) updates every bound text of the open windows at once. The XAML holds no player text
 * of its own (NoFrenchInCodeTests reads the .xaml files too).
 */
public sealed class UiTexts : INotifyPropertyChanged
{
    public static readonly UiTexts Instance = new();

    public event PropertyChangedEventHandler? PropertyChanged;

    public string this[string name] => Texts.Get(name);

    /** Every binding of the indexer reads its text again. */
    public void Refresh() => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(Binding.IndexerName));
}

/** {local:Tr Name}: a one-way binding to UiTexts.Instance[Name]. */
[MarkupExtensionReturnType(typeof(object))]
public sealed class TrExtension : MarkupExtension
{
    public TrExtension()
    {
    }

    public TrExtension(string name) => Name = name;

    [ConstructorArgument("name")]
    public string Name { get; set; } = "";

    public override object ProvideValue(IServiceProvider serviceProvider)
        => new Binding($"[{Name}]") { Source = UiTexts.Instance, Mode = BindingMode.OneWay }.ProvideValue(serviceProvider);
}
