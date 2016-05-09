#pragma once

#include <string>
#include <ctime>
#include <iostream>
#include <memory>

#include "Cmd.hpp"
#include "RNG.hpp"
#include "stat.hpp"
#include "Logging.hpp"
#include "walker/Walker.hpp"
#include "walker/LatticeWalker.hpp"
#include "walker/LoopErasedWalker.hpp"
#include "walker/SelfAvoidingWalker.hpp"
#include "walker/RealWalker.hpp"
#include "walker/GaussWalker.hpp"
#include "simulation/Metropolis.hpp"


void benchmark();
