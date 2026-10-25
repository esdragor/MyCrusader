using System.Net;
using System.Security.Cryptography;
using System.Text;
using HOLManager.Lobbies;

namespace HOLManager.Api;

/**
 * L11: the key shared with the friends' kit. When the manager has one, every request not from the loopback must carry
 * it in X-HOL-Key, else 401 bad_key (PROTOCOL.md, Clé d'accès). No key: everything stays open, as before.
 */
public static class AccessKey
{
    public const string Header = "X-HOL-Key";

    /** The key in this file, trimmed; null when the file is absent or empty. */
    public static string? Read(string file)
    {
        if (!File.Exists(file))
        {
            return null;
        }
        var key = File.ReadAllText(file).Trim().TrimStart('﻿').Trim();
        return key.Length > 0 ? key : null;
    }

    /** The game servers started by the manager and local trials call from the loopback (IPv4, IPv6, IPv4 mapped). */
    public static bool IsExempt(IPAddress? remote) =>
        remote != null && IPAddress.IsLoopback(remote.IsIPv4MappedToIPv6 ? remote.MapToIPv4() : remote);

    /** True when the header equals the key, compared in constant time. */
    public static bool Matches(string key, string? given) =>
        given != null && CryptographicOperations.FixedTimeEquals(Encoding.UTF8.GetBytes(key), Encoding.UTF8.GetBytes(given));

    /** Before every route, after UseLobbyErrors (which writes the 401 body); does nothing without a key. */
    public static void UseAccessKey(this WebApplication app, string? key)
    {
        if (key == null)
        {
            return;
        }
        app.Use(async (context, next) =>
        {
            if (!IsExempt(context.Connection.RemoteIpAddress)
                && !Matches(key, context.Request.Headers.TryGetValue(Header, out var given) ? given.ToString() : null))
            {
                // RequestLog writes the line, limited per address (L12).
                context.Items[RequestLog.RefusedItem] = true;
                throw new LobbyException(LobbyErrors.BadKey, $"Missing or wrong {Header}: this manager needs the key of its kit.");
            }
            await next(context);
        });
    }
}
