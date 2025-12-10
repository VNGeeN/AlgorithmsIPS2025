#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include <string>
#include <vector>
#include "CustomStack.h"
#include <map>
#include <functional>
#include <limits>

class ExpressionParser
{
private:
    struct OperatorInfo
    {
        int priority;
        bool isRightAssociative;
    };

    std::map<std::string, OperatorInfo> operatorInfo;
    std::map<std::string, std::function<double(double)>> unaryFunctions;
    std::map<std::string, std::function<double(double, double)>> binaryOperators;

    using TokenStack = CustomStack<std::string>;
    using ValueStack = CustomStack<double>;

    bool isOperator(const std::string &token) const;
    bool isFunction(const std::string &token) const;
    bool isNumber(const std::string &token) const;
    std::vector<std::string> tokenize(const std::string &expression);

    void addToOutput(std::string *&output, int &outputSize,
                     int &outputCapacity, const std::string &token);

    void checkDivisionByZero(double divisor, const std::string &operation);
    void checkOverflow(double result, const std::string &operation);
    void checkUnderflow(double result, const std::string &operation);
    void checkNegativePower(double base, double exponent, const std::string &operation);
    void checkDomainConstraints(const std::string &function, double value);

public:
    ExpressionParser();
    ~ExpressionParser();

     void toPostfix(const std::string& expression, 
                   std::string*& output, int& outputSize);
    double evaluatePostfix(const std::string* postfix, int size);
    double evaluate(const std::string &expression);
};

#endif