"""Reads an Unreal network trace (.utrace, -trace=net -NetTrace=3) without Unreal Insights and reports,
per replicated data, how many bytes the server sent to each client.

  python Tools\\Tests\\net_trace_report.py Saved_Tests\\Logs\\s5_net.utrace [--server-log Saved_Tests\\Logs\\net_server.log] [--top 40] [--depth 9] [--min-share 0.3] [--events]

The trace format (TraceLog transport 4 / protocol 7) and the PacketContentEvent encoding are mirrored from
Engine/Source/Developer/TraceAnalysis/Private/Analysis/Engine.cpp and
Engine/Source/Developer/TraceServices/Private/Analyzers/NetTraceAnalyzer.cpp.
Measure window: the stress test S2's measure in the server log (after its warm-up, until its report),
mapped to the trace clock through the "NetSample server ... t=" lines (FPlatformTime::Seconds()).
"""
import re
import struct
import sys
from collections import defaultdict
from datetime import datetime


# ---------------------------------------------------------------- LZ4 block

def lz4_block_decode(src, decoded_size):
    """Pure Python LZ4 block decompression (what TraceLog's Private::Decode does)."""
    dst = bytearray()
    i, n = 0, len(src)
    while i < n:
        token = src[i]
        i += 1
        length = token >> 4
        if length == 15:
            while True:
                b = src[i]
                i += 1
                length += b
                if b != 255:
                    break
        if length:
            dst += src[i:i + length]
            i += length
        if i >= n:
            break
        offset = src[i] | (src[i + 1] << 8)
        i += 2
        length = token & 15
        if length == 15:
            while True:
                b = src[i]
                i += 1
                length += b
                if b != 255:
                    break
        length += 4
        start = len(dst) - offset
        if offset >= length:
            dst += dst[start:start + length]
        else:
            for k in range(length):
                dst.append(dst[start + k])
    if len(dst) != decoded_size:
        raise ValueError("LZ4: expected %d bytes, decoded %d" % (decoded_size, len(dst)))
    return bytes(dst)


# ---------------------------------------------------------------- transport

TID_EVENTS, TID_IMPORTANTS, TID_PSEUDO_IMPORTANTS, TID_SYNC = 0, 1, 0x3ffe, 0x3fff


def read_transport(data):
    """Returns ({thread id: decoded bytes}, transport version, protocol version)."""
    magic = data[0:4]
    if magic not in (b"2CRT", b"ECRT"):
        raise ValueError("not a TRC2 trace (magic %r)" % magic)
    pos = 4
    if magic == b"2CRT":
        (metadata_size,) = struct.unpack_from("<H", data, pos)
        pos += 2 + metadata_size
    transport, protocol = data[pos], data[pos + 1]
    pos += 2
    if transport != 4 or protocol < 5:
        raise ValueError("unsupported transport %d / protocol %d" % (transport, protocol))
    threads = defaultdict(list)
    end = len(data)
    while pos + 4 <= end:
        size, tid = struct.unpack_from("<HH", data, pos)
        if size < 4 or pos + size > end:
            break  # truncated last packet
        thread = tid & 0x3fff
        if thread != TID_SYNC:
            if tid & 0x8000:
                (decoded_size,) = struct.unpack_from("<H", data, pos + 4)
                threads[thread].append(lz4_block_decode(data[pos + 6:pos + size], decoded_size))
            else:
                threads[thread].append(data[pos + 4:pos + size])
        pos += size
    return {tid: b"".join(chunks) for tid, chunks in threads.items()}, transport, protocol


# ---------------------------------------------------------------- events

UID_NEW_EVENT, UID_AUX_DATA, UID_AUX_TERMINAL = 0, 1, 3
FLAG_IMPORTANT, FLAG_MAYBE_HAS_AUX, FLAG_NO_SYNC = 1, 2, 4
KNOWN_SIZES_P7 = {4: 0, 5: 0, 6: 8, 7: 8, 8: 7, 9: 7}  # EnterScope, LeaveScope, *_TA, *_TB


