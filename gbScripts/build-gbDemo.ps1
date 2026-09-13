param(
    [ValidateSet("debug-client", "release-client")]
    [string]$Configuration = "debug-client",
    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"
$workspaceRoot = Split-Path -Parent $PSScriptRoot
$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vulkanSdk = [Environment]::GetEnvironmentVariable("VULKAN_SDK", "Machine")

if (-not $vulkanSdk -or -not (Test-Path -LiteralPath $vulkanSdk)) {
    throw "Vulkan SDK was not found. Install it from https://vulkan.lunarg.com/."
}

$env:VULKAN_SDK = $vulkanSdk

if (-not (Test-Path -LiteralPath $vswhere)) {
    throw "Visual Studio Installer (vswhere.exe) was not found. Install Visual Studio 2022 with Desktop development with C++."
}

$visualStudioRoot = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
if (-not $visualStudioRoot) {
    throw "MSBuild was not found. Install the Desktop development with C++ workload."
}

$msbuild = Join-Path $visualStudioRoot "MSBuild\Current\Bin\MSBuild.exe"
$projects = @(
    "gbCore\gbCore.vcxproj",
    "gbSound\gb_sound.vcxproj",
    "gbDatabase\gb_database.vcxproj",
    "gbUI\gbUI.vcxproj",
    "gbNetwork\gbNetwork.vcxproj",
    "gbDemo\gbDemo.vcxproj"
)

foreach ($project in $projects) {
    Write-Host "Building $project ($Configuration|$Platform)..." -ForegroundColor Cyan
    & $msbuild (Join-Path $workspaceRoot $project) /m /t:Build `
        "/p:Configuration=$Configuration" "/p:Platform=$Platform" `
        /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0 `
        /p:CharacterSet=NotSet `
        /nologo /v:minimal
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed: $project"
    }
}

$architectureFolder = if ($Platform -eq "x64") { "x64" } else { "x86" }
$buildKind = if ($Configuration -eq "debug-client") { "debug" } else { "release" }
$executable = Join-Path $workspaceRoot "gbDemo\output\$buildKind\$architectureFolder\gb_demo.exe"
$outputRoot = Split-Path -Parent $executable
$shadercLibrary = Join-Path $vulkanSdk "Bin\shaderc_shared.dll"
$shadercOutputLibrary = Join-Path $outputRoot "shaderc_shared.dll"
if (-not (Test-Path -LiteralPath $shadercOutputLibrary) -or
    (Get-FileHash -LiteralPath $shadercLibrary).Hash -ne (Get-FileHash -LiteralPath $shadercOutputLibrary).Hash) {
    Copy-Item -LiteralPath $shadercLibrary -Destination $outputRoot -Force
}

$bundleRoot = Join-Path $workspaceRoot "gbBundle"
$resourcesRoot = Join-Path $workspaceRoot "gbResources"
$sharedResourcesRoot = Join-Path $workspaceRoot "gbWin32SharedResources"
$resourceExtensions = @(".xml", ".json", ".vert", ".frag", ".ani", ".png", ".gb3dmesh", ".gb3danim", ".ttf", ".otf", ".tmx", ".tsx", ".mp3", ".fcl")
New-Item -ItemType Directory -Path $sharedResourcesRoot -Force | Out-Null
foreach ($resourceRoot in @($bundleRoot, $resourcesRoot)) {
    Get-ChildItem -LiteralPath $resourceRoot -Recurse -File | Where-Object {
        $resourceExtensions -contains $_.Extension.ToLowerInvariant()
    } | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $sharedResourcesRoot $_.Name) -Force
    }
}

Write-Host "Build completed: $executable" -ForegroundColor Green
