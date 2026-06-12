#include "aig/IO.h"
#include <cassert>

using aig::AigNetwork;
using aig::Signal;

static bool eval(AigNetwork& net, uint64_t pattern) {
    return net.simulate(pattern);
}

int main() {
    // "8" = 0b1000 (2-input AND)
    {
        AigNetwork net = aig::read_truth("8");
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == false);
        assert(eval(net, 0b10) == false);
        assert(eval(net, 0b11) == true);
    }

    // "6" = 0b0110 (2-input XOR)
    {
        AigNetwork net = aig::read_truth("6");
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == true);
        assert(eval(net, 0b10) == true);
        assert(eval(net, 0b11) == false);
    }

    // "e" = 0b1110 (2-input OR)
    {
        AigNetwork net = aig::read_truth("e");
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == true);
        assert(eval(net, 0b10) == true);
        assert(eval(net, 0b11) == true);
    }

    {
        AigNetwork net = aig::read_truth("0000");
        assert(eval(net, 0) == false);
        assert(eval(net, 7) == false);
    }

    {
        AigNetwork net = aig::read_truth("ff");
        for (uint64_t p = 0; p < 8; ++p)
            assert(eval(net, p) == true);
    }

    return 0;
}