class EventType:
    def __init__(self, raw):
        uid, field_count, flags, logger_size, name_size = struct.unpack_from("<HBBBB", raw, 0)
        cursor = 6 + 8 * field_count
        self.logger = raw[cursor:cursor + logger_size].decode("ascii", "replace")
        cursor += logger_size
        self.name = raw[cursor:cursor + name_size].decode("ascii", "replace")
        cursor += name_size
        self.uid, self.flags = uid, flags
        self.fields = []  # (name, offset, size, type info, is array)
        self.size = 0
        for i in range(field_count):
            family, _, offset, size_or_ref, type_info, field_name_size = struct.unpack_from("<BBHHBB", raw, 6 + 8 * i)
            if family == 0:  # Regular
                name = raw[cursor:cursor + field_name_size].decode("ascii", "replace")
                cursor += field_name_size
                size = size_or_ref
            elif family == 1:  # Reference
                name = raw[cursor:cursor + field_name_size].decode("ascii", "replace")
                cursor += field_name_size
                size = 1 << (type_info & 3)
            else:  # DefinitionId: TypeInfo is the last byte
                name, size = "DefinitionId", 1 << (field_name_size & 3)
                type_info = field_name_size
            self.fields.append((name, offset, size, type_info, bool(type_info & 0o200)))
            self.size += size
        self.field_index = {f[0]: i for i, f in enumerate(self.fields)}

    def full_name(self):
        return "%s.%s" % (self.logger, self.name)

    def value(self, fixed, name):
        _, offset, size, type_info, _ = self.fields[self.field_index[name]]
        return int.from_bytes(fixed[offset:offset + size], "little", signed=bool(type_info & 0o020))


class Event:
    __slots__ = ("type", "fixed", "aux")

    def __init__(self, event_type, fixed):
        self.type, self.fixed, self.aux = event_type, fixed, None

    def get(self, name):
        return self.type.value(self.fixed, name)

    def attachment(self, name):
        """The aux data (array / string) of a field, or b""."""
        if not self.aux:
            return b""
        return self.aux.get(self.type.field_index.get(name, -1), b"")


class EventReader:
    def __init__(self, protocol):
        self.protocol = protocol
        self.types = {}
        self.counts = defaultdict(int)

    def parse_important(self, stream, sink):
        pos, end = 0, len(stream)
        while pos + 4 <= end:
            uid, size = struct.unpack_from("<HH", stream, pos)
            body = stream[pos + 4:pos + 4 + size]
            pos += 4 + size
            if uid == UID_NEW_EVENT:
                event_type = EventType(body)
                self.types[event_type.uid] = event_type
                continue
            event_type = self.types.get(uid)
            if event_type is None:
                continue
            event = Event(event_type, body[:event_type.size])
            if event_type.flags & FLAG_MAYBE_HAS_AUX:
                cursor = event_type.size
                while cursor < len(body) and body[cursor] not in (UID_AUX_TERMINAL, UID_AUX_TERMINAL << 1):
                    (pack,) = struct.unpack_from("<I", body, cursor)
                    aux_size, field = pack >> 13, (pack >> 8) & 31
                    if event.aux is None:
                        event.aux = {}
                    event.aux[field] = event.aux.get(field, b"") + body[cursor + 4:cursor + 4 + aux_size]
                    cursor += 4 + aux_size
            self.counts[event_type.full_name()] += 1
            sink(event)

    def parse_normal(self, stream, sink):
        """One thread's stream; events owning aux data are sent when their aux terminal comes."""
        pos, end = 0, len(stream)
        types, counts = self.types, self.counts
        stack = []
        while pos < end:
            byte = stream[pos]
            if byte & 1:
                uid = struct.unpack_from("<H", stream, pos)[0] >> 1
                pos += 2
            else:
                uid = byte >> 1
                pos += 1
            if uid < 16:
                if uid == UID_AUX_DATA:
                    (pack,) = struct.unpack_from("<I", stream, pos - 1)
                    aux_size, field = pack >> 13, (pack >> 8) & 31
                    payload = stream[pos + 3:pos + 3 + aux_size]
                    pos += 3 + aux_size
                    if stack:
                        owner = stack[-1]
                        if owner.aux is None:
                            owner.aux = {}
                        owner.aux[field] = owner.aux.get(field, b"") + payload
                elif uid == UID_AUX_TERMINAL:
                    if stack:
                        event = stack.pop()
                        counts[event.type.full_name()] += 1
                        sink(event)
                elif uid in KNOWN_SIZES_P7:
                    pos += KNOWN_SIZES_P7[uid] if self.protocol >= 7 else (7 if uid in (8, 12) else 0)
                else:
                    raise ValueError("unknown well-known uid %d at %d" % (uid, pos))
                continue
            event_type = types.get(uid)
            if event_type is None:
                raise ValueError("uid %d not declared (offset %d)" % (uid, pos))
            if not (event_type.flags & FLAG_NO_SYNC):
                pos += 3  # 24-bit serial
            event = Event(event_type, stream[pos:pos + event_type.size])
            pos += event_type.size
            if event_type.flags & FLAG_MAYBE_HAS_AUX:
                stack.append(event)
            else:
                counts[event_type.full_name()] += 1
                sink(event)


# ---------------------------------------------------------------- net trace

def decode_7bit(buffer, pos):
    value = shift = 0
    while True:
        b = buffer[pos]
        pos += 1
        value |= (b & 0x7f) << shift
        if not b & 0x80:
            return value, pos
        shift += 7


class NetTrace:
    def __init__(self):
        self.names = {}            # name id -> name
        self.objects = {}          # (game instance, object id) -> name id
        self.object_types = {}     # (game instance, object id) -> type id
        self.servers = set()       # game instance ids that are servers
        self.instance_names = {}
        self.pending = defaultdict(list)  # (gi, connection, type) -> raw content data, until the packet event
        self.packets = []          # (timestamp, gi, connection, type, bits, [content data])
        self.cycle_frequency = None
        self.start_cycle = 0
        self.bunches = 0

    def on_event(self, event):
        name = event.type.full_name()
        if name == "NetTrace.PacketContentEvent":
            key = (event.get("GameInstanceId"), event.get("ConnectionId"), event.get("PacketType"))
            self.pending[key].append(event.attachment("Data"))
        elif name == "NetTrace.PacketEvent":
            key = (event.get("GameInstanceId"), event.get("ConnectionId"), event.get("PacketType"))
            self.packets.append((event.get("Timestamp"),) + key + (event.get("PacketBits"), self.pending.pop(key, [])))
        elif name == "NetTrace.NameEvent":
            self.names[event.get("NameId")] = event.attachment("Name").decode("utf-8", "replace").rstrip(chr(0))
        elif name in ("NetTrace.ObjectCreatedEvent", "NetTrace.ObjectExistsEvent"):
            key = (event.get("GameInstanceId"), event.get("ObjectId"))
            self.objects[key] = event.get("NameId")
            self.object_types[key] = event.get("TypeId")
        elif name == "NetTrace.InstanceUpdatedEvent":
            gi = event.get("GameInstanceId")
            if event.get("bIsServer"):
                self.servers.add(gi)
            self.instance_names[gi] = event.attachment("Name").decode("utf-16-le", "replace").rstrip(chr(0))
        elif name == "$Trace.NewTrace":
            fields = event.type.field_index
            if "CycleFrequency" in fields:
                self.cycle_frequency = event.get("CycleFrequency")
            if "StartCycle" in fields:
                self.start_cycle = event.get("StartCycle")

    def content_events(self, gi, datas):
        """Decodes a packet's content data: list of (start bit, end bit, level, label)."""
        out = []
        for data in datas:
            pos, end, last_offset, bunch_offset = 0, len(data), 0, 0
            while pos < end:
                kind = data[pos]
                pos += 1
                if kind in (0, 1):
                    level = data[pos]
                    pos += 1
                    ident, pos = decode_7bit(data, pos)
                    start, pos = decode_7bit(data, pos)
                    start += last_offset
                    last_offset = start
                    length, pos = decode_7bit(data, pos)
                    if kind == 1:
                        label = self.names.get(ident, "<name %d>" % ident)
                    else:
                        name_id = self.objects.get((gi, ident))
                        label = "[obj] " + (strip_instance(self.names.get(name_id, "?")) if name_id is not None else "<object pending>")
                    out.append((start + bunch_offset, start + length + bunch_offset, level, label))
                elif kind == 2:
                    self.bunches += 1
                    _, pos = decode_7bit(data, pos)
                    bunch_bits, pos = decode_7bit(data, pos)
                    last_offset = 0
                    current_bunch_bits = bunch_bits
                elif kind == 3:
                    _, pos = decode_7bit(data, pos)
                    header_bits, pos = decode_7bit(data, pos)
                    if header_bits:
                        _, pos = decode_7bit(data, pos)
                        bunch_offset = 0
                    else:
                        bunch_offset = current_bunch_bits
                else:
                    raise ValueError("unknown content event type %d" % kind)
        return out


UNIT_STATES = "HOLUnitChunk_State > ... > States"


def is_unit_states(path):
    """The unit states: the 'States' member of the unit chunks' replicated state."""
    return len(path) >= 2 and path[-1].strip() == "States" and any(p.strip() == "HOLUnitChunk_State" for p in path[:-1])


def shorten(path):
    """Drops the generic wrappers so the data's own names stay readable."""
    skip = {"ReplicationData", "Batch", "FArrayPropertyNetSerializer", "FStructNetSerializer"}
    kept = [p.strip() for p in path if p.strip() not in skip] or [path[-1].strip()]
    return " > ".join(kept)


