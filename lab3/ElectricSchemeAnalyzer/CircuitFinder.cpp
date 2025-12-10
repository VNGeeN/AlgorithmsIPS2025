#include "CircuitFinder.h"
#include <iostream>

std::vector<ShortCircuit> CircuitFinder::findShortCircuitsDFS(GraphAnalyzer& analyzer, double threshold) {
    return analyzer.findShortCircuitsDFS(threshold);
}

std::vector<ShortCircuit> CircuitFinder::findShortCircuitsBFS(GraphAnalyzer& analyzer, double threshold) {
    return analyzer.findShortCircuitsBFS(threshold);
}

void CircuitFinder::displayCircuit(const ShortCircuit& circuit, int index) {
    std::vector<std::string> closedPath = circuit.path;
    if (!closedPath.empty()) {
        closedPath.push_back(closedPath[0]); // Замыкаем цепь
    }
    
    std::cout << "Замыкание #" << index << ":" << std::endl;
    std::cout << "  Цепь: ";
    for (size_t j = 0; j < closedPath.size(); ++j) {
        if (j > 0) std::cout << " -> ";
        std::cout << closedPath[j];
    }
    std::cout << std::endl;
    std::cout << "  Сопротивление пути: " << circuit.pathResistance << std::endl;
    std::cout << "  Сопротивление с источником: " << circuit.totalResistance << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}

void CircuitFinder::displayResults(const std::vector<ShortCircuit>& circuits) {
    if (circuits.empty()) {
        std::cout << "Коротких замыканий не найдено" << std::endl;
        return;
    }
    
    std::cout << "\nНайдено коротких замыканий: " << circuits.size() << std::endl;
    std::cout << "==================================================" << std::endl;
    
    for (size_t i = 0; i < circuits.size(); ++i) {
        displayCircuit(circuits[i], i + 1);
    }
}