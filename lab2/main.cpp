#include "FileSystem/FileSystem.h"
#include <iostream>

void displayHelp()
{
    std::cout << "=== Менеджер файловой системы ===" << std::endl;
    std::cout << "Функцианал:" << std::endl;
    std::cout << "- Загрузка/сохранение в бинарный файл или из него" << std::endl;
    std::cout << "- Навгация по файловой системе" << std::endl;
    std::cout << "- Операции над файлами и папками" << std::endl;
    std::cout << "- Импорт вашей файловой системы" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << "- help для вывода списка команд" << std::endl;
    std::cout << "============================" << std::endl;
}

int main()
{
    displayHelp();

    FileSystem fs;

    // Загружаем из бинарного файла
    if (!fs.loadFromBinaryFile("filesystem.dat"))
    {
        std::cout << "Starting with empty file system." << std::endl;
    }

    // Запуск интерактивного режима (используем встроенный метод navigate)
    std::cout << "\nStarting interactive mode..." << std::endl;
    fs.navigate();

    // Автоматическое сохранение при выходе
    std::cout << "\nSaving changes..." << std::endl;

    if (fs.getIsModified())
    {
        if (fs.saveToBinaryFile("filesystem.dat"))
        {
            std::cout << "Binary file saved successfully." << std::endl;
        }
        else
        {
            std::cout << "Warning: Failed to save binary file!" << std::endl;
        }
    }

    // Предложить экспорт в текстовый формат
    std::cout << "\nExport to text format? (y/n): ";
    std::string answer;
    std::getline(std::cin, answer);

    if (answer == "y" || answer == "Y")
    {
        fs.exportToTextFile("filesystem.txt");
    }

    std::cout << "Goodbye!" << std::endl;
    return 0;
}