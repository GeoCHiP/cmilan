#include <cstdlib>
#include <fstream>
#include <iostream>

#include "parser.h"

void PrintHelp() {
    std::cout << "Usage: cmilan input_file" << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        PrintHelp();
        return EXIT_FAILURE;
    }

    std::ifstream input;
    input.open(argv[1]);

    if (input) {
        Parser p(argv[1], input);
        p.Parse();
        return EXIT_SUCCESS;
    } else {
        std::cerr << "File '" << argv[1] << "' not found" << std::endl;
        return EXIT_FAILURE;
    }
}
