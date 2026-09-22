param(
    [ValidateSet('compilar', 'testar', 'demonstrar', 'limpar')]
    [string]$Acao = 'testar',
    [string]$Msys2Root = $env:MSYS2_ROOT
)

$ErrorActionPreference = 'Stop'
$raizProjeto = Split-Path -Parent $PSScriptRoot
$candidatos = @($Msys2Root, 'E:\Tools\msys64', 'C:\msys64', 'E:\msys64')
$bash = $null
foreach ($candidato in $candidatos) {
    if ($candidato) {
        $caminho = Join-Path $candidato 'usr\bin\bash.exe'
        if (Test-Path -LiteralPath $caminho) { $bash = $caminho; break }
    }
}
if (-not $bash) {
    throw 'MSYS2 nao encontrado. Instale conforme TESTES.md ou defina MSYS2_ROOT.'
}

$comandos = @{
    compilar = 'make'
    testar = 'make test'
    demonstrar = 'make && ./simulador --produtores 3 --consumidores 4 --capacidade 5 --recursos-a 2 --recursos-b 2 --itens 30 --semente 1 --ordem global > simulacao.log && python3 verificar.py simulacao.log --itens 30 --recursos-a 2 --recursos-b 2'
    limpar = 'make clean'
}
$msystemAnterior = $env:MSYSTEM
$chereAnterior = $env:CHERE_INVOKING
Push-Location -LiteralPath $raizProjeto
try {
    $env:MSYSTEM = 'UCRT64'
    $env:CHERE_INVOKING = '1'
    & $bash --login -c $comandos[$Acao]
    $codigo = $LASTEXITCODE
} finally {
    Pop-Location
    $env:MSYSTEM = $msystemAnterior
    $env:CHERE_INVOKING = $chereAnterior
}
exit $codigo
