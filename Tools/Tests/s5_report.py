"""S5 report (D273): the unit replication of a dedicated server to its clients, from the logs of net_match.ps1 -Units.
  python s5_report.py <server log> <client log> [<client log>...] [--budget 32000] [--error 25]
Over the stress test's measure (after its warm-up, until its report):
- server: sends per unit and per second, bytes sent per client and per second (averaged from the connections' totals),
  packets per second, cost of the replication per step, and the simulation's own time per step (S2's report);
- clients: bytes received per second, frame rate, corrections;
- displayed error: the positions a client displays for a sample of units (hol.Net.Sample), against where the server has
  them at the same instant (its positions at the steps around it, interpolated over the steps' times), and the client's
  estimate of the server clock against the server's step at that instant. Server and clients run on the same machine,
  so their FPlatformTime clocks agree.
PASS: bytes per client <= budget (D218: 32 KB/s), mean displayed error <= error (D273: ~25 cm). Exit code 1 on FAIL.
"""
import bisect
import math
import re
import sys
from datetime import datetime

STEP_SECONDS = 0.05
# Bits of one unit state on the wire (FHOLNetUnitState::TotalBits: 57 since D344 (59 from D343), an unhurt unit that carries nothing and
# does not fight; the extras of the others are not counted).
STATE_BITS = 57


def log_time(line):
    match = re.match(r"\[(\d{4})\.(\d{2})\.(\d{2})-(\d{2})\.(\d{2})\.(\d{2}):(\d{3})\]", line)
    if not match:
        return None
    y, mo, d, h, mi, s, ms = (int(value) for value in match.groups())
    return datetime(y, mo, d, h, mi, s, ms * 1000).timestamp()


def parse_positions(text):
    positions = {}
    for item in text.split():
        unit, _, xy = item.partition(":")
        x, _, y = xy.partition(",")
        positions[int(unit)] = (float(x), float(y))
    return positions


def numbers(text):
    return [float(value) for value in text.split()]


def mean(values):
    return sum(values) / len(values) if values else float("nan")


def percentile(values, share):
    if not values:
        return float("nan")
    ordered = sorted(values)
    return ordered[min(len(ordered) - 1, int(share * len(ordered)))]


def read(path):
    with open(path, encoding="utf-8", errors="replace") as file:
        return file.read().splitlines()


