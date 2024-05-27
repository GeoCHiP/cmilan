#include "scanner.h"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Provide input file" << std::endl;
		return 1;
	}

	std::ifstream inputFile(argv[1]);

	// Create scanner object
	Scanner scanner(argv[1], inputFile);

	// Print all tokens with attributes
	Token token;
	while (true)
	{
		scanner.nextToken();
		token = scanner.token();
		if (token == T_EOF)
			break;

		std::cout << tokenToString(token);

		switch (token)
		{
		case T_ADDOP:
		case T_MULOP:
			std::cout << "  attr: " << scanner.getArithmeticValue();
			break;
		case T_CMP:
			std::cout << "  attr: " << scanner.getCmpValue();
			break;
		case T_IDENTIFIER:
		case T_NUMBER:
			std::cout << "  attr: " << scanner.getStringValue();
			break;
		default:
			break;
		}

		std::cout << std::endl;
	}

	return 0;
}
