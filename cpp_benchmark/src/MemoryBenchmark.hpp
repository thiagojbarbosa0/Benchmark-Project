#pragma once
#include "Benchmark.hpp"
#include "Timer.hpp"
#include <vector>
#include <cstring>
#include <random>
#include <cstdint>

// Mede a largura de banda de memória copiando blocos grandes (memcpy).
class MemoryBandwidthBenchmark : public Benchmark {
public:
    explicit MemoryBandwidthBenchmark(size_t blockSizeMB = 256)
        : Benchmark("Memoria - Largura de Banda (MB/s)", 1),
          blockSizeBytes_(blockSizeMB * 1024ULL * 1024ULL) {}

    BenchResult runOnce() override {
        std::vector<char> src(blockSizeBytes_, 0x5A);
        std::vector<char> dst(blockSizeBytes_, 0x00);

        Timer t;
        std::memcpy(dst.data(), src.data(), blockSizeBytes_);
        double ms = t.elapsedMs();

        // impede otimização agressiva
        volatile char check = dst[blockSizeBytes_ / 2];
        (void)check;

        BenchResult r;
        r.name = name_;
        r.unit = "MB/s";
        r.timeMs = ms;
        double mb = blockSizeBytes_ / (1024.0 * 1024.0);
        r.value = mb / (ms / 1000.0);
        return r;
    }

private:
    size_t blockSizeBytes_;
};

// Mede a latência de acesso a memória via caminhada em ponteiros
// (padrão de acesso aleatório que evita prefetch eficiente do hardware).
class MemoryLatencyBenchmark : public Benchmark {
public:
    explicit MemoryLatencyBenchmark(size_t numElements = 8'000'000, int hops = 50'000'000)
        : Benchmark("Memoria - Latencia de Acesso Aleatorio (M acessos/s)", 1),
          numElements_(numElements), hops_(hops) {}

    BenchResult runOnce() override {
        std::vector<uint32_t> indices(numElements_);
        for (size_t i = 0; i < numElements_; ++i) indices[i] = static_cast<uint32_t>(i);

        std::mt19937 rng(42);
        std::shuffle(indices.begin(), indices.end(), rng);

        // constrói uma lista circular de "próximo índice" para gerar
        // uma cadeia de dependências verdadeiras entre acessos
        std::vector<uint32_t> next(numElements_);
        for (size_t i = 0; i < numElements_; ++i) {
            next[indices[i]] = indices[(i + 1) % numElements_];
        }

        Timer t;
        volatile uint32_t pos = 0;
        for (int i = 0; i < hops_; ++i) {
            pos = next[pos];
        }
        double ms = t.elapsedMs();
        sink_ = pos;

        BenchResult r;
        r.name = name_;
        r.unit = "M acessos/s";
        r.timeMs = ms;
        r.value = (hops_ / 1e6) / (ms / 1000.0);
        return r;
    }

    uint32_t sink() const { return sink_; }

private:
    size_t numElements_;
    int hops_;
    uint32_t sink_ = 0;
};
