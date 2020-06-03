#pragma once

#include "Interceptor.h"

namespace configuration
{

    template<class T>
    class back_insert_iterator_interceptor {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using pointer = void;
        using reference = void;

        using container_type = std::vector<configuration::Interceptor>;

        using difference_type = void;

        explicit back_insert_iterator_interceptor(std::vector<configuration::Interceptor>& _Cont) noexcept : container(&_Cont) {}

        back_insert_iterator_interceptor& operator=(const T& _Val) {
            container->push_back(_Val.Interceptor);
            return *this;
        }

        back_insert_iterator_interceptor& operator=(T&& _Val) {
            container->push_back(std::move(_Val.Interceptor));
            return *this;
        }

        back_insert_iterator_interceptor& operator*() noexcept {
            return *this;
        }

        back_insert_iterator_interceptor& operator++() noexcept {
            return *this;
        }

        back_insert_iterator_interceptor operator++(int) noexcept {
            return *this;
        }

    protected:
        std::vector<configuration::Interceptor>* container = nullptr;
    };

    template<class T>
    back_insert_iterator_interceptor<T> back_inserter(std::vector<configuration::Interceptor>& _Cont) noexcept {
        return back_insert_iterator_interceptor<T>(_Cont);
    }
}