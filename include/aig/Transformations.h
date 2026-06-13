#pragma once
#include "aig/AigNetwork.h"

namespace aig {

// Restructures AND trees to minimize logic depth. Returns the reduction in logic level (0 if no improvement).
uint32_t balance(AigNetwork& net);

}
