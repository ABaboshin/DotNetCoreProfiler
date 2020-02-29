#pragma once

#include <corhlpr.h>
#include <sstream>
#include <string>
#include "miniutf.hpp"

typedef std::basic_string<WCHAR> WSTRING;
typedef std::basic_stringstream<WCHAR> WSTRINGSTREAM;

const size_t kNameMaxSize = 1024;
