#include "util.h"
#include <cstdlib>
#include <iostream>

std::string ToString(const WSTRING& wstr) {
  std::u16string ustr(reinterpret_cast<const char16_t*>(wstr.c_str()));
  return miniutf::to_utf8(ustr);
}

WSTRING ToWSTRING(const std::string& str) {
  auto ustr = miniutf::to_utf16(str);
  return WSTRING(reinterpret_cast<const WCHAR*>(ustr.c_str()));
}

WSTRING GetEnvironmentValue(const WSTRING &name) {
  auto cstr = std::getenv(ToString(name).c_str());
  if (cstr == nullptr) {
    return ""_W;
  }
  std::string str(cstr);
  auto wstr = ToWSTRING(str);
  return wstr;
}

std::vector<WSTRING> GetEnvironmentValues(const WSTRING &name,
                                          const wchar_t delim) {
                                            // std::cout << "GetEnvironmentValues" << std::endl;
  std::vector<WSTRING> values;
  for (auto s : Split(GetEnvironmentValue(name), delim)) {
    s = Trim(s);
    if (!s.empty()) {
      values.push_back(s);
    }
  }
  return values;
}

std::vector<WSTRING> Split(const WSTRING &s, wchar_t delim) {
  std::vector<WSTRING> elems;
  Split(s, delim, std::back_inserter(elems));
  return elems;
}

WSTRING Trim(const WSTRING &str) {
  if (str.length() == 0) {
    return ""_W;
  }

  WSTRING trimmed = str;

  auto lpos = trimmed.find_first_not_of(" \t"_W);
  if (lpos != WSTRING::npos && lpos > 0) {
    trimmed = trimmed.substr(lpos);
  }

  auto rpos = trimmed.find_last_not_of(" \t"_W);
  if (rpos != WSTRING::npos) {
    trimmed = trimmed.substr(0, rpos + 1);
  }

  return trimmed;
}

WCHAR operator"" _W(const char c) { return WCHAR(c); }

WSTRING operator"" _W(const char* arr, size_t size) {
  std::string str(arr, size);
  return ToWSTRING(str);
}

constexpr char HexMap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

WSTRING HexStr(const unsigned char* data, int len) {
    WSTRING s(len * 2, ' ');
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