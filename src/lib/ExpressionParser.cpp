#include "ExpressionParser.h"
#include <cmath>
#include <sstream>
#include <cctype>
#include <stdexcept>

ExpressionParser::ExpressionParser() {
    // Приоритеты операторов
    operatorPriority = {
        {"+", 1}, {"-", 1},
        {"*", 2}, {"/", 2},
        {"^", 3},
        {"~", 4}  // унарный минус
    };
    
    // Унарные функции
    unaryFunctions = {
        {"SIN", [](double x) { return sin(x); }},
        {"COS", [](double x) { return cos(x); }},
        {"EXP", [](double x) { return exp(x); }}
    };
    
    // Бинарные операторы
    binaryOperators = {
        {"+", [](double a, double b) { return a + b; }},
        {"-", [](double a, double b) { return a - b; }},
        {"*", [](double a, double b) { return a * b; }},
        {"/", [](double a, double b) { 
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b; 
        }},
        {"^", [](double a, double b) { return pow(a, b); }}
    };
}

bool ExpressionParser::isOperator(const std::string& token) const {
    return operatorPriority.find(token) != operatorPriority.end() || 
           token == "(" || token == ")";
}

bool ExpressionParser::isFunction(const std::string& token) const {
    return unaryFunctions.find(token) != unaryFunctions.end();
}

bool ExpressionParser::isNumber(const std::string& token) const {
    if (token.empty()) return false;
    
    // Проверка на целое число
    for (char c : token) {
        if (!std::isdigit(c) && c != '-' && c != '.') {
            return false;
        }
    }
    return true;
}

std::vector<std::string> ExpressionParser::tokenize(const std::string& expression) {
    std::vector<std::string> tokens;
    std::string currentToken;
    
    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];
        
        // Пропускаем пробелы
        if (std::isspace(c)) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            continue;
        }
        
        // Проверка на операторы и скобки
        if (c == '(' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            tokens.push_back(std::string(1, c));
            continue;
        }
        
        // Проверка на буквы (функции)
        if (std::isalpha(c)) {
            if (!currentToken.empty() && !std::isalpha(currentToken[0])) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            currentToken += c;
        } 
        // Числа
        else if (std::isdigit(c) || c == '.' || (c == '-' && currentToken.empty())) {
            if (!currentToken.empty() && std::isalpha(currentToken[0])) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            currentToken += c;
        }
    }
    
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }
    
    return tokens;
}

std::vector<std::string> ExpressionParser::toPostfix(const std::string& expression) {
    std::vector<std::string> tokens = tokenize(expression);
    std::vector<std::string> output;
    std::stack<std::string> operatorStack;
    
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];
        
        if (isNumber(token)) {
            output.push_back(token);
        }
        else if (isFunction(token)) {
            operatorStack.push(token);
        }
        else if (token == "(") {
            operatorStack.push(token);
        }
        else if (token == ")") {
            while (!operatorStack.empty() && operatorStack.top() != "(") {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
            if (!operatorStack.empty() && operatorStack.top() == "(") {
                operatorStack.pop();
            }
            // Если после скобки была функция, добавляем её
            if (!operatorStack.empty() && isFunction(operatorStack.top())) {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
        }
        else if (isOperator(token)) {
            // Обработка унарного минуса
            std::string currentOperator = token;
            if (token == "-") {
                if (i == 0 || tokens[i-1] == "(" || isOperator(tokens[i-1])) {
                    currentOperator = "~"; // унарный минус
                }
            }
            
            // Выталкиваем операторы с higher or equal priority
            while (!operatorStack.empty() && 
                   operatorStack.top() != "(" &&
                   operatorPriority[operatorStack.top()] >= operatorPriority[currentOperator]) {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
            
            operatorStack.push(currentOperator);
        }
    }
    
    // Выталкиваем оставшиеся операторы
    while (!operatorStack.empty()) {
        output.push_back(operatorStack.top());
        operatorStack.pop();
    }
    
    return output;
}

double ExpressionParser::evaluatePostfix(const std::vector<std::string>& postfix) {
    std::stack<double> valueStack;
    
    for (const std::string& token : postfix) {
        if (isNumber(token)) {
            valueStack.push(std::stod(token));
        }
        else if (token == "~") {
            // Унарный минус
            double value = valueStack.top();
            valueStack.pop();
            valueStack.push(-value);
        }
        else if (isFunction(token)) {
            double value = valueStack.top();
            valueStack.pop();
            valueStack.push(unaryFunctions[token](value));
        }
        else if (binaryOperators.find(token) != binaryOperators.end()) {
            double b = valueStack.top();
            valueStack.pop();
            double a = valueStack.top();
            valueStack.pop();
            valueStack.push(binaryOperators[token](a, b));
        }
    }
    
    if (valueStack.size() != 1) {
        throw std::runtime_error("Invalid expression");
    }
    
    return valueStack.top();
}

double ExpressionParser::evaluate(const std::string& expression) {
    std::vector<std::string> postfix = toPostfix(expression);
    return evaluatePostfix(postfix);
}