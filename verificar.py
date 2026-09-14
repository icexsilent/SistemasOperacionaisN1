#!/usr/bin/env python3
"""Verifica os invariantes do log do Projeto 1.

    python3 verificar.py simulacao.log --itens 30 --recursos-a 2 --recursos-b 2
    python3 verificar.py travado.log --deadlock

Codigo de saida 0 se tudo passou, 1 se houve falha.
"""

import argparse
import sys
from collections import defaultdict

EVENTOS = {
    "CRIADA", "PRODUZIU", "BLOQUEIA_CHEIA", "BLOQUEIA_VAZIA",
    "SOLICITA", "OBTEM", "LIBERA", "CONSUMIU", "ENCERRADA",
}

falhas = []
avisos = []


def falha(msg):
    falhas.append(msg)


def aviso(msg):
    avisos.append(msg)


def ler(caminho):
    eventos = []
    with open(caminho, encoding="utf-8", errors="replace") as f:
        for n, linha in enumerate(f, 1):
            linha = linha.rstrip("\n")
            if not linha.strip():
                continue
            campos = linha.split(";")
            if len(campos) != 5:
                falha(f"linha {n}: {len(campos)} campos, esperado 5 -> {linha[:70]!r}")
                continue
            t, tid, ev, obj, det = campos
            if not t.strip().isdigit():
                falha(f"linha {n}: tempo nao numerico -> {t!r}")
                continue
            if ev not in EVENTOS:
                falha(f"linha {n}: evento fora do vocabulario -> {ev!r}")
                continue
            if not tid.strip():
                falha(f"linha {n}: thread_id vazio")
                continue
            eventos.append((int(t), tid, ev, obj, det, n))
    return eventos


def ciclo_de_vida(ev):
    criadas, encerradas = defaultdict(int), defaultdict(int)
    for _, tid, e, _, _, n in ev:
        if e == "CRIADA":
            criadas[tid] += 1
            if criadas[tid] > 1:
                falha(f"linha {n}: CRIADA repetida para {tid}")
        elif e == "ENCERRADA":
            encerradas[tid] += 1
            if encerradas[tid] > 1:
                falha(f"linha {n}: ENCERRADA repetida para {tid}")

    for tid in criadas:
        if encerradas[tid] == 0:
            falha(f"thread {tid} tem CRIADA sem ENCERRADA")
    for tid in encerradas:
        if criadas[tid] == 0:
            falha(f"thread {tid} tem ENCERRADA sem CRIADA")

    primeiro, ultimo = {}, {}
    for _, tid, e, _, _, _ in ev:
        primeiro.setdefault(tid, e)
        ultimo[tid] = e
    for tid, e in primeiro.items():
        if e != "CRIADA":
            falha(f"thread {tid}: primeiro evento e {e}, deveria ser CRIADA")
    for tid, e in ultimo.items():
        if e != "ENCERRADA":
            falha(f"thread {tid}: ultimo evento e {e}, deveria ser ENCERRADA")


def itens(ev, esperado):
    prod, cons = [], []
    ja_produzidos = set()
    for _, _, e, obj, _, n in ev:
        if e == "PRODUZIU":
            if not obj.strip():
                falha(f"linha {n}: PRODUZIU sem id de item")
            prod.append(obj)
            ja_produzidos.add(obj)
        elif e == "CONSUMIU":
            if not obj.strip():
                falha(f"linha {n}: CONSUMIU sem id de item")
            if obj not in ja_produzidos:
                falha(f"linha {n}: CONSUMIU do item {obj!r} antes do "
                      f"PRODUZIU correspondente")
            cons.append(obj)

    if len(prod) != len(set(prod)):
        rep = sorted({i for i in prod if prod.count(i) > 1})
        falha(f"itens produzidos mais de uma vez: {rep[:10]}")
    if len(cons) != len(set(cons)):
        rep = sorted({i for i in cons if cons.count(i) > 1})
        falha(f"itens consumidos mais de uma vez: {rep[:10]}")

    if esperado is not None:
        if len(prod) != esperado:
            falha(f"PRODUZIU {len(prod)} vezes, --itens era {esperado}")
        if len(cons) != esperado:
            falha(f"CONSUMIU {len(cons)} vezes, --itens era {esperado}")

    perdidos = set(prod) - set(cons)
    fantasmas = set(cons) - set(prod)
    if perdidos:
        falha(f"itens produzidos e nunca consumidos: {sorted(perdidos)[:10]}")
    if fantasmas:
        falha(f"itens consumidos sem terem sido produzidos: {sorted(fantasmas)[:10]}")


