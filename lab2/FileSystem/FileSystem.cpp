#include "FileSystem.h"

// Определяем fs в зависимости от доступности файловой системы
#if __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "No filesystem support"
#endif

namespace ch = std::chrono;

FileSystem::Node::Node(const std::string &name, bool isFolder, Node *parent = nullptr) : name(name), isFolder(isFolder), parent(parent), lastModified(ch::system_clock::now())
{
}

FileSystem::FileSystem()
{
    root = std::make_unique<Node>("root", true);
    current = root.get();
    binaryFilePath = "filesystem.dat";
    textFilePath = "filesystem.txt";
    isModified = false;
    isTextFileModified = false;
    lastTextSyncTimeNs = 0;
}

bool fileExists(const std::string &filename)
{
    return access(filename.c_str(), F_OK) != -1;
}

int64_t getLastWriteTime(const std::string &filename)
{
    struct stat fileInfo;
    if (stat(filename.c_str(), &fileInfo) != 0)
    {
        return 0; // Файл не существует или ошибка
    }

// Преобразуем время в наносекунды
#ifdef __APPLE__
    // macOS
    return static_cast<int64_t>(fileInfo.st_mtimespec.tv_sec) * 1000000000LL +
           static_cast<int64_t>(fileInfo.st_mtimespec.tv_nsec);
#else
    // Linux и другие системы
    return static_cast<int64_t>(fileInfo.st_mtim.tv_sec) * 1000000000LL +
           static_cast<int64_t>(fileInfo.st_mtim.tv_nsec);
#endif
}

bool FileSystem::checkTextFileModified()
{
    if (!fileExists(textFilePath))
    {
        return false;
    }

    auto currentTimeNs = getLastWriteTime(textFilePath);
    return currentTimeNs > lastTextSyncTimeNs;
}

void FileSystem::updateTextFileTimestamp()
{
    if (fileExists(textFilePath))
    {
        lastTextSyncTimeNs = getLastWriteTime(textFilePath);
    }
}

