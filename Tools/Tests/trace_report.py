"""Insights report of a traced stress test (stress_ab.ps1 -Trace), adapted from ProjectShiva's ue_trace_scopes.py and
ue_trace_tree.py. Over the measure window (the HOLMeasure region of UHOLStressTestSubsystem):
- the step (HOLSimulationStep, game thread): ms per step, its parts (births, each Mass processor, destructions);
- the worker threads during the steps: busy ms per step by scope (the processors' parallel work, and who waits);
- the game thread's frame outside the step (FEngineLoop::Tick minus the steps);
- the longest steps, each with its game-thread scope tree.
  python Tools\\Tests\\trace_report.py Saved_Tests\\Logs\\<run>.utrace [--top 25] [--long 5] [--min-ms 0.05]
         [--scopes "Name" "Prefix*" ...] [--region HOLMeasure]
Insights exports the region's events once, next to the trace (<run>_insights\\); later calls read them in seconds.
"""
import argparse
import bisect
import csv
import json
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

STEP = "HOLSimulationStep"
FRAME = "FEngineLoop::Tick"
# Mass's own scopes around each processor of UHOLSimulationSubsystem::RunStep, skipped in the parts and the trees.
WRAPPERS = {"MassExecutor Run", "MassExecutor_RunProcessorsView"}


def insights_exe(trace):
    """UnrealInsights.exe of the engine of the project that holds the trace (Saved_Tests\\Logs\\ is two levels down)."""
    project = trace.parent.parent.parent
    uproject = next(project.glob("*.uproject"), None)
    if uproject is None:
        sys.exit(f"no .uproject above {trace}")
    association = json.loads(uproject.read_text(encoding="utf-8"))["EngineAssociation"]
    import winreg
    with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\Epic Games\Unreal Engine\Builds") as key:
        root = Path(winreg.QueryValueEx(key, association)[0])
    return root / "Engine" / "Binaries" / "Win64" / "UnrealInsights.exe"


def export(trace, out, region):
    """Region bounds (from the Insights log of the statistics export) and the region's events of every thread."""
    events = out / "events.csv"
    log = out / "insights.log"
    if not events.exists():
        out.mkdir(exist_ok=True)
        rsp = out / "export.rsp"
        # Forward slashes: Insights reads backslashes in a response file as escapes.
        rsp.write_text(
            f'TimingInsights.ExportTimerStatistics "{out.as_posix()}/region_{{region}}.csv" -region={region}'
            f' -maxTimerCount=1\n'
            f'TimingInsights.ExportTimingEvents "{events.as_posix()}" -region={region}'
            f' -columns=ThreadName,TimerName,StartTime,EndTime,Depth\n')
        subprocess.run([str(insights_exe(trace)), f"-OpenTraceFile={trace.as_posix()}", "-NoUI", "-AutoQuit", "-log",
                        f"-ABSLOG={log.as_posix()}", f"-ExecOnAnalysisCompleteCmd=@={rsp.as_posix()}"],
                       capture_output=True)
    text = log.read_text(errors="replace") if log.exists() else ""
    match = re.search(r"for region '%s' \[([\d.]+) \.\. ([\d.]+)\]" % re.escape(region), text)
    if not match or not events.exists():
        sys.exit(f"no '{region}' region in the trace (see {log}): a trace of stress_ab.ps1 -Trace, with the build of"
                 f" the region (UHOLStressTestSubsystem::BeginMeasure)?")
    return (float(match.group(1)), float(match.group(2))), events


def read_events(path, window):
    """(thread, name, start, end, depth) of the events that start inside the window."""
    rows = []
    with path.open(newline="", encoding="utf-8", errors="replace") as f:
        for r in csv.DictReader(f):
            start, end = float(r["StartTime"]), float(r["EndTime"])
            if not window[0] <= start < window[1]:
                continue
            rows.append((r["ThreadName"], r["TimerName"], start, end if end >= start else start, int(r["Depth"])))
    return rows


def inclusive(rows):
    """Inclusive ms and calls per name; a scope nested in itself counts once in the time."""
    ms, calls = defaultdict(float), defaultdict(int)
    open_until = defaultdict(lambda: -1.0)
    for thread, name, start, end, _ in sorted(rows, key=lambda r: (r[0], r[2])):
        calls[name] += 1
        key = (thread, name)
        if start >= open_until[key]:
            ms[name] += (end - start) * 1000.0
        open_until[key] = max(open_until[key], end)
    return ms, calls


