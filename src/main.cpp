#include <iostream>
#include <string>
#include <type_traits>
#include "parser.h"

struct ParameterFile {
    BEGIN_PARAMETER_DECLARATION();

    ADD_PARAMETER_H(s, SVSize, true, true, double, 0.5, "size of the SVs");
    ADD_PARAMETER(b, binding, false, true, double, 1.0);
    ADD_PARAMETER_DEFAULT_VALUE(d, defEx, std::string, "kecske");
    ADD_PARAMETER_NO_DEFAULT(n, nd, true, double);
    ADD_PARAMETER_FLAG(w, suppressWarning);

    END_PARAMETER_DECLARATION();
};

int main(int argc, char const *argv[]) {
    auto params = PARSE_PARAMETERS(argc, argv, ParameterFile);
    if(!params.isHelpWasCalled() && !params.isErrorHappened()) {
        if(params.SVSize) { std::cout << params.SVSize.value(); }
        std::cout << " - " << params.binding;
        std::cout << " - " << params.suppressWarning;
        std::cout << " - " << params.defEx;
        if(params.nd) { std::cout << " - " << params.nd.value() << std::endl; }
    }
    std::cout << params.isErrorHappened() << std::endl;
    return 0;
}