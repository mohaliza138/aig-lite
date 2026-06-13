#include "aig/Transformations.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace aig {

static void collect_leaves(const AigNetwork& net,
                            Signal s,
                            std::unordered_set<uint32_t>& unfolded,
                            std::vector<Signal>& leaves) {
    uint32_t id = s.node_id();

    if (s.complement() || net.is_constant(id) || net.is_pi(id)
            || net.node(id).ref_count > 1) {
        leaves.push_back(s);
        return;
    }

    if (!unfolded.insert(id).second) {
        leaves.push_back(s);
        return;
    }

    const AigNode& n = net.node(id);
    collect_leaves(net, n.fanin0, unfolded, leaves);
    collect_leaves(net, n.fanin1, unfolded, leaves);
}

static Signal build_balanced(AigNetwork& net, std::vector<Signal> leaves) {
    assert(!leaves.empty());

    auto level_of = [&](Signal s) -> uint32_t {
        return net.node(s.node_id()).level;
    };

    auto cmp = [&](Signal a, Signal b) { return level_of(a) > level_of(b); };
    std::make_heap(leaves.begin(), leaves.end(), cmp);

    while (leaves.size() > 1) {
        std::pop_heap(leaves.begin(), leaves.end(), cmp);
        Signal lo = leaves.back(); leaves.pop_back();
        std::pop_heap(leaves.begin(), leaves.end(), cmp);
        Signal hi = leaves.back(); leaves.pop_back();

        Signal combined = net.get_or_create_and(lo, hi);
        net.compute_levels();

        leaves.push_back(combined);
        std::push_heap(leaves.begin(), leaves.end(), cmp);
    }

    return leaves[0];
}

uint32_t balance(AigNetwork& net) {
    net.compute_levels();
    uint32_t level_before = net.level();

    std::unordered_map<uint32_t, Signal> cache;

    std::function<Signal(Signal)> rebuild = [&](Signal s) -> Signal {
        uint32_t id = s.node_id();

        if (net.is_constant(id) || net.is_pi(id))
            return s;

        auto it = cache.find(id);
        if (it != cache.end()) {
            bool flip = it->second.complement() ^ s.complement();
            return Signal(it->second.node_id(), flip);
        }

        std::unordered_set<uint32_t> unfolded;
        std::vector<Signal> leaves;
        collect_leaves(net, Signal(id, false), unfolded, leaves);

        for (Signal& leaf : leaves)
            leaf = rebuild(leaf);

        Signal result = build_balanced(net, leaves);
        cache[id] = result;

        bool flip = result.complement() ^ s.complement();
        return Signal(result.node_id(), flip);
    };

    std::vector<Signal> old_pos = net.pos();
    std::vector<Signal> new_pos;
    new_pos.reserve(old_pos.size());
    for (const Signal& po : old_pos)
        new_pos.push_back(rebuild(po));

    net.reset_pos();
    for (const Signal& po : new_pos)
        net.create_po(po);

    net.compact();
    net.compute_levels();

    uint32_t level_after = net.level();
    return level_before > level_after ? level_before - level_after : 0;
}

}
