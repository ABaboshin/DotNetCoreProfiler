#include <cstdlib>
#include <iostream>
#include "miniutf/miniutf.hpp"
#include "util.h"

namespace util
{
    std::string GetEnvironmentValue(const std::string& name) {
        auto cstr = std::getenv(name.c_str());
        if (cstr == nullptr) {
            return "";
        }

        return std::string(cstr);
    }

	wstring ToString(const std::vector<WCHAR>& data, size_t length)
	{
        if (data.empty() || length == 0)
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

    wstring Trim(const wstring& str) {
        if (str.length() == 0) {
            return ""_W;
        }

        wstring trimmed = str;

        auto trimSymbols = " \t\r\n\0"_W;

        auto lpos = trimmed.find_first_not_of(trimSymbols);
        if (lpos != std::string::npos && lpos > 0) {
            trimmed = trimmed.substr(lpos);
        }

        auto rpos = trimmed.find_last_not_of(trimSymbols);
        if (rpos != std::string::npos) {
            trimmed = trimmed.substr(0, rpos + 1);
        }

        return trimmed;
    }

    std::string Trim(const std::string& str) {
        if (str.length() == 0) {
            return "";
        }

        std::string trimmed = str;

        auto trimSymbols = " \t\r\n\0";

        auto lpos = trimmed.find_first_not_of(trimSymbols);
        if (lpos != std::string::npos && lpos > 0) {
            trimmed = trimmed.substr(lpos);
        }

        auto rpos = trimmed.find_last_not_of(trimSymbols);
        if (rpos != std::string::npos) {
            trimmed = trimmed.substr(0, rpos + 1);
        }

        return trimmed;
    }
}