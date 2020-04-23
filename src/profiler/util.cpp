#include "util.h"
#include <cstdlib>
#include <iostream>
#include "miniutf.hpp"

wstring GetEnvironmentValue(const wstring&name) {
  auto cstr = std::getenv(ToString(name).c_str());
  if (cstr == nullptr) {
    return ""_W;
  }

  return ToWSTRING(std::string(cstr));
}

std::vector<wstring> GetEnvironmentValues(const wstring&name,
                                          wchar_t delim) {
  std::vector<wstring> values;
  for (auto s : Split(GetEnvironmentValue(name), delim)) {
    s = Trim(s);
    if (!s.empty()) {
      values.push_back(s);
    }
  }
  return values;
}

template <typename Out>
void Split(const wstring& s, wchar_t delim, Out result) {
    size_t lpos = 0;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == delim) {
            *(result++) = s.substr(lpos, (i - lpos));
            lpos = i + 1;
        }
    }
    *(result++) = s.substr(lpos);
}

std::vector<wstring> Split(const wstring&s, wchar_t delim) {
  std::vector<wstring> elems;
  Split(s, delim, std::back_inserter(elems));
  return elems;
}

wstring Trim(const wstring&str) {
  if (str.length() == 0) {
    return ""_W;
  }

  wstring trimmed = str;

  auto lpos = trimmed.find_first_not_of(" \t"_W);
  if (lpos != std::string::npos && lpos > 0) {
    trimmed = trimmed.substr(lpos);
  }

  auto rpos = trimmed.find_last_not_of(" \t"_W);
  if (rpos != std::string::npos) {
    trimmed = trimmed.substr(0, rpos + 1);
  }

  return trimmed;
}

constexpr char HexMap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

wstring HexStr(const unsigned char* data, int len) {
    wstring s(len * 2, ' ');
    for (int i = 0; i < len; ++i) {
        s[2 * i] = HexMap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = HexMap[data[i] & 0x0F];
    }
    return s;
}

std::ostream& operator<<(std::ostream& os, REFGUID guid) {

    os << std::uppercase;
    os.width(8);
    os << std::hex << guid.Data1 << '-';

    os.width(4);
    os << std::hex << guid.Data2 << '-';

    os.width(4);
    os << std::hex << guid.Data3 << '-';

    os.width(2);
    os << std::hex
        << static_cast<short>(guid.Data4[0])
        << static_cast<short>(guid.Data4[1])
        << '-'
        << static_cast<short>(guid.Data4[2])
        << static_cast<short>(guid.Data4[3])
        << static_cast<short>(guid.Data4[4])
        << static_cast<short>(guid.Data4[5])
        << static_cast<short>(guid.Data4[6])
        << static_cast<short>(guid.Data4[7]);
    os << std::nouppercase;
    return os;
}

std::ostream& operator<<(std::ostream& os, std::vector<BYTE> vec)
{
    os << std::uppercase;

    for (const auto& el : vec)
    {
        os << std::hex << el << std::endl;
    }

    os << std::nouppercase;
    return os;
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