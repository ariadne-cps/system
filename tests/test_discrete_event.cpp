#include <sstream>
#include "system/discrete_event.hpp"
#include "ariadne-testing.hpp"
using namespace Ariadne;

class TestDiscreteEvent : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;
    Void test_event() {
    DiscreteEvent tick("tick"), same("tick"), tock("tock");
    ARIADNE_TESTING_PROVE(tick == same);
    ARIADNE_TESTING_PROVE(tick != tock);
    ARIADNE_TESTING_PROVE(tick < tock);
    ARIADNE_TESTING_PROVE(tock > tick);
    ARIADNE_TESTING_PROVE(tick <= same);
    ARIADNE_TESTING_PROVE(tick <= tock);
    ARIADNE_TESTING_PROVE(tock >= tick);
    ARIADNE_TESTING_EXPECT_EQUAL(tick.name(),StringType("tick"));
    DiscreteEvent unnamed;
    ARIADNE_TESTING_EXPECT_EQUAL(unnamed.name(),StringType("e?"));
    std::ostringstream os;
    os << tick;
    ARIADNE_TESTING_EXPECT_EQUAL(os.str(),StringType("tick"));
    
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_discrete_event");
    ARIADNE_TESTING_ADD_TEST(TestDiscreteEvent, test_event);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}
