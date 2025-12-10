#include "UserInterface.h"
#include <iostream>
#include <string>
#include <cstdlib>

void UserInterface::printHeader() const {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "АНАЛИЗАТОР ЭЛЕКТРИЧЕСКИХ СХЕМ" << std::endl;
    std::cout << "==================================================" << std::endl;
}

void UserInterface::printMenu() const {
    std::cout << "\nМЕНЮ:" << std::endl;
    std::cout << "1. Загрузить граф из файла" << std::endl;
    std::cout << "2. Показать информацию о графе" << std::endl;
    std::cout << "3. Найти короткие замыкания" << std::endl;
    std::cout << "4. Создать примерный файл" << std::endl;
    std::cout << "5. Выход" << std::endl;
}

void UserInterface::handleLoadGraph() {
    std::string filename;
    std::cout << "Введите имя файла с графом: ";
    std::getline(std::cin, filename);
    
    if (!filename.empty()) {
        analyzer.loadGraph(filename);
    }
}

void UserInterface::handleShowGraphInfo() {
    analyzer.printGraphInfo();
}

void UserInterface::handleFindShortCircuits() {
    if (!analyzer.isGraphLoaded()) {
        std::cout << "Сначала загрузите граф из файла!" << std::endl;
        return;
    }
    
    std::string thresholdStr;
    std::cout << "Введите пороговое значение сопротивления: ";
    std::getline(std::cin, thresholdStr);
    
    try {
        double threshold = std::stod(thresholdStr);
        if (threshold <= 0) {
            std::cout << "Пороговое значение должно быть положительным!" << std::endl;
            return;
        }
        
        std::cout << "\nВыберите алгоритм поиска:" << std::endl;
        std::cout << "1. DFS (глубина) - находит все пути" << std::endl;
        std::cout << "2. BFS (ширина) - быстрее для небольших порогов" << std::endl;
        
        std::string algoChoice;
        std::cout << "Ваш выбор (1-2, по умолчанию 1): ";
        std::getline(std::cin, algoChoice);
        
        std::vector<ShortCircuit> shortCircuits;
        
        if (algoChoice == "2") {
            shortCircuits = CircuitFinder::findShortCircuitsBFS(analyzer, threshold);
        } else {
            shortCircuits = CircuitFinder::findShortCircuitsDFS(analyzer, threshold);
        }
        
        CircuitFinder::displayResults(shortCircuits);
        
    } catch (...) {
        std::cout << "Ошибка: введите числовое значение!" << std::endl;
    }
}

void UserInterface::handleCreateExampleFile() {
    std::string filename;
    std::cout << "Введите имя файла для примера: ";
    std::getline(std::cin, filename);
    
    if (!filename.empty()) {
        FileHandler::createExampleFile(filename);
    }
}

void UserInterface::run() {
    // Установка локали для корректного вывода
    std::locale::global(std::locale(""));
    
    printHeader();
    
    // Создаем примерный файл при первом запуске
    FileHandler::createExampleFile("circuit_example.txt");
    
    while (true) {
        printMenu();
        
        std::string choice;
        std::cout << "Выберите действие (1-5): ";
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            handleLoadGraph();
        } else if (choice == "2") {
            handleShowGraphInfo();
        } else if (choice == "3") {
            handleFindShortCircuits();
        } else if (choice == "4") {
            handleCreateExampleFile();
        } else if (choice == "5") {
            std::cout << "Выход из программы..." << std::endl;
            break;
        } else {
            std::cout << "Неверный выбор. Попробуйте снова." << std::endl;
        }
    }
}