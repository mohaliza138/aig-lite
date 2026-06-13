#pragma once
#include "aig/AigNetwork.h"

namespace aig {

// Returns true if a and b compute the same Boolean function on all inputs. For n_inputs <= 20 brute-force simulation
// over all 2^n patterns. Otherwise, delegates to ABC's cec command via popen(). Both networks must have the same
// number of primary inputs and exactly one output.
bool verify_equivalent(AigNetwork& a, AigNetwork& b);

}
