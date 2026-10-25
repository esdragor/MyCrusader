"""S5 report (D273): the unit replication of a dedicated server to its clients, from the logs of net_match.ps1 -Units.
  python s5_report.py <server log> <client log> [<client log>...] [--budget 34000] [--error 25]
The stress test is the server's: S2 (the walk, S5) or S3 / S4 (S5 in combat, J7, net_match.ps1 -Test S3); a battle's
measure begins once the armies are joined (its "measure begins" line), the others' after their warm-up.
Over the stress test's measure (after its warm-up, until its report):
- server: sends per unit and per second, bytes sent per client and per second (averaged from the connections' totals),
  packets per second, cost of the replication per step, and the simulation's own time per step (the stress test's report);
- clients: bytes received per second, frame rate, corrections;
- displayed error: the positions a client displays for a sample of units (hol.Net.Sample), against where the server has
  them at the same instant (its positions at the steps around it, interpolated over the steps' times), and the client's
  estimate of the server clock against the server's step at that instant. Server and clients run on the same machine,
  so their FPlatformTime clocks agree.
- combat (S3, S4): the hits of the measure (the battle's summary) against the server's "activity" sends: the health of
  a hit is a change of action (J7-Q5 revised, sent at once), counted with the other changes of action (begins or ends
  striking, another target, falls); of them, the server counts apart those for the health alone ("health", J7c.4: the
  unit would not have been sent with its health unchanged), their exact rate per second and per melee hit.
PASS: bytes per client <= budget (D218: 32 KB/s), mean displayed error <= error (D273: ~25 cm). Exit code 1 on FAIL.
"""
import bisect
import math
import re
import sys
from datetime import datetime

STEP_SECONDS = 0.05
# Bits of one unit state on the wire (FHOLNetUnitState::TotalBits: 57 since D344 (59 from D343), an unhurt unit that
# carries nothing and does not fight), the least: a wounded unit writes HEALTH_BITS more, an attacker its target's slot
# (12), a unit facing its front the yaw (6).
STATE_BITS = 57
HEALTH_BITS = 8


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
    budget, error_limit, logs = 34000.0, 25.0, []  # D450: 34 Ko/s per client (32, D218, before)
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

    # The measure: from the end of the stress test's warm-up (a battle: once the armies are joined) to its report
    # (wall clock of the log lines).
    start = end = None
    warm_up = 0.0
    units = 0
    test = name = None
    simulation = None
    joined = None
    hits = None
    for line in server_lines:
        match = re.search(r"Stress test (\w+) \(([^)]*)\): (\d+) units .* measuring for (\d+) s after (\d+) s of warm-up", line)
        if match and test is None:
            test, name = match.group(1), match.group(2)
            units, warm_up = int(match.group(3)), float(match.group(5))
            start = log_time(line) + warm_up
        if test is None:
            continue
        match = re.search(r"%s \([^)]*\): measure begins (.*)$" % test, line)
        if match and end is None:
            # S3, S4: the measure begins at contact, before the longest warm-up.
            start, joined = log_time(line), match.group(1)
        if end is None and re.search(r"Stress test %s \([^)]*\), \d+ units" % test, line):
            end = log_time(line)
        match = re.search(r"Simulation: ([\d.]+) ms average per step \(max ([\d.]+) ms", line)
        if match and end is not None and simulation is None:
            simulation = (float(match.group(1)), float(match.group(2)))
        match = re.search(r"Battle during the measure: (\d+) hits \(([\d.]+) per second, (\d+) of them arrows released\), (\d+) kills", line)
        if match and end is not None and hits is None:
            hits = (int(match.group(1)), float(match.group(2)), int(match.group(3)), int(match.group(4)))
    if start is None or end is None:
        print("S5 : no complete stress test in the server log")
        return 1

    # Server reports wholly inside the measure (a report covers the 5 s before its line).
    sends, out_bytes, packets, step_ms, step_max, frequencies, wire_states = [], [], [], [], [], [], []
    activity_rates, health_rates = [], []
    for line in server_lines:
        if "Net (server):" not in line:
            continue
        time = log_time(line)
        match = re.search(r"Net \(server\): (\d+) units, \d+ steps in ([\d.]+) s, ([\d.]+) sends per unit and per second \(birth (\d+)", line)
        if not match or time - float(match.group(2)) < start or time > end + 0.5 or int(match.group(4)) > 0:
            continue
        sends.append(float(match.group(3)))
        # Sends for a change of action, all units, per second (the reasons: birth, activity, error, refresh).
        activity = re.search(r"activity (\d+)", line)
        if activity:
            activity_rates.append(int(activity.group(1)) / float(match.group(2)))
        # Of them, the sends for the health alone (J7c.4; absent from the reports of an older server).
        health = re.search(r"; health (\d+)\)", line)
        if health:
            health_rates.append(int(health.group(1)) / float(match.group(2)))
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

    print("S5 : stress test %s (%s), %d units, measure of %.0f s (%d server reports inside it)%s" % (
        test, name, units, end - start, len(sends), ", begun %s" % joined if joined else ""))
    print("  Server: %.2f sends per unit and per second, %s updates per second" % (
        mean(sends), "/".join("%.0f" % value for value in sorted(set(frequencies))) or "?"))
    print("  Server: replication %.3f ms per step (max %.3f)%s" % (
        mean(step_ms), max(step_max),
        ", simulation %.3f ms per step (max %.3f, the stress test's report)" % simulation if simulation else ""))
    print("  Server: out per client %s B/s, %s packets/s; all clients %.0f B/s" % (
        " / ".join("%.0f" % value for value in per_client_out),
        " / ".join("%.0f" % mean(column) for column in zip(*packets)), sum(per_client_out)))
    if wire_states:
        # Serializations of the server, rolled back ones included (an object that overflows a packet is written again).
        payload = mean(wire_states) * STATE_BITS / 8
        print("  Server: %.0f states serialized per client and per second (%.0f B/s if all were sent), %.0f B/s of the rest" % (
            mean(wire_states), payload, mean(per_client_out) - payload))

    if hits:
        # Combat (J7-Q5 revised): the health goes with each hit, as a change of action sent at once.
        melee_rate = hits[1] * (hits[0] - hits[2]) / hits[0] if hits[0] > 0 else 0.0
        activity_rate = mean(activity_rates)
        print("  Combat: %d hits in the measure (%.0f per second, %d arrows released), %d kills" % (
            hits[0], hits[1], hits[2], hits[3]))
        if activity_rates and melee_rate > 0:
            # Each send leaves for every client: one state, at least STATE_BITS + HEALTH_BITS once wounded.
            print("  Combat: %.0f activity sends per second, %.2f per melee hit (health changes with the other changes of action), "
                  "~%.0f B/s per client of their states (%d bits each at least)" % (
                      activity_rate, activity_rate / melee_rate, activity_rate * (STATE_BITS + HEALTH_BITS) / 8, STATE_BITS + HEALTH_BITS))
        if health_rates and melee_rate > 0:
            # Sent for the health alone: what the health costs (a wounded state, at least STATE_BITS + HEALTH_BITS).
            health_rate = mean(health_rates)
            print("  Combat: %.0f health sends per second, %.2f per melee hit (the health alone, exact), "
                  "~%.0f B/s per client of their states (%d bits each at least)" % (
                      health_rate, health_rate / melee_rate, health_rate * (STATE_BITS + HEALTH_BITS) / 8, STATE_BITS + HEALTH_BITS))
        elif not health_rates:
            print("  Combat: no \"health\" count in the server's reports (a server older than J7c.4)")

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
