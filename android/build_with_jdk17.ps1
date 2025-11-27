<#
build_with_jdk17.ps1

This script prefers a locally-installed JDK 17 (recommended). Only when no suitable
JDK 17 is found will it download a portable OpenJDK 17 into the `android/` folder.

Usage (recommended):
  # Use system JAVA_HOME (PowerShell)
  $env:JAVA_HOME = 'C:\Program Files\Eclipse Adoptium\jdk-17.0.2'
  .\build_with_jdk17.ps1

If you don't have JDK 17 installed, the script will download OpenJDK 17 automatically.
Note: Do NOT commit downloaded JDK into the repo; `.gitignore` already ignores it.
#>

$ErrorActionPreference = 'Stop'

function Test-Java17($javaExe) {
    if (-not (Test-Path $javaExe)) { return $false }
    try {
        $out = & "$javaExe" -version 2>&1 | Out-String
        return $out -match '"?17(\.|$)'
    } catch {
        return $false
    }
}

# 1) Prefer explicit environment variable if set and valid
if ($env:JAVA_HOME) {
    $javabin = Join-Path $env:JAVA_HOME 'bin\java.exe'
    if (Test-Java17 $javabin) {
        Write-Host "Using existing JAVA_HOME: $env:JAVA_HOME" -ForegroundColor Yellow
    } else {
        Write-Host "WARNING: Environment JAVA_HOME does not appear to be JDK 17. Ignoring." -ForegroundColor Yellow
        Remove-Item Env:JAVA_HOME -ErrorAction SilentlyContinue
    }
}

# 2) If no JAVA_HOME, try common installation directories
if (-not $env:JAVA_HOME) {
    $candidates = @(
        'C:\Program Files\Eclipse Adoptium',
        'C:\Program Files\AdoptOpenJDK',
        'C:\Program Files\Zulu',
        'C:\Program Files\Amazon Corretto'
    )
    foreach ($dir in $candidates) {
        if (Test-Path $dir) {
            $found = Get-ChildItem -Path $dir -Directory -Filter 'jdk*' -ErrorAction SilentlyContinue | Sort-Object Name -Descending | Select-Object -First 1
            if ($found) {
                $javabin = Join-Path $found.FullName 'bin\java.exe'
                if (Test-Java17 $javabin) {
                    $env:JAVA_HOME = $found.FullName
                    Write-Host "Found JDK 17 at: $env:JAVA_HOME" -ForegroundColor Yellow
                    break
                }
            }
        }
    }
}

# 3) Fallback: download portable OpenJDK 17 into android/ (only if no JDK found)
if (-not $env:JAVA_HOME) {
    $jdk17Url = 'https://download.java.net/java/GA/jdk17.0.2/dfd4a8d0985749f896bed50d7138ee7f/8/GPL/openjdk-17.0.2_windows-x64_bin.zip'
    $jdk17Zip = '.\openjdk-17.zip'
    $jdk17Dir = '.\jdk-17'

    if (-not (Test-Path $jdk17Dir)) {
        Write-Host "No JDK 17 found locally — downloading a portable OpenJDK 17 into the android folder..." -ForegroundColor Cyan
        Invoke-WebRequest -Uri $jdk17Url -OutFile $jdk17Zip -UseBasicParsing
        Write-Host "Extracting JDK 17..." -ForegroundColor Cyan
        Expand-Archive -Path $jdk17Zip -DestinationPath '.' -Force
        if (Test-Path '.\jdk-17.0.2') {
            Rename-Item -Path '.\jdk-17.0.2' -NewName 'jdk-17' -Force
        }
        Remove-Item $jdk17Zip -Force -ErrorAction SilentlyContinue
        Write-Host 'Portable JDK 17 is ready (not committed).' -ForegroundColor Green
    }
    $env:JAVA_HOME = (Resolve-Path $jdk17Dir).Path
}

# Ensure ANDROID_HOME is set (users can override)
if (-not $env:ANDROID_HOME) {
    $env:ANDROID_HOME = 'C:\Users\hakri\AppData\Local\Android\Sdk'
}

Write-Host "Using JAVA_HOME: $env:JAVA_HOME" -ForegroundColor Yellow
Write-Host "Building APK..." -ForegroundColor Cyan

& .\gradlew.bat clean assembleDebug

Write-Host "`nBuild complete!" -ForegroundColor Green
