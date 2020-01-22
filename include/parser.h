#pragma once
#include <optional>
#include <type_traits>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include "parserErrors.h"

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
        
        #define _START_LIST() \
        static ::pparser::internal::Void __list_maker_helper(::pparser::internal::Counter<__COUNTER__>)
        
        #define _ADD_TO_LIST(type) \
        static ::pparser::internal::RecursiveTypelist<type, decltype(__list_maker_helper(::pparser::internal::Counter<__COUNTER__>{}))> \
        __list_maker_helper(::pparser::internal::Counter<__COUNTER__>)
        
        #define _END_LIST() \
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
        struct removeOptionalOptional<std::optional<T>> {
            typedef T value;
        };

        template<typename T, int ID>
        struct parameterObject {
            typedef typename removeOptionalOptional<T>::value paramType;

            static std::optional<char> shortName;
            static std::string longName;
            static T* memberPointer;
            static bool isOptional; //calculate it from the types
            static bool hasArgument;

            parameterObject(char shortName_p, const std::string& longName_p, T* memberPointer_p, bool isOptional_p, bool hasArgument_p) {
                shortName = shortName_p;
                longName = longName_p;
                memberPointer = memberPointer_p;
                isOptional = isOptional_p;
                hasArgument = hasArgument_p;
            }

            parameterObject(const std::string& longName_p, T* memberPointer_p, bool isOptional_p, bool hasArgument_p) {
                shortName = {};
                longName = longName_p;
                memberPointer = memberPointer_p;
                isOptional = isOptional_p;
                hasArgument = hasArgument_p;
            }
        };

        template<typename T, int ID> std::optional<char> parameterObject<T, ID>::shortName;
        template<typename T, int ID> std::string parameterObject<T, ID>::longName;
        template<typename T, int ID> T* parameterObject<T, ID>::memberPointer;
        template<typename T, int ID> bool parameterObject<T, ID>::isOptional;
        template<typename T, int ID> bool parameterObject<T, ID>::hasArgument;


        template <typename TL>
        struct __decode_member_list;
        
        template <typename R>
        struct __object_value_decoder {
            static R get(std::map<std::string, std::optional<std::stringstream>>& longNames
                        , std::map<char, std::optional<std::stringstream>>& shortNames) {
                R ret;
                __decode_member_list<typename R::__member_typelist>::decode(longNames, shortNames);
                return ret;
            }
        };

        template <typename T>
        struct __decode_member;

        template <typename T>
        struct __decode_member {
            static void decode(std::map<std::string, std::optional<std::stringstream>>& longNames
                                    , std::map<char, std::optional<std::stringstream>>& shortNames) {
                //TODO handle bad optional and argument and no default
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
                        (*T::memberPointer) = true;
                    }
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
                            (*T::memberPointer) = true;
                        }
                    }
                    if(!found && !T::isOptional) { throw missingParameter(T::shortName.value(), T::longName); }
                } else if (!found && !T::isOptional) { throw missingParameter(T::longName); }
            }
        };

        template <typename H, typename... T>
        struct __decode_member_list<::pparser::internal::Typelist<H, T...>> {
            static void decode(std::map<std::string, std::optional<std::stringstream>>& longNames
                                    , std::map<char, std::optional<std::stringstream>>& shortNames) {
                __decode_member<H>::decode(longNames, shortNames);
                __decode_member_list<::pparser::internal::Typelist<T...>>::decode(longNames, shortNames);
            }
        };
        
        template <typename H>
        struct __decode_member_list<::pparser::internal::Typelist<H>> {
            static void decode(std::map<std::string, std::optional<std::stringstream>>& longNames
                                    , std::map<char, std::optional<std::stringstream>>& shortNames) {
                __decode_member<H>::decode(longNames, shortNames);
            }
        };
    }

    #define BEGIN_PARAMETER_DECLARATION() \
    _START_LIST()

    #define ADD_PARAMETER(shortName, longName, isOptional, hasArgument, parType, defaultValue) \
    std::conditional<isOptional, std::optional<parType>, parType>::type longName = defaultValue; \
    typedef typename ::pparser::impl::parameterObject<decltype(longName), __COUNTER__> __##longName##_type; \
    __##longName##_type __##longName##_instance = __##longName##_type((#shortName)[0], #longName, &longName, isOptional, hasArgument); \
    _ADD_TO_LIST(__##longName##_type)

    #define END_PARAMETER_DECLARATION() \
    _END_LIST()

    template<typename parameterFileType>
    class parser {
        static std::vector<std::string> formatArgv(int argc, char const** argv) {
            std::vector<std::string> ret;
            ret.reserve(argc);
            for(int i = 1; i < argc; ++i) {
                ret.emplace_back(argv[i]);
            }
            return ret;
        }

        static auto parseTheArgsToMaps(int argc, char const** argv) {
            const std::vector<std::string> arguments = formatArgv(argc, argv);
            std::map<std::string, std::optional<std::stringstream>> longNames;
            std::map<char, std::optional<std::stringstream>> shortNames;
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
                        longNames.emplace(current, *it);
                        ++it;
                    } else {
                        longNames.emplace(current, std::optional<std::stringstream>());
                    }
                } else {
                    ++it;
                    if((it != arguments.end()) && (*it)[0] != '-') {
                        shortNames.emplace(current[0], *it);
                        ++it;
                    } else {
                        shortNames.emplace(current[0], std::optional<std::stringstream>());
                    }
                }
            }
            return std::make_pair(std::move(longNames), std::move(shortNames));
        }

    public:
        static parameterFileType createParameterFile(int argc, char const **argv) {
            auto[longNames, shortNames] = parseTheArgsToMaps(argc, argv);
            return ::pparser::impl::__object_value_decoder<parameterFileType>::get(longNames, shortNames);
        }
    };
}
