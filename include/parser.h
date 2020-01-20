#include <optional>
#include <type_traits>
#include <string>

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
        template<typename T, int ID>
        class parameterObject {
            static std::string longName;
            static T* memberPointer;
        public:
            parameterObject(const std::string& longName_p, T* memberPointer_p) {
                longName = longName_p;
                memberPointer = memberPointer_p;
            }
        };

        template<typename T, int ID> std::string parameterObject<T, ID>::longName;
        template<typename T, int ID> T* parameterObject<T, ID>::memberPointer;
    }

    #define BEGIN_PARAMETER_DECLARATION() \
    _START_LIST()

    #define ADD_PARAMETER(shortName, longName, isOptional, parType, defaultValue) \
    std::conditional<isOptional, std::optional<parType>, parType>::type longName = defaultValue; \
    typedef typename ::pparser::impl::parameterObject<decltype(longName), __COUNTER__> __##longName##_type; \
    __##longName##_type __##longName##_instance = __##longName##_type(#longName, &longName); \
    _ADD_TO_LIST(__##longName##_type)

    #define END_PARAMETER_DECLARATION() \
    _END_LIST()
}
