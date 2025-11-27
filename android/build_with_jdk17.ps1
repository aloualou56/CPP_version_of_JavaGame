# Download and use JDK 17 for Gradle build
$ErrorActionPreference = "Stop"

$jdk17Url = "https://download.java.net/java/GA/jdk17.0.2/dfd4a8d0985749f896bed50d7138ee7f/8/GPL/openjdk-17.0.2_windows-x64_bin.zip"
$jdk17Zip = ".\openjdk-17.zip"
$jdk17Dir = ".\jdk-17"

if (-not (Test-Path $jdk17Dir)) {
    Write-Host "Downloading JDK 17..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri $jdk17Url -OutFile $jdk17Zip -UseBasicParsing
    
    Write-Host "Extracting JDK 17..." -ForegroundColor Cyan
    Expand-Archive -Path $jdk17Zip -DestinationPath "." -Force
    
    if (Test-Path ".\jdk-17.0.2") {
        Rename-Item -Path ".\jdk-17.0.2" -NewName "jdk-17" -Force
    }
    
    Remove-Item $jdk17Zip -Force -ErrorAction SilentlyContinue
    Write-Host "JDK 17 ready!" -ForegroundColor Green
}

if (-not (Test-Path $jdk17Dir) -and (Test-Path ".\jdk-17.0.2")) {
    $jdk17Dir = ".\jdk-17.0.2"
}

$env:JAVA_HOME = (Resolve-Path "$jdk17Dir").Path
$env:ANDROID_HOME = "C:\Users\hakri\AppData\Local\Android\Sdk"

Write-Host "Using JAVA_HOME: $env:JAVA_HOME" -ForegroundColor Yellow
Write-Host "Building APK..." -ForegroundColor Cyan

& .\gradlew.bat clean assembleDebug

Write-Host "`nBuild complete!" -ForegroundColor Green
