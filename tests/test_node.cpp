#include "aig/AigNode.h"
#include <cassert>

using aig::Signal;
using aig::AigNode;

int main() {
    Signal a(1u, false);
    Signal b(2u, true);
    AigNode n1(a, b);
    assert(n1.fanin0 == a);
    assert(n1.fanin1 == b);
    assert(n1.level     == 0);
    assert(n1.ref_count == 0);

    assert(n1.fanin0.node_id()    == 1u);
    assert(n1.fanin0.complement() == false);
    assert(n1.fanin1.node_id()    == 2u);
    assert(n1.fanin1.complement() == true);

    return 0;
}
