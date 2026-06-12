#include "aig/IO.h"
#include <cassert>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <vector>

namespace aig {

static uint8_t hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::invalid_argument(std::string("invalid hex character: ") + c);
}

AigNetwork read_truth(const std::string& hex) {
    assert(!hex.empty());

    // hex[0] holds the highest minterms.
    // I reverse so bit index matches minterm index directly.
    const size_t n_bits = hex.size() * 4;
    std::vector<bool> tt(n_bits);
    for (size_t i = 0; i < hex.size(); ++i) {
        uint8_t nibble = hex_digit(hex[hex.size() - 1 - i]);
        for (int b = 0; b < 4; ++b)
            tt[i * 4 + b] = (nibble >> b) & 1;
    }

    // Ensure number of bits, to be a power of 2.
    size_t n_inputs = 0;
    while ((size_t(1) << n_inputs) < n_bits) ++n_inputs;
    assert((size_t(1) << n_inputs) == n_bits && "hex length must be a power of 2");

    AigNetwork net;
    std::vector<Signal> pis;
    pis.reserve(n_inputs);
    for (size_t i = 0; i < n_inputs; ++i)
        pis.push_back(net.create_pi());

    if (*std::max_element(tt.begin(), tt.end()) == false) {
        net.create_po(Signal::zero());
        return net;
    }

    if (*std::min_element(tt.begin(), tt.end()) == true) {
        net.create_po(Signal::one());
        return net;
    }

    // Build one AND-chain per minterm, then OR them.
    Signal result = Signal::zero();
    for (size_t m = 0; m < n_bits; ++m) {
        if (!tt[m]) continue;

        // AND of all literals for this minterm
        Signal term = Signal::one();
        for (size_t i = 0; i < n_inputs; ++i) {
            Signal lit = (m >> i) & 1 ? pis[i] : ~pis[i];
            term = net.get_or_create_and(term, lit);
        }

        // result = result + term (De Morgan)
        result = ~net.get_or_create_and(~result, ~term);
    }

    net.create_po(result);
    return net;
}

}
