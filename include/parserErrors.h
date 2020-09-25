#pragma once
#include <exception>
#include <string>

namespace pparser {
    class parserError : public std::exception {
        std::string error;
    public:
        explicit parserError(const std::string& error_p) : error(error_p) {}

        const char *what() const noexcept override {
           return error.c_str();
        }
    };

    struct tooMuchArguments : public parserError {
        explicit tooMuchArguments(const std::string& value) : parserError(value + " cannot bind to any parameter, because the previous object is a value!\n") {}
    };

    struct badFormatLongArgument : public parserError {
        explicit badFormatLongArgument(const std::string& value) : parserError("Long parameters need two hyphens (" + value + " is wrong)\n") {}
    };

    struct sameLongAndShortParameters : public parserError {
        sameLongAndShortParameters(char s, std::string& l) : parserError("-" + std::string(1, s) + " and " + "--" + l + " describes the same parameters don't use them both!\n") {}
    };

    struct missingArgument : public parserError {
        explicit missingArgument(std::string& l) : parserError("--" + l + " requires an argument!\n") {}
        explicit missingArgument(char s) : parserError("-" + std::string(1, s) + " requires an argument!\n") {}
    };

    struct unnecessaryArgument : public parserError {
        explicit unnecessaryArgument(std::string& l) : parserError("--" + l + " does not requires any argument!\n") {}
        explicit unnecessaryArgument(char s) : parserError("-" + std::string(1, s) + " does not requires any argument!\n") {}
    };

    struct missingParameter : public parserError {
        missingParameter(char s, std::string& l) : parserError("-" + std::string(1, s) + " or " + "--" + l + " need to be defined!\n") {};
        explicit missingParameter(std::string& l) : parserError("--" + l + " need to be defined!\n") {};
    };
}