bool FileSystem::saveToBinaryFile(const std::string &filename)
{
    binaryFilePath = filename;

    std::ofstream out(filename, std::ios::binary);
    if (!out)
    {
        std::cout << "Cannot open binary file for writing: " << filename << std::endl;
        return false;
    }

    try
    {
        // Сохраняем время последней синхронизации
        out.write(reinterpret_cast<const char *>(&lastTextSyncTimeNs), sizeof(lastTextSyncTimeNs));

        // Сохраняем дерево
        saveNode(out, root.get());

        std::cout << "File system saved to binary file: " << filename << std::endl;
        isModified = false;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "Error saving binary file: " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::loadFromBinaryFile(const std::string &filename)
{
    binaryFilePath = filename;

    std::ifstream in(filename, std::ios::binary);
    if (!in)
    {
        std::cout << "Creating new file system (binary file not found)." << std::endl;
        return false;
    }

    try
    {
        // Читаем время последней синхронизации
        in.read(reinterpret_cast<char *>(&lastTextSyncTimeNs), sizeof(lastTextSyncTimeNs));

        // Загружаем дерево
        root = loadNode(in, nullptr);
        current = root.get();

        std::cout << "File system loaded from binary file: " << filename << std::endl;
        isModified = false;

        // Проверяем, изменился ли текстовый файл
        isTextFileModified = checkTextFileModified();
        if (isTextFileModified)
        {
            std::cout << "Warning: Text file '" << textFilePath << "' has been modified externally." << std::endl;
        }

        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "Error loading binary file: " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::autoSync()
{
    bool success = true;

    // Если есть изменения в памяти, сохраняем в бинарный файл
    if (isModified)
    {
        success = success && saveToBinaryFile(binaryFilePath);
    }

    // Проверяем, изменился ли текстовый файл
    if (checkTextFileModified())
    {
        std::cout << "Text file has been modified. Do you want to import changes? (y/n): ";
        std::string answer;
        std::getline(std::cin, answer);

        if (answer == "y" || answer == "Y")
        {
            success = success && importFromTextFile(textFilePath);
        }
        else
        {
            // Обновляем время даже если не импортируем
            updateTextFileTimestamp();
        }
    }

    return success;
}

bool FileSystem::exportToTextFile(const std::string &filename)
{
    textFilePath = filename;

    std::ofstream out(filename);
    if (!out)
    {
        std::cout << "Cannot open text file for writing: " << filename << std::endl;
        return false;
    }

    try
    {
        // Заголовок файла
        auto now = ch::system_clock::now();
        auto in_time_t = ch::system_clock::to_time_t(now);
        out << "# File System Structure - Generated "
            << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
        out << "# Format: [TYPE] name" << std::endl;
        out << "# File content follows after empty line" << std::endl;
        out << "==========================================" << std::endl
            << std::endl;

        saveNodeToText(out, root.get(), 0);

        std::cout << "File system exported to text file: " << filename << std::endl;

        // Обновляем время синхронизации
        updateTextFileTimestamp();

        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "Error exporting to text file: " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::importFromTextFile(const std::string &filename)
{
    textFilePath = filename;

    std::ifstream in(filename);
    if (!in)
    {
        std::cout << "Cannot open text file for reading: " << filename << std::endl;
        return false;
    }

    int lineNumber = 0;

    try
    {
        // Пропускаем заголовок
        std::string line;
        while (std::getline(in, line))
        {
            if (line.find("==========================================") != std::string::npos)
            {
                break;
            }
        }

        // Загружаем дерево
        root = std::make_unique<Node>("root", true, nullptr);
        current = root.get();

        std::stack<std::pair<Node *, int>> nodeStack;
        nodeStack.push({root.get(), 0});

        Node *currentNode = root.get();
        int currentDepth = 0;
        std::string currentContent;
        bool readingContent = false;
        std::string currentFileName;

        while (std::getline(in, line))
        {
            lineNumber++;

            if (line.empty())
            {
                if (readingContent)
                {
                    // Завершаем чтение содержимого
                    for (auto &child : currentNode->children)
                    {
                        if (child->name == currentFileName)
                        {
                            child->content = currentContent;
                            break;
                        }
                    }
                    readingContent = false;
                    currentContent.clear();
                }
                continue;
            }

            // Если читаем содержимое файла
            if (readingContent)
            {
                currentContent += line + "\n";
                continue;
            }

            // Определяем уровень вложенности
            int depth = 0;
            while (depth * 2 < line.length() && line[depth * 2] == ' ')
            {
                depth++;
            }

            // Убираем отступы
            line = line.substr(depth * 2);

            // Восстанавливаем стек
            while (!nodeStack.empty() && nodeStack.top().second >= depth)
            {
                nodeStack.pop();
            }

            if (!nodeStack.empty())
            {
                currentNode = nodeStack.top().first;
            }

            // Парсим строку
            if (line.find("[FOLDER] ") == 0)
            {
                std::string name = line.substr(9);
                auto newNode = std::make_unique<Node>(name, true, currentNode);
                nodeStack.push({newNode.get(), depth});
                currentNode->children.push_back(std::move(newNode));
            }
            else if (line.find("[FILE] ") == 0)
            {
                std::string name = line.substr(7);
                auto newNode = std::make_unique<Node>(name, false, currentNode);
                currentFileName = name;
                currentNode->children.push_back(std::move(newNode));
                readingContent = true;
            }
        }

        std::cout << "File system imported from text file: " << filename << std::endl;

        // Обновляем время синхронизации
        updateTextFileTimestamp();
        isModified = true;

        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "Error importing from text file at line " << lineNumber << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::syncFromTextIfModified()
{
    if (checkTextFileModified())
    {
        std::cout << "Text file has been modified. Importing changes..." << std::endl;
        if (importFromTextFile(textFilePath))
        {
            // После импорта сохраняем в бинарный файл
            return saveToBinaryFile(binaryFilePath);
        }
        return false;
    }
    std::cout << "Text file is up to date. No sync needed." << std::endl;
    return true;
}

bool FileSystem::isSyncRequired()
{
    return isModified || checkTextFileModified();
}

// Пометить как измененное
void FileSystem::markAsModified()
{
    isModified = true;
}

// Сохранение узла в текстовый файл
void FileSystem::saveNodeToText(std::ofstream &out, Node *node, int depth)
{
    std::string indent(depth * 2, ' ');

    if (node->name == "root" && depth == 0)
    {
        // Пропускаем корневой узел в выводе
    }
    else
    {
        if (node->isFolder)
        {
            out << indent << "[FOLDER] " << node->name << std::endl;
        }
        else
        {
            out << indent << "[FILE] " << node->name << std::endl;
        }
    }

    // Для файлов сохраняем содержимое
    if (!node->isFolder && !node->content.empty())
    {
        out << std::endl;
        std::istringstream contentStream(node->content);
        std::string line;
        while (std::getline(contentStream, line))
        {
            out << indent << "  " << line << std::endl;
        }
        out << std::endl;
    }

    // Рекурсивно сохраняем детей
    for (auto &child : node->children)
    {
        saveNodeToText(out, child.get(), depth + 1);
    }
}

bool FileSystem::importFromRealPath(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        std::cout << "Нельзя открыть дирректорию: " << path << std::endl;
        return false;
    }

    std::cout << "Импортировать файловую систему в дирректории: " << path << std::endl;

    std::string folderName = path;
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos && lastSlash + 1 < path.length())
    {
        folderName = path.substr(lastSlash + 1);
    }

    root = std::make_unique<Node>(folderName, true);
    current = root.get();

    importFromRealPathRecursive(path, root.get());

    closedir(dir);
    std::cout << "Файловая система загружена" << std::endl;
    return true;
}

void FileSystem::importFromRealPathRecursive(const std::string &currentPath, Node *currentNode)
{
    DIR *dir = opendir(currentPath.c_str());
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
        {
            continue;
        }

        std::string fullPath = currentPath + "/" + entry->d_name;

        struct stat statBuf;
        if (stat(fullPath.c_str(), &statBuf) != 0)
        {
            continue;
        }

        bool isDirectory = S_ISDIR(statBuf.st_mode);
        auto newNode = std::make_unique<Node>(entry->d_name, isDirectory, currentNode);

        if (isDirectory)
        {
            importFromRealPathRecursive(fullPath, newNode.get());
        }

        currentNode->children.push_back(std::move(newNode));
    }

    closedir(dir);
}

void FileSystem::saveNode(std::ofstream &out, Node *node)
{
    // Сохраняем имя
    size_t nameLength = node->name.size();
    out.write(reinterpret_cast<const char *>(&nameLength), sizeof(nameLength));
    out.write(node->name.c_str(), nameLength);

    // Сохраняем тип
    out.write(reinterpret_cast<const char *>(&node->isFolder), sizeof(node->isFolder));

    // Сохраняем время модификации
    auto timeCount = node->lastModified.time_since_epoch().count();
    out.write(reinterpret_cast<const char *>(&timeCount), sizeof(timeCount));

    // Сохраняем содержимое файла
    if (!node->isFolder)
    {
        size_t contentLength = node->content.size();
        out.write(reinterpret_cast<const char *>(&contentLength), sizeof(contentLength));
        out.write(node->content.c_str(), contentLength);
    }
    else
    {
        size_t contentLength = 0;
        out.write(reinterpret_cast<const char *>(&contentLength), sizeof(contentLength));
    }

    // Сохраняем количество детей
    size_t childCount = node->children.size();
    out.write(reinterpret_cast<const char *>(&childCount), sizeof(childCount));

    // Рекурсивно сохраняем детей
    for (auto &child : node->children)
    {
        saveNode(out, child.get());
    }
}

std::unique_ptr<FileSystem::Node> FileSystem::loadNode(std::ifstream &in, Node *parent)
{
    // Читаем имя
    size_t nameLength;
    in.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));

    std::string name;
    name.resize(nameLength);
    in.read(&name[0], nameLength);

    // Читаем тип
    bool isFolder;
    in.read(reinterpret_cast<char *>(&isFolder), sizeof(isFolder));

    // Читаем время модификации
    int64_t timeCount;
    in.read(reinterpret_cast<char *>(&timeCount), sizeof(timeCount));
    ch::system_clock::time_point lastModified = ch::system_clock::time_point(ch::nanoseconds(timeCount));

    // Читаем содержимое файла
    std::string content;
    if (!isFolder)
    {
        size_t contentLength;
        in.read(reinterpret_cast<char *>(&contentLength), sizeof(contentLength));

        if (contentLength > 0)
        {
            content.resize(contentLength);
            in.read(&content[0], contentLength);
        }
    }

    // Создаем узел
    auto node = std::make_unique<Node>(name, isFolder, parent);
    node->lastModified = lastModified;
    node->content = content;

    // Читаем количество детей
    size_t childCount;
    in.read(reinterpret_cast<char *>(&childCount), sizeof(childCount));

    // Рекурсивно читаем детей
    for (size_t i = 0; i < childCount; ++i)
    {
        auto child = loadNode(in, node.get());
        node->children.push_back(std::move(child));
    }

    return node;
}

FileSystem::Node *FileSystem::findNode(Node *start, const std::string &name)
{
    if (start->name == name)
        return start;

    if (name.find('/') != std::string::npos)
    {
        return findNodeByPath(name);
    }

    for (auto &child : start->children)
    {
        if (child->name == name)
            return child.get();
        if (child->isFolder)
        {
            Node *found = findNode(child.get(), name);
            if (found)
                return found;
        }
    }
    return nullptr;
}

void FileSystem::printTree(Node *node, int depth)
{
    std::string indent(depth * 2, ' ');
    std::cout << indent << (node->isFolder ? "[FOLDER] " : "[FILE] ") << node->name << std::endl;

    for (auto &child : node->children)
    {
        printTree(child.get(), depth + 1);
    }
}

bool FileSystem::loadFromFile(const std::string &filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in)
    {
        std::cout << "Ошбика открытия файла с деревом для чтения: " << filename << std::endl;
        return false;
    }

    try
    {
        root = loadNode(in, nullptr);
        current = root.get();
        std::cout << "Файл с деревом сохранён " << filename << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "Ошибка сохранения файла с деревом:" << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::saveToFile(const std::string &filename)
{
    std::ofstream out(filename, std::ios::binary);
    if (!out)
    {
        std::cout << "Ошбика открытия файла с деревом: " << filename << std::endl;
        return false;
    }

    try
    {
        saveNode(out, root.get());
        std::cout << "Файл с деревом сохранён " << filename << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "Ошибка сохранения файла с деревом: " << e.what() << std::endl;
        return false;
    }
}

void FileSystem::navigate()
{
    std::string command;
    int commandCount = 0;

    auto periodicCheck = [this]()
    {
        if (this->checkTextFileModified())
        {
            std::cout << "\n[NOTICE] Text file has been modified externally!" << std::endl;
            std::cout << "Run 'sync_text' to import changes." << std::endl;
        }
    };

    while (true)
    {
        std::cout << "\n[" << binaryFilePath << "] > ";
        std::getline(std::cin, command);

        if (command == "exit")
        {
            break;
        }

        // Выполняем команду (теперь это private метод, доступный только внутри класса)
        executeCommand(command);

        // Периодическая проверка изменений
        if (++commandCount % 5 == 0)
        {
            periodicCheck();
        }
    }
}

void FileSystem::help()
{
    std::cout << "\nТекущая директория: " << current->name << std::endl;
    std::cout << "Список команд: \nls - содержание в текущей дирректории;"
              << std::endl;
    std::cout << "import .          - импорт файловой системы из текущей дирректории"
              << std::endl;
    std::cout << "import /home/user - импорт файловой системы из указанной дирректории"
              << std::endl;
    std::cout << "cd <name> - перейти в дирректорию;"
              << std::endl;
    std::cout << "cd .. - перейти на дирректорию ниже"
              << std::endl;
    std::cout << "mkdir <name> - создать новую пупку"
              << std::endl;
    std::cout << "touch <name> - создать новый файл"
              << std::endl;
    std::cout << "rm <name> - удалить папку/файл"
              << std::endl;
    std::cout << "find <name> - найти папку/файл"
              << std::endl;
    std::cout << "tree - показать дерево"
              << std::endl;
    std::cout << "  export_text   - export to text file (filesystem.txt)" << std::endl;
    std::cout << "  import_text   - import from text file" << std::endl;
    std::cout << "  sync_text     - sync if text file was modified" << std::endl;
    std::cout << "  auto_sync     - auto-sync changes" << std::endl;
    std::cout << "  save          - save to binary file" << std::endl;
    std::cout << "  status        - show sync status" << std::endl;
    std::cout << "exit - выход"
              << std::endl;
}

void FileSystem::createFolder(const std::string &name)
{
    for (auto &child : current->children)
    {
        if (child->name == name)
        {
            std::cout << "Item with name '" << name << "' already exists!" << std::endl;
            return;
        }
    }

    current->children.push_back(std::make_unique<Node>(name, true, current));
    markAsModified();
    std::cout << "Folder '" << name << "' created successfully." << std::endl;
}

void FileSystem::createFile(const std::string &name)
{
    createFileWithContent(name, "");
}

void FileSystem::createFileWithContent(const std::string &name, const std::string &content)
{
    for (auto &child : current->children)
    {
        if (child->name == name)
        {
            std::cout << "Item with name '" << name << "' already exists!" << std::endl;
            return;
        }
    }

    auto newNode = std::make_unique<Node>(name, false, current);
    newNode->content = content;
    current->children.push_back(std::move(newNode));
    markAsModified();
    std::cout << "File '" << name << "' created successfully." << std::endl;
}

void FileSystem::remove(const std::string &name)
{
    auto it = std::find_if(current->children.begin(), current->children.end(),
                           [&name](const std::unique_ptr<Node> &node)
                           {
                               return node->name == name;
                           });

    if (it != current->children.end())
    {
        current->children.erase(it);
        std::cout << "Элемент '" << name << "' удалён" << std::endl;
    }
    else
    {
        std::cout << "Элемент '" << name << "' не найден" << std::endl;
    }
}

void FileSystem::rename(const std::string &oldName, const std::string &newName)
{
    for (auto &child : current->children)
    {
        if (child->name == oldName)
        {
            child->name = newName;
            std::cout << "Переименнование '" << oldName << "' в '" << newName << "' успешно " << std::endl;
            return;
        }
    }
    std::cout << "Элемент '" << oldName << "' не найден" << std::endl;
}

void FileSystem::executeCommand(const std::string &command)
{
    if (command == "ls")
    {
        listContents();
    }
    else if (command == "tree")
    {
        printTree(current);
    }
    else if (command.find("cd ") == 0)
    {
        changeDirectory(command.substr(3));
    }
    else if (command.find("mkdir ") == 0)
    {
        createFolder(command.substr(6));
    }
    else if (command.find("touch ") == 0)
    {
        createFile(command.substr(6));
    }
    else if (command.find("write ") == 0)
    {
        size_t spacePos = command.find(' ', 6);
        if (spacePos != std::string::npos)
        {
            std::string filename = command.substr(6, spacePos - 6);
            std::string content = command.substr(spacePos + 1);
            if (content.size() >= 2 && content.front() == '"' && content.back() == '"')
            {
                content = content.substr(1, content.size() - 2);
            }
            writeToFile(filename, content);
        }
        else
        {
            std::cout << "Usage: write <filename> \"<content>\"" << std::endl;
        }
    }
    else if (command.find("rm ") == 0)
    {
        remove(command.substr(3));
    }
    else if (command.find("find ") == 0)
    {
        findItem(command.substr(5));
    }
    else if (command.find("copy ") == 0)
    {
        size_t firstSpace = command.find(' ');
        size_t secondSpace = command.find(' ', firstSpace + 1);
        if (secondSpace != std::string::npos)
        {
            std::string source = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);
            std::string destination = command.substr(secondSpace + 1);
            copy(source, destination);
        }
        else
        {
            std::cout << "Usage: copy <source> <destination>" << std::endl;
        }
    }
    else if (command.find("move ") == 0)
    {
        size_t firstSpace = command.find(' ');
        size_t secondSpace = command.find(' ', firstSpace + 1);
        if (secondSpace != std::string::npos)
        {
            std::string source = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);
            std::string destination = command.substr(secondSpace + 1);
            move(source, destination);
        }
        else
        {
            std::cout << "Usage: move <source> <destination>" << std::endl;
        }
    }
    else if (command == "export_text")
    {
        exportToTextFile(textFilePath);
    }
    else if (command == "import_text")
    {
        importFromTextFile(textFilePath);
    }
    else if (command == "sync_text")
    {
        syncFromTextIfModified();
    }
    else if (command == "auto_sync")
    {
        autoSync();
    }
    else if (command == "status")
    {
        std::cout << "Status:" << std::endl;
        std::cout << "  Binary file: " << binaryFilePath << std::endl;
        std::cout << "  Text file: " << textFilePath << std::endl;
        std::cout << "  Memory modified: " << (isModified ? "YES" : "NO") << std::endl;
        std::cout << "  Text file modified: " << (checkTextFileModified() ? "YES" : "NO") << std::endl;
    }
    else if (command == "save")
    {
        saveToBinaryFile(binaryFilePath);
    }
    else if (command == "help")
    {
        help();
    }
    else if (command.find("import ") == 0)
    {
        std::string importPath = command.substr(7);
        if (importPath.empty())
        {
            importPath = "."; // Текущая директория по умолчанию
        }
        importFromRealPath(importPath);
    }
    else
    {
        std::cout << "Unknown command: " << command << std::endl;
    }
}

void FileSystem::writeToFile(const std::string &filename, const std::string &content)
{
    for (auto &child : current->children)
    {
        if (!child->isFolder && child->name == filename)
        {
            child->content = content;
            child->lastModified = ch::system_clock::now();
            markAsModified();
            std::cout << "Content written to file '" << filename << "'." << std::endl;
            return;
        }
    }
    std::cout << "File '" << filename << "' not found!" << std::endl;
}

void FileSystem::listContents()
{
    if (current->children.empty())
    {
        std::cout << "Дирректория пуста" << std::endl;
        return;
    }

    for (auto &child : current->children)
    {
        std::cout << (child->isFolder ? "[FOLDER] " : "[FILE]   ") << child->name << std::endl;
    }
}

void FileSystem::changeDirectory(const std::string &name)
{
    if (name == "..")
    {
        if (current->parent)
        {
            current = current->parent;
        }
        else
        {
            std::cout << "Уже в корневой дирректории" << std::endl;
        }
        return;
    }

    for (auto &child : current->children)
    {
        if (child->isFolder && child->name == name)
        {
            current = child.get();
            return;
        }
    }
    std::cout << "Папка '" << name << "' не найдена" << std::endl;
}

void FileSystem::findItem(const std::string &name)
{
    Node *found = findNode(current, name);
    if (found)
    {
        std::cout << "Найден: " << (found->isFolder ? "[FOLDER] " : "[FILE] ") << name << std::endl;
    }
    else
    {
        std::cout << "Элемент '" << name << "' не найден" << std::endl;
    }
}

std::vector<std::string> FileSystem::splitPath(const std::string &path)
{
    std::vector<std::string> components;
    std::string component;
    std::istringstream pathStream(path);

    while (std::getline(pathStream, component, '/'))
    {
        if (!component.empty())
        {
            components.push_back(component);
        }
    }

    return components;
}

std::string FileSystem::getParentPath(const std::string &path)
{
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash == std::string::npos)
    {
        return "";
    }
    return path.substr(0, lastSlash);
}

std::string FileSystem::getBaseName(const std::string &path)
{
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash == std::string::npos)
    {
        return path;
    }
    return path.substr(lastSlash + 1);
}

