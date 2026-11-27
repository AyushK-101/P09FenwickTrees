#requires -Version 5.1
[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $Executable,
    [Parameter(Mandatory)] [string] $OutputCsv
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$exe = (Resolve-Path -LiteralPath $Executable).Path
if (Test-Path -LiteralPath $OutputCsv) { throw 'Choose a new CSV path.' }
$rows = @(
    foreach ($trial in 1..6) {
        $point = @('point-fenwick', 'point-segment')
        $range = @('range-fenwick', 'range-lazy')
        if ($trial % 2 -eq 0) {
            [array]::Reverse($point)
            [array]::Reverse($range)
        }
        $samples = @{}
        foreach ($mode in ($point + $range + @('assign-lazy'))) {
            $fields = & $exe --profile $mode 30
            if ($LASTEXITCODE) { throw "Profile process failed for $mode." }
            $pattern = '^operations=(\d+) checksum=(\d+) replay_ms=([\d.eE+-]+) storage_bytes=(\d+) thread_cycles=(\d+)$'
            if ($fields -notmatch $pattern) { throw "No valid Windows cycle reading for $mode." }
            $operations = [long]$Matches[1]
            $cycles = [long]$Matches[5]
            if ($operations -ne 3000000 -or $cycles -le 0) { throw "Unexpected sample for $mode." }
            $samples[$mode] = $Matches[2]
            [pscustomobject][ordered]@{
                mode = $mode
                trial = $trial
                operations = $operations
                checksum = $Matches[2]
                replay_ms = $Matches[3]
                storage_bytes = $Matches[4]
                thread_cycles = $cycles
            }
        }
        if ($samples['point-fenwick'] -ne $samples['point-segment'] -or
            $samples['range-fenwick'] -ne $samples['range-lazy']) {
            throw "Paired profile checksums differ in trial $trial."
        }
    }
)
$rows | Export-Csv -LiteralPath $OutputCsv -NoClobber -NoTypeInformation -Encoding UTF8
