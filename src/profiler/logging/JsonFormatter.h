#pragma once
#include "spdlog/formatter.h"

namespace logging
{
    class JsonFormatter :
        public spdlog::formatter
    {
        // Inherited via formatter
        virtual void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override;
        virtual std::unique_ptr<spdlog::formatter> clone() const override;
    };
}
