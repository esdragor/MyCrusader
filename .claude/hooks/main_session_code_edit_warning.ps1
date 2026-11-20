# PreToolUse hook on Edit|Write (2026-11-20, lesson L19-1): warn, WITHOUT blocking, when the MAIN session
# (not a subagent) edits code under Script\ or Source\ of MyCrusader or a HeroesOfLiberty-* worktree.
# A subagent's tool call carries an agent_id in the hook input: no warning then.
$ErrorActionPreference = 'SilentlyContinue'
$raw = [Console]::In.ReadToEnd()
try { $in = $raw | ConvertFrom-Json } catch { exit 0 }
if ($in.agent_id) { exit 0 }
$path = [string]$in.tool_input.file_path
if (-not $path) { exit 0 }
$norm = $path.Replace('/', '\')
if ($norm -notmatch '(?i)\\(MyCrusader|HeroesOfLiberty-[^\\]+)\\(Script|Source)\\') { exit 0 }

$msg = "Chef d'orchestre (L19-1) : la session principale edite du code ($path). Ce travail devrait aller a un sous-agent (correction, diagnostic, retouche)."
$out = @{
    systemMessage = $msg
    hookSpecificOutput = @{ hookEventName = 'PreToolUse'; additionalContext = $msg }
}
$out | ConvertTo-Json -Depth 4 -Compress
exit 0
