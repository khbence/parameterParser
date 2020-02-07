#include <iostream>
#include "parameterFile.h"
#include "parser.h"
#include "parserErrors.h"

int main(int argc, char const *argv[]) {
    auto params = ::pparser::parser<ParameterFile>::createParameterFile(argc, argv);
    if(!params.help) {
        std::cout << params.SVSize.value() << " - " << params.binding << " - " << params.suppressWarning 
                    << " - " << params.defEx << " - " << params.nd.value() << std::endl;
    }
    return 0;
}
