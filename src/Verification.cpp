#include "aig/Verification.h"
#include "aig/IO.h"
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace aig {

static bool brute_force(AigNetwork& a, AigNetwork& b) {
    uint32_t n = a.num_pis();
    uint64_t patterns = uint64_t(1) << n;
    for (uint64_t p = 0; p < patterns; ++p)
        if (a.simulate(p) != b.simulate(p)) return false;
    return true;
}

static bool abc_cec(AigNetwork& a, AigNetwork& b) {
    const std::string path_a = "/tmp/aig_cec_a.aig";
    const std::string path_b = "/tmp/aig_cec_b.aig";
    write_aiger(a, path_a);
    write_aiger(b, path_b);

    std::string cmd = "abc -c \"cec " + path_a + " " + path_b + "\" 2>&1";
    std::FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("verify_equivalent: popen failed");

    std::string output;
    char buf[256];
    while (std::fgets(buf, sizeof(buf), pipe))
        output += buf;
    pclose(pipe);

    if (output.find("Networks are equivalent") != std::string::npos) return true;
    if (output.find("Networks are NOT equivalent") != std::string::npos) return false;
    throw std::runtime_error("verify_equivalent: unexpected ABC output:\n" + output);
}

bool verify_equivalent(AigNetwork& a, AigNetwork& b) {
    assert(a.num_pis() == b.num_pis() && "both networks must have the same number of inputs");
    assert(a.num_pos() == 1 && b.num_pos() == 1 && "both networks must have exactly one output");

    if (a.num_pis() <= 20)
        return brute_force(a, b);
    else
        return abc_cec(a, b);
}

}
