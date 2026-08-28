#pragma once
#include "Benchmark.hpp"
#include "Timer.hpp"
#include <vector>
#include <algorithm>
#include <random>

// Mede o desempenho de std::sort sobre um vetor grande de inteiros aleatórios.
class SortBenchmark : public Benchmark {
public:
    explicit SortBenchmark(size_t numElements = 20'000'000)
        : Benchmark("Ordenacao - std::sort (M elementos/s)", 1), numElements_(numElements) {}

    BenchResult runOnce() override {
        std::vector<int> data(numElements_);
        std::mt19937 rng(1234);
        std::uniform_int_distribution<int> dist(0, 1'000'000'000);
        for (auto& v : data) v = dist(rng);

        Timer t;
        std::sort(data.begin(), data.end());
        double ms = t.elapsedMs();

        BenchResult r;
        r.name = name_;
        r.unit = "M elem/s";
        r.timeMs = ms;
        r.value = (numElements_ / 1e6) / (ms / 1000.0);

        sorted_ = std::is_sorted(data.begin(), data.end());
        return r;
    }

    bool wasSorted() const { return sorted_; }

private:
    size_t numElements_;
    bool sorted_ = false;
};
