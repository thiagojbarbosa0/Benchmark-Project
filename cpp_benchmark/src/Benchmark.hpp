#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

// Resultado de uma única execução (uma "amostra") de um benchmark.
struct BenchResult {
    std::string name;          // nome do benchmark/subteste
    std::string unit;          // unidade da métrica principal (ex: "MB/s", "ops/s", "ms")
    double value = 0.0;        // valor da métrica principal (quanto MAIOR, melhor, salvo indicação em 'lowerIsBetter')
    double timeMs = 0.0;       // tempo bruto gasto na amostra
    bool lowerIsBetter = false;
};

// Estatísticas agregadas de várias amostras do mesmo benchmark.
struct BenchStats {
    std::string name;
    std::string unit;
    double mean = 0.0;
    double stddev = 0.0;
    double min = 0.0;
    double max = 0.0;
    double median = 0.0;
    bool lowerIsBetter = false;

    static BenchStats fromResults(const std::vector<BenchResult>& results) {
        BenchStats s;
        if (results.empty()) return s;
        s.name = results.front().name;
        s.unit = results.front().unit;
        s.lowerIsBetter = results.front().lowerIsBetter;

        std::vector<double> values;
        values.reserve(results.size());
        for (const auto& r : results) values.push_back(r.value);

        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        s.mean = sum / values.size();

        double sqSum = 0.0;
        for (double v : values) sqSum += (v - s.mean) * (v - s.mean);
        s.stddev = values.size() > 1 ? std::sqrt(sqSum / (values.size() - 1)) : 0.0;

        s.min = *std::min_element(values.begin(), values.end());
        s.max = *std::max_element(values.begin(), values.end());

        std::vector<double> sorted = values;
        std::sort(sorted.begin(), sorted.end());
        size_t mid = sorted.size() / 2;
        s.median = (sorted.size() % 2 == 0)
                       ? (sorted[mid - 1] + sorted[mid]) / 2.0
                       : sorted[mid];
        return s;
    }
};

// Interface base que todo módulo de benchmark deve implementar.
class Benchmark {
public:
    Benchmark(std::string name, int defaultIterations)
        : name_(std::move(name)), iterations_(defaultIterations) {}

    virtual ~Benchmark() = default;

    // Executa uma única amostra e retorna o resultado.
    virtual BenchResult runOnce() = 0;

    const std::string& name() const { return name_; }
    int iterations() const { return iterations_; }
    void setIterations(int it) { iterations_ = it; }

protected:
    std::string name_;
    int iterations_;
};
