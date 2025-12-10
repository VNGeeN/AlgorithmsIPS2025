#ifndef GRAPH_ANALYZER_H
#define GRAPH_ANALYZER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include "Edge.h"
#include "ShortCircuit.h"

class GraphAnalyzer {
private:
    std::map<std::string, std::vector<std::pair<std::string, double>>> graph;
    std::set<std::string> vertices;
    Edge* sourceEdge;
    int edgeCount;
    
public:
    GraphAnalyzer();
    ~GraphAnalyzer();
    
    // Основные методы
    bool loadGraph(const std::string& filename);
    std::vector<ShortCircuit> findShortCircuitsDFS(double threshold);
    std::vector<ShortCircuit> findShortCircuitsBFS(double threshold);
    
    // Вспомогательные методы
    void clear();
    void printGraphInfo() const;
    bool isGraphLoaded() const;
    
    // Геттеры
    Edge* getSourceEdge() const { return sourceEdge; }
    const std::set<std::string>& getVertices() const { return vertices; }
    const std::map<std::string, std::vector<std::pair<std::string, double>>>& getGraph() const { return graph; }
    int getEdgeCount() const { return edgeCount; }
    
private:
    // Вспомогательные методы для поиска
    void dfsShortCircuits(const std::string& current, const std::string& target,
                         std::set<std::string>& visited, std::vector<std::string>& path,
                         double currentRes, double threshold, std::vector<ShortCircuit>& result);
    
    // Добавление ребра в граф
    void addEdge(const std::string& from, const std::string& to, double resistance);
};

#endif // GRAPH_ANALYZER_H