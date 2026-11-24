/***************************************************************************
 *            tests/test_difference_algebraic_equation.cpp
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

/** @file tests/test_difference_algebraic_equation.cpp
 *  @brief Test difference algebraic equation construction and auxiliary assignments.
 */

#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"
#include "system/difference_algebraic_equation.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestDifferenceAlgebraicEquation : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_difference_algebraic_equation() {
        RealVariable x("x"), y("y"), a("a");
        {
            DifferenceAlgebraicEquation map(List<PrimedRealAssignment>{prime(x) = x + 1, prime(y) = y});
            ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 2u);
            ARIADNE_TESTING_PROVE(map.state_space() == RealSpace({x, y}));
            ARIADNE_TESTING_EXPECT_EQUAL(map.auxiliary_space().size(), 0u);
            ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 2u);
            ARIADNE_TESTING_PROVE(&map.function() == &map.update_function());
            DifferenceAlgebraicEquation* copy = map.clone();
            ARIADNE_TESTING_PROVE(copy != nullptr);
            ARIADNE_TESTING_EXPECT_EQUAL(copy->dimension(), 2u);
            delete copy;
            ARIADNE_TESTING_NOTE(map);
        }
        {
            DifferenceAlgebraicEquation map(List<PrimedRealAssignment>{prime(x) = x + a},
                            List<RealAssignment>{let(a) = x + 1});
            ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 1u);
            ARIADNE_TESTING_PROVE(map.auxiliary_space() == RealSpace({a}));
            ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 2u);
            ARIADNE_TESTING_NOTE(map);
        }
        {
            // Chained auxiliary assignments exercise the sorting loop.
            RealVariable b("b");
            DifferenceAlgebraicEquation map(List<PrimedRealAssignment>{prime(x) = x + a},
                            List<RealAssignment>{let(a) = x, let(b) = a + 1});
            ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 1u);
            ARIADNE_TESTING_EXPECT_EQUAL(map.auxiliary_space().size(), 2u);
            ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 3u);
            ARIADNE_TESTING_NOTE(map);
        }
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_difference_algebraic_equation");
    ARIADNE_TESTING_ADD_TEST(TestDifferenceAlgebraicEquation, test_difference_algebraic_equation);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
