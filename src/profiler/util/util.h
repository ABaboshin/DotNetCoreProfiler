#pragma once

#include <vector>
#include <windows.h>
#include "types.h"

WCHAR operator"" _W(const char c);
wstring operator"" _W(const char* arr, size_t size);
wstring ToWSTRING(const char* str);
wstring ToWSTRING(const std::string& str);
std::string ToString(const wstring& wstr);

wstring GetEnvironmentValue(const wstring& name);

wstring Trim(const wstring&str);

#include <numeric>
#include <random>
#include <vector>
#include <iostream>
#include <iterator>
#include <functional>

std::string random_string(std::size_t length);