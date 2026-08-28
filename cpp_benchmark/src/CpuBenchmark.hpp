#pragma once
#include "Benchmark.hpp"
#include "Timer.hpp"
#include <cstdint>
#include <vector>
#include <cmath>

// Testa desempenho de operações inteiras (soma, multiplicação, XOR, deslocamento)
// em um laço fortemente dependente para evitar otimizações triviais do compilador.
class CpuIntegerBenchmark : public Benchmark {
public:
    explicit CpuIntegerBenchmark(long long workload = 300'000'000LL)
        : Benchmark("CPU - Inteiros (ops/s)", 1), workload_(workload) {}

    BenchResult runOnce() override {
        Timer t;
        volatile uint64_t acc = 0xABCDEF1234567890ULL;
        for (long long i = 0; i < workload_; ++i) {
            acc = (acc * 6364136223846793005ULL + 1442695040888963407ULL);
            acc ^= (acc >> 17);
            acc += static_cast<uint64_t>(i);
        }
        double ms = t.elapsedMs();

        BenchResult r;
        r.name = name_;
        r.unit = "M ops/s";
        r.timeMs = ms;
        r.value = (workload_ / 1e6) / (ms / 1000.0);
        sink_ = acc; // evita que o compilador elimine o laço
        return r;
    }

    uint64_t sink() const { return sink_; }

private:
    long long workload_;
    uint64_t sink_ = 0;
};

// Testa desempenho de ponto flutuante (FLOPS) com operações de multiplicação-soma.
class CpuFloatBenchmark : public Benchmark {
public:
    explicit CpuFloatBenchmark(long long workload = 200'000'000LL)
        : Benchmark("CPU - Ponto Flutuante (MFLOPS)", 1), workload_(workload) {}

    BenchResult runOnce() override {
        Timer t;
        volatile double a = 1.0000001;
        volatile double b = 0.9999999;
        double x = 1.5, y = 2.5;
        for (long long i = 0; i < workload_; ++i) {
            x = x * a + b;
            y = y * b + a;
            x = std::fma(x, 1.0000003, -y * 0.0000001);
        }
        double ms = t.elapsedMs();

        BenchResult r;
        r.name = name_;
        r.unit = "M FLOPS";
        r.timeMs = ms;
        // 3 operações de ponto flutuante por iteração (aprox.)
        r.value = (workload_ * 3.0 / 1e6) / (ms / 1000.0);
        sink_ = x + y;
        return r;
    }

    double sink() const { return sink_; }

private:
    long long workload_;
    double sink_ = 0;
};

// Testa desempenho em uma carga "real" de CPU: crivo de Eratóstenes.
class CpuPrimeSieveBenchmark : public Benchmark {
public:
    explicit CpuPrimeSieveBenchmark(long long limit = 20'000'000LL)
        : Benchmark("CPU - Crivo de Primos (M elementos/s)", 1), limit_(limit) {}

    BenchResult runOnce() override {
        Timer t;
        std::vector<bool> isComposite(static_cast<size_t>(limit_) + 1, false);
        long long count = 0;
        for (long long i = 2; i <= limit_; ++i) {
            if (!isComposite[static_cast<size_t>(i)]) {
                ++count;
                for (long long j = i * 2; j <= limit_; j += i) {
                    isComposite[static_cast<size_t>(j)] = true;
                }
            }
        }
        double ms = t.elapsedMs();

        BenchResult r;
        r.name = name_;
        r.unit = "M elem/s";
        r.timeMs = ms;
        r.value = (limit_ / 1e6) / (ms / 1000.0);
        primeCount_ = count;
        return r;
    }

    long long primeCount() const { return primeCount_; }

private:
    long long limit_;
    long long primeCount_ = 0;
};
