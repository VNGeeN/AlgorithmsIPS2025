#include <iostream>
#include <vector>
#include "ExpressionParser.h"

void demonstrateBasicUsage() {
    std::cout << "=== Basic Expression Parser Demo ===" << std::endl;
    
    ExpressionParser parser;
    
    // Примеры выражений
    std::vector<std::string> expressions = {
        "3 + 4 * 2",
        "(1 + 2) * 3", 
        "SIN(0) + COS(0)",
        "2 ^ 3 + 1",
        "-5 + 3",
        "EXP(0)"
    };
    
    for (const auto& expr : expressions) {
        try {
            auto postfix = parser.toPostfix(expr);
            double result = parser.evaluate(expr);
            
            std::cout << "Expression: " << expr << std::endl;
            std::cout << "Postfix: ";
            for (const auto& token : postfix) {
                std::cout << token << " ";
            }
            std::cout << std::endl;
            std::cout << "Result: " << result << std::endl;
            std::cout << "---" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main() {
    demonstrateBasicUsage();
    return 0;
}