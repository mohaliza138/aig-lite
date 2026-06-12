#pragma once
#include "aig/Signal.h"
#include "aig/AigNode.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace aig {

class AigNetwork {
public:
    AigNetwork();

    Signal create_pi();
    void create_po(Signal s);

    // Returns a Signal for AND(a,b).
    Signal get_or_create_and(Signal a, Signal b);

    uint32_t node_count() const;
    uint32_t num_pis() const;
    uint32_t num_pos() const;

    // Must call compute_levels() first.
    uint32_t level() const;

    bool is_constant(uint32_t node_id) const;
    bool is_pi(uint32_t node_id) const;
    bool is_and(uint32_t node_id) const;

    const AigNode& node(uint32_t node_id) const;
    const std::vector<uint32_t>& pis() const;
    const std::vector<Signal>& pos() const;

    void compute_levels();

    bool simulate(uint64_t input_pattern) const;

private:
    static uint64_t make_key(Signal lo, Signal hi);
    uint32_t alloc_node(Signal f0, Signal f1);

    std::vector<AigNode> nodes_; // index 0 = constant node
    std::vector<uint32_t> pi_indices_;
    std::vector<Signal> pos_;
    std::unordered_map<uint64_t, uint32_t> hash_;
};

}
