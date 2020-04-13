#pragma once

#include <vector>
#include "types.h"
#include "ComPtr.h"

WCHAR operator"" _W(const char c);
WSTRING operator"" _W(const char* arr, size_t size);

WSTRING GetEnvironmentValue(const WSTRING &name);
std::vector<WSTRING> GetEnvironmentValues(const WSTRING &name, const wchar_t delim);

std::string ToString(const WSTRING& wstr);

WSTRING ToWSTRING(const std::string& str);

template <typename Out>
void Split(const WSTRING &s, wchar_t delim, Out result) {
  size_t lpos = 0;
  for (size_t i = 0; i < s.length(); i++) {
    if (s[i] == delim) {
      *(result++) = s.substr(lpos, (i - lpos));
      lpos = i + 1;
    }
  }
  *(result++) = s.substr(lpos);
}

std::vector<WSTRING> Split(const WSTRING &s, wchar_t delim);

WSTRING Trim(const WSTRING &str);

WSTRING HexStr(const unsigned char* data, int len);

std::ostream& operator<<(std::ostream& os, REFGUID guid);

std::ostream& operator<<(std::ostream& os, std::vector<BYTE> vec);