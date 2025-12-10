#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "GraphAnalyzer.h"
#include "CircuitFinder.h"
#include "FileHandler.h"
#include <string>

class UserInterface {
private:
    GraphAnalyzer analyzer;
    
public:
    // Запуск интерактивного режима
    void run();
    
private:
    // Методы для обработки команд меню
    void handleLoadGraph();
    void handleShowGraphInfo();
    void handleFindShortCircuits();
    void handleCreateExampleFile();
    
    // Вспомогательные методы
    void printMenu() const;
    void printHeader() const;
};

#endif // USER_INTERFACE_H