#include "aig/Transformations.h"
#include <functional>
#include <unordered_map>
#include <vector>

namespace aig {

static Signal apply_subst(const std::unordered_map<uint32_t, Signal>& subst, Signal s) {
    auto it = subst.find(s.node_id());
    if (it == subst.end()) return s;
    const Signal& rep = it->second;
    return Signal(rep.node_id(), rep.complement() ^ s.complement());
}

uint32_t rewrite(AigNetwork& net) {
    uint32_t nodes_before = net.node_count();

    // Snapshot before modification.
    std::vector<uint32_t> and_ids;
    for (uint32_t id = 1; id < net.next_id(); ++id)
        if (net.is_and(id) && !net.node(id).dead)
            and_ids.push_back(id);

    std::unordered_map<uint32_t, Signal> subst;

    for (uint32_t id : and_ids) {
        if (net.node(id).dead) continue;
        const AigNode& n = net.node(id);
        Signal a = apply_subst(subst, n.fanin0);
        Signal b = apply_subst(subst, n.fanin1);

        uint32_t aid = a.node_id(), bid = b.node_id();
        bool a_is_and = !a.complement() && net.is_and(aid);
        bool b_is_and = !b.complement() && net.is_and(bid);

        Signal cand; cand.data = UINT32_MAX;

        // Common divisor.
        if (a_is_and && b_is_and) {
            Signal c = net.node(aid).fanin0, d = net.node(aid).fanin1;
            Signal e = net.node(bid).fanin0, f = net.node(bid).fanin1;

            auto factor = [&](Signal x, Signal p, Signal q) -> Signal {
                return net.get_or_create_and(x, net.get_or_create_and(p, q));
            };

            if      (c == e) cand = factor(c, d, f);
            else if (c == f) cand = factor(c, d, e);
            else if (d == e) cand = factor(d, c, f);
            else if (d == f) cand = factor(d, c, e);
        }

        if (cand.data != UINT32_MAX && cand != Signal(id, false))
            subst[id] = cand;
    }

    if (subst.empty()) return 0;

    std::vector<Signal> old_pos = net.pos();
    net.reset_pos();
    for (const Signal& po : old_pos)
        net.create_po(apply_subst(subst, po));

    net.compact();

    uint32_t nodes_after = net.node_count();
    return nodes_before > nodes_after ? nodes_before - nodes_after : 0;
}

}
