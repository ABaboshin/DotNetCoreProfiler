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

        auto now_c = std::chrono::system_clock::to_time_t(msg.time);
        char buf[sizeof "2011-10-08T07:07:09Z"];
        std::strftime(buf, sizeof buf, "%FT%TZ", std::gmtime(&now_c));

        j["time"] = buf;
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