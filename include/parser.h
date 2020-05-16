#pragma once
#include "CXX17/optional.h"
#include <type_traits>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include "parserErrors.h"
#include <iostream>

namespace pparser {
    namespace internal {
        /*
         RecursiveTypelist
         */
        struct Void {};
        
        template <typename H, typename T>
        struct RecursiveTypelist {};
        
        /*
         Typelist
         */
        template <typename ...T>
        struct Typelist {};
        
        template <typename... T>
        struct Typelist_Merge {};
        
        template <typename... T, typename... U>
        struct Typelist_Merge<Typelist<T...>, Typelist<U...>> {
            typedef Typelist<T..., U...> value;
        };
        
        /*
         Flatten RecursiveTypelist
         */
        template <typename... T>
        struct StripRecursiveList__ {};
        
        template <typename H, typename... T, typename... U>
        struct StripRecursiveList__<Typelist<RecursiveTypelist<H, RecursiveTypelist<T...>>, U...>> {
            typedef typename StripRecursiveList__<Typelist<RecursiveTypelist<T...>, H, U...>>::value value;
        };
        
        template <typename H, typename... U>
        struct StripRecursiveList__<Typelist<RecursiveTypelist<H, Void>, U...>> {
            typedef Typelist<H, U...> value;
        };
        
        template <typename... T>
        struct FlattenRecursiveTypelist {};
        
        template <typename H, typename... T>
        struct FlattenRecursiveTypelist<RecursiveTypelist<H, RecursiveTypelist<T...>>> {
            typedef typename StripRecursiveList__<Typelist<RecursiveTypelist<T...>, H>>::value value;
        };
        
        template <typename H>
        struct FlattenRecursiveTypelist<RecursiveTypelist<H, ::pparser::internal::Void>> {
            typedef ::pparser::internal::Typelist<H> value;
        };
        
        /*
         Incremental typelist construction
         
         This solution is based on the genius solution to this problem as described here: https://stackoverflow.com/questions/24088373/building-a-compile-time-list-incrementally-in-c/24092292#24092292
         */
        template <int N> struct Counter : Counter<N - 1> {};
        template <> struct Counter<0> {};
        
        #define _START_LIST_PPARSER() \
        static ::pparser::internal::Void __list_maker_helper(::pparser::internal::Counter<__COUNTER__>) \
        
        #define _ADD_TO_LIST_PPARSER(type) \
        static ::pparser::internal::RecursiveTypelist<type, decltype(__list_maker_helper(::pparser::internal::Counter<__COUNTER__>{}))> \
        __list_maker_helper(::pparser::internal::Counter<__COUNTER__>)
        
        #define _END_LIST_PPARSER() \
        private: \
        ADD_PARAMETER_FLAG_H(h, help, "Prints this help"); \
        bool __errorHappened = false; \
        template<typename T> \
        friend class ::pparser::parser; \
        public: \
        bool isHelpWasCalled() const { return help; } \
        bool isErrorHappened() const { return __errorHappened; } \
        typedef \
        ::pparser::internal::FlattenRecursiveTypelist<decltype(__list_maker_helper(::pparser::internal::Counter<__COUNTER__>{}))>::value \
        __member_typelist;
    }

    namespace impl {
        template<typename T>
        struct removeOptionalOptional {
            typedef T value;
        };

        template<typename T>
        struct removeOptionalOptional<pparser::CXX17::optional<T>> {
            typedef T value;
        };

        template<typename T, int ID>
        struct parameterObject {
            typedef typename removeOptionalOptional<T>::value paramType;

            static pparser::CXX17::optional<char> shortName;
            static std::string longName;
            static T* memberPointer;
            static bool required; //calculate it from the types
            static bool hasArgument;
            static pparser::CXX17::optional<std::string> help;

            parameterObject(char shortName_p
                            , const std::string& longName_p
                            , T* memberPointer_p
                            , bool isRequired_p
                            , bool hasArgument_p
                            , const std::string& help_p) {
                if(shortName_p == '-') {
                    shortName = {};
                } else {
                    shortName = shortName_p;
                }
                longName = longName_p;
                memberPointer = memberPointer_p;
                required = isRequired_p;
                hasArgument = hasArgument_p;
                help = help_p;
            }
        };

        template<typename T, int ID> pparser::CXX17::optional<char> parameterObject<T, ID>::shortName;
        template<typename T, int ID> std::string parameterObject<T, ID>::longName;
        template<typename T, int ID> T* parameterObject<T, ID>::memberPointer;
        template<typename T, int ID> bool parameterObject<T, ID>::required;
        template<typename T, int ID> bool parameterObject<T, ID>::hasArgument;
        template<typename T, int ID> pparser::CXX17::optional<std::string> parameterObject<T, ID>::help;


        template <typename TL>
        struct __decode_member_list;
        
