#pragma once
#include "parser.h"

class ParameterFile {
public:
    BEGIN_PARAMETER_DECLARATION();

    ADD_PARAMETER(s, SVSize, true, true, double, 0.5);

    END_PARAMETER_DECLARATION();
};