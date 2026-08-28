#pragma once
#include <chrono>

// Timer de alta resolução usado por todos os benchmarks para medir
// intervalos de tempo com precisão de nanossegundos.
class Timer {
public:
    Timer() { reset(); }

    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }

    // Retorna o tempo decorrido em milissegundos (double, com casas decimais)
    double elapsedMs() const {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> diff = end - start_;
        return diff.count();
    }

    // Retorna o tempo decorrido em segundos
    double elapsedSec() const {
        return elapsedMs() / 1000.0;
    }

private:
    std::chrono::high_resolution_clock::time_point start_;
};
