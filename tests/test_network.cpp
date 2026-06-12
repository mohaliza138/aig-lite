#include "aig/AigNetwork.h"
#include <cassert>

using aig::AigNetwork;
using aig::Signal;

int main() {
    {
        AigNetwork net;
        Signal x = net.create_pi();
        assert(net.get_or_create_and(Signal::zero(), x) == Signal::zero());
        assert(net.get_or_create_and(x, Signal::zero()) == Signal::zero());
        assert(net.get_or_create_and(Signal::one(), x)  == x);
        assert(net.get_or_create_and(x, Signal::one())  == x);
        assert(net.get_or_create_and(x, x)              == x);
        assert(net.get_or_create_and(x, ~x)             == Signal::zero());
        assert(net.node_count() == 0);
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        Signal r1 = net.get_or_create_and(a, b);
        Signal r2 = net.get_or_create_and(a, b);
        Signal r3 = net.get_or_create_and(b, a);
        assert(r1 == r2);
        assert(r1 == r3);
        assert(net.node_count() == 1);
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        Signal r = net.get_or_create_and(a, b);
        assert(net.is_constant(0));
        assert(net.is_pi(a.node_id()));
        assert(net.is_pi(b.node_id()));
        assert(net.is_and(r.node_id()));
        assert(!net.is_and(a.node_id()));
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        Signal c = net.create_pi();
        net.get_or_create_and(a, b);
        net.get_or_create_and(a, c);
        assert(net.node(a.node_id()).ref_count == 2);
        assert(net.node(b.node_id()).ref_count == 1);
    }

    {
        AigNetwork net;
        Signal a   = net.create_pi();
        Signal b   = net.create_pi();
        Signal c   = net.create_pi();
        Signal ab  = net.get_or_create_and(a, b);
        Signal abc = net.get_or_create_and(ab, c);
        net.create_po(abc);
        net.compute_levels();
        assert(net.node(ab.node_id()).level  == 1);
        assert(net.node(abc.node_id()).level == 2);
        assert(net.level() == 2);
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        Signal r = net.get_or_create_and(a, b);
        net.create_po(r);
        assert(net.simulate(0b00) == false);
        assert(net.simulate(0b01) == false);
        assert(net.simulate(0b10) == false);
        assert(net.simulate(0b11) == true);
    }

    {
        AigNetwork net2;
        Signal x  = net2.create_pi();
        Signal y  = net2.create_pi();
        Signal xy = net2.get_or_create_and(x, y);
        Signal z  = net2.create_pi();
        Signal r  = net2.get_or_create_and(xy, z);
        net2.create_po(r);

        net2.remove_node(r.node_id());
        assert(net2.node(r.node_id()).dead);
        assert(net2.node(xy.node_id()).ref_count == 0);

        net2.remove_node(xy.node_id());
        assert(net2.node(xy.node_id()).dead);
        assert(net2.node_count() == 0);
    }

    // compact restores contiguous topological IDs
    {
        AigNetwork net;
        Signal a  = net.create_pi();
        Signal b  = net.create_pi();
        Signal ab = net.get_or_create_and(a, b);
        Signal c  = net.create_pi();
        Signal r  = net.get_or_create_and(ab, c);
        net.create_po(r);

        net.remove_node(r.node_id());
        net.remove_node(ab.node_id());
        assert(net.node_count() == 0);

        net.compact();
        assert(net.node_count() == 0);
        assert(net.num_pis()    == 3);
    }

    return 0;
}
