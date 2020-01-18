#include "parser.h"

class ParameterFile {
    BEGIN_PARAMETER_DECLARATION();

    ADD_PARAMETER(s, SVSize, true, double, 0.1);

    END_PARAMETER_DECLARATION();
};