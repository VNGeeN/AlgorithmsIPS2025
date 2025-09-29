#include <iostream>
#include <vector>
#include "ExpressionParser.h"

void testErrorCases() {
    std::cout << "=== Arithmetic Error Tests ===" << std::endl;
    
    ExpressionParser parser;
    
    // Тестовые случаи с ожидаемыми ошибками
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"1 / 0", "Division by zero"},
        {"1 / 0.0000000000000000000000001", "Division by extremely small number"},
        {"1e300 * 1e300", "Arithmetic overflow"},
        {"1e-300 / 1e300", "Arithmetic underflow"},
        {"(-2) ^ 0.5", "Negative base with fractional exponent"},
        {"0 ^ -2", "Zero to negative power"},
        {"EXP(1000)", "EXP argument too large"},
        {"EXP(-1000)", "EXP argument too small"},
        {"1e999", "Number too large"}
    };
    
    for (const auto& [expression, expectedError] : testCases) {
        std::cout << "Testing: " << expression << std::endl;
        try {
            double result = parser.evaluate(expression);
            std::cout << "  UNEXPECTED SUCCESS: Got result " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "  Expected error: " << e.what() << std::endl;
            if (std::string(e.what()).find(expectedError) != std::string::npos) {
                std::cout << "  ✓ Correctly caught: " << expectedError << std::endl;
            } else {
                std::cout << "  ✗ Wrong error type" << std::endl;
            }
        }
        std::cout << "---" << std::endl;
    }
}

void testEdgeCases() {
    std::cout << "=== Edge Case Tests ===" << std::endl;
    
    ExpressionParser parser;
    
    // Граничные случаи, которые должны работать
    std::vector<std::string> validCases = {
        "1 / 1e-300",  // Делитель очень мал, но не нуль
        "1e300 * 1",   // Умножение на большое число
        "EXP(700)",    // Граничное значение для EXP
        "EXP(-700)",   // Граничное значение для EXP
        "(-2) ^ 2",    // Отрицательное основание с целой степенью
        "0 ^ 2"        // Ноль в положительной степени
    };
    
    for (const auto& expression : validCases) {
        std::cout << "Testing: " << expression << std::endl;
        try {
            double result = parser.evaluate(expression);
            std::cout << "  ✓ Success: " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "  ✗ Unexpected error: " << e.what() << std::endl;
        }
        std::cout << "---" << std::endl;
    }
}

int main() {
    testErrorCases();
    testEdgeCases();
    return 0;
}