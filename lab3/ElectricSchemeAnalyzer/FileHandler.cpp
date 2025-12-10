#include "FileHandler.h"
#include <fstream>
#include <iostream>

bool FileHandler::createExampleFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Ошибка: не удалось создать файл '" << filename << "'" << std::endl;
        return false;
    }
    
    file << "# Пример файла с описанием электрической схемы\n";
    file << "# Формат: вершина1 вершина2 сопротивление [source]\n";
    file << "# source указывается только для ребра с источником питания\n\n";
    file << "A B 10.0\n";
    file << "B C 5.0\n";
    file << "C D 8.0\n";
    file << "D A 12.0\n";
    file << "A C 3.0 source  # источник питания на этом ребре\n";
    file << "B D 7.0\n";
    file << "C E 6.0\n";
    file << "D E 4.0\n";
    file << "E A 9.0\n";
    
    file.close();
    std::cout << "Примерный файл создан: " << filename << std::endl;
    return true;
}

bool FileHandler::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}