        template <typename R>
        struct __object_value_decoder {
            static R get(std::map<std::string, pparser::CXX17::optional<std::stringstream>>& longNames
                        , std::map<char, pparser::CXX17::optional<std::stringstream>>& shortNames) {
                R ret;
                __decode_member_list<typename R::__member_typelist>::decode(longNames, shortNames);
                return ret;
            }
        };

        template<typename B, typename T>
        struct setTrue {
            void operator()() {}
        };

        template<typename T>
        struct setTrue<bool, T> {
            void operator()() { (*T::memberPointer) = true; }
        };

        template <typename T>
        class __decode_member;

        template <typename T>
        class __decode_member {
            static constexpr unsigned paddingForHelp = 30;
            static constexpr unsigned lengthOfWindow = 80;

            static void printHelp() {
                std::cout << "  ";
                size_t length = 2;
                if(T::shortName) {
                    std::cout << "-" << T::shortName.value() << ", ";
                    length += 4;
                }
                std::cout << "--" << T::longName;
                length += 2 + T::longName.size();
                if(T::help) {
                    int indent = static_cast<int>(paddingForHelp) - static_cast<int>(length);
                    if(indent < 2) {
                        std::cout << std::string(4, ' ');
                        length += 4;
                    } else {
                        std::cout << std::string(static_cast<size_t>(indent), ' ');
                        length = paddingForHelp;
                    }
                    std::stringstream ss(T::help.value());
                    while(ss) {
                        std::string tmp;
                        ss >> tmp;
                        length += tmp.size();
                        if(length > lengthOfWindow) {
                            std::cout << '\n' << std::string(30, ' ');
                            length = 30 + tmp.size();
                        }
                        std::cout << tmp << " ";
                    }
                }
                std::cout << std::endl;
            }

        public:
            static void decode(std::map<std::string, pparser::CXX17::optional<std::stringstream>>& longNames
                                    , std::map<char, pparser::CXX17::optional<std::stringstream>>& shortNames) {
                if(longNames.count("help") || shortNames.count('h')) {
                    printHelp();
                    if((T::longName != "help")) { return; }
                }
                auto itl = longNames.find(T::longName);
                bool found = false;
                if(itl != longNames.end()) {
                    found = true;
                    if(T::hasArgument) {
                        if(!itl->second) { throw missingArgument(T::longName); }
                        typename T::paramType tmp;
                        itl->second.value() >> tmp;
                        (*T::memberPointer) = tmp;
                    } else {
                        if(itl->second) { throw unnecessaryArgument(T::longName); }
                        setTrue<typename T::paramType, T> t; t();
                    }
                    longNames.erase(itl);
                }
                if(T::shortName) {
                    auto its = shortNames.find(T::shortName.value());
                    if(its != shortNames.end()) {
                        if(found) { throw sameLongAndShortParameters(T::shortName.value(), T::longName); }
                        found = true;
                        if(T::hasArgument) {
                            if(!its->second) { throw missingArgument(T::shortName.value()); }
                            typename T::paramType tmp;
                            its->second.value() >> tmp;
                            (*T::memberPointer) = tmp;
                        } else {
                            if(its->second) { throw unnecessaryArgument(T::shortName.value()); }
                            setTrue<typename T::paramType, T> t; t();
                        }
                        shortNames.erase(its);
                    }
                    if(!found && T::required) { throw missingParameter(T::shortName.value(), T::longName); }
                } else if (!found && T::required) { throw missingParameter(T::longName); }
            }
        };

        template <typename H, typename... T>
        struct __decode_member_list<::pparser::internal::Typelist<H, T...>> {
            static void decode(std::map<std::string, pparser::CXX17::optional<std::stringstream>>& longNames
                                    , std::map<char, pparser::CXX17::optional<std::stringstream>>& shortNames) {
                __decode_member<H>::decode(longNames, shortNames);
                __decode_member_list<::pparser::internal::Typelist<T...>>::decode(longNames, shortNames);
            }
        };
        
        template <typename H>
        struct __decode_member_list<::pparser::internal::Typelist<H>> {
            static void decode(std::map<std::string, pparser::CXX17::optional<std::stringstream>>& longNames
                                    , std::map<char, pparser::CXX17::optional<std::stringstream>>& shortNames) {
                __decode_member<H>::decode(longNames, shortNames);
            }
        };
    }

    template<typename T>
    std::string createHelp(std::string help, T defaultValue) {
        std::stringstream ss;
        ss << help << " [default: " << defaultValue << "]";
        std::string ret = ss.str();
        return ret;
    }

    #define BEGIN_PARAMETER_DECLARATION() \
    _START_LIST_PPARSER()

