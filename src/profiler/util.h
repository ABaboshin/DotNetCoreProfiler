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

inline bool ends_with(WSTRING const& value, WSTRING const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

template<class T>
T base_name(T const& path, T const& delims)
{
    return path.substr(path.find_last_of(delims) + 1);
}
template<class T>
T remove_extension(T const& filename)
{
    typename T::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != T::npos ? filename.substr(0, p) : filename;
}

HRESULT CreateAssemblyRef(const ComPtr< IMetaDataAssemblyEmit> pMetadataAssemblyEmit, mdAssemblyRef* mscorlib_ref, std::vector<BYTE> public_key, ASSEMBLYMETADATA metadata, WSTRING assemblyName);

WSTRING HexStr(const unsigned char* data, int len);

std::ostream& operator<<(std::ostream& os, REFGUID guid);