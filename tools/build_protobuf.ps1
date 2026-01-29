# Tools/build_protobuf.ps1
param(
  [ValidateSet("Release","Debug")]
  [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

# repo root 기준 경로
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$SrcDir   = Join-Path $RepoRoot "External/protobuf"
$BuildDir = Join-Path $RepoRoot "External/protobuf_build"

function Require-Command($name) {
  if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
    throw "Required command not found: $name. Install it and ensure it's on PATH."
  }
}

Write-Host "[protobuf] repo root: $RepoRoot"
Require-Command "cmake"

if (-not (Test-Path $SrcDir)) {
  throw "Protobuf source not found at $SrcDir. Did you init submodules? (git submodule update --init --recursive)"
}

# VS generator가 시스템에 있는지 확인 (없으면 안내)
# -G "Visual Studio 17 2022"는 VS2022 Build Tools 이상이 설치되어야 동작
$Generator = "Visual Studio 17 2022"

Write-Host "[protobuf] configure..."
cmake -S $SrcDir -B $BuildDir -G $Generator -A x64 `
  -Dprotobuf_BUILD_TESTS=OFF `
  -Dprotobuf_BUILD_EXAMPLES=OFF `
  -Dprotobuf_WITH_ZLIB=OFF `
  -Dprotobuf_BUILD_SHARED_LIBS=OFF

Write-Host "[protobuf] build ($Config)..."
cmake --build $BuildDir --config $Config --target protoc libprotobuf

# 결과물 위치 안내
$BinDir = Join-Path $BuildDir $Config
Write-Host "[protobuf] done."
Write-Host "  protoc:      $BinDir\protoc.exe"
Write-Host "  libprotobuf: $BinDir\libprotobuf.lib"