    #define ADD_PARAMETER_RAW(shortName, longName, isOptional, hasArgument, required, parType, defaultValue, help) \
    std::conditional<isOptional, pparser::CXX17::optional<parType>, parType>::type longName = defaultValue; \
    typedef typename ::pparser::impl::parameterObject<decltype(longName), __COUNTER__> __##longName##_type; \
    __##longName##_type __##longName##_instance = __##longName##_type((#shortName)[0], #longName, &longName, required, hasArgument, help); \
    _ADD_TO_LIST_PPARSER(__##longName##_type)

    #define ADD_PARAMETER_H(shortName, longName, isOptional, hasArgument, parType, defaultValue, help) \
    ADD_PARAMETER_RAW(shortName, longName, isOptional, hasArgument, !isOptional, parType, defaultValue, ::pparser::createHelp(help, defaultValue))
    #define ADD_PARAMETER(shortName, longName, isOptional, hasArgument, parType, defaultValue) \
    ADD_PARAMETER_H(shortName, longName, isOptional, hasArgument, parType, defaultValue, "")

    #define ADD_PARAMETER_DEFAULT_VALUE_H(shortName, longName, parType, defaultValue, help) \
    ADD_PARAMETER_RAW(shortName, longName, false, true, false, parType, defaultValue, ::pparser::createHelp(help, defaultValue))
    #define ADD_PARAMETER_DEFAULT_VALUE(shortName, longName, parType, defaultValue) \
    ADD_PARAMETER_DEFAULT_VALUE_H(shortName, longName, parType, defaultValue, "")

    #define ADD_PARAMETER_NO_DEFAULT_H(shortName, longName, isOptional, parType, help) \
    ADD_PARAMETER_RAW(shortName, longName, isOptional, true, !isOptional, parType, {}, help)
    #define ADD_PARAMETER_NO_DEFAULT(shortName, longName, isOptional, parType) \
    ADD_PARAMETER_NO_DEFAULT_H(shortName, longName, isOptional, parType, "")

    #define ADD_PARAMETER_FLAG_H(shortName, longName, help) \
    ADD_PARAMETER_RAW(shortName, longName, false, false, false, bool, false, help)
    #define ADD_PARAMETER_FLAG(shortName, longName) \
    ADD_PARAMETER_FLAG_H(shortName, longName, "")

    #define END_PARAMETER_DECLARATION() \
    _END_LIST_PPARSER()

    template<typename parameterFileType>
    class parser {
        static std::vector<std::string> formatArgv(int argc, char const** argv) {
            std::vector<std::string> ret;
            ret.reserve(static_cast<size_t>(argc));
            for(int i = 1; i < argc; ++i) {
                ret.emplace_back(argv[i]);
            }
            return ret;
        }

        static auto parseTheArgsToMaps(int argc, char const** argv) {
            const std::vector<std::string> arguments = formatArgv(argc, argv);
            std::map<std::string, pparser::CXX17::optional<std::stringstream>> longNames;
            std::map<char, pparser::CXX17::optional<std::stringstream>> shortNames;
            auto it = arguments.begin();
            while(it != arguments.end()) {
                //TODO handle ugly cases
                std::string current = *it;
                if(current[0] != '-') { throw tooMuchArguments(current); }
                current.erase(0, 1);
                if(current.size() > 1) {
                    if(current[0] != '-') { throw badFormatLongArgument('-' + current); }
                    current.erase(0, 1);
                    ++it;
                    if((it != arguments.end()) && (*it)[0] != '-') {
                        longNames.emplace(current, pparser::CXX17::optional<std::stringstream>(std::stringstream(*it)));
                        ++it;
                    } else {
                        longNames.emplace(current, pparser::CXX17::optional<std::stringstream>());
                    }
                } else {
                    ++it;
                    if((it != arguments.end()) && (*it)[0] != '-') {
                        shortNames.emplace(current[0], pparser::CXX17::optional<std::stringstream>(std::stringstream(*it)));
                        ++it;
                    } else {
                        shortNames.emplace(current[0], pparser::CXX17::optional<std::stringstream>());
                    }
                }
            }
            return std::make_pair(std::move(longNames), std::move(shortNames));
        }

    public:
        static parameterFileType createParameterFile(int argc, char const **argv) {
            parameterFileType ret;
            try {
                auto tmp = parseTheArgsToMaps(argc, argv);
                auto longNames = tmp.first;
                auto shortNames = tmp.second;
                ret = ::pparser::impl::__object_value_decoder<parameterFileType>::get(longNames, shortNames);
                for(const auto& e : longNames) {
                    std::cout << "[WARNING] Unused parameter: " << e.first;
                }
                for(const auto& e : shortNames) {
                    std::cout << "[WARNING] Unused parameter: " << e.first;
                }
            } catch(parserError& e) {
                std::cerr << e.what();
                ret.__errorHappened = true;
            }
            return ret;
        }
    };
    #define PARSE_PARAMETERS(argc, argv, parameterClass) \
    ::pparser::parser<parameterClass>::createParameterFile(argc, argv)
}
