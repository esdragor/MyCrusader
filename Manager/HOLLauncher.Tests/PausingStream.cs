namespace HOLLauncher.Tests;

/**
 * A body that gives its first `cut` bytes, then plays the player's "Pause" click (L18): pause() cancels the update's
 * token, and the read ends like a cancelled download (OperationCanceledException), the bytes before it received.
 */
public sealed class PausingStream(byte[] data, int cut, Action pause) : MemoryStream(data)
{
    public override int Read(byte[] buffer, int offset, int count)
    {
        if (Position >= cut)
        {
            pause();
            throw new OperationCanceledException("paused");
        }
        return base.Read(buffer, offset, (int)Math.Min(count, cut - Position));
    }

    public override Task<int> ReadAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
        => Task.FromResult(Read(buffer, offset, count));

    public override ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken cancellationToken = default)
    {
        var array = new byte[buffer.Length];
        var read = Read(array, 0, array.Length);
        array.AsMemory(0, read).CopyTo(buffer);
        return ValueTask.FromResult(read);
    }
}
