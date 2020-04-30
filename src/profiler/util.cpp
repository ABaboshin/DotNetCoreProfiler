#include "util.h"
#include <cstdlib>
#include <iostream>
#include "miniutf.hpp"
#include <random>

wstring GetEnvironmentValue(const wstring&name) {
  auto cstr = std::getenv(ToString(name).c_str());
  if (cstr == nullptr) {
    return ""_W;
  }

  return ToWSTRING(std::string(cstr));
}

wstring Trim(const wstring&str) {
  if (str.length() == 0) {
    return ""_W;
  }

  wstring trimmed = str;

  auto lpos = trimmed.find_first_not_of(" \t\r\n"_W);
  if (lpos != std::string::npos && lpos > 0) {
    trimmed = trimmed.substr(lpos);
  }

  auto rpos = trimmed.find_last_not_of(" \t\r\n"_W);
  if (rpos != std::string::npos) {
    trimmed = trimmed.substr(0, rpos + 1);
  }

  return trimmed;
}

wstring ToWSTRING(const std::string& str) {
    auto ustr = miniutf::to_utf16(str);
    return wstring(reinterpret_cast<const WCHAR*>(ustr.c_str()));
}

std::string ToString(const wstring& wstr) {
    std::u16string ustr(reinterpret_cast<const char16_t*>(wstr.c_str()));
    return miniutf::to_utf8(ustr);
}

WCHAR operator"" _W(const char c) { return WCHAR(c); }

wstring operator"" _W(const char* arr, size_t size) {
    std::string str(arr, size);
    return ToWSTRING(str);
}

wstring ToWSTRING(const char* str)
{
    return ToWSTRING(std::string(str));
}

std::string random_string(std::size_t length)
{
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (std::size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}