def print_tree(tree, prefix, total, to_bytes, min_share, depth):
    if len(prefix) >= depth:
        return
    children = sorted(((p, b) for p, b in tree.items() if len(p) == len(prefix) + 1 and p[:-1] == prefix), key=lambda kv: -kv[1])
    for path, bits in children:
        if 100.0 * bits / total < min_share:
            continue
        print("    %8.0f  %5.1f%%  %s%s" % (to_bytes(bits), 100.0 * bits / total, "  " * len(prefix), path[-1].strip()))
        print_tree(tree, path, total, to_bytes, min_share, depth)


def strip_instance(name):
    """'BP_Unit_C_12' -> 'BP_Unit_C', '/Game/Map.Map:PersistentLevel.Thing_3' -> 'Thing'."""
    name = name.rsplit(".", 1)[-1].rsplit(":", 1)[-1]
    return re.sub(r"_\d+$", "", name)


# ---------------------------------------------------------------- measure window

def log_time(line):
    match = re.match(r"\[(\d{4})\.(\d\d)\.(\d\d)-(\d\d)\.(\d\d)\.(\d\d):(\d{3})\]", line)
    if not match:
        return None
    y, mo, d, h, mi, s, ms = (int(g) for g in match.groups())
    return datetime(y, mo, d, h, mi, s, ms * 1000).timestamp()


def measure_window(log_path):
    """(start, end) in FPlatformTime::Seconds() of the S2 measure, or None."""
    with open(log_path, encoding="utf-8", errors="replace") as handle:
        lines = handle.readlines()
    start = end = clock_offset = None
    for line in lines:
        match = re.search(r"Stress test S2 \(moving\): (\d+) units .* measuring for (\d+) s after (\d+) s of warm-up", line)
        if match:
            start = log_time(line) + float(match.group(3))
        if start is not None and end is None and re.search(r"Stress test S2 \(moving\), \d+ units", line):
            end = log_time(line)
        if clock_offset is None:
            match = re.search(r"NetSample server step=\d+ t=([\d.]+)", line)
            if match:
                clock_offset = float(match.group(1)) - log_time(line)
    if start is None or end is None or clock_offset is None:
        return None
    return start + clock_offset, end + clock_offset


# ---------------------------------------------------------------- report

