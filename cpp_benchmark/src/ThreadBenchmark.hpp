#pragma once
#include "Benchmark.hpp"
#include "Timer.hpp"
#include <thread>
#include <vector>
#include <cstdint>
#include <cstdio>

// Mede o ganho de desempenho ao distribuir uma carga de CPU entre N threads.
// Cada chamada de runOnce() usa o número de threads configurado via setThreadCount().
class ThreadScalingBenchmark : public Benchmark {
public:
    explicit ThreadScalingBenchmark(long long totalWork = 800'000'000LL)
        : Benchmark("Multithread - Escalabilidade", 1), totalWork_(totalWork) {}

    void setThreadCount(unsigned n) { threadCount_ = n; }
    unsigned threadCount() const { return threadCount_; }

    BenchResult runOnce() override {
        unsigned n = threadCount_ == 0 ? 1 : threadCount_;
        std::vector<std::thread> workers;
        std::vector<uint64_t> partial(n, 0);
        long long chunk = totalWork_ / n;

        Timer t;
        for (unsigned i = 0; i < n; ++i) {
            workers.emplace_back([&partial, i, chunk]() {
                uint64_t acc = 0xFEED0000ULL + i;
                for (long long k = 0; k < chunk; ++k) {
                    acc = acc * 6364136223846793005ULL + 1442695040888963407ULL;
                    acc ^= (acc >> 21);
                }
                partial[i] = acc;
            });
        }
        for (auto& w : workers) w.join();
        double ms = t.elapsedMs();

        uint64_t sink = 0;
        for (auto v : partial) sink ^= v;
        sink_ = sink;

        BenchResult r;
        r.name = name_ + " [" + std::to_string(n) + " thread(s)]";
        r.unit = "M ops/s";
        r.timeMs = ms;
        r.value = (totalWork_ / 1e6) / (ms / 1000.0);
        return r;
    }

    uint64_t sink() const { return sink_; }

private:
    long long totalWork_;
    unsigned threadCount_ = 1;
    uint64_t sink_ = 0;
};
