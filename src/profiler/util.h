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
std::vector<wstring> GetEnvironmentValues(const wstring& name, char delim);

std::vector<wstring> Split(const wstring&s, wchar_t delim);

wstring Trim(const wstring&str);

wstring HexStr(const unsigned char* data, int len);

std::ostream& operator<<(std::ostream& os, REFGUID guid);

std::ostream& operator<<(std::ostream& os, std::vector<BYTE> vec);