def main():
    args = sys.argv[1:]
    if not args or args[0].startswith("-"):
        print(__doc__)
        return 2
    trace_path, log_path, top, show_events, depth, min_share = args[0], None, 40, False, 9, 0.3
    i = 1
    while i < len(args):
        if args[i] == "--server-log":
            log_path, i = args[i + 1], i + 2
        elif args[i] == "--top":
            top, i = int(args[i + 1]), i + 2
        elif args[i] == "--depth":
            depth, i = int(args[i + 1]), i + 2
        elif args[i] == "--min-share":
            min_share, i = float(args[i + 1]), i + 2
        elif args[i] == "--events":
            show_events, i = True, i + 1
        else:
            print("unknown argument %s" % args[i])
            return 2

    with open(trace_path, "rb") as handle:
        data = handle.read()
    threads, transport, protocol = read_transport(data)
    reader = EventReader(protocol)
    net = NetTrace()
    for tid in (TID_EVENTS, TID_IMPORTANTS):
        if tid in threads:
            reader.parse_important(threads[tid], net.on_event)
    for tid in sorted(threads):
        if tid not in (TID_EVENTS, TID_IMPORTANTS):
            reader.parse_normal(threads[tid], net.on_event)
    print("Trace: transport %d, protocol %d, %d threads, %d event types declared, %.1f MB decoded" % (
        transport, protocol, len(threads), len(reader.types), sum(len(t) for t in threads.values()) / 1e6))
    if show_events:
        for name, count in sorted(reader.counts.items(), key=lambda kv: -kv[1]):
            print("  %8d  %s" % (count, name))
    else:
        for name, count in sorted(reader.counts.items(), key=lambda kv: -kv[1]):
            if name.startswith("NetTrace."):
                print("  %8d  %s" % (count, name))

    frequency = net.cycle_frequency or 10_000_000
    packets = net.packets
    if not packets:
        print("No NetTrace.PacketEvent in the trace")
        return 1
    seconds = lambda ts: ts / frequency
    first, last = seconds(packets[0][0]), seconds(packets[-1][0])
    window = measure_window(log_path) if log_path else None
    if window and not (first <= window[0] < last) and first <= window[0] - 16777216.0 < last:
        # FPlatformTime::Seconds() adds 2^24 s to the cycle clock (precision trick of the Windows platform).
        window = (window[0] - 16777216.0, window[1] - 16777216.0)
    if window and not (first <= window[0] < last):
        # Timestamps may be relative to the trace start.
        shifted = [(p[0] + net.start_cycle,) + p[1:] for p in packets]
        if seconds(shifted[0][0]) <= window[0] < seconds(shifted[-1][0]):
            packets = shifted
        else:
            print("Measure window %.1f-%.1f s is outside the trace (%.1f-%.1f s): whole trace used" % (
                window[0], window[1], first, last))
            window = None
    if window:
        print("Measure window: %.1f s (S2 measure from the server log)" % (window[1] - window[0]))
    else:
        window = (seconds(packets[0][0]), seconds(packets[-1][0]))
        print("No measure window%s: whole trace, %.1f s" % ("" if log_path else " (no --server-log)", window[1] - window[0]))
    duration = window[1] - window[0]

    servers = net.servers or {gi for _, gi, _, _, _, _ in packets}
    # Per connection: packets, bits, bits by (level, label), tree paths, uncovered bits.
    connections = defaultdict(lambda: {"packets": 0, "bits": 0, "flat": defaultdict(int), "tree": defaultdict(int), "covered": 0, "exclusive": defaultdict(int)})
    for ts, gi, connection, ptype, bits, datas in packets:
        if gi not in servers or ptype != 0:
            continue
        t = seconds(ts)
        if t < window[0] or t > window[1]:
            continue
        c = connections[(gi, connection)]
        c["packets"] += 1
        c["bits"] += bits
        events = sorted(net.content_events(gi, datas), key=lambda e: (e[0], e[2], -e[1]))
        stack = []
        top_level = min((e[2] for e in events), default=0)
        for start, end, level, label in events:
            size = end - start
            c["flat"][(level, label)] += size
            while stack and (stack[-1][0] >= level or stack[-1][2] <= start):
                stack.pop()
            path = tuple(s[1] for s in stack) + (label,)
            c["tree"][path] += size
            if stack:
                c["exclusive"][tuple(s[1] for s in stack)] -= size
            c["exclusive"][path] += size
            stack.append((level, label, end))
            if level == top_level:
                c["covered"] += size

    if not connections:
        print("No outgoing server packet in the window")
        return 1
    to_bytes = lambda bits: bits / 8.0 / duration
    for (gi, connection), c in sorted(connections.items()):
        total = to_bytes(c["bits"])
        print()
        print("Server %s, connection %d: %.1f packets/s, %.0f B/s (%.2f KB/s)" % (
            net.instance_names.get(gi, gi), connection, c["packets"] / duration, total, total / 1000.0))
        print("  Not covered by any content event (overhead): %.0f B/s (%.1f%%)" % (
            to_bytes(c["bits"] - c["covered"]), 100.0 * (c["bits"] - c["covered"]) / max(1, c["bits"])))
        print("  By name and level (B/s, share of the connection):")
        for (level, label), bits in sorted(c["flat"].items(), key=lambda kv: -kv[1])[:top]:
            print("    L%d %8.0f  %5.1f%%  %s" % (level, to_bytes(bits), 100.0 * bits / c["bits"], label))
        print("  Tree (B/s, share; children >= %.1f%% of the connection, %d levels):" % (min_share, depth))
        print_tree(c["tree"], (), c["bits"], to_bytes, min_share, depth)
        # The rest: everything outside the unit states, by data (exclusive bits of each path).
        states_bits = sum(b for p, b in c["tree"].items() if is_unit_states(p) and not is_unit_states(p[:-1]))
        rest = defaultdict(int)
        for path, bits in c["exclusive"].items():
            if not any(is_unit_states(path[:k]) for k in range(1, len(path) + 1)):
                rest[shorten(path)] += bits
        rest["(not covered by any content event)"] += c["bits"] - c["covered"]
        rest_total = c["bits"] - states_bits
        print("  Unit states (%s): %.0f B/s; the rest: %.0f B/s, by data (own bits of each path):" % (
            UNIT_STATES, to_bytes(states_bits), to_bytes(rest_total)))
        for path, bits in sorted(rest.items(), key=lambda kv: -kv[1])[:top]:
            if bits > 0:
                print("    %7.0f  %5.1f%% of the rest  %s" % (to_bytes(bits), 100.0 * bits / max(1, rest_total), path))
    if net.bunches:
        print("\n(%d bunch events seen: positions inside merged bunches are approximate)" % net.bunches)
    return 0


if __name__ == "__main__":
    sys.exit(main())
