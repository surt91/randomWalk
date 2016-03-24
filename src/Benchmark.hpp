#pragma once

#include <string>
#include <ctime>
#include <iostream>
#include <cstdio>
#include <memory>

// TODO: ifdef für unix?
#include <unistd.h> //for getpid

#include "Cmd.hpp"
#include "RNG.hpp"
#include "Logging.hpp"
#include "Walker.hpp"
#include "LoopErasedWalker.hpp"


std::string vmPeak();
std::string time_diff(clock_t start, clock_t end);

void benchmark();
