#include "system/discrete_event.hpp"
#include "macro/testing.hpp"
using namespace Ariadne;
int main() {
    DiscreteEvent tick("tick"), same("tick"), tock("tock");
    ARIADNE_TEST_ASSERT(tick == same);
    ARIADNE_TEST_ASSERT(tick != tock);
    ARIADNE_TEST_ASSERT(tick < tock);
    ARIADNE_TEST_EQUALS(tick.name(),StringType("tick"));
    return ARIADNE_TEST_FAILURES;
}
