namespace HOLManager;

/**
 * Writes the manager's log to <dir>\manager-<yyyy-MM-dd>.log, one file per day (R2.12): started by the Windows
 * startup shortcut, the manager's console is minimized and may be closed. Keeps the last KeptFiles days.
 */
public sealed class FileLoggerProvider : ILoggerProvider
{
    public const int KeptFiles = 14;
    private readonly object gate = new();
    private readonly string dir;
    private string? day;
    private StreamWriter? writer;

    public FileLoggerProvider(string dir)
    {
        this.dir = dir;
        Directory.CreateDirectory(dir);
    }

    public string CurrentFile => Path.Combine(dir, $"manager-{DateTime.Now:yyyy-MM-dd}.log");

    public ILogger CreateLogger(string categoryName) => new FileLogger(this, categoryName);

    internal void Write(string line)
    {
        lock (gate)
        {
            try
            {
                var today = DateTime.Now.ToString("yyyy-MM-dd");
                if (today != day)
                {
                    writer?.Dispose();
                    writer = new StreamWriter(new FileStream(CurrentFile, FileMode.Append, FileAccess.Write, FileShare.ReadWrite)) { AutoFlush = true };
                    day = today;
                    foreach (var old in Directory.GetFiles(dir, "manager-*.log").OrderDescending().Skip(KeptFiles))
                    {
                        File.Delete(old);
                    }
                }
                writer!.WriteLine(line);
            }
            catch (IOException)
            {
                // The log never stops the manager.
            }
        }
    }

    public void Dispose()
    {
        lock (gate)
        {
            writer?.Dispose();
            writer = null;
            day = null;
        }
    }

    private sealed class FileLogger(FileLoggerProvider provider, string category) : ILogger
    {
        public IDisposable? BeginScope<TState>(TState state) where TState : notnull => null;

        public bool IsEnabled(LogLevel logLevel) => logLevel != LogLevel.None;

        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception? exception,
            Func<TState, Exception?, string> formatter)
        {
            var line = $"{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff} {logLevel,-11} {category}: {formatter(state, exception)}";
            provider.Write(exception == null ? line : $"{line}{Environment.NewLine}{exception}");
        }
    }
}
