#pragma once
#include <exception>
#include <string>
#include <format>

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
        explicit tooMuchArguments(const std::string& value) : parserError(std::format("{} cannot bind to any parameter, because the previous object is a value!\n", value)) {}
    };

    struct badFormatLongArgument : public parserError {
        explicit badFormatLongArgument(const std::string& value) : parserError(std::format("Long parameters need two hyphens ({} is wrong)\n", value)) {}
    };

    struct sameLongAndShortParameters : public parserError {
        sameLongAndShortParameters(char s, std::string& l) : parserError(std::format("-{} and --{} describes the same parameters don't use them both!\n", s, l)) {}
    };

    struct missingArgument : public parserError {
        explicit missingArgument(std::string& l) : parserError(std::format("--{} requires an argument!\n", l)) {}
        explicit missingArgument(char s) : parserError(std::format("-{} requires an argument!\n", s)) {}
    };

    struct unnecessaryArgument : public parserError {
        explicit unnecessaryArgument(std::string& l) : parserError(std::format("--{} not requires an argument!\n", l)) {}
        explicit unnecessaryArgument(char s) : parserError(std::format("-{} not requires an argument!\n", s)) {}
    };

    struct missingParameter : public parserError {
        missingParameter(char s, std::string& l) : parserError(std::format("-{} or --{} need to be defined!\n", s, l)) {};
        explicit missingParameter(std::string& l) : parserError(std::format("--{} need to be defined!\n", l)) {};
    };
}
