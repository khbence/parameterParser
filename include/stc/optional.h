#pragma once
#include <exception>
#include <utility>
#include <sstream>

// standard custom, to replace the used c++17 features
namespace stc {
    class bad_optional_access : public std::exception {
        [[nodiscard]] const char* what() const noexcept override { return "Bad optional access!"; }
    };

    template<typename T>
    class optional {
        T data;
        bool hasData = false;

    public:
        // TODO handle if data is not default constructible
        optional() = default;
        optional(T&& data_p) : data(std::forward<T>(data_p)), hasData(true) {}
        optional(const T& data_p) : data(data_p), hasData(true) {}
        optional(optional&& other) noexcept : data(std::move(other.data)), hasData(other.hasData) {}
        optional(const optional& other) : data(other.data), hasData(other.hasData) {}
        optional& operator=(optional&& other) noexcept {
            data = std::move(other.data);
            hasData = other.hasData;
            return *this;
        }

        optional& operator=(const optional& other) {
            data = other.data;
            hasData = other.hasData;
            return *this;
        }

        operator bool() const { return hasData; }


        T& value() {
            if (hasData) { return data; }
            throw stc::bad_optional_access();
        }

        const T& value() const {
            if (hasData) { return data; }
            throw stc::bad_optional_access();
        }

        decltype(auto) value_or(T&& default_value) {
            if (hasData) { return data; }
            return default_value;
        }

        decltype(auto) value_or(T&& default_value) const {
            if (hasData) { return data; }
            return default_value;
        }

        void reset() {
            hasData = false;
            data.~T();
        }
    };

    template<>
    optional<std::stringstream>::optional(const stc::optional<std::stringstream>& other) {
        if(other.hasData) {
            hasData = true;
            data << other.data.rdbuf();
        }
        hasData = false;
    }
}// namespace stc