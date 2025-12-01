#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include "ExpressionParser.h"

// обёртка для работы с файловой системой
class FileSystemHelper
{
public:
    static bool createDirectory(const std::string &path)
    {
#ifdef _WIN32
        return _mkdir(path.c_str()) == 0;
#else
        return mkdir(path.c_str(), 0755) == 0;
#endif
    }

    static bool directoryExists(const std::string &path)
    {
        struct stat info;
        if (stat(path.c_str(), &info) != 0)
            return false;
        return (info.st_mode & S_IFDIR) != 0;
    }

    static bool fileExists(const std::string &path)
    {
        struct stat info;
        return stat(path.c_str(), &info) == 0;
    }

    static bool isRegularFile(const std::string &path)
    {
        struct stat info;
        if (stat(path.c_str(), &info) != 0)
            return false;
        return S_ISREG(info.st_mode);
    }

    static std::vector<std::string> getFilesInDirectory(const std::string &path)
    {
        std::vector<std::string> files;

        DIR *dir = opendir(path.c_str());
        if (dir == nullptr)
        {
            return files;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            // Пропускаем . и ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            // Формируем полный путь
            std::string fullPath = path + "/" + entry->d_name;

            // Проверяем, что это обычный файл
            if (isRegularFile(fullPath))
            {
                files.push_back(entry->d_name);
            }
        }

        closedir(dir);
        std::sort(files.begin(), files.end());
        return files;
    }
};

class InteractiveMenu
{
private:
    ExpressionParser parser;
    std::string testsDir = "tests";

    void displayMainMenu()
    {
        std::cout << "\n=== Expression Parser ===" << std::endl;
        std::cout << "1. Enter expression from terminal" << std::endl;
        std::cout << "2. Choose test file from tests directory" << std::endl;
        std::cout << "3. Exit" << std::endl;
        std::cout << "Select option (1-3): ";
    }

