#include "aig/AigNetwork.h"
#include <algorithm>
#include <cassert>

namespace aig {

AigNetwork::AigNetwork() : next_id_(1) {
    // The reserved constant 0 node.
    nodes_.emplace_back(Signal::zero(), Signal::zero());
}

Signal AigNetwork::create_pi() {
    uint32_t id = next_id_++;
    nodes_.emplace_back(Signal::zero(), Signal::zero());
    pi_indices_.push_back(id);
    return Signal(id, false);
}

void AigNetwork::create_po(Signal s) {
    pos_.push_back(s);
}

Signal AigNetwork::get_or_create_and(Signal a, Signal b) {
    if (a.is_zero() || b.is_zero()) return Signal::zero();
    if (a.is_one())                 return b;
    if (b.is_one())                 return a;
    if (a == b)                     return a;
    if (a == ~b)                    return Signal::zero();

    if (b < a) std::swap(a, b);

    uint64_t key = make_key(a, b);
    auto it = hash_.find(key);
    if (it != hash_.end())
        return Signal(it->second, false);

    uint32_t id = alloc_node(a, b);
    hash_[key] = id;

    nodes_[a.node_id()].ref_count++;
    nodes_[b.node_id()].ref_count++;

    return Signal(id, false);
}

void AigNetwork::reset_pos() {
    pos_.clear();
}

void AigNetwork::remove_node(uint32_t node_id) {
    assert(is_and(node_id) && "remove_node: node_id is not an AND node");
    AigNode& n = nodes_[node_id];
    assert(!n.dead      && "remove_node: node is already dead");
    assert(n.ref_count == 0 && "remove_node: node still has references");

    n.dead = true;
    hash_.erase(make_key(n.fanin0, n.fanin1));

    nodes_[n.fanin0.node_id()].ref_count--;
    nodes_[n.fanin1.node_id()].ref_count--;
}

void AigNetwork::compact() {
    // Mark every node reachable from the current PO list.
    std::vector<bool> reachable(nodes_.size(), false);
    reachable[0] = true;
    for (uint32_t id : pi_indices_) reachable[id] = true;

    // Traverse in reverse topological order.
    for (const Signal& po : pos_) reachable[po.node_id()] = true;
    for (uint32_t id = nodes_.size(); id-- > 1;) {
        if (!reachable[id] || !is_and(id)) continue;
        reachable[nodes_[id].fanin0.node_id()] = true;
        reachable[nodes_[id].fanin1.node_id()] = true;
    }

    std::vector<uint32_t> remap(nodes_.size(), 0);
    std::vector<AigNode> new_nodes;
    // Add the reserved constant 0.
    new_nodes.emplace_back(Signal::zero(), Signal::zero());

    uint32_t new_id = 1;
    for (uint32_t old_id = 1; old_id < nodes_.size(); ++old_id) {
        if (!reachable[old_id] || nodes_[old_id].dead) continue;
        remap[old_id] = new_id++;
        new_nodes.push_back(nodes_[old_id]);
    }

    // Rewrite fanin signals using remap.
    auto remap_signal = [&](Signal s) {
        return Signal(remap[s.node_id()], s.complement());
    };

    for (uint32_t id = 1; id < new_nodes.size(); ++id) {
        AigNode& n = new_nodes[id];
        n.fanin0 = remap_signal(n.fanin0);
        n.fanin1 = remap_signal(n.fanin1);
    }

    // Rebuild ref_counts and hash map.
    for (auto& n : new_nodes) n.ref_count = 0;
    hash_.clear();
    for (uint32_t id = 1; id < new_nodes.size(); ++id) {
        AigNode& n = new_nodes[id];
        if (!is_pi(id)) {
            new_nodes[n.fanin0.node_id()].ref_count++;
            new_nodes[n.fanin1.node_id()].ref_count++;
            hash_[make_key(n.fanin0, n.fanin1)] = id;
        }
    }

    // Remap pi_indices_ and pos_.
    for (uint32_t& pi : pi_indices_) pi = remap[pi];
    pi_indices_.erase(
        std::remove(pi_indices_.begin(), pi_indices_.end(), 0u),
        pi_indices_.end());

    for (Signal& po : pos_) po = remap_signal(po);

    nodes_ = std::move(new_nodes);
    next_id_ = static_cast<uint32_t>(nodes_.size());
}

uint32_t AigNetwork::node_count() const {
    uint32_t dead_count = 0;
    for (const auto& n : nodes_) if (n.dead) dead_count++;
    return static_cast<uint32_t>(nodes_.size()) - 1
           - static_cast<uint32_t>(pi_indices_.size()) - dead_count;
}

uint32_t AigNetwork::num_pis() const {
    return static_cast<uint32_t>(pi_indices_.size());
}

uint32_t AigNetwork::num_pos() const {
    return static_cast<uint32_t>(pos_.size());
}

uint32_t AigNetwork::level() const {
    uint32_t max_level = 0;
    for (const Signal& po : pos_)
        max_level = std::max(max_level, nodes_[po.node_id()].level);
    return max_level;
}

bool AigNetwork::is_constant(uint32_t node_id) const {
    return node_id == 0;
}

bool AigNetwork::is_pi(uint32_t node_id) const {
    for (uint32_t id : pi_indices_)
        if (id == node_id) return true;
    return false;
}

bool AigNetwork::is_and(uint32_t node_id) const {
    return node_id != 0 && !is_pi(node_id);
}

const AigNode& AigNetwork::node(uint32_t node_id) const {
    return nodes_[node_id];
}

const std::vector<uint32_t>& AigNetwork::pis() const {
    return pi_indices_;
}

const std::vector<Signal>& AigNetwork::pos() const {
    return pos_;
}

void AigNetwork::compute_levels() {
    // Forward pass is sufficient because nodes are in topological order.
    for (uint32_t id = 0; id < nodes_.size(); ++id) {
        if (nodes_[id].dead) continue;
        if (!is_and(id)) { nodes_[id].level = 0; continue; }
        AigNode& n = nodes_[id];
        n.level = 1 + std::max(nodes_[n.fanin0.node_id()].level,
                               nodes_[n.fanin1.node_id()].level);
    }
}

bool AigNetwork::simulate(uint64_t input_pattern) const {
    assert(num_pis() <= 64);
    assert(num_pos() >= 1);

    std::vector<uint64_t> val(nodes_.size(), 0);

    for (uint32_t i = 0; i < pi_indices_.size(); ++i)
        val[pi_indices_[i]] = (input_pattern >> i) & 1u ? ~uint64_t(0) : 0;

    for (uint32_t id = 1; id < nodes_.size(); ++id) {
        if (!is_and(id) || nodes_[id].dead) continue;
        const AigNode& n = nodes_[id];
        uint64_t v0 = val[n.fanin0.node_id()];
        uint64_t v1 = val[n.fanin1.node_id()];
        if (n.fanin0.complement()) v0 = ~v0;
        if (n.fanin1.complement()) v1 = ~v1;
        val[id] = v0 & v1;
    }

    const Signal& po = pos_[0];
    uint64_t out = val[po.node_id()];
    if (po.complement()) out = ~out;
    return (out & 1u) != 0;
}

uint64_t AigNetwork::make_key(Signal lo, Signal hi) {
    return (static_cast<uint64_t>(lo.data) << 32) | hi.data;
}

uint32_t AigNetwork::alloc_node(Signal f0, Signal f1) {
    uint32_t id = next_id_++;
    nodes_.emplace_back(f0, f1);
    return id;
}

}
