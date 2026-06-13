#include "aig/IO.h"
#include <cassert>
#include <cstdio>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace aig {

static void write_uint(std::FILE* f, uint32_t val) {
    while (val & ~0x7fu) {
        std::fputc(static_cast<uint8_t>((val & 0x7f) | 0x80), f);
        val >>= 7;
    }
    std::fputc(static_cast<uint8_t>(val), f);
}

void write_aiger(const AigNetwork& net, const std::string& path) {
    const auto& pis = net.pis();
    const auto& pos = net.pos();
    const uint32_t I = net.num_pis();
    const uint32_t O = net.num_pos();

    std::unordered_map<uint32_t, uint32_t> var;
    var[0] = 0;
    for (uint32_t i = 0; i < I; ++i)
        var[pis[i]] = i + 1;

    // Collect AND nodes reachable from POs in topological order via DFS.
    std::unordered_set<uint32_t> visited;
    std::vector<uint32_t> order;

    std::function<void(uint32_t)> dfs = [&](uint32_t id) {
        if (!visited.insert(id).second) return;
        if (!net.is_and(id)) return;
        const AigNode& n = net.node(id);
        dfs(n.fanin0.node_id());
        dfs(n.fanin1.node_id());
        order.push_back(id);
    };

    for (const Signal& po : pos)
        dfs(po.node_id());

    uint32_t next_var = I + 1;
    for (uint32_t id : order)
        var[id] = next_var++;

    const uint32_t A = static_cast<uint32_t>(order.size());
    const uint32_t M = I + A;

    auto to_lit = [&](Signal s) -> uint32_t {
        return var.at(s.node_id()) * 2 + (s.complement() ? 1 : 0);
    };

    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) throw std::runtime_error("write_aiger: cannot open " + path);

    std::fprintf(f, "aig %u %u 0 %u %u\n", M, I, O, A);

    for (const Signal& po : pos)
        std::fprintf(f, "%u\n", to_lit(po));

    for (uint32_t id : order) {
        const AigNode& n = net.node(id);
        uint32_t lhs = var[id] * 2;
        uint32_t rhs0 = to_lit(n.fanin0);
        uint32_t rhs1 = to_lit(n.fanin1);
        if (rhs0 < rhs1) std::swap(rhs0, rhs1);
        assert(lhs > rhs0 && rhs0 >= rhs1);
        write_uint(f, lhs - rhs0);
        write_uint(f, rhs0 - rhs1);
    }

    std::fclose(f);
}

}
