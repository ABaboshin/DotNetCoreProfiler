#include <cstdlib>
#include <iostream>

#include "miniutf/miniutf.hpp"
#include "util.h"

namespace util
{
    wstring GetEnvironmentValue(const wstring& name) {
        auto cstr = std::getenv(ToString(name).c_str());
        if (cstr == nullptr) {
            return ""_W;
        }

        return ToWSTRING(std::string(cstr));
    }

	wstring ToString(const std::vector<WCHAR>& data, size_t length)
	{
        if (data.empty())
        {
            return wstring();
        }

        auto result = wstring(data.begin(), data.begin() + length);
        while (result[result.length() - 1] == 0)
        {
            result.resize(result.length() - 1);
        }

        return result;
	}

    std::vector<WCHAR> ToRaw(const wstring& str)
    {
        return std::vector<WCHAR>(str.begin(), str.end());
    }

	std::vector<BYTE> ToRaw(PCCOR_SIGNATURE signature, ULONG length)
	{
        return std::vector<BYTE>(&signature[0], &signature[length]);
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
}