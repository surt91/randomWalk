#pragma once

#include <ctime>
#include <string>
#include <iostream>
#include <cstdio>
#include <memory>
#include <string>

#ifdef __unix__
#include <unistd.h> //for getpid
#endif

std::string vmPeak();

std::string time_diff(clock_t start, clock_t end, int op=1);
