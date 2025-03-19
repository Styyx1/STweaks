#pragma once

#define NOMINMAX
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <REX/REX/INI.h>
#include "SimpleIni.h"

using namespace std::literals;

namespace logger = SKSE::log;

namespace stl
{
    using namespace SKSE::stl;

    template <std::integral T, std::size_t N>
    void safe_write(std::uintptr_t a_dst, const std::array<T, N>& a_data)
    {
        REL::safe_write(a_dst, a_data.data(), a_data.size() * sizeof(T));
    }

} // namespace stl
