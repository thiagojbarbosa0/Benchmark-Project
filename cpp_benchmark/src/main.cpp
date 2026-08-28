// ============================================================================
// CppBench - Suite de benchmarking de sistema em C++
//
// Mede desempenho de CPU (inteiros, ponto flutuante, crivo de primos),
// memória (largura de banda e latência), ordenação e escalabilidade
// multithread. Gera tabela no console e, opcionalmente, um CSV.
// ============================================================================

#include "Benchmark.hpp"
#include "CpuBenchmark.hpp"
#include "MemoryBenchmark.hpp"
#include "SortBenchmark.hpp"
#include "ThreadBenchmark.hpp"
#include "ResultsReporter.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <memory>
#include <cstdlib>

struct Options {
    int repeats = 3;                 // quantas vezes cada benchmark roda p/ estatística
    bool runCpu = true;
    bool runMemory = true;
    bool runSort = true;
    bool runThreads = true;
    bool quickMode = false;          // reduz o tamanho das cargas p/ testes rápidos
    std::string csvPath;             // vazio = não exporta
    unsigned maxThreads = std::thread::hardware_concurrency() == 0
                               ? 4
                               : std::thread::hardware_concurrency();
};

static void printHelp(const char* prog) {
    std::cout <<
        "CppBench - Suite de benchmarking de sistema\n\n"
        "Uso: " << prog << " [opcoes]\n\n"
        "Opcoes:\n"
        "  --only <lista>       Executa apenas os grupos indicados (separados por virgula).\n"
        "                       Grupos disponiveis: cpu, memoria, ordenacao, threads\n"
        "  --repeats <N>        Numero de repeticoes por benchmark para estatistica (padrao: 3)\n"
        "  --threads <N>        Numero maximo de threads no teste de escalabilidade (padrao: nucleos da maquina)\n"
        "  --quick              Modo rapido: reduz o tamanho das cargas de trabalho\n"
        "  --csv <arquivo>      Exporta os resultados agregados para um arquivo CSV\n"
        "  -h, --help           Mostra esta mensagem de ajuda\n\n"
        "Exemplos:\n"
        "  " << prog << "                       Executa a suite completa\n"
        "  " << prog << " --quick               Executa uma versao rapida de todos os testes\n"
        "  " << prog << " --only cpu,memoria    Executa somente CPU e memoria\n"
        "  " << prog << " --csv resultados.csv  Salva os resultados em CSV\n";
}

static Options parseArgs(int argc, char** argv) {
    Options opt;
    std::set<std::string> only;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            std::exit(0);
        } else if (arg == "--only" && i + 1 < argc) {
            std::string list = argv[++i];
            size_t pos = 0;
            while (pos < list.size()) {
                size_t comma = list.find(',', pos);
                std::string tok = list.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
                only.insert(tok);
                if (comma == std::string::npos) break;
                pos = comma + 1;
            }
        } else if (arg == "--repeats" && i + 1 < argc) {
            opt.repeats = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--threads" && i + 1 < argc) {
            opt.maxThreads = static_cast<unsigned>(std::max(1, std::atoi(argv[++i])));
        } else if (arg == "--quick") {
            opt.quickMode = true;
        } else if (arg == "--csv" && i + 1 < argc) {
            opt.csvPath = argv[++i];
        } else {
            std::cerr << "Argumento desconhecido: " << arg << "\n\n";
            printHelp(argv[0]);
            std::exit(1);
        }
    }

    if (!only.empty()) {
        opt.runCpu = only.count("cpu") > 0;
        opt.runMemory = only.count("memoria") > 0;
        opt.runSort = only.count("ordenacao") > 0;
        opt.runThreads = only.count("threads") > 0;
    }
    return opt;
}

// Executa um benchmark 'opt.repeats' vezes e agrega em BenchStats,
// imprimindo progresso incremental no console.
template <typename BenchT>
static BenchStats runWithStats(BenchT& bench, int repeats) {
    std::vector<BenchResult> results;
    results.reserve(repeats);
    std::cout << "  -> " << bench.name() << " ";
    std::cout.flush();
    for (int i = 0; i < repeats; ++i) {
        results.push_back(bench.runOnce());
        std::cout << "." << std::flush;
    }
    std::cout << " ok\n";
    return BenchStats::fromResults(results);
}

int main(int argc, char** argv) {
    Options opt = parseArgs(argc, argv);

    std::cout << "==============================================\n";
    std::cout << " CppBench - Suite de Benchmark de Sistema\n";
    std::cout << "==============================================\n";
    std::cout << "Nucleos de hardware detectados: " << std::thread::hardware_concurrency() << "\n";
    std::cout << "Repeticoes por teste: " << opt.repeats << "\n";
    std::cout << "Modo rapido: " << (opt.quickMode ? "sim" : "nao") << "\n\n";

    ResultsReporter reporter;

    // ---------------- CPU ----------------
    if (opt.runCpu) {
        std::cout << "[CPU]\n";
        long long intWork = opt.quickMode ? 40'000'000LL : 300'000'000LL;
        long long floatWork = opt.quickMode ? 25'000'000LL : 200'000'000LL;
        long long sieveLimit = opt.quickMode ? 2'000'000LL : 20'000'000LL;

        CpuIntegerBenchmark cpuInt(intWork);
        reporter.add(runWithStats(cpuInt, opt.repeats));

        CpuFloatBenchmark cpuFloat(floatWork);
        reporter.add(runWithStats(cpuFloat, opt.repeats));

        CpuPrimeSieveBenchmark cpuSieve(sieveLimit);
        reporter.add(runWithStats(cpuSieve, opt.repeats));
        std::cout << "\n";
    }

    // ---------------- Memória ----------------
    if (opt.runMemory) {
        std::cout << "[Memoria]\n";
        size_t blockMB = opt.quickMode ? 64 : 256;
        size_t latElements = opt.quickMode ? 1'000'000 : 8'000'000;
        int latHops = opt.quickMode ? 5'000'000 : 50'000'000;

        MemoryBandwidthBenchmark memBw(blockMB);
        reporter.add(runWithStats(memBw, opt.repeats));

        MemoryLatencyBenchmark memLat(latElements, latHops);
        reporter.add(runWithStats(memLat, opt.repeats));
        std::cout << "\n";
    }

    // ---------------- Ordenação ----------------
    if (opt.runSort) {
        std::cout << "[Ordenacao]\n";
        size_t n = opt.quickMode ? 3'000'000 : 20'000'000;
        SortBenchmark sortBench(n);
        reporter.add(runWithStats(sortBench, opt.repeats));
        std::cout << "\n";
    }

    // ---------------- Threads ----------------
    if (opt.runThreads) {
        std::cout << "[Multithread - Escalabilidade]\n";
        long long totalWork = opt.quickMode ? 150'000'000LL : 800'000'000LL;

        std::vector<unsigned> threadCounts;
        for (unsigned t = 1; t <= opt.maxThreads; t *= 2) threadCounts.push_back(t);
        if (threadCounts.back() != opt.maxThreads) threadCounts.push_back(opt.maxThreads);

        for (unsigned t : threadCounts) {
            ThreadScalingBenchmark tb(totalWork);
            tb.setThreadCount(t);
            reporter.add(runWithStats(tb, opt.repeats));
        }
        std::cout << "\n";
    }

    reporter.printTable();

    if (!opt.csvPath.empty()) {
        reporter.exportCsv(opt.csvPath);
    }

    std::cout << "Benchmark concluido.\n";
    return 0;
}