    void processTerminalInput()
    {
        std::cout << "\n=== Terminal Input Mode ===" << std::endl;
        std::cout << "Enter mathematical expression (or 'back' to return):" << std::endl;
        std::cout << "Supported operations: +, -, *, /, ^, SIN, COS, EXP" << std::endl;
        std::cout << "Example: (2 + 3) * SIN(0) + COS(0)" << std::endl;

        std::string input;
        while (true)
        {
            std::cout << "> ";
            std::getline(std::cin, input);

            if (input == "back" || input == "exit")
            {
                break;
            }

            if (input.empty())
            {
                continue;
            }

            try
            {
                std::vector<std::string> postfix = parser.toPostfix(input);
                double result = parser.evaluate(input);

                std::cout << "Postfix notation: ";
                for (const auto &token : postfix)
                {
                    std::cout << token << " ";
                }
                std::cout << std::endl;
                std::cout << "Result: " << result << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void ensureTestsDirectory()
    {
        if (!FileSystemHelper::directoryExists(testsDir))
        {
            std::cout << "Tests directory doesn't exist. Creating '" << testsDir << "' directory..." << std::endl;
            if (!FileSystemHelper::createDirectory(testsDir))
            {
                std::cerr << "Failed to create tests directory!" << std::endl;
                return;
            }

            // Создаём простые тестовые файлы
            createSampleTestFiles();
        }
    }

    void createSampleTestFiles()
    {
        // Простые тестовые данные 1: базовые операции
        std::ofstream file1(testsDir + "/basic_operations.txt");
        if (file1.is_open())
        {
            file1 << "# Basic arithmetic operations test file" << std::endl;
            file1 << "2 + 3 * 4" << std::endl;
            file1 << "(1 + 2) * 3" << std::endl;
            file1 << "10 - 4 / 2" << std::endl;
            file1 << "2 ^ 3 + 1" << std::endl;
            file1.close();
            std::cout << "Created: " << testsDir << "/basic_operations.txt" << std::endl;
        }

        // Простые тестовые данные 2: функции
        std::ofstream file2(testsDir + "/functions.txt");
        if (file2.is_open())
        {
            file2 << "# Mathematical functions test file" << std::endl;
            file2 << "SIN(0) + COS(0)" << std::endl;
            file2 << "EXP(0)" << std::endl;
            file2 << "SIN(3.14159) + 1" << std::endl;
            file2.close();
            std::cout << "Created: " << testsDir << "/functions.txt" << std::endl;
        }

        // Простые тестовые данные 3: составные операции
        std::ofstream file3(testsDir + "/edge_cases.txt");
        if (file3.is_open())
        {
            file3 << "# Edge cases and error handling" << std::endl;
            file3 << "-5 + 3" << std::endl;
            file3 << "1 + 2 * (3 - 4)" << std::endl;
            file3 << "3 + 4 * 2 / (1 - 5) ^ 2" << std::endl;
            file3.close();
            std::cout << "Created: " << testsDir << "/edge_cases.txt" << std::endl;
        }

        // Простые тестовые данные 4: остальные операции
        std::ofstream file4(testsDir + "/complex.txt");
        if (file4.is_open())
        {
            file4 << "# Complex expressions" << std::endl;
            file4 << "SIN(0) * COS(0) + EXP(0)" << std::endl;
            file4 << "(2 + 3) * (4 - 1) ^ 2" << std::endl;
            file4 << "10 / 2 + 3 * 4 - 1" << std::endl;
            file4.close();
            std::cout << "Created: " << testsDir << "/complex.txt" << std::endl;
        }

        std::cout << "Sample test files created in '" << testsDir << "' directory." << std::endl;
    }

    std::vector<std::string> getTestFiles()
    {
        return FileSystemHelper::getFilesInDirectory(testsDir);
    }

    void displayFileMenu(const std::vector<std::string> &files)
    {
        std::cout << "\n=== Available Test Files ===" << std::endl;
        if (files.empty())
        {
            std::cout << "No test files found." << std::endl;
            return;
        }

        for (size_t i = 0; i < files.size(); ++i)
        {
            std::cout << i + 1 << ". " << files[i] << std::endl;
        }
        std::cout << "0. Back to main menu" << std::endl;
        std::cout << "Select file (0-" << files.size() << "): ";
    }

    void processFileSelection()
    {
        ensureTestsDirectory();
        std::vector<std::string> files = getTestFiles();

        if (files.empty())
        {
            std::cout << "No test files found in '" << testsDir << "' directory." << std::endl;
            std::cout << "Sample files should be created automatically. You can also create your own .txt files in the 'tests' directory." << std::endl;
            return;
        }

        while (true)
        {
            displayFileMenu(files);

            std::string input;
            std::getline(std::cin, input);

            if (input == "0")
            {
                break;
            }

            try
            {
                int choice = std::stoi(input);
                if (choice < 1 || choice > static_cast<int>(files.size()))
                {
                    std::cout << "Invalid selection. Please try again." << std::endl;
                    continue;
                }

                std::string selectedFile = testsDir + "/" + files[choice - 1];
                std::cout << "Selected file: " << selectedFile << std::endl;
                processTestFile(selectedFile);

                std::cout << "\nProcess another file? (y/n): ";
                std::string response;
                std::getline(std::cin, response);
                if (response != "y" && response != "Y")
                {
                    break;
                }
            }
            catch (const std::exception &)
            {
                std::cout << "Invalid input. Please enter a number." << std::endl;
            }
        }
    }

    void processTestFile(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
            return;
        }

        std::cout << "\n=== Processing File: " << filename << " ===" << std::endl;

        std::string line;
        int lineNumber = 0;
        int successCount = 0;
        int errorCount = 0;

        while (std::getline(file, line))
        {
            lineNumber++;

            // Пропуск пустых строк и коментариев
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            // Очистка пустых символов слево и справа
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty())
            {
                continue;
            }

            std::cout << "\nLine " << lineNumber << ": " << line << std::endl;

            try
            {
                std::vector<std::string> postfix = parser.toPostfix(line);
                double result = parser.evaluate(line);

                std::cout << "  Postfix: ";
                for (const auto &token : postfix)
                {
                    std::cout << token << " ";
                }
                std::cout << std::endl;
                std::cout << "  Result: " << result << std::endl;
                successCount++;
            }
            catch (const std::exception &e)
            {
                std::cerr << "  Error: " << e.what() << std::endl;
                errorCount++;
            }
        }

        file.close();

        std::cout << "\n=== File Processing Complete ===" << std::endl;
        std::cout << "Successfully processed: " << successCount << " expressions" << std::endl;
        std::cout << "Errors: " << errorCount << " expressions" << std::endl;
        std::cout << "Total: " << (successCount + errorCount) << " expressions" << std::endl;
    }

    void processSingleFile(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
            return;
        }

        std::cout << "Processing file: " << filename << std::endl;
        std::string line;
        int lineNumber = 0;

        while (std::getline(file, line))
        {
            lineNumber++;
            if (line.empty() || line[0] == '#')
                continue;

            // Очистка от пробелов
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty())
                continue;

            std::cout << "Line " << lineNumber << ": " << line << std::endl;
            try
            {
                std::vector<std::string> postfix = parser.toPostfix(line);
                double result = parser.evaluate(line);

                std::cout << "  Postfix: ";
                for (const auto &token : postfix)
                {
                    std::cout << token << " ";
                }
                std::cout << std::endl;
                std::cout << "  Result: " << result << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "  Error: " << e.what() << std::endl;
            }
            std::cout << std::endl;
        }

        file.close();
    }

public:
    void run()
    {
        std::cout << "Welcome to Expression Parser!" << std::endl;
        std::cout << "Supports: +, -, *, /, ^, SIN, COS, EXP, parentheses" << std::endl;

        while (true)
        {
            displayMainMenu();

            std::string choice;
            std::getline(std::cin, choice);

            if (choice == "1")
            {
                processTerminalInput();
            }
            else if (choice == "2")
            {
                processFileSelection();
            }
            else if (choice == "3" || choice == "exit" || choice == "quit")
            {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            else
            {
                std::cout << "Invalid option. Please try again." << std::endl;
            }
        }
    }

    void runWithFile(const std::string &filename)
    {
        if (!FileSystemHelper::fileExists(filename))
        {
            std::cerr << "Error: File '" << filename << "' does not exist." << std::endl;
            return;
        }
        processSingleFile(filename);
    }
};

int main(int argc, char *argv[])
{
    InteractiveMenu menu;

    // Если передаётся файл, то применяем его
    if (argc == 2)
    {
        menu.runWithFile(argv[1]);
    }
    else
    {
        // Если без файла, то запускаем меню
        menu.run();
    }

    return 0;
}