/***************************************************************************
 *            tests/test_iterated_map.cpp
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

/** @file tests/test_iterated_map.cpp
 *  @brief Test iterated map construction and auxiliary assignments.
 */

#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"
#include "system/iterated_map.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestIteratedMap : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_iterated_map() {
        RealVariable x("x"), y("y"), a("a");
        {
            IteratedMap map(List<PrimedRealAssignment>{prime(x) = x + 1, prime(y) = y});
            ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 2u);
            ARIADNE_TESTING_PROVE(map.state_space() == RealSpace({x, y}));
            ARIADNE_TESTING_EXPECT_EQUAL(map.auxiliary_space().size(), 0u);
            ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 2u);
            ARIADNE_TESTING_PROVE(&map.function() == &map.update_function());
            ARIADNE_TESTING_PROVE(&map.auxiliary_function() == &map.auxiliary_mapping());
            IteratedMap* copy = map.clone();
            ARIADNE_TESTING_PROVE(copy != nullptr);
            ARIADNE_TESTING_EXPECT_EQUAL(copy->dimension(), 2u);
            delete copy;
            ARIADNE_TESTING_NOTE(map);
        }
        {
            IteratedMap map(List<PrimedRealAssignment>{prime(x) = x + a},
                            List<RealAssignment>{let(a) = x + 1});
            ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 1u);
            ARIADNE_TESTING_PROVE(map.auxiliary_space() == RealSpace({a}));
            ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 2u);
            ARIADNE_TESTING_NOTE(map);
        }
        {
            // Chained auxiliary assignments exercise the sorting loop.
            RealVariable b("b");
            IteratedMap map(List<PrimedRealAssignment>{prime(x) = x + a},
                            List<RealAssignment>{let(a) = x, let(b) = a + 1});
            ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 1u);
            ARIADNE_TESTING_EXPECT_EQUAL(map.auxiliary_space().size(), 2u);
            ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 3u);
            ARIADNE_TESTING_NOTE(map);
        }
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_iterated_map");
    ARIADNE_TESTING_ADD_TEST(TestIteratedMap, test_iterated_map);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