bool FileSystem::isValidPath(const std::vector<std::string> &pathComponents, Node *start)
{
    Node *currentNode = start ? start : root.get();

    for (const auto &component : pathComponents)
    {
        if (component == "..")
        {
            if (currentNode->parent)
            {
                currentNode = currentNode->parent;
            }
            else
            {
                return false;
            }
        }
        else if (component != ".")
        {
            bool found = false;
            for (auto &child : currentNode->children)
            {
                if (child->name == component)
                {
                    currentNode = child.get();
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return false;
            }
        }
    }
    return true;
}

FileSystem::Node *FileSystem::findNodeByPath(const std::string &path)
{
    if (path.empty() || path == "/")
    {
        return root.get();
    }

    std::vector<std::string> components = splitPath(path);
    Node *currentNode = root.get();

    for (const auto &component : components)
    {
        if (component == "..")
        {
            if (currentNode->parent)
            {
                currentNode = currentNode->parent;
            }
            else
            {
                return nullptr;
            }
        }
        else if (component != ".")
        {
            bool found = false;
            for (auto &child : currentNode->children)
            {
                if (child->name == component)
                {
                    currentNode = child.get();
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return nullptr;
            }
        }
    }

    return currentNode;
}

std::unique_ptr<FileSystem::Node> FileSystem::copyNode(Node *source, Node *newParent)
{
    auto newNode = std::make_unique<Node>(source->name, source->isFolder, newParent);
    newNode->content = source->content;

    for (auto &child : source->children)
    {
        newNode->children.push_back(copyNode(child.get(), newNode.get()));
    }

    return newNode;
}

bool FileSystem::copy(const std::string &source, const std::string &destination)
{
    Node *sourceNode = findNodeByPath(source);
    if (!sourceNode)
    {
        std::cout << "Путь не найден  " << source << std::endl;
        return false;
    }

    std::string destDirPath = getParentPath(destination);
    std::string newName = getBaseName(destination);

    if (newName.empty())
    {
        newName = sourceNode->name;
    }

    Node *destDir = nullptr;
    if (destDirPath.empty())
    {
        destDir = current;
    }
    else
    {
        destDir = findNodeByPath(destDirPath);
    }

    if (!destDir || !destDir->isFolder)
    {
        std::cout << "Выбранной папки не существует или это не папка: " << destDirPath << std::endl;
        return false;
    }

    for (auto &child : destDir->children)
    {
        if (child->name == newName)
        {
            std::cout << "Элемент с таким именем '" << newName << "' уже существует" << std::endl;
            return false;
        }
    }

    auto copiedNode = copyNode(sourceNode, destDir);
    copiedNode->name = newName;
    destDir->children.push_back(std::move(copiedNode));

    std::cout << "Скопировали '" << source << "' в '" << destination << std::endl;
    return true;
}

bool FileSystem::move(const std::string &source, const std::string &destination)
{
    Node *sourceNode = findNodeByPath(source);
    if (!sourceNode)
    {
        std::cout << "Путь не найден " << source << std::endl;
        return false;
    }

    Node *sourceParent = sourceNode->parent;
    if (!sourceParent)
    {
        std::cout << "Нельзя перемещать корневую дирректорию" << std::endl;
        return false;
    }

    std::string destDirPath = getParentPath(destination);
    std::string newName = getBaseName(destination);

    if (newName.empty())
    {
        newName = sourceNode->name;
    }

    Node *destDir = nullptr;
    if (destDirPath.empty())
    {
        destDir = current;
    }
    else
    {
        destDir = findNodeByPath(destDirPath);
    }

    if (!destDir || !destDir->isFolder)
    {
        std::cout << "Выбранной папки не существует или это не папка: " << destDirPath << std::endl;
        return false;
    }

    if (sourceNode->isFolder)
    {
        Node *checkNode = destDir;
        while (checkNode)
        {
            if (checkNode == sourceNode)
            {
                std::cout << "Нельзя переместить папку в себя или в свою подпапку" << std::endl;
                return false;
            }
            checkNode = checkNode->parent;
        }
    }

    for (auto &child : destDir->children)
    {
        if (child->name == newName)
        {
            std::cout << "Элемент с таким именем '" << newName << "' уже существует" << std::endl;
            return false;
        }
    }

    auto it = std::find_if(sourceParent->children.begin(), sourceParent->children.end(),
                           [sourceNode](const std::unique_ptr<Node> &node)
                           {
                               return node.get() == sourceNode;
                           });

    if (it == sourceParent->children.end())
    {
        std::cout << "Ошибка: исходный узел не найден в дочерних узлах родителя" << std::endl;
        return false;
    }

    auto nodeToMove = std::move(*it);
    sourceParent->children.erase(it);

    nodeToMove->parent = destDir;
    nodeToMove->name = newName;
    destDir->children.push_back(std::move(nodeToMove));

    std::cout << "Переместили '" << source << "' в '" << destination << std::endl;
    return true;
}