def main(arguments):
    budget, error_limit, logs = 32000.0, 25.0, []
    iterator = iter(arguments)
    for argument in iterator:
        if argument == "--budget":
            budget = float(next(iterator))
        elif argument == "--error":
            error_limit = float(next(iterator))
        else:
            logs.append(argument)
    if len(logs) < 2:
        print(__doc__)
        return 2
    server_lines, client_paths = read(logs[0]), logs[1:]

    # The measure: from the end of S2's warm-up to its report (wall clock of the log lines).
    start = end = None
    warm_up = 0.0
    units = 0
    simulation = None
    for line in server_lines:
        match = re.search(r"Stress test S2 \(moving\): (\d+) units .* measuring for (\d+) s after (\d+) s of warm-up", line)
        if match:
            units, warm_up = int(match.group(1)), float(match.group(3))
            start = log_time(line) + warm_up
        if start is not None and end is None and re.search(r"Stress test S2 \(moving\), \d+ units", line):
            end = log_time(line)
        match = re.search(r"Simulation: ([\d.]+) ms average per step \(max ([\d.]+) ms", line)
        if match and end is not None and simulation is None:
            simulation = (float(match.group(1)), float(match.group(2)))
    if start is None or end is None:
        print("S5 : no complete S2 in the server log")
        return 1

    # Server reports wholly inside the measure (a report covers the 5 s before its line).
    sends, out_bytes, packets, step_ms, step_max, frequencies, wire_states = [], [], [], [], [], [], []
    for line in server_lines:
        if "Net (server):" not in line:
            continue
        time = log_time(line)
        match = re.search(r"Net \(server\): (\d+) units, \d+ steps in ([\d.]+) s, ([\d.]+) sends per unit and per second \(birth (\d+)", line)
        if not match or time - float(match.group(2)) < start or time > end + 0.5 or int(match.group(4)) > 0:
            continue
        sends.append(float(match.group(3)))
        cost = re.search(r"; ([\d.]+) ms per step \(max ([\d.]+)\)", line)
        step_ms.append(float(cost.group(1)))
        step_max.append(float(cost.group(2)))
        frequency = re.search(r"; ([\d.]+) updates per second", line)
        if frequency:
            frequencies.append(float(frequency.group(1)))
        wire = re.search(r"; ([\d.]+) states on the wire per client", line)
        if wire:
            wire_states.append(float(wire.group(1)))
        clients = numbers(re.search(r"out B/s per client:([\d. ]+)", line).group(1))
        if clients:
            # A report written after the clients left has no connection ("none").
            out_bytes.append(clients)
            packets.append(numbers(re.search(r"packets/s:([\d. ]+)", line).group(1)))
    if not sends:
        print("S5 : no server report inside the measure")
        return 1
    per_client_out = [mean(column) for column in zip(*out_bytes)]

    # True positions: step -> (time, positions).
    steps, step_times, step_positions = [], [], []
    for line in server_lines:
        match = re.search(r"NetSample server step=(\d+) t=([\d.]+)(.*)$", line)
        if match:
            steps.append(int(match.group(1)))
            step_times.append(float(match.group(2)))
            step_positions.append(parse_positions(match.group(3)))
    # Measure window in the FPlatformTime clock: the samples' clock against the log's wall clock.
    clock_offset = None
    for line in server_lines:
        match = re.search(r"NetSample server step=\d+ t=([\d.]+)", line)
        if match:
            clock_offset = float(match.group(1)) - log_time(line)
            break

    print("S5 : %d units, measure of %.0f s (%d server reports of the walk)" % (units, end - start, len(sends)))
    print("  Server: %.2f sends per unit and per second, %s updates per second" % (
        mean(sends), "/".join("%.0f" % value for value in sorted(set(frequencies))) or "?"))
    print("  Server: replication %.3f ms per step (max %.3f)%s" % (
        mean(step_ms), max(step_max),
        ", simulation %.3f ms per step (max %.3f, S2's report)" % simulation if simulation else ""))
    print("  Server: out per client %s B/s, %s packets/s; all clients %.0f B/s" % (
        " / ".join("%.0f" % value for value in per_client_out),
        " / ".join("%.0f" % mean(column) for column in zip(*packets)), sum(per_client_out)))
    if wire_states:
        # Serializations of the server, rolled back ones included (an object that overflows a packet is written again).
        payload = mean(wire_states) * STATE_BITS / 8
        print("  Server: %.0f states serialized per client and per second (%.0f B/s if all were sent), %.0f B/s of the rest" % (
            mean(wire_states), payload, mean(per_client_out) - payload))

    all_errors, failed = [], False
    for index, path in enumerate(client_paths, start=1):
        lines = read(path)
        in_bytes, fps, low_fps, corrections, snaps, read_states = [], [], [], [], 0, []
        for line in lines:
            if "Net (display):" not in line:
                continue
            time = log_time(line)
            if time - 5.0 < start or time > end + 0.5:
                continue
            match = re.search(r"correction mean ([\d.]+) cm max ([\d.]+) cm, (\d+) snaps", line)
            corrections.append(float(match.group(1)))
            snaps += int(match.group(3))
            match = re.search(r"\(([\d.]+) read off the wire\)", line)
            if match:
                read_states.append(float(match.group(1)))
            match = re.search(r"in ([\d.]+) B/s; ([\d.]+) fps \(1% low ([\d.]+)\)", line)
            if match:
                in_bytes.append(float(match.group(1)))
                fps.append(float(match.group(2)))
                low_fps.append(float(match.group(3)))

        errors, clock_gaps = [], []
        for line in lines:
            match = re.search(r"NetSample client t=([\d.]+) clock=([\d.]+)(.*)$", line)
            if not match or clock_offset is None:
                continue
            time = float(match.group(1))
            if time - clock_offset < start or time - clock_offset > end:
                continue
            after = bisect.bisect_right(step_times, time)
            if after == 0 or after >= len(step_times) or steps[after] != steps[after - 1] + 1:
                continue
            alpha = (time - step_times[after - 1]) / (step_times[after] - step_times[after - 1])
            clock_gaps.append(float(match.group(2)) - (steps[after - 1] + alpha))
            before_positions, after_positions = step_positions[after - 1], step_positions[after]
            for unit, (x, y) in parse_positions(match.group(3)).items():
                if unit in before_positions and unit in after_positions:
                    (x0, y0), (x1, y1) = before_positions[unit], after_positions[unit]
                    errors.append(math.hypot(x - (x0 + (x1 - x0) * alpha), y - (y0 + (y1 - y0) * alpha)))
        all_errors += errors
        print("  Client %d: in %s B/s, %s fps (1%% low %s), corrections %.1f cm on average, %d snaps" % (
            index, "%.0f" % mean(in_bytes) if in_bytes else "?", "%.1f" % mean(fps) if fps else "?",
            "%.1f" % min(low_fps) if low_fps else "?", mean(corrections), snaps))
        if read_states:
            # The states that really arrived: their own bits, and the rest of what was received.
            payload = mean(read_states) * STATE_BITS / 8
            print("  Client %d: %.0f states read off the wire per second, %.0f B/s of states, %.0f B/s of the rest" % (
                index, mean(read_states), payload, (mean(in_bytes) if in_bytes else float("nan")) - payload))
        print("  Client %d: displayed error %.1f cm on average, median %.1f, 95 %% %.1f, max %.1f (%d positions)" % (
            index, mean(errors), percentile(errors, 0.5), percentile(errors, 0.95),
            max(errors) if errors else float("nan"), len(errors)))
        print("  Client %d: clock ahead of the server by %.2f steps (median), 5 %% %.2f, 95 %% %.2f, extremes %.2f / %.2f" % (
            index, percentile(clock_gaps, 0.5), percentile(clock_gaps, 0.05), percentile(clock_gaps, 0.95),
            min(clock_gaps) if clock_gaps else float("nan"), max(clock_gaps) if clock_gaps else float("nan")))

    worst_out = max(per_client_out)
    bandwidth_ok = worst_out <= budget
    error_ok = bool(all_errors) and mean(all_errors) <= error_limit
    print("  Bandwidth per client (D218): %.0f B/s - threshold %.0f B/s: %s" % (worst_out, budget, "PASS" if bandwidth_ok else "FAIL"))
    print("  Displayed error (D273): %.1f cm on average - threshold %.0f cm: %s" % (
        mean(all_errors), error_limit, "PASS" if error_ok else "FAIL"))
    failed = not (bandwidth_ok and error_ok)
    print("S5 : %s" % ("FAIL" if failed else "PASS"))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