def in_steps(rows, steps):
    """(row, its step, its level under the step, wrappers skipped) of each game-thread row inside a step (sorted)."""
    out = []
    i = 0
    stack = []  # (end, depth, is wrapper) of the open ancestors inside the current step
    for r in rows:
        while i < len(steps) and steps[i][3] < r[2]:
            i += 1
            stack = []
        if i == len(steps) or not (steps[i][2] <= r[2] and r[3] <= steps[i][3]) or r[4] <= steps[i][4]:
            continue
        stack = [a for a in stack if a[0] > r[2] and a[1] < r[4]]
        wrapper = r[1] in WRAPPERS
        stack.append((r[3], r[4], wrapper))
        if not wrapper:
            out.append((r, steps[i], sum(1 for a in stack if not a[2])))
    return out


def matches(name, scope):
    return name.startswith(scope[:-1]) if scope.endswith("*") else name == scope


def table(title, ms, calls, steps, top, total=None):
    print(f"\n## {title}\n")
    print("| Scope | ms per step | calls per step |" + (" share |" if total else ""))
    print("|---|---|---|" + ("---|" if total else ""))
    for name, value in sorted(ms.items(), key=lambda kv: -kv[1])[:top]:
        share = f" {100.0 * value / total:.1f} % |" if total else ""
        print(f"| {name[:90]} | {value / steps:.3f} | {calls[name] / steps:.2f} |{share}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--region", default="HOLMeasure")
    parser.add_argument("--top", type=int, default=25)
    parser.add_argument("--long", type=int, default=5, help="longest steps shown with their tree")
    parser.add_argument("--min-ms", type=float, default=0.05, help="smallest scope in a step tree")
    parser.add_argument("--scopes", nargs="*", default=[], help="scopes to report on every thread ('Prefix*')")
    args = parser.parse_args()

    trace = args.trace.resolve()
    window, events = export(trace, trace.with_name(trace.stem + "_insights"), args.region)
    rows = read_events(events, window)
    game = sorted((r for r in rows if r[0] == "GameThread"), key=lambda r: (r[2], r[4]))
    workers = [r for r in rows if r[0] != "GameThread" and not r[0].startswith(("GPU", "RHI", "RenderThread"))]
    steps = sorted((r for r in game if r[1] == STEP), key=lambda r: r[2])
    if not steps:
        sys.exit(f"no {STEP} scope in the region: a build older than the trace scopes?")
    count = len(steps)
    step_ms = [(r[3] - r[2]) * 1000.0 for r in steps]
    print(f"# {trace.name}: {args.region} {window[0]:.3f} .. {window[1]:.3f} s ({window[1] - window[0]:.1f} s)")
    print(f"\n{count} steps, {sum(step_ms) / count:.3f} ms per step (max {max(step_ms):.2f}); traced: compare with"
          f" traced runs only")

    # The step's parts: the game-thread scopes right under each step (births, each processor, destructions).
    tree = in_steps(game, steps)
    ms, calls = inclusive([r for r, _, level in tree if level == 1])
    table("Step parts (game thread)", ms, calls, count, args.top, sum(step_ms))

    # Worker scopes that start during a step: the processors' parallel work, and the rest of the frame beside it.
    starts = [r[2] for r in steps]
    during = []
    for r in workers:
        k = bisect.bisect_right(starts, r[2]) - 1
        if k >= 0 and r[2] < steps[k][3]:
            during.append(r)
    ms, calls = inclusive(during)
    table("Worker threads during the steps (busy time, nested scopes included)", ms, calls, count, args.top)

    frames = [r for r in game if r[1] == FRAME and r[4] == 0]
    if frames:
        frame_ms = sum((r[3] - r[2]) * 1000.0 for r in frames)
        print(f"\n## Game thread frame\n\n{len(frames)} frames, {frame_ms / len(frames):.3f} ms per frame, of which"
              f" {sum(step_ms) / len(frames):.3f} ms of steps ({count / len(frames):.2f} steps per frame)")

    if args.scopes:
        chosen = [r for r in rows if any(matches(r[1], s) for s in args.scopes)]
        ms, calls = inclusive(chosen)
        table("Chosen scopes (every thread)", ms, calls, count, len(ms))

    print(f"\n## Longest steps (game thread scopes over {args.min_ms} ms)")
    for step in sorted(steps, key=lambda r: r[2] - r[3])[:args.long]:
        print(f"\n=== step at {step[2] - window[0]:.3f} s: {(step[3] - step[2]) * 1000.0:.2f} ms")
        for r, owner, level in tree:
            if owner is step and (r[3] - r[2]) * 1000.0 >= args.min_ms:
                print(f"{'  ' * level}{(r[3] - r[2]) * 1000.0:7.2f} {r[1][:100]}")


if __name__ == "__main__":
    main()
