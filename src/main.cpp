#include <iostream>
#include "parameterFile.h"
#include "parser.h"

int main(int argc, char const *argv[]) {
    auto params = ::pparser::parser<ParameterFile>::createParameterFile(argc, argv);
    std::cout << params.SVSize.value() << std::endl;
    return 0;
}
