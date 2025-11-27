# setup_sdl3_android.ps1
# Extracts SDL3 and SDL3_image AARs and copies native libraries for CMake

$ErrorActionPreference = "Stop"
$cpp = "C:\Users\hakri\Downloads\CPP_version_of_JavaGame\android\app\src\main\cpp"

Write-Host "Setting up SDL3 Android libraries..." -ForegroundColor Cyan

# Extract SDL3 AAR
Write-Host "Extracting SDL3 AAR..." -ForegroundColor Yellow
Set-Location $cpp
if (Test-Path ".\SDL3\aar_unpacked") { Remove-Item -Recurse -Force ".\SDL3\aar_unpacked" }
Expand-Archive -Path ".\SDL3\SDL3-3.2.26.aar" -DestinationPath ".\SDL3\aar_unpacked" -Force

# Extract SDL3_image AAR
Write-Host "Extracting SDL3_image AAR..." -ForegroundColor Yellow
if (Test-Path ".\SDL3_image\aar_unpacked") { Remove-Item -Recurse -Force ".\SDL3_image\aar_unpacked" }
Expand-Archive -Path ".\SDL3_image\SDL3_image-3.2.4.aar" -DestinationPath ".\SDL3_image\aar_unpacked" -Force

# Copy SDL3 native libraries
Write-Host "Copying SDL3 native libraries..." -ForegroundColor Yellow
$sdl3LibsPath = ".\SDL3\aar_unpacked\prefab\modules\SDL3-shared\libs"
if (Test-Path $sdl3LibsPath) {
    Get-ChildItem -Path $sdl3LibsPath -Directory | ForEach-Object {
        $abiFolder = $_.Name
        $abi = $abiFolder -replace '^android\.', ''
        $destDir = Join-Path ".\SDL3\lib" $abi
        New-Item -ItemType Directory -Force -Path $destDir | Out-Null
        
        $srcFile = Join-Path $_.FullName "libSDL3.so"
        if (Test-Path $srcFile) {
            $destFile = Join-Path $destDir "libSDL3.so"
            Copy-Item -Path $srcFile -Destination $destFile -Force
            Write-Host "  Copied libSDL3.so for $abi" -ForegroundColor Green
        }
    }
} else {
    Write-Host "  Warning: No SDL3 libs found in AAR" -ForegroundColor Red
}

# Copy SDL3_image native libraries
Write-Host "Copying SDL3_image native libraries..." -ForegroundColor Yellow
$sdl3ImageLibsPath = ".\SDL3_image\aar_unpacked\prefab\modules\SDL3_image-shared\libs"
if (Test-Path $sdl3ImageLibsPath) {
    Get-ChildItem -Path $sdl3ImageLibsPath -Directory | ForEach-Object {
        $abiFolder = $_.Name
        $abi = $abiFolder -replace '^android\.', ''
        $destDir = Join-Path ".\SDL3_image\lib" $abi
        New-Item -ItemType Directory -Force -Path $destDir | Out-Null
        
        $srcFile = Join-Path $_.FullName "libSDL3_image.so"
        if (Test-Path $srcFile) {
            $destFile = Join-Path $destDir "libSDL3_image.so"
            Copy-Item -Path $srcFile -Destination $destFile -Force
            Write-Host "  Copied libSDL3_image.so for $abi" -ForegroundColor Green
        }
    }
} else {
    Write-Host "  Warning: No SDL3_image libs found in AAR" -ForegroundColor Red
}

# Copy SDL3 headers if needed
Write-Host "Checking SDL3 headers..." -ForegroundColor Yellow
if (Test-Path ".\SDL3\aar_unpacked\prefab\modules\SDL3-Headers\include") {
    if (-not (Test-Path ".\SDL3\include")) {
        New-Item -ItemType Directory -Force -Path ".\SDL3\include" | Out-Null
    }
    Copy-Item -Path ".\SDL3\aar_unpacked\prefab\modules\SDL3-Headers\include\*" -Destination ".\SDL3\include\" -Recurse -Force
    Write-Host "  Copied SDL3 headers" -ForegroundColor Green
}

# Copy SDL3_image headers if needed
Write-Host "Checking SDL3_image headers..." -ForegroundColor Yellow
if (Test-Path ".\SDL3_image\aar_unpacked\prefab\modules\SDL3_image-shared\include") {
    if (-not (Test-Path ".\SDL3_image\include")) {
        New-Item -ItemType Directory -Force -Path ".\SDL3_image\include" | Out-Null
    }
    Copy-Item -Path ".\SDL3_image\aar_unpacked\prefab\modules\SDL3_image-shared\include\*" -Destination ".\SDL3_image\include\" -Recurse -Force
    Write-Host "  Copied SDL3_image headers" -ForegroundColor Green
}

Write-Host "`nSetup complete!" -ForegroundColor Green
Write-Host "`nLibraries installed:" -ForegroundColor Cyan
if (Test-Path ".\SDL3\lib") {
    Get-ChildItem -Recurse -Path ".\SDL3\lib" -Filter "*.so" | ForEach-Object {
        Write-Host "  $($_.FullName)" -ForegroundColor Gray
    }
}
if (Test-Path ".\SDL3_image\lib") {
    Get-ChildItem -Recurse -Path ".\SDL3_image\lib" -Filter "*.so" | ForEach-Object {
        Write-Host "  $($_.FullName)" -ForegroundColor Gray
    }
}

Write-Host "`nYou can now run: cd ..\..\..\.. ; cd android ; .\gradlew.bat assembleDebug" -ForegroundColor Cyan
