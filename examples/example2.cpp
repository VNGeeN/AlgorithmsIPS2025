#include <iostream>
#include "ExpressionParser.h"

void demonstrateInteractive()
{
    std::cout << "=== Interactive Expression Evaluator ===" << std::endl;
    std::cout << "Enter expressions to evaluate (type 'quit' to exit)" << std::endl;

    ExpressionParser parser;
    std::string input;

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "quit" || input == "exit")
        {
            break;
        }

        if (input.empty())
        {
            continue;
        }

        try
        {
            double result = parser.evaluate(input);
            auto postfix = parser.toPostfix(input);

            std::cout << "Postfix: ";
            for (const auto &token : postfix)
            {
                std::cout << token << " ";
            }
            std::cout << std::endl;
            std::cout << "Result: " << result << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main()
{
    demonstrateInteractive();
    return 0;
}