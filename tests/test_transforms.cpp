#include "aig/Transformations.h"
#include "aig/Verification.h"
#include "aig/IO.h"
#include <cassert>

using aig::AigNetwork;
using aig::Signal;

int main() {
    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        Signal c = net.create_pi();
        Signal d = net.create_pi();
        // ((a & b) & c) & d
        Signal ab  = net.get_or_create_and(a, b);
        Signal abc = net.get_or_create_and(ab, c);
        Signal r   = net.get_or_create_and(abc, d);
        net.create_po(r);
        net.compute_levels();
        assert(net.level() == 3);

        // Keep a copy for equivalence check.
        AigNetwork orig = net;

        uint32_t improvement = aig::balance(net);
        assert(improvement == 1);
        assert(net.level() == 2);
        assert(aig::verify_equivalent(orig, net));
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        net.create_po(net.get_or_create_and(a, b));
        net.compute_levels();
        assert(net.level() == 1);

        AigNetwork orig = net;
        aig::balance(net);
        assert(net.level() == 1);
        assert(aig::verify_equivalent(orig, net));
    }

    {
        AigNetwork net = aig::read_truth("6");
        AigNetwork orig = net;
        net.compute_levels();
        uint32_t before = net.level();

        aig::balance(net);
        assert(net.level() <= before);
        assert(aig::verify_equivalent(orig, net));
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        Signal c = net.create_pi();
        Signal ab = net.get_or_create_and(a, b);
        Signal ac = net.get_or_create_and(a, c);
        Signal r  = net.get_or_create_and(ab, ac);
        net.create_po(r);
        assert(net.node_count() == 3);

        AigNetwork orig = net;
        uint32_t eliminated = aig::rewrite(net);
        assert(eliminated > 0);
        assert(net.node_count() < 3);
        assert(aig::verify_equivalent(orig, net));
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        net.create_po(net.get_or_create_and(a, b));

        AigNetwork orig = net;
        uint32_t eliminated = aig::rewrite(net);
        assert(eliminated == 0);
        assert(aig::verify_equivalent(orig, net));
    }

    return 0;
}
