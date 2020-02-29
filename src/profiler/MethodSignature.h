#pragma once

#include <iomanip>
#include "util.h"

struct MethodSignature {
 public:
  const std::vector<BYTE> data;

  MethodSignature() {}
  MethodSignature(const std::vector<BYTE>& data) : data(data) {}

  inline bool operator==(const MethodSignature& other) const {
    return data == other.data;
  }

  CorCallingConvention CallingConvention() const {
    return CorCallingConvention(data.empty() ? 0 : data[0]);
  }

  size_t NumberOfTypeArguments() const {
    if (data.size() > 1 &&
        (CallingConvention() & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0) {
      return data[1];
    }
    return 0;
  }

  size_t NumberOfArguments() const {
    if (data.size() > 2 &&
        (CallingConvention() & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0) {
      return data[2];
    }
    if (data.size() > 1) {
      return data[1];
    }
    return 0;
  }

  bool ReturnTypeIsObject() const {
    if (data.size() > 2 &&
        (CallingConvention() & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0) {
      return data[3] == ELEMENT_TYPE_OBJECT;
    }
    if (data.size() > 1) {
      return data[2] == ELEMENT_TYPE_OBJECT;
    }

    return false;
  }

  WSTRING str() const {
    WSTRINGSTREAM ss;
    for (auto& b : data) {
      ss << std::hex << std::setfill('0'_W) << std::setw(2) << static_cast<int>(b);
    }
    return ss.str();
  }

  BOOL IsInstanceMethod() const {
    return (CallingConvention() & IMAGE_CEE_CS_CALLCONV_HASTHIS) != 0;
  }
};
