#ifndef CIRCUIT_FINDER_H
#define CIRCUIT_FINDER_H

#include "GraphAnalyzer.h"
#include <vector>

class CircuitFinder {
public:
    // Статические методы для поиска замыканий
    static std::vector<ShortCircuit> findShortCircuitsDFS(GraphAnalyzer& analyzer, double threshold);
    static std::vector<ShortCircuit> findShortCircuitsBFS(GraphAnalyzer& analyzer, double threshold);
    
    // Отображение результатов
    static void displayResults(const std::vector<ShortCircuit>& circuits);
    
private:
    // Вспомогательные методы
    static void displayCircuit(const ShortCircuit& circuit, int index);
};

#endif // CIRCUIT_FINDER_H