// # В строке текстового файла  задано  выражение  из  целых
// # чисел и операций '+', '-', '*', '/', '^', SIN, COS, EXP. Порядок
// # вычислений  определяется  приоритетом  операций   и   круглыми
// # скобками. Возможен одноместный минус в  начале  выражения  или
// # после открывающей скобки. Преобразовать выражение в постфиксную
// # форму (алгоритм Дейкстры) и вычислить его  значение.  Показать
// # этапы  выполнения (11).

// # Автор: Винокуренко Н.Г.

// # Среда выполнения: gcc version 9.4.0 (Ubuntu 9.4.0-1ubuntu1~20.04.2)

// # Источники:
// # https://habr.com/ru/articles/111361/
// # https://habr.com/ru/articles/856166/
// # https://en.cppreference.com/w/cpp/filesystem.html (C++ Reference)
// # https://www.cppstories.com/2024/common-filesystem-cpp20/ (C++ Stories)
// # https://www.studyplan.dev/pro-cpp/file-system

#include "ExpressionParser.h"
#include <cmath>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <cfenv>

#pragma STDC FENV_ACCESS ON

ExpressionParser::ExpressionParser()
{
    // Приоритеты операторов и их ассоциативность
    operatorInfo = {
        {"+", {1, false}}, // левоассоциативный
        {"-", {1, false}}, // левоассоциативный
        {"*", {2, false}}, // левоассоциативный
        {"/", {2, false}}, // левоассоциативный
        {"^", {3, true}},  // правоассоциативный
        {"~", {4, true}}   // правоассоциативный
    };

    // Унарные функции
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

    // Бинарные операторы (остаются без изменений)
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
    return operatorInfo.find(token) != operatorInfo.end() ||
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

void ExpressionParser::addToOutput(std::string *&output, int &outputSize,
                                   int &outputCapacity, const std::string &token)
{
    if (outputSize >= outputCapacity)
    {
        // Увеличиваем емкость в 2 раза
        int newCapacity = outputCapacity == 0 ? 10 : outputCapacity * 2;
        std::string *newOutput = new std::string[newCapacity];

        // Копируем существующие элементы
        for (int i = 0; i < outputSize; i++)
        {
            newOutput[i] = output[i];
        }

        // Освобождаем старую память
        delete[] output;
        output = newOutput;
        outputCapacity = newCapacity;
    }

    output[outputSize++] = token;
}

void ExpressionParser::toPostfix(const std::string &expression,
                                 std::string *&output, int &outputSize)
{
    // Инициализация
    output = nullptr;
    outputSize = 0;
    int outputCapacity = 0;

    std::vector<std::string> tokens = tokenize(expression);
    TokenStack operatorStack;

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
                addToOutput(output, outputSize, outputCapacity, token);
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
            while (!operatorStack.isEmpty() && operatorStack.top() != "(")
            {
                addToOutput(output, outputSize, outputCapacity, operatorStack.pop());
            }
            if (!operatorStack.isEmpty() && operatorStack.top() == "(")
            {
                operatorStack.pop(); // Удаляем "("
            }
            // Если после скобки была функция, добавляем её
            if (!operatorStack.isEmpty() && isFunction(operatorStack.top()))
            {
                addToOutput(output, outputSize, outputCapacity, operatorStack.pop());
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

            // Обработка ассоциативности
            while (!operatorStack.isEmpty() &&
                   operatorStack.top() != "(" &&
                   operatorInfo.find(operatorStack.top()) != operatorInfo.end())
            {

                const OperatorInfo &topOp = operatorInfo[operatorStack.top()];
                const OperatorInfo &currOp = operatorInfo[currentOperator];

                if (topOp.priority > currOp.priority ||
                    (topOp.priority == currOp.priority && !currOp.isRightAssociative))
                {
                    addToOutput(output, outputSize, outputCapacity, operatorStack.pop());
                }
                else
                {
                    break;
                }
            }

            operatorStack.push(currentOperator);
        }
    }

    // Выталкиваем оставшиеся операторы
    while (!operatorStack.isEmpty())
    {
        addToOutput(output, outputSize, outputCapacity, operatorStack.pop());
    }
}

double ExpressionParser::evaluatePostfix(const std::string *postfix, int size)
{
    std::feclearexcept(FE_ALL_EXCEPT);

    ValueStack valueStack;

    for (int i = 0; i < size; i++)
    {
        const std::string &token = postfix[i];

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
            if (valueStack.isEmpty())
            {
                throw std::runtime_error("Stack underflow for unary minus");
            }

            double value = valueStack.pop();
            double result = -value;

            checkOverflow(result, "unary minus");
            checkUnderflow(result, "unary minus");

            valueStack.push(result);
        }
        else if (isFunction(token))
        {
            if (valueStack.isEmpty())
            {
                throw std::runtime_error("Stack underflow for function: " + token);
            }

            double value = valueStack.pop();
            double result = unaryFunctions[token](value);

            valueStack.push(result);
        }
        else if (binaryOperators.find(token) != binaryOperators.end())
        {
            if (valueStack.size() < 2)
            {
                throw std::runtime_error("Stack underflow for operator: " + token);
            }

            double b = valueStack.pop();
            double a = valueStack.pop();

            double result = binaryOperators[token](a, b);

            valueStack.push(result);
        }
    }

    if (valueStack.size() != 1)
    {
        throw std::runtime_error("Invalid expression: stack has " +
                                 std::to_string(valueStack.size()) + " elements instead of 1");
    }

    return valueStack.pop();
}

double ExpressionParser::evaluate(const std::string &expression)
{
    std::string *postfix = nullptr;
    int postfixSize = 0;

    toPostfix(expression, postfix, postfixSize);
    double result = evaluatePostfix(postfix, postfixSize);

    delete[] postfix;
    return result;
}