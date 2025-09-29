#include <iostream>
#include <fstream>
#include "ExpressionParser.h"

void processFile(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }
    
    ExpressionParser parser;
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(inputFile, line)) {
        lineNumber++;
        if (line.empty()) continue;
        
        std::cout << "Line " << lineNumber << ": " << line << std::endl;
        
        try {
            std::vector<std::string> postfix = parser.toPostfix(line);
            double result = parser.evaluate(line);
            
            std::cout << "  Postfix: ";
            for (const auto& token : postfix) {
                std::cout << token << " ";
            }
            std::cout << std::endl;
            std::cout << "  Result: " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "  Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    
    inputFile.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        std::cerr << "Or run examples: make example1 or make example2" << std::endl;
        return 1;
    }
    
    processFile(argv[1]);
    return 0;
}