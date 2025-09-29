#include "ExpressionParser.h"
#include <cmath>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <cfenv>

#pragma STDC FENV_ACCESS ON

ExpressionParser::ExpressionParser()
{
    // Приоритеты операторов
    operatorPriority = {
        {"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"^", 3}, {"~", 4} // унарный минус
    };

    // Унарные функции с проверками
    unaryFunctions = {
        {"SIN", [this](double x)
         {
             checkDomainConstraints("SIN", x);
             double result = sin(x);
             checkOverflow(result, "SIN");
             checkUnderflow(result, "SIN");
             return result;
         }},
        {"COS", [this](double x)
         {
             checkDomainConstraints("COS", x);
             double result = cos(x);
             checkOverflow(result, "COS");
             checkUnderflow(result, "COS");
             return result;
         }},
        {"EXP", [this](double x)
         {
             checkDomainConstraints("EXP", x);
             if (x > 700)
                 throw std::overflow_error("EXP argument too large: " + std::to_string(x));
             if (x < -700)
                 throw std::underflow_error("EXP argument too small: " + std::to_string(x));
             double result = exp(x);
             checkOverflow(result, "EXP");
             checkUnderflow(result, "EXP");
             return result;
         }}};

    // Бинарные операторы с проверками
    binaryOperators = {
        {"+", [this](double a, double b)
         {
             double result = a + b;
             checkOverflow(result, "addition");
             checkUnderflow(result, "addition");
             return result;
         }},
        {"-", [this](double a, double b)
         {
             double result = a - b;
             checkOverflow(result, "subtraction");
             checkUnderflow(result, "subtraction");
             return result;
         }},
        {"*", [this](double a, double b)
         {
             double result = a * b;
             checkOverflow(result, "multiplication");
             checkUnderflow(result, "multiplication");
             return result;
         }},
        {"/", [this](double a, double b)
         {
             checkDivisionByZero(b, "division");
             double result = a / b;
             checkOverflow(result, "division");
             checkUnderflow(result, "division");
             return result;
         }},
        {"^", [this](double a, double b)
         {
             checkNegativePower(a, b, "power");
             if (a == 0 && b < 0)
             {
                 throw std::runtime_error("Zero to negative power is undefined");
             }
             double result = pow(a, b);
             checkOverflow(result, "power");
             checkUnderflow(result, "power");
             return result;
         }}};
}

void ExpressionParser::checkDivisionByZero(double divisor, const std::string &operation)
{
    if (divisor == 0.0)
    {
        throw std::runtime_error("Division by zero in " + operation);
    }

    // Проверка на очень маленькие числа, которые могут привести к переполнению
    if (std::abs(divisor) < 1e-300)
    {
        throw std::runtime_error("Division by extremely small number in " + operation +
                                 " (would cause overflow)");
    }
}

void ExpressionParser::checkOverflow(double result, const std::string &operation)
{
    if (std::fetestexcept(FE_OVERFLOW))
    {
        std::feclearexcept(FE_OVERFLOW);
        throw std::overflow_error("Arithmetic overflow in " + operation);
    }

    if (std::isinf(result))
    {
        throw std::overflow_error("Result is infinite in " + operation);
    }

    if (result > std::numeric_limits<double>::max())
    {
        throw std::overflow_error("Result exceeds maximum value in " + operation);
    }
}

void ExpressionParser::checkUnderflow(double result, const std::string &operation)
{
    if (std::fetestexcept(FE_UNDERFLOW))
    {
        std::feclearexcept(FE_UNDERFLOW);
        throw std::underflow_error("Arithmetic underflow in " + operation);
    }

    if (result != 0 && std::abs(result) < std::numeric_limits<double>::min())
    {
        throw std::underflow_error("Result below minimum value in " + operation);
    }
}

void ExpressionParser::checkNegativePower(double base, double exponent, const std::string &operation)
{
    if (base < 0 && std::fmod(exponent, 1.0) != 0.0)
    {
        throw std::domain_error("Negative base with fractional exponent in " + operation);
    }
}

void ExpressionParser::checkDomainConstraints(const std::string &function, double value)
{
    if (function == "EXP" && value > 700)
    {
        throw std::overflow_error("EXP argument too large: would cause overflow");
    }
    if (function == "EXP" && value < -700)
    {
        throw std::underflow_error("EXP argument too small: would cause underflow");
    }
    // Для SIN и COS нет ограничений по domain, но можно добавить проверки при необходимости
}

bool ExpressionParser::isOperator(const std::string &token) const
{
    return operatorPriority.find(token) != operatorPriority.end() ||
           token == "(" || token == ")";
}

bool ExpressionParser::isFunction(const std::string &token) const
{
    return unaryFunctions.find(token) != unaryFunctions.end();
}

bool ExpressionParser::isNumber(const std::string &token) const
{
    if (token.empty())
        return false;

    // Проверка на число (целое или с плавающей точкой)
    char *end;
    std::strtod(token.c_str(), &end);
    return end != token.c_str() && *end == '\0';
}

std::vector<std::string> ExpressionParser::tokenize(const std::string &expression)
{
    std::vector<std::string> tokens;
    std::string currentToken;

    for (size_t i = 0; i < expression.length(); ++i)
    {
        char c = expression[i];

        // Пропускаем пробелы
        if (std::isspace(c))
        {
            if (!currentToken.empty())
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            continue;
        }

        // Проверка на операторы и скобки
        if (c == '(' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/' || c == '^')
        {
            if (!currentToken.empty())
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            tokens.push_back(std::string(1, c));
            continue;
        }

        // Проверка на буквы (функции)
        if (std::isalpha(c))
        {
            if (!currentToken.empty() && !std::isalpha(currentToken[0]))
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            currentToken += c;
        }
        // Числа
        else if (std::isdigit(c) || c == '.' || (c == '-' && currentToken.empty()))
        {
            if (!currentToken.empty() && std::isalpha(currentToken[0]))
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            currentToken += c;
        }
    }

    if (!currentToken.empty())
    {
        tokens.push_back(currentToken);
    }

    return tokens;
}

std::vector<std::string> ExpressionParser::toPostfix(const std::string &expression)
{
    std::vector<std::string> tokens = tokenize(expression);
    std::vector<std::string> output;
    std::stack<std::string> operatorStack;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string &token = tokens[i];

        if (isNumber(token))
        {
            // Проверка корректности числа
            try
            {
                double value = std::stod(token);
                if (std::isinf(value))
                {
                    throw std::overflow_error("Number too large: " + token);
                }
                output.push_back(token);
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Invalid number format: " + token);
            }
        }
        else if (isFunction(token))
        {
            operatorStack.push(token);
        }
        else if (token == "(")
        {
            operatorStack.push(token);
        }
        else if (token == ")")
        {
            while (!operatorStack.empty() && operatorStack.top() != "(")
            {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
            if (!operatorStack.empty() && operatorStack.top() == "(")
            {
                operatorStack.pop();
            }
            // Если после скобки была функция, добавляем её
            if (!operatorStack.empty() && isFunction(operatorStack.top()))
            {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
        }
        else if (isOperator(token))
        {
            // Обработка унарного минуса
            std::string currentOperator = token;
            if (token == "-")
            {
                if (i == 0 || tokens[i - 1] == "(" || isOperator(tokens[i - 1]))
                {
                    currentOperator = "~"; // унарный минус
                }
            }

            // Выталкиваем операторы с higher or equal priority
            while (!operatorStack.empty() &&
                   operatorStack.top() != "(" &&
                   operatorPriority[operatorStack.top()] >= operatorPriority[currentOperator])
            {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }

            operatorStack.push(currentOperator);
        }
    }

    // Выталкиваем оставшиеся операторы
    while (!operatorStack.empty())
    {
        output.push_back(operatorStack.top());
        operatorStack.pop();
    }

    return output;
}

double ExpressionParser::evaluatePostfix(const std::vector<std::string> &postfix)
{
    // Очистка флагов исключений с плавающей точкой
    std::feclearexcept(FE_ALL_EXCEPT);

    std::stack<double> valueStack;

    for (const std::string &token : postfix)
    {
        if (isNumber(token))
        {
            try
            {
                double value = std::stod(token);
                if (std::isinf(value))
                {
                    throw std::overflow_error("Number too large: " + token);
                }
                valueStack.push(value);
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Invalid number in evaluation: " + token);
            }
        }
        else if (token == "~")
        {
            // Унарный минус
            if (valueStack.empty())
            {
                throw std::runtime_error("Stack underflow for unary minus");
            }

            double value = valueStack.top();
            valueStack.pop();
            double result = -value;

            checkOverflow(result, "unary minus");
            checkUnderflow(result, "unary minus");

            valueStack.push(result);
        }
        else if (isFunction(token))
        {
            if (valueStack.empty())
            {
                throw std::runtime_error("Stack underflow for function: " + token);
            }

            double value = valueStack.top();
            valueStack.pop();
            double result = unaryFunctions[token](value);

            valueStack.push(result);
        }
        else if (binaryOperators.find(token) != binaryOperators.end())
        {
            if (valueStack.size() < 2)
            {
                throw std::runtime_error("Stack underflow for operator: " + token);
            }

            double b = valueStack.top();
            valueStack.pop();
            double a = valueStack.top();
            valueStack.pop();

            double result = binaryOperators[token](a, b);

            valueStack.push(result);
        }
    }

    if (valueStack.size() != 1)
    {
        throw std::runtime_error("Invalid expression: stack has " +
                                 std::to_string(valueStack.size()) + " elements instead of 1");
    }

    return valueStack.top();
}

double ExpressionParser::evaluate(const std::string &expression)
{
    std::vector<std::string> postfix = toPostfix(expression);
    return evaluatePostfix(postfix);
}