def recursos(ev, cap_a, cap_b):
    """Confere posse de recursos e limite de instancias em uso."""
    cap = {"A": cap_a, "B": cap_b}
    em_uso = defaultdict(int)          # classe -> instancias fora do pool
    posse = defaultdict(set)           # thread -> classes que segura
    pendente = defaultdict(set)        # thread -> classes com SOLICITA aberto
    pico = defaultdict(int)
    classes_vistas = set()

    for _, tid, e, obj, _, n in ev:
        if e not in ("SOLICITA", "OBTEM", "LIBERA"):
            if e == "CONSUMIU" and len(posse[tid]) < 2:
                falha(f"linha {n}: {tid} registrou CONSUMIU segurando "
                      f"{sorted(posse[tid]) or 'nenhum recurso'}; o requisito "
                      f"exige as duas classes ao mesmo tempo")
            continue

        if not obj.strip():
            falha(f"linha {n}: {e} sem classe de recurso no campo objeto")
            continue
        if obj not in ("A", "B"):
            falha(f"linha {n}: classe de recurso {obj!r} desconhecida; "
                  f"as classes se chamam A e B")
            continue
        classes_vistas.add(obj)

        if e == "SOLICITA":
            pendente[tid].add(obj)
        elif e == "OBTEM":
            if obj not in pendente[tid]:
                falha(f"linha {n}: {tid} obteve {obj} sem SOLICITA antes")
            pendente[tid].discard(obj)
            if obj in posse[tid]:
                falha(f"linha {n}: {tid} obteve {obj} sem ter liberado o anterior")
            posse[tid].add(obj)
            em_uso[obj] += 1
            pico[obj] = max(pico[obj], em_uso[obj])
            limite = cap.get(obj)
            if limite is not None and em_uso[obj] > limite:
                falha(f"linha {n}: {em_uso[obj]} instancias de {obj} em uso, "
                      f"limite era {limite}")
        elif e == "LIBERA":
            if obj not in posse[tid]:
                falha(f"linha {n}: {tid} liberou {obj} sem ter obtido")
            else:
                posse[tid].discard(obj)
                em_uso[obj] -= 1

    if len(classes_vistas) < 2:
        falha(f"o log usa {len(classes_vistas)} classe(s) de recurso "
              f"({sorted(classes_vistas)}); o requisito exige duas")

    presos = {t: sorted(c) for t, c in posse.items() if c}
    if presos:
        falha(f"recursos nunca liberados ao fim da execucao: {presos}")

    for c in sorted(pico):
        if cap.get(c) is not None and pico[c] < cap[c]:
            aviso(f"pico de uso da classe {c} foi {pico[c]} de {cap[c]}: "
                  f"o recurso nunca chegou a ser disputado")


def deadlock(ev):
    """Confere que o log termina em espera circular."""
    posse = defaultdict(set)
    pendente = {}
    for _, tid, e, obj, _, _ in ev:
        if e == "OBTEM":
            posse[tid].add(obj)
            pendente.pop(tid, None)
        elif e == "LIBERA":
            posse[tid].discard(obj)
        elif e == "SOLICITA":
            pendente[tid] = obj

    bloqueadas = {t: (sorted(posse[t]), pendente[t])
                  for t in pendente if posse[t] and pendente[t] not in posse[t]}

    if len(bloqueadas) < 2:
        falha("nao ha espera circular no fim do log: menos de duas threads "
              "segurando um recurso e aguardando outro")
        return

    classes_seguras = {c for seg, _ in bloqueadas.values() for c in seg}
    classes_pedidas = {ped for _, ped in bloqueadas.values()}
    if not (classes_seguras & classes_pedidas) or len(classes_pedidas) < 2:
        falha(f"as threads bloqueadas nao formam ciclo: seguram "
              f"{sorted(classes_seguras)}, pedem {sorted(classes_pedidas)}")
        return

    print("espera circular encontrada:")
    for t, (seg, ped) in sorted(bloqueadas.items()):
        print(f"  {t} segura {seg} e aguarda {ped}")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("log")
    p.add_argument("--itens", type=int)
    p.add_argument("--recursos-a", type=int, dest="ra")
    p.add_argument("--recursos-b", type=int, dest="rb")
    p.add_argument("--deadlock", action="store_true",
                   help="espera um log travado, com espera circular no fim")
    a = p.parse_args()

    ev = ler(a.log)
    if not ev and not falhas:
        falha("log vazio")

    if a.deadlock:
        deadlock(ev)
    else:
        ciclo_de_vida(ev)
        itens(ev, a.itens)
        recursos(ev, a.ra, a.rb)

    for m in avisos:
        print(f"aviso: {m}")
    if falhas:
        for m in falhas:
            print(f"FALHA: {m}")
        print(f"\n{len(falhas)} falha(s).")
        return 1
    print(f"\nOK. {len(ev)} eventos verificados.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
