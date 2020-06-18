#include "JsonFormatter.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <time.h>
#include <iostream>

namespace logging
{
    void JsonFormatter::format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest)
    {
        nlohmann::json j;

        j["level"] = spdlog::level::to_string_view(msg.level).data();
        j["message"] = msg.payload.data();
        j["source"] = "profiler";

        auto str = j.dump() + "\n";

        dest.append(str.data(), str.data() + str.size());
    }

    std::unique_ptr<spdlog::formatter> JsonFormatter::clone() const
    {
        return spdlog::details::make_unique<JsonFormatter>();
    }
}