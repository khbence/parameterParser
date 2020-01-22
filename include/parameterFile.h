#pragma once
#include "parser.h"
#include <string>

class ParameterFile {
public:
    BEGIN_PARAMETER_DECLARATION();

    ADD_PARAMETER(s, SVSize, true, true, double, 0.5);
    ADD_PARAMETER(b, binding, false, true, double, 1.0);
    ADD_PARAMETER(w, suppressWarning, true, false, bool, false);
    ADD_PARAMETER_DEFAULT_VALUE(d, defEx, std::string, "kecske");
    ADD_PARAMETER_NO_DEFAULT(n, nd, true, double);
    //ADD_PARAMETER(afile, false, true, std::string)

    END_PARAMETER_DECLARATION();
};