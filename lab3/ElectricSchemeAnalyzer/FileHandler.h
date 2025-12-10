#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>

class FileHandler {
public:
    // Создание примерного файла
    static bool createExampleFile(const std::string& filename);
    
    // Проверка существования файла
    static bool fileExists(const std::string& filename);
    
private:
    FileHandler() = delete; // Статический класс
};

#endif // FILE_HANDLER_H