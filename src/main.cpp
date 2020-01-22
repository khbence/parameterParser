#include <iostream>
#include "parameterFile.h"
#include "parser.h"
#include "parserErrors.h"

int main(int argc, char const *argv[]) {
    try {
        auto params = ::pparser::parser<ParameterFile>::createParameterFile(argc, argv);
        std::cout << params.SVSize.value() << " - " << params.binding << " - " << params.suppressWarning.value() 
        << " - " << params.defEx << std::endl;
    } catch(pparser::parserError& e) {
        std::cerr << e.what();
    }
    return 0;
}
