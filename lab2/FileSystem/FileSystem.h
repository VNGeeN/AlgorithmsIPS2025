#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>    
#include <ctime>       
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <stack>

class FileSystem
{
private:
    struct Node
    {
        std::string name;
        bool isFolder;
        std::vector<std::unique_ptr<Node>> children;
        Node *parent;
        std::string content;

        std::chrono::system_clock::time_point lastModified;

        Node(const std::string &name, bool isFolder, Node *parent);
    };

    std::unique_ptr<Node> root;
    Node *current;

    int64_t lastTextSyncTimeNs;
    std::string binaryFilePath;
    std::string textFilePath;

    bool isModified;
    bool isTextFileModified;

    void saveNode(std::ofstream &out, Node *node);
    std::unique_ptr<Node> loadNode(std::ifstream &in, Node *parent);
    Node *findNode(Node *start, const std::string &name);
    void printTree(Node *node, int depth = 0);
    void executeCommand(const std::string &command);
    void listContents();
    void changeDirectory(const std::string &name);
    void findItem(const std::string &name);

    std::unique_ptr<Node> copyNode(Node *source, Node *newParent);
    Node *findNodeByPath(const std::string &path);
    std::vector<std::string> splitPath(const std::string &path);
    std::string getParentPath(const std::string &path);
    std::string getBaseName(const std::string &path);
    bool isValidPath(const std::vector<std::string> &pathComponents, Node *start = nullptr);

    void importFromRealPathRecursive(const std::string &currentPath, Node *currentNode);

    void saveNodeToText(std::ofstream &out, Node *node, int depth = 0);
    std::unique_ptr<Node> loadNodeFromText(std::ifstream &in, Node *parent, int &lineNumber);
    bool checkTextFileModified();
    void updateTextFileTimestamp();

public:
    FileSystem();

    bool loadFromBinaryFile(const std::string &filename);
    bool saveToBinaryFile(const std::string &filename);
    bool autoSync();

    bool loadFromFile(const std::string &filename);
    bool saveToFile(const std::string &filename);
    void navigate();
    void help();

    void createFolder(const std::string &name);
    void createFile(const std::string &name);
    void createFileWithContent(const std::string &name, const std::string &content);
    void remove(const std::string &name);
    void rename(const std::string &oldName, const std::string &newName);
    bool copy(const std::string &source, const std::string &destination);
    bool move(const std::string &source, const std::string &destination);

    bool exportToTextFile(const std::string &filename);
    bool importFromTextFile(const std::string &filename);

    bool syncFromTextIfModified();
    bool isSyncRequired();
    void markAsModified();
    
    void writeToFile(const std::string& filename, const std::string& content);
    std::string readFromFile(const std::string& filename);
    
    bool getIsModified() const { return isModified; }
    bool getIsTextFileModified() const { return isTextFileModified; }
    std::string getBinaryFilePath() const { return binaryFilePath; }
    std::string getTextFilePath() const { return textFilePath; }

    bool importFromRealPath(const std::string &path = ".");    
};

#endif
