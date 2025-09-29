#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include <string>
#include <vector>
#include <stack>
#include <map>
#include <functional>
#include <limits>

class ExpressionParser {
private:
    std::map<std::string, int> operatorPriority;
    std::map<std::string, std::function<double(double)>> unaryFunctions;
    std::map<std::string, std::function<double(double, double)>> binaryOperators;
    
    bool isOperator(const std::string& token) const;
    bool isFunction(const std::string& token) const;
    bool isNumber(const std::string& token) const;
    std::vector<std::string> tokenize(const std::string& expression);

    // Вспомогательные функции для проверок
    void checkDivisionByZero(double divisor, const std::string& operation);
    void checkOverflow(double result, const std::string& operation);
    void checkUnderflow(double result, const std::string& operation);
    void checkNegativePower(double base, double exponent, const std::string& operation);
    void checkDomainConstraints(const std::string& function, double value);
    
public:
    ExpressionParser();
    std::vector<std::string> toPostfix(const std::string& expression);
    double evaluatePostfix(const std::vector<std::string>& postfix);
    double evaluate(const std::string& expression);
};

#endif