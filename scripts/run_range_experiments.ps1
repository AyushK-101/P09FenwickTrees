#requires -Version 5.1

[CmdletBinding()]
param(
    [string] $OutputDirectory,
    [string] $Compiler,
    [string] $PythonExecutable,
    [switch] $TestsOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Find-Application {
    param([string] $Name)

    $command = Get-Command -Name $Name -CommandType Application -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($null -ne $command) {
        return $command.Source
    }
}

function Find-Compiler {
    if ($Compiler) {
        $selected = Find-Application $Compiler
        if (-not $selected) { throw "Compiler executable not found at '$Compiler'." }
        return $selected
    }

    $drive = $env:SystemDrive
    $candidates = @(
        'cl.exe'
        'g++.exe'
        "$drive\msys64\ucrt64\bin\g++.exe"
        "$drive\msys64\mingw64\bin\g++.exe"
        "$drive\mingw64\bin\g++.exe"
    )
    foreach ($candidate in $candidates) {
        $selected = Find-Application $candidate
        if ($selected) { return $selected }
    }
    throw 'No native C++ compiler found. Use an x64 Developer PowerShell with MSVC, or pass the path to MinGW-w64 g++.exe with -Compiler.'
}

function Find-Python {
    $candidates = @('py.exe', 'python.exe')
    if ($env:LOCALAPPDATA) {
        $pythonRoot = Join-Path $env:LOCALAPPDATA 'Python'
        $candidates += @(Get-ChildItem -LiteralPath $pythonRoot -Directory -Filter 'pythoncore-*' -ErrorAction SilentlyContinue |
            ForEach-Object { Join-Path $_.FullName 'python.exe' })
    }
    if ($PythonExecutable) {
        $candidates = @($PythonExecutable)
    }

    foreach ($candidate in $candidates) {
        $selected = Find-Application $candidate
        if ($selected) {
            try {
                & $selected -c 'import sys, matplotlib; from packaging.version import Version; sys.exit(0 if sys.version_info >= (3,8) and Version(matplotlib.__version__) >= Version(''3.6'') else 1)' 2>$null
                if ($LASTEXITCODE -eq 0) { return $selected }
            } catch {
                # A Windows Store alias can be present even when it cannot launch Python.
                continue
            }
        }
    }
    throw 'Python 3.8+ with Matplotlib 3.6+ is required for a full run. To compile and run just the tests, pass -TestsOnly.'
}

function Invoke-Checked {
    param([string] $Program, [string[]] $Arguments, [string] $Step)

    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Step failed with exit code $LASTEXITCODE."
    }
}

function Build-Target {
    param([string] $Source, [string] $Name)

    $executable = Join-Path $buildDirectory "$Name.exe"
    if ($isMsvc) {
        $object = Join-Path $buildDirectory "$Name.obj"
        $arguments = $compilerFlags + @("/Fo$object", "/Fe$executable", $Source)
    } else {
        $arguments = $compilerFlags + @($Source, '-o', $executable)
    }
    Invoke-Checked $nativeCompiler $arguments "Build $Name"
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$savedPath = $env:Path
Push-Location -LiteralPath $projectRoot
try {
    $nativeCompiler = Find-Compiler
    $compilerName = [IO.Path]::GetFileName($nativeCompiler)
    if ($compilerName -notin @('cl.exe', 'g++.exe')) {
        throw "Unsupported compiler '$compilerName'. Use MSVC cl.exe or MinGW-w64 g++.exe."
    }
    $env:Path = "$(Split-Path -Parent $nativeCompiler);$savedPath"
    $isMsvc = $compilerName -eq 'cl.exe'
    if ($isMsvc) {
        $compilerFlags = @('/nologo', '/std:c++17', '/O2', '/EHsc', '/utf-8')
        $compilerVersion = (Get-Item -LiteralPath $nativeCompiler).VersionInfo.FileVersion
        $toolchain = 'msvc'
    } else {
        $target = & $nativeCompiler -dumpmachine
        if ($LASTEXITCODE -ne 0 -or $target -notmatch '^[^-]+-w64-mingw32$') {
            throw "Use a MinGW-w64 Windows target. Compiler target reported '$target'."
        }
        $compilerFlags = @('-std=c++17', '-O2')
        $compilerVersion = & $nativeCompiler -dumpfullversion -dumpversion
        if ($LASTEXITCODE -ne 0) { throw 'Could not query the compiler version.' }
        $toolchain = 'mingw'
    }

    if (-not $TestsOnly) {
        if (-not $OutputDirectory) {
            $OutputDirectory = 'results/range_windows_{0:yyyyMMdd_HHmmss_fff}' -f (Get-Date)
        }
        $runDirectory = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
        if (Test-Path -LiteralPath $runDirectory) {
            throw "Choose a new output directory. '$runDirectory' already exists."
        }
        $python = Find-Python
    }

    $buildDirectory = Join-Path $projectRoot "build/windows-$toolchain"
    New-Item -ItemType Directory -Path $buildDirectory -Force | Out-Null
    Write-Host "Building with $compilerName $compilerVersion"
    foreach ($name in @('test_correctness', 'test_fenwick_advanced', 'test_range_structures')) {
        Build-Target -Source (Join-Path $projectRoot "tests/$name.cpp") -Name $name
        Invoke-Checked (Join-Path $buildDirectory "$name.exe") @() "Run $name"
    }

    if (-not $TestsOnly) {
        $benchmarkName = 'benchmark_range_structures'
        Build-Target -Source (Join-Path $projectRoot "benchmarks/$benchmarkName.cpp") -Name $benchmarkName
        Invoke-Checked (Join-Path $buildDirectory "$benchmarkName.exe") @($runDirectory) 'Benchmark'
        @(
            "Compiler: $compilerName $compilerVersion"
            "Flags: $($compilerFlags -join ' ')"
            "Windows version: $([Environment]::OSVersion.VersionString)"
            "Processor: $env:PROCESSOR_IDENTIFIER"
            "Run time: $([DateTimeOffset]::Now.ToString('o'))"
        ) | Set-Content -LiteralPath (Join-Path $runDirectory 'run_environment.txt') -Encoding UTF8
        $analysisScript = Join-Path $PSScriptRoot 'analyze_range_structures.py'
        Invoke-Checked $python @($analysisScript, $runDirectory) 'Analyze benchmark'
        Write-Host "Results: $runDirectory"
    }
} finally {
    $env:Path = $savedPath
    Pop-Location
}
