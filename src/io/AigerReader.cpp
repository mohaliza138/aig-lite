#include "aig/IO.h"
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace aig {

static uint32_t read_uint(std::FILE* f) {
    uint32_t val = 0;
    int shift = 0;
    int c;
    while ((c = std::fgetc(f)) != EOF) {
        val |= static_cast<uint32_t>(c & 0x7f) << shift;
        if (!(c & 0x80)) break;
        shift += 7;
    }
    return val;
}

AigNetwork read_aiger(const std::string& path) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) throw std::runtime_error("read_aiger: cannot open " + path);

    uint32_t M, I, L, O, A;
    if (std::fscanf(f, "aig %u %u %u %u %u\n", &M, &I, &L, &O, &A) != 5)
        throw std::runtime_error("read_aiger: invalid header");
    if (L != 0)
        throw std::runtime_error("read_aiger: latches not supported");

    AigNetwork net;

    std::vector<Signal> lit_to_signal(2 * (M + 1));
    lit_to_signal[0] = Signal::zero();
    lit_to_signal[1] = Signal::one();

    for (uint32_t i = 0; i < I; ++i) {
        Signal s = net.create_pi();
        lit_to_signal[2 * (i + 1)] = s;
        lit_to_signal[2 * (i + 1) + 1] = ~s;
    }

    // Read output literals.
    std::vector<uint32_t> po_lits(O);
    for (uint32_t i = 0; i < O; ++i)
        if (std::fscanf(f, "%u\n", &po_lits[i]) != 1)
            throw std::runtime_error("read_aiger: failed to read output literal");

    // Read AND gates. binary delta encoding. AND gate i has LHS variable I + i + 1.
    for (uint32_t i = 0; i < A; ++i) {
        uint32_t var = I + i + 1;
        uint32_t lhs = var * 2;
        uint32_t d0 = read_uint(f);
        uint32_t d1 = read_uint(f);
        uint32_t rhs0 = lhs - d0;
        uint32_t rhs1 = rhs0 - d1;

        Signal a = lit_to_signal[rhs0];
        Signal b = lit_to_signal[rhs1];
        Signal s = net.get_or_create_and(a, b);

        lit_to_signal[lhs]     = s;
        lit_to_signal[lhs + 1] = ~s;
    }

    for (uint32_t lit : po_lits)
        net.create_po(lit_to_signal[lit]);

    std::fclose(f);
    return net;
}

}
