/***************************************************************************
 *            tests/test_discrete_event.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne System.
 *
 *  Ariadne System is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne System is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file tests/test_discrete_event.cpp
 *  @brief Test discrete event behavior.
 */

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
