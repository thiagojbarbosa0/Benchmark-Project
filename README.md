# CppBench — Suite de Benchmarking de Sistema em C++

Um programa de benchmarking em C++17, modular e sem dependências externas, que mede o desempenho de **CPU**, **memória**, **ordenação** e **escalabilidade multithread** da máquina onde é executado. Gera uma tabela estatística no console (média, mediana, desvio padrão, mínimo e máximo) e pode exportar os resultados para **CSV**.

## Sumário

- [Recursos](#recursos)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Requisitos](#requisitos)
- [Compilação](#compilação)
  - [Usando Make](#usando-make)
  - [Usando CMake](#usando-cmake)
- [Uso](#uso)
  - [Opções de linha de comando](#opções-de-linha-de-comando)
  - [Exemplos](#exemplos)
- [Benchmarks incluídos](#benchmarks-incluídos)
- [Interpretando os resultados](#interpretando-os-resultados)
- [Exportação em CSV](#exportação-em-csv)
- [Como estender](#como-estender)
- [Notas de precisão e limitações](#notas-de-precisão-e-limitações)
- [Licença](#licença)

## Recursos

- **Sem dependências externas** — usa apenas a biblioteca padrão do C++ (`<chrono>`, `<thread>`, `<algorithm>` etc).
- **Módulos independentes**: CPU (inteiros, ponto flutuante e crivo de primos), memória (largura de banda e latência), ordenação (`std::sort`) e escalabilidade multithread.
- **Estatísticas agregadas**: cada benchmark pode ser repetido N vezes, reportando média, mediana, desvio padrão, mínimo e máximo.
- **Saída em tabela** formatada no console e **exportação para CSV**.
- **Modo rápido** (`--quick`) para testes de fumaça em segundos, e modo completo para medições mais confiáveis.
- **Seleção de grupos de testes** via `--only`.
- **Configuração de threads** no teste de escalabilidade.

## Estrutura do projeto

```
cpp_benchmark/
├── CMakeLists.txt          # build alternativo via CMake
├── Makefile                # build via make (não requer CMake)
├── README.md
└── src/
    ├── main.cpp             # ponto de entrada, CLI e orquestração
    ├── Benchmark.hpp         # classe base + estruturas de resultado/estatística
    ├── Timer.hpp             # cronômetro de alta resolução
    ├── CpuBenchmark.hpp      # benchmarks de CPU
    ├── MemoryBenchmark.hpp   # benchmarks de memória
    ├── SortBenchmark.hpp     # benchmark de ordenação
    ├── ThreadBenchmark.hpp   # benchmark de escalabilidade multithread
    └── ResultsReporter.hpp   # tabela no console + exportação CSV
```

## Requisitos

- Compilador com suporte a **C++17** (testado com GCC 13, deve funcionar em Clang ≥ 7 e MSVC ≥ 2019).
- **pthreads** (Linux/macOS) — no Windows, o `std::thread` do MSVC já resolve isso automaticamente.
- Opcional: **CMake ≥ 3.10**, caso prefira esse fluxo de build ao Makefile.

## Compilação

### Usando Make

```bash
make            # compila em build/cppbench com otimização -O3
make run        # compila (se necessário) e executa com opções padrão
make clean      # remove os artefatos de build
```

Variáveis que podem ser sobrescritas:

```bash
make CXX=clang++ CXXFLAGS="-std=c++17 -O2 -pthread"
```

### Usando CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./cppbench
```

## Uso

```bash
./build/cppbench [opções]
```

### Opções de linha de comando

| Opção                | Descrição                                                                                   |
|-----------------------|-----------------------------------------------------------------------------------------------|
| `--only <lista>`      | Executa apenas os grupos indicados, separados por vírgula: `cpu`, `memoria`, `ordenacao`, `threads` |
| `--repeats <N>`       | Número de repetições de cada benchmark para estatística (padrão: `3`)                        |
| `--threads <N>`       | Número máximo de threads no teste de escalabilidade (padrão: núcleos detectados na máquina)  |
| `--quick`             | Modo rápido: reduz o tamanho das cargas de trabalho (útil para testes de fumaça)             |
| `--csv <arquivo>`     | Exporta os resultados agregados para um arquivo CSV                                          |
| `-h`, `--help`        | Mostra a mensagem de ajuda                                                                    |

### Exemplos

Executar a suite completa com as configurações padrão:

```bash
./build/cppbench
```

Rodar uma versão rápida, útil para verificar se está tudo funcionando:

```bash
./build/cppbench --quick
```

Rodar apenas os testes de CPU e memória, com 5 repetições cada:

```bash
./build/cppbench --only cpu,memoria --repeats 5
```

Rodar o teste de escalabilidade limitando a 8 threads e salvar tudo em CSV:

```bash
./build/cppbench --only threads --threads 8 --csv resultados.csv
```

## Benchmarks incluídos

### CPU

| Benchmark                         | O que mede                                                                                   |
|-----------------------------------|-----------------------------------------------------------------------------------------------|
| **Inteiros**                      | Débito de operações inteiras (multiplicação, XOR, deslocamento) em um laço com dependência de dados, reportado em milhões de operações por segundo. |
| **Ponto flutuante**                | Débito de operações de ponto flutuante (incluindo FMA), reportado em milhões de FLOPS.        |
| **Crivo de primos**                | Uma carga de CPU "real" (Crivo de Eratóstenes) sensível a padrões de acesso à cache e branch prediction. |

### Memória

| Benchmark                          | O que mede                                                                                  |
|-------------------------------------|-----------------------------------------------------------------------------------------------|
| **Largura de banda**                | Velocidade de cópia de um grande bloco contíguo de memória (`memcpy`), em MB/s.               |
| **Latência de acesso aleatório**    | Tempo de acesso à memória seguindo uma cadeia de ponteiros embaralhada, que evita prefetch eficiente e expõe a latência real de cache/RAM. |

### Ordenação

| Benchmark      | O que mede                                                                    |
|-----------------|---------------------------------------------------------------------------------|
| **std::sort**   | Débito de ordenação de um vetor grande de inteiros aleatórios (milhões de elementos por segundo). |

### Multithread

| Benchmark            | O que mede                                                                                          |
|------------------------|--------------------------------------------------------------------------------------------------------|
| **Escalabilidade**     | Distribui uma carga fixa de CPU entre 1, 2, 4, 8... threads (potências de 2 até o limite configurado), permitindo observar o ganho de desempenho conforme mais núcleos são usados. |

## Interpretando os resultados

A tabela final mostra, para cada benchmark:

- **Media / Mediana**: valores centrais da métrica ao longo das repetições — quanto **maior**, melhor (todas as métricas atuais são "quanto mais, melhor": ops/s, MFLOPS, MB/s, elementos/s).
- **Desv.Pad**: o quão consistentes foram as execuções. Valores altos podem indicar ruído do sistema (outros processos competindo por CPU, throttling térmico, etc).
- **Min / Max**: piores e melhores execuções observadas.

Para comparar máquinas diferentes, prefira rodar sem o `--quick` e com `--repeats 5` ou mais, evitando outros processos pesados rodando em paralelo.

## Exportação em CSV

O arquivo gerado por `--csv` tem o seguinte formato:

```csv
benchmark,unidade,media,mediana,desvio_padrao,min,max,lower_is_better
CPU - Inteiros (ops/s),M ops/s,367.075,367.075,23.304,350.597,383.554,false
...
```

Pode ser aberto diretamente em Excel, Google Sheets, ou processado com pandas/Python para gerar gráficos comparativos entre execuções.

## Como estender

Cada benchmark é uma classe que herda de `Benchmark` (em `Benchmark.hpp`) e implementa `runOnce()`, retornando um `BenchResult`. Para adicionar um novo benchmark:

1. Crie um novo header (ex: `MeuBenchmark.hpp`) com uma classe derivada de `Benchmark`.
2. Implemente `runOnce()`, preenchendo `name`, `unit`, `value` e `timeMs` do `BenchResult`.
3. Inclua o header em `main.cpp` e adicione a chamada correspondente usando o helper `runWithStats(...)`.
4. (Opcional) Adicione uma nova flag em `Options`/`parseArgs` se quiser que o novo grupo seja selecionável via `--only`.

Dica: use `volatile` ou variáveis "sink" (como já é feito nos benchmarks existentes) para impedir que o compilador elimine o código por otimização agressiva quando o resultado do cálculo não é usado externamente.

## Notas de precisão e limitações

- Este é um benchmark **sintético**, útil para comparações relativas (mesma máquina em momentos diferentes, ou entre máquinas similares) — não substitui benchmarks especializados (ex: SPEC CPU, STREAM, Geekbench) para análises rigorosas.
- Resultados podem variar conforme: frequência dinâmica da CPU (turbo boost/throttling), outros processos em execução, política de governor de energia, e estado da cache no momento do teste.
- O teste de latência de memória usa uma cadeia de ponteiros embaralhada; em máquinas com caches muito grandes, aumentar `numElements` (em `MemoryBenchmark.hpp`) pode ser necessário para realmente "estourar" a cache L3 e medir a latência da RAM.
- Em ambientes de máquina virtual ou containers com CPU limitada, o teste de escalabilidade multithread pode não refletir o comportamento de hardware dedicado.

## Licença

Este projeto é fornecido como está, livre para uso, modificação e distribuição.
