#include "aig/IO.h"
#include <cassert>
#include <cstdio>

using aig::AigNetwork;
using aig::Signal;

static bool eval(AigNetwork& net, uint64_t pattern) {
    return net.simulate(pattern);
}

int main() {
    // "8" = 0b1000 (2-input AND)
    {
        AigNetwork net = aig::read_truth("8");
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == false);
        assert(eval(net, 0b10) == false);
        assert(eval(net, 0b11) == true);
    }

    // "6" = 0b0110 (2-input XOR)
    {
        AigNetwork net = aig::read_truth("6");
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == true);
        assert(eval(net, 0b10) == true);
        assert(eval(net, 0b11) == false);
    }

    // "e" = 0b1110 (2-input OR)
    {
        AigNetwork net = aig::read_truth("e");
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == true);
        assert(eval(net, 0b10) == true);
        assert(eval(net, 0b11) == true);
    }

    {
        AigNetwork net = aig::read_truth("0000");
        assert(eval(net, 0) == false);
        assert(eval(net, 7) == false);
    }

    {
        AigNetwork net = aig::read_truth("ff");
        for (uint64_t p = 0; p < 8; ++p)
            assert(eval(net, p) == true);
    }

    {
        const char* path = "/tmp/test_and.aig";
        std::FILE* f = std::fopen(path, "wb");
        std::fputs("aig 3 2 0 1 1\n6\n", f);
        std::fputc(2, f); std::fputc(2, f);
        std::fclose(f);

        AigNetwork net = aig::read_aiger(path);
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == false);
        assert(eval(net, 0b10) == false);
        assert(eval(net, 0b11) == true);
    }

    {
        const char* path = "/tmp/test_or.aig";
        std::FILE* f = std::fopen(path, "wb");
        std::fputs("aig 3 2 0 1 1\n7\n", f);
        std::fputc(1, f); std::fputc(2, f);
        std::fclose(f);

        AigNetwork net = aig::read_aiger(path);
        assert(eval(net, 0b00) == false);
        assert(eval(net, 0b01) == true);
        assert(eval(net, 0b10) == true);
        assert(eval(net, 0b11) == true);
    }

    {
        AigNetwork net;
        Signal a = net.create_pi();
        Signal b = net.create_pi();
        Signal r = net.get_or_create_and(a, b);
        net.create_po(r);

        const char* path = "/tmp/test_write_and.aig";
        aig::write_aiger(net, path);

        // Read back as raw bytes and verify header and structure
        std::FILE* f = std::fopen(path, "rb");
        assert(f);

        uint32_t M, I, L, O, A;
        assert(std::fscanf(f, "aig %u %u %u %u %u\n", &M, &I, &L, &O, &A) == 5);
        assert(M == 3 && I == 2 && L == 0 && O == 1 && A == 1);

        uint32_t out_lit;
        assert(std::fscanf(f, "%u\n", &out_lit) == 1);
        assert(out_lit == 6);

        int d0 = std::fgetc(f);
        int d1 = std::fgetc(f);
        assert(d0 == 2 && d1 == 2);

        std::fclose(f);
    }

    return 0;
}
