#pragma once
#include "aig/AigNetwork.h"
#include <string>

namespace aig {

// Builds an AIG from a hex truth table string.
// TODO: Improve! Current method simply creates an AIG, representing the SOP of minterms.
AigNetwork read_truth(const std::string& hex);

}
