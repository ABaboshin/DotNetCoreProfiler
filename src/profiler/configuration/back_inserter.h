// #pragma once

// #include "TypeInfo.h"

// namespace configuration
// {

//     template<class T>
//     class back_insert_iterator_interceptor {
//     public:
//         using iterator_category = std::output_iterator_tag;
//         using value_type = void;
//         using pointer = void;
//         using reference = void;

//         using container_type = std::vector<configuration::TypeInfo>;

//         using difference_type = void;

//         explicit back_insert_iterator_interceptor(std::vector<configuration::TypeInfo>& _Cont) noexcept : container(&_Cont) {}

//         back_insert_iterator_interceptor& operator=(const T& _Val) {
//             container->push_back(_Val.Interceptor);
//             return *this;
//         }

//         back_insert_iterator_interceptor& operator=(T&& _Val) {
//             container->push_back(std::move(_Val.Interceptor));
//             return *this;
//         }

//         back_insert_iterator_interceptor& operator*() noexcept {
//             return *this;
//         }

//         back_insert_iterator_interceptor& operator++() noexcept {
//             return *this;
//         }

//         back_insert_iterator_interceptor operator++(int) noexcept {
//             return *this;
//         }

//     protected:
//         std::vector<configuration::TypeInfo>* container = nullptr;
//     };

//     template<class T>
//     back_insert_iterator_interceptor<T> back_inserter(std::vector<configuration::TypeInfo>& _Cont) noexcept {
//         return back_insert_iterator_interceptor<T>(_Cont);
//     }
// }
