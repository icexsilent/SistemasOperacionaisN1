#!/usr/bin/env python3
"""Executa cenarios reais e valida os logs com o verificador da disciplina."""
from collections import Counter
from pathlib import Path
import json
import platform
import subprocess
import sys

RAIZ = Path(__file__).resolve().parent
SAIDA = RAIZ / "resultados"
CAMPOS = ("produtores", "consumidores", "capacidade", "recursos-a", "recursos-b", "itens")
GLOBAIS = [
    ("minimo", (1, 1, 1, 1, 1, 1)),
    ("padrao", (3, 4, 5, 2, 2, 30)),
    ("mais_produtores", (12, 2, 1, 1, 1, 3)),
    ("mais_consumidores", (2, 12, 1, 1, 1, 3)),
    ("fila_ampla", (4, 6, 100, 2, 3, 30)),
    ("recurso_a_limitado", (4, 12, 3, 1, 4, 300)),
    ("recurso_b_limitado", (4, 12, 3, 4, 1, 300)),
    ("concorrencia", (8, 12, 1, 2, 2, 1000)),
]
DEADLOCKS = [
    ("minimo", (1, 2, 1, 1, 1, 20)),
    ("padrao", (3, 4, 5, 2, 2, 30)),
]


def executar(executavel, nome, valores, ordem, semente=1):
    cfg = dict(zip(CAMPOS, valores))
    comando = [str(executavel)]
    for campo, valor in cfg.items():
        comando.extend(["--" + campo, str(valor)])
    comando.extend(["--semente", str(semente), "--ordem", ordem])
    log = SAIDA / (nome + ".log")
    diagnostico = SAIDA / (nome + ".stderr.txt")
    travou = False
    with log.open("wb") as saida, diagnostico.open("wb") as erros:
        try:
            processo = subprocess.run(
                comando, cwd=RAIZ, stdout=saida, stderr=erros,
                timeout=2 if ordem == "ingenua" else 15,
            )
        except subprocess.TimeoutExpired:
            travou = True
    if ordem == "global":
        if travou:
            raise RuntimeError("modo global ultrapassou 15 segundos")
        if processo.returncode != 0:
            raise RuntimeError("simulador terminou com codigo " + str(processo.returncode))
    elif not travou:
        raise RuntimeError("modo ingenua terminou sem reproduzir o deadlock")

    verificacao = [sys.executable, str(RAIZ / "verificar.py"), str(log)]
    if ordem == "ingenua":
        verificacao.append("--deadlock")
    else:
        for campo in ("itens", "recursos-a", "recursos-b"):
            verificacao.extend(["--" + campo, str(cfg[campo])])
    resultado = subprocess.run(
        verificacao, cwd=RAIZ, capture_output=True, text=True,
        encoding="utf-8", errors="replace", timeout=15,
    )
    (SAIDA / (nome + ".verificacao.txt")).write_text(
        resultado.stdout + resultado.stderr, encoding="utf-8",
    )
    if resultado.returncode != 0:
        raise RuntimeError(resultado.stdout + resultado.stderr)

    # Checagens adicionais: o verificador original nao exige a contagem
    # configurada de threads nem IDs exatamente no intervalo 1..N.
    eventos = [linha.split(";") for linha in log.read_text(encoding="utf-8").splitlines()]
    esperadas = {
        *(f"prod-{i + 1}" for i in range(cfg["produtores"])),
        *(f"cons-{i + 1}" for i in range(cfg["consumidores"])),
    }
    criadas = Counter(e[1] for e in eventos if e[2] == "CRIADA")
    if criadas != Counter({nome: 1 for nome in esperadas}):
        raise RuntimeError("quantidade ou identidade das threads difere da configuracao")
    if ordem == "global":
        produzidos = sorted(int(e[3]) for e in eventos if e[2] == "PRODUZIU")
        if produzidos != list(range(1, cfg["itens"] + 1)):
            raise RuntimeError("IDs produzidos nao formam a sequencia 1..N")
    else:
        em_uso = Counter()
        for e in eventos:
            if e[2] == "OBTEM":
                em_uso[e[3]] += 1
            elif e[2] == "LIBERA":
                em_uso[e[3]] -= 1
        if em_uso != Counter({"A": cfg["recursos-a"], "B": cfg["recursos-b"]}):
            raise RuntimeError("o ciclo relatado nao esgotou as instancias de A e B")
    return {"nome": nome, "ordem": ordem, "config": cfg, "eventos": len(eventos), "ok": True}


def main():
    executavel = RAIZ / ("simulador.exe" if sys.platform in ("win32", "msys", "cygwin") else "simulador")
    if not executavel.exists():
        print("Compile primeiro com make.", file=sys.stderr)
        return 1
    SAIDA.mkdir(exist_ok=True)
    resultados = []
    casos = [("global_" + nome, cfg, "global", 1) for nome, cfg in GLOBAIS]
    # Duas execucoes por configuracao verificam a reproducao do ciclo.
    casos += [
        (f"deadlock_{nome}_{repeticao}", cfg, "ingenua", repeticao)
        for nome, cfg in DEADLOCKS for repeticao in (1, 2)
    ]
    for nome, cfg, ordem, semente in casos:
        try:
            resultado = executar(executavel, nome, cfg, ordem, semente)
            print(f"OK: {nome} ({resultado['eventos']} eventos)", flush=True)
        except (RuntimeError, OSError, ValueError, subprocess.TimeoutExpired) as erro:
            resultado = {"nome": nome, "ok": False, "erro": str(erro)}
            print(f"FALHA: {nome}: {erro}", flush=True)
        resultados.append(resultado)
    (SAIDA / "resumo.json").write_text(
        json.dumps({"plataforma": platform.platform(), "casos": resultados},
                   ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    aprovados = sum(r["ok"] for r in resultados)
    print(f"\n{aprovados}/{len(resultados)} cenarios aprovados. Logs em {SAIDA}")
    return 0 if aprovados == len(resultados) else 1


if __name__ == "__main__":
    sys.exit(main())
