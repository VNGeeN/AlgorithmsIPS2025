#include "GraphAnalyzer.h"
#include <iostream>
#include <functional>
#include <queue>
#include <fstream>
#include <sstream>

Edge::Edge(const std::string& f, const std::string& t, double r, bool src)
    : from(f), to(t), resistance(r), isSource(src) {}

GraphAnalyzer::GraphAnalyzer() : sourceEdge(nullptr), edgeCount(0) {}

GraphAnalyzer::~GraphAnalyzer() {
    clear();
}

void GraphAnalyzer::clear() {
    graph.clear();
    vertices.clear();
    edgeCount = 0;
    if (sourceEdge) {
        delete sourceEdge;
        sourceEdge = nullptr;
    }
}

void GraphAnalyzer::addEdge(const std::string& from, const std::string& to, double resistance) {
    graph[from].push_back(std::make_pair(to, resistance));
    graph[to].push_back(std::make_pair(from, resistance));
    vertices.insert(from);
    vertices.insert(to);
    edgeCount++;
}

bool GraphAnalyzer::loadGraph(const std::string& filename) {
    clear();
    
    // Используем FileHandler для чтения файла
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Ошибка: не удалось открыть файл '" << filename << "'" << std::endl;
        return false;
    }
    
    std::string line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        // Пропускаем пустые строки и комментарии
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Удаляем комментарии в конце строки
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        
        // Разбиваем строку на токены
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        // Проверяем корректность формата
        if (tokens.size() != 3 && tokens.size() != 4) {
            std::cout << "Ошибка в строке " << lineNum << ": неверный формат" << std::endl;
            continue;
        }
        
        std::string vertex1 = tokens[0];
        std::string vertex2 = tokens[1];
        double resistance;
        
        // Парсим сопротивление
        try {
            resistance = std::stod(tokens[2]);
            if (resistance <= 0) {
                std::cout << "Ошибка в строке " << lineNum 
                          << ": сопротивление должно быть положительным" << std::endl;
                continue;
            }
        } catch (...) {
            std::cout << "Ошибка в строке " << lineNum 
                      << ": некорректное значение сопротивления" << std::endl;
            continue;
        }
        
        // Проверяем, есть ли источник питания
        bool isSource = (tokens.size() == 4 && tokens[3] == "source");
        
        if (isSource) {
            if (sourceEdge != nullptr) {
                std::cout << "Ошибка в строке " << lineNum 
                          << ": источник питания может быть только на одном ребре" << std::endl;
                continue;
            }
            sourceEdge = new Edge(vertex1, vertex2, resistance, true);
        }
        
        // Добавляем ребро в граф
        addEdge(vertex1, vertex2, resistance);
    }
    
    file.close();
    
    if (sourceEdge == nullptr) {
        std::cout << "Ошибка: в графе отсутствует источник питания" << std::endl;
        return false;
    }
    
    std::cout << "Граф успешно загружен из файла '" << filename << "'" << std::endl;
    std::cout << "Вершин: " << vertices.size() << ", ребер: " << edgeCount << std::endl;
    std::cout << "Источник питания на ребре: " << sourceEdge->from 
              << " - " << sourceEdge->to 
              << " (сопротивление: " << sourceEdge->resistance << ")" << std::endl;
    
    return true;
}

std::vector<ShortCircuit> GraphAnalyzer::findShortCircuitsDFS(double threshold) {
    std::vector<ShortCircuit> result;
    
    if (sourceEdge == nullptr) {
        std::cout << "Ошибка: граф не загружен или отсутствует источник питания" << std::endl;
        return result;
    }
    
    std::string sourceV1 = sourceEdge->from;
    std::string sourceV2 = sourceEdge->to;
    double sourceRes = sourceEdge->resistance;
    
    std::set<std::string> visited;
    std::vector<std::string> path;
    double currentResistance = 0;
    
    path.push_back(sourceV1);
    dfsShortCircuits(sourceV1, sourceV2, visited, path, currentResistance, threshold, result);
    
    return result;
}

