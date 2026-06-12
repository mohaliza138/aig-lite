#pragma once
#include <cstdint>

namespace aig {

// A Signal is a packed uint32_t: upper 31 bits = node_id, LSB = complement bit.
struct Signal {
    uint32_t data;

    constexpr Signal() : data(0) {}
    constexpr Signal(uint32_t node_id, bool complement)
        : data((node_id << 1) | static_cast<uint32_t>(complement)) {}

    constexpr uint32_t node_id()    const { return data >> 1; }
    constexpr bool     complement() const { return data & 1u; }

    constexpr Signal operator~() const { return Signal(node_id(), !complement()); }

    constexpr bool operator==(Signal o) const { return data == o.data; }
    constexpr bool operator!=(Signal o) const { return data != o.data; }

    constexpr bool operator<(Signal o) const { return data < o.data; }

    static constexpr Signal zero() { return Signal(0u, false); }
    static constexpr Signal one()  { return Signal(0u, true);  }

    // Node 0 is the reserved constant 0 node.
    constexpr bool is_constant() const { return node_id() == 0; }
    constexpr bool is_zero()     const { return data == 0; }
    constexpr bool is_one()      const { return data == 1; }
};

}
