# agents_table.ps1 (L15-4) : BOUCHON du commit rouge, remplace par l'outil au commit vert.
param(
    [string] $Add,
    [string] $Id,
    [string] $Worktree,
    [string] $Task,
    [ValidateSet('oui', 'non')] [string] $Cpp,
    [string] $Update,
    [string] $Summary,
    [string] $Jour
)
Write-Output 'agents_table : PAS IMPLEMENTE (bouchon)'
exit 9
