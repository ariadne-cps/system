#include <sstream>
#include "system/discrete_event.hpp"
#include "macro/testing.hpp"
using namespace Ariadne;
int main() {
    DiscreteEvent tick("tick"), same("tick"), tock("tock");
    ARIADNE_TEST_ASSERT(tick == same);
    ARIADNE_TEST_ASSERT(tick != tock);
    ARIADNE_TEST_ASSERT(tick < tock);
    ARIADNE_TEST_ASSERT(tock > tick);
    ARIADNE_TEST_ASSERT(tick <= same);
    ARIADNE_TEST_ASSERT(tick <= tock);
    ARIADNE_TEST_ASSERT(tock >= tick);
    ARIADNE_TEST_EQUALS(tick.name(),StringType("tick"));
    DiscreteEvent unnamed;
    ARIADNE_TEST_EQUALS(unnamed.name(),StringType("e?"));
    std::ostringstream os;
    os << tick;
    ARIADNE_TEST_EQUALS(os.str(),StringType("tick"));
    return ARIADNE_TEST_FAILURES;
}
