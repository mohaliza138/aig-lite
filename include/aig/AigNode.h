#pragma once
#include "aig/Signal.h"

namespace aig {

// A single AND node in the graph.
// Note: Node 0 is the reserved constant 0 node (fanins are meaningless for it).
struct AigNode {
    Signal fanin0;
    Signal fanin1;

    uint32_t level;
    // Count of nodes that reference this one.
    uint32_t ref_count;
    AigNode(Signal f0, Signal f1)
        : fanin0(f0), fanin1(f1), level(0), ref_count(0) {}
};

}
