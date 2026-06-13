#include "aig/Verification.h"
#include "aig/IO.h"
#include <cassert>

using aig::AigNetwork;
using aig::Signal;

int main() {
    {
        // AND
        AigNetwork a = aig::read_truth("8");
        AigNetwork b;
        Signal x = b.create_pi();
        Signal y = b.create_pi();
        b.create_po(b.get_or_create_and(x, y));
        assert(aig::verify_equivalent(a, b));
    }

    {
        // AND
        AigNetwork a = aig::read_truth("8");
        // XOR
        AigNetwork b = aig::read_truth("6");
        assert(!aig::verify_equivalent(a, b));
    }

    {
        // OR
        AigNetwork a = aig::read_truth("e");

        AigNetwork b;
        Signal x = b.create_pi();
        Signal y = b.create_pi();
        b.create_po(~b.get_or_create_and(~x, ~y));
        assert(aig::verify_equivalent(a, b));
    }

    return 0;
}
