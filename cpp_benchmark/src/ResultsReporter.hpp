#pragma once
#include "Benchmark.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

class ResultsReporter {
public:
    void add(const BenchStats& s) { stats_.push_back(s); }

    void printTable(std::ostream& os = std::cout) const {
        // calcula largura da coluna de nome dinamicamente
        size_t nameWidth = 20;
        for (auto& s : stats_) nameWidth = std::max(nameWidth, s.name.size());
        nameWidth += 2;

        os << "\n";
        printSeparator(os, nameWidth);
        os << "| " << std::left << std::setw(static_cast<int>(nameWidth)) << "Benchmark"
           << "| " << std::right << std::setw(12) << "Media"
           << " | " << std::setw(12) << "Mediana"
           << " | " << std::setw(10) << "Desv.Pad"
           << " | " << std::setw(12) << "Min"
           << " | " << std::setw(12) << "Max"
           << " | " << std::setw(10) << "Unidade" << " |\n";
        printSeparator(os, nameWidth);

        for (auto& s : stats_) {
            os << "| " << std::left << std::setw(static_cast<int>(nameWidth)) << s.name
               << "| " << std::right << std::fixed << std::setprecision(2) << std::setw(12) << s.mean
               << " | " << std::setw(12) << s.median
               << " | " << std::setw(10) << s.stddev
               << " | " << std::setw(12) << s.min
               << " | " << std::setw(12) << s.max
               << " | " << std::setw(10) << s.unit << " |\n";
        }
        printSeparator(os, nameWidth);
        os << "\n";
    }

    void exportCsv(const std::string& path) const {
        std::ofstream f(path);
        if (!f) {
            std::cerr << "Aviso: nao foi possivel abrir '" << path << "' para escrita.\n";
            return;
        }
        f << "benchmark,unidade,media,mediana,desvio_padrao,min,max,lower_is_better\n";
        for (auto& s : stats_) {
            f << csvEscape(s.name) << "," << csvEscape(s.unit) << ","
              << s.mean << "," << s.median << "," << s.stddev << ","
              << s.min << "," << s.max << "," << (s.lowerIsBetter ? "true" : "false") << "\n";
        }
        std::cout << "Resultados exportados para: " << path << "\n";
    }

    const std::vector<BenchStats>& stats() const { return stats_; }

private:
    static void printSeparator(std::ostream& os, size_t nameWidth) {
        os << "+" << std::string(nameWidth + 1, '-')
           << "+" << std::string(14, '-')
           << "+" << std::string(14, '-')
           << "+" << std::string(12, '-')
           << "+" << std::string(14, '-')
           << "+" << std::string(14, '-')
           << "+" << std::string(12, '-') << "+\n";
    }

    static std::string csvEscape(const std::string& s) {
        if (s.find(',') != std::string::npos) {
            std::ostringstream oss;
            oss << '"' << s << '"';
            return oss.str();
        }
        return s;
    }

    std::vector<BenchStats> stats_;
};
