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

HRESULT CreateAssemblyRef(const ComPtr<IMetaDataAssemblyEmit> pMetadataAssemblyEmit, mdAssemblyRef* mscorlib_ref, std::vector<BYTE> public_key, ASSEMBLYMETADATA metadata, WSTRING assemblyName) {
    HRESULT hr = pMetadataAssemblyEmit->DefineAssemblyRef(
        (void*)public_key.data(),
        (ULONG)public_key.size(),
        assemblyName.c_str(), &metadata, NULL, 0, 0,
        mscorlib_ref);

    return hr;
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