void GraphAnalyzer::dfsShortCircuits(const std::string& current, const std::string& target,
                                    std::set<std::string>& visited, std::vector<std::string>& path,
                                    double currentRes, double threshold, std::vector<ShortCircuit>& result) {
    if (current == target) {
        double sourceRes = sourceEdge->resistance;
        if (currentRes + sourceRes < threshold) {
            std::vector<std::string> fullPath = path;
            fullPath.push_back(target);
            result.push_back(ShortCircuit(fullPath, currentRes, currentRes + sourceRes));
        }
        return;
    }
    
    visited.insert(current);
    
    if (graph.find(current) != graph.end()) {
        for (const auto& neighbor : graph.at(current)) {
            std::string nextVertex = neighbor.first;
            double edgeResistance = neighbor.second;
            
            // Пропускаем прямое ребро с источником
            if ((current == sourceEdge->from && nextVertex == sourceEdge->to) ||
                (current == sourceEdge->to && nextVertex == sourceEdge->from)) {
                continue;
            }
            
            if (visited.find(nextVertex) == visited.end()) {
                double newResistance = currentRes + edgeResistance;
                double sourceRes = sourceEdge->resistance;
                
                // Отсечение: если уже превысили порог, не продолжаем
                if (newResistance + sourceRes >= threshold) {
                    continue;
                }
                
                path.push_back(current);
                dfsShortCircuits(nextVertex, target, visited, path, newResistance, threshold, result);
                path.pop_back();
            }
        }
    }
    
    visited.erase(current);
}

std::vector<ShortCircuit> GraphAnalyzer::findShortCircuitsBFS(double threshold) {
    std::vector<ShortCircuit> result;
    
    if (sourceEdge == nullptr) {
        return result;
    }
    
    std::string sourceV1 = sourceEdge->from;
    std::string sourceV2 = sourceEdge->to;
    double sourceRes = sourceEdge->resistance;
    
    // Максимальное сопротивление для пути без учета источника
    double maxPathResistance = threshold - sourceRes;
    
    if (maxPathResistance <= 0) {
        return result;
    }
    
    // Структура для хранения состояния в BFS
    struct BFSState {
        std::string current;
        std::vector<std::string> path;
        double resistance;
    };
    
    std::queue<BFSState> q;
    q.push({sourceV1, {sourceV1}, 0});
    
    while (!q.empty()) {
        BFSState state = q.front();
        q.pop();
        
        if (state.current == sourceV2) {
            if (state.resistance + sourceRes < threshold) {
                result.push_back(ShortCircuit(state.path, state.resistance, state.resistance + sourceRes));
            }
            continue;
        }
        
        if (graph.find(state.current) != graph.end()) {
            for (const auto& neighbor : graph.at(state.current)) {
                std::string nextVertex = neighbor.first;
                double edgeResistance = neighbor.second;
                
                // Пропускаем прямое ребро с источником
                if ((state.current == sourceV1 && nextVertex == sourceV2) ||
                    (state.current == sourceV2 && nextVertex == sourceV1)) {
                    continue;
                }
                
                // Проверяем, не посещали ли уже эту вершину на текущем пути
                bool visited = false;
                for (const auto& v : state.path) {
                    if (v == nextVertex) {
                        visited = true;
                        break;
                    }
                }
                
                if (!visited) {
                    double newResistance = state.resistance + edgeResistance;
                    
                    if (newResistance > maxPathResistance) {
                        continue;
                    }
                    
                    std::vector<std::string> newPath = state.path;
                    newPath.push_back(nextVertex);
                    q.push({nextVertex, newPath, newResistance});
                }
            }
        }
    }
    
    return result;
}

void GraphAnalyzer::printGraphInfo() const {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "ИНФОРМАЦИЯ О ГРАФЕ" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    if (graph.empty()) {
        std::cout << "Граф не загружен" << std::endl;
        return;
    }
    
    std::cout << "Всего вершин: " << vertices.size() << std::endl;
    std::cout << "Всего ребер: " << edgeCount << std::endl;
    
    if (sourceEdge) {
        std::cout << "Источник питания: " << sourceEdge->from 
                  << " - " << sourceEdge->to << std::endl;
        std::cout << "Сопротивление источника: " << sourceEdge->resistance << std::endl;
    }
    
    std::cout << "\nСтруктура графа:" << std::endl;
    for (const auto& vertex : vertices) {
        if (graph.find(vertex) != graph.end() && !graph.at(vertex).empty()) {
            std::cout << "  " << vertex << ": ";
            const auto& neighbors = graph.at(vertex);
            for (size_t i = 0; i < neighbors.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << neighbors[i].first << "(" << neighbors[i].second << ")";
            }
            std::cout << std::endl;
        }
    }
}

bool GraphAnalyzer::isGraphLoaded() const {
    return !graph.empty() && sourceEdge != nullptr;
}