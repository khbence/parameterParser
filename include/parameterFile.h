#pragma once
#include "parser.h"
#include <string>

class ParameterFile {
public:
    BEGIN_PARAMETER_DECLARATION();

    ADD_PARAMETER_H(s, SVSize, true, true, double, 0.5, "size of the SVs");
    ADD_PARAMETER(b, binding, false, true, double, 1.0);
    ADD_PARAMETER_DEFAULT_VALUE(d, defEx, std::string, "kecske");
    ADD_PARAMETER_NO_DEFAULT(n, nd, true, double);
    ADD_PARAMETER_FLAG(w, suppressWarning);

    END_PARAMETER_DECLARATION();
};