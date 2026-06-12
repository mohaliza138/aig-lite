#include "aig/Signal.h"
#include <cassert>

using aig::Signal;

int main() {
    static_assert(Signal::zero().is_zero());
    static_assert(Signal::one().is_one());
    static_assert(Signal::zero().is_constant());
    static_assert(Signal::one().is_constant());

    constexpr Signal s(42u, true);
    static_assert(s.node_id()    == 42u);
    static_assert(s.complement() == true);

    constexpr Signal t(7u, false);
    static_assert(t.node_id()    == 7u);
    static_assert(t.complement() == false);

    static_assert((~s).node_id()    == 42u);
    static_assert((~s).complement() == false);
    static_assert(~Signal::zero() == Signal::one());
    static_assert(~Signal::one()  == Signal::zero());

    static_assert(Signal(3u, false) < Signal(3u, true));
    static_assert(Signal(3u, true)  < Signal(4u, false));
    static_assert(!(Signal(5u, false) < Signal(5u, false)));

    static_assert(!Signal(1u, false).is_constant());

    return 0;
}
