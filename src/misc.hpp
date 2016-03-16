#pragma once

#include <string>
#include <ctime>

std::string time_diff(clock_t start, clock_t end)
{
    return std::to_string((double)(end - start) / CLOCKS_PER_SEC) + "s";
}
