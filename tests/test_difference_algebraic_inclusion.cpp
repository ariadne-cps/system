/***************************************************************************
 *            test_difference_algebraic_inclusion.cpp
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

/** @file tests/test_difference_algebraic_inclusion.cpp
 *  @brief Contract tests for structured discrete-time set-valued dynamics.
 */

#include <memory>
#include <stdexcept>

#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/variable.hpp"
#include "system/difference_algebraic_inclusion.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestDifferenceAlgebraicInclusion : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_bounded_input_semantics() {
        const RealVariable x("x"), y("y"), u("u"), a("a");
        const RealVariablesBox bounds({-1 <= u <= 1});
        const DifferenceAlgebraicInclusion model(
            List<PrimedRealAssignment>{next(x) = x + a + u, next(y) = y + 2 * u},
            List<RealAssignment>{let(a) = 2 * x}, bounds);
        ARIADNE_TESTING_EXPECT_EQUAL(model.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(model.number_of_inputs(), 1u);
        ARIADNE_TESTING_PROVE(model.state_space() == RealSpace({x, y}));
        ARIADNE_TESTING_PROVE(model.auxiliary_space() == RealSpace({a}));
        ARIADNE_TESTING_PROVE(model.input_space() == RealSpace({u}));
        ARIADNE_TESTING_PROVE(model.argument_space() == RealSpace({x, y, u}));
        ARIADNE_TESTING_EXPECT_EQUAL(model.updates().size(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(model.auxiliary_assignments().size(), 1u);
        ARIADNE_TESTING_PROVE(model.inputs() == BoxDomainType({{-1.0_x, 1.0_x}}));
        ARIADNE_TESTING_EXPECT_EQUAL(model.function().argument_size(), 3u);
        ARIADNE_TESTING_EXPECT_EQUAL(model.function().result_size(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(model.auxiliary_function().result_size(), 1u);

        std::unique_ptr<DifferenceAlgebraicInclusion> cloned(model.clone());
        ARIADNE_TESTING_PROVE(cloned->input_space() == RealSpace({u}));
        ARIADNE_TESTING_PROVE(cloned->inputs() == model.inputs());
    }

    Void test_invalid_declarations() {
        const RealVariable x("x"), y("y"), u("u"), v("v"), a("a");
        const RealVariablesBox inputs({-1 <= u <= 1});
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
            DifferenceAlgebraicInclusion(List<PrimedRealAssignment>{next(x) = x + u},
                                         RealVariablesBox()));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
            DifferenceAlgebraicInclusion(List<PrimedRealAssignment>{next(x) = x},
                                         inputs));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
            DifferenceAlgebraicInclusion(List<PrimedRealAssignment>{next(x) = x + u + v},
                                         inputs));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
            DifferenceAlgebraicInclusion(List<PrimedRealAssignment>{next(x) = x + u, next(x) = x + u},
                                         inputs));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
            DifferenceAlgebraicInclusion(List<PrimedRealAssignment>{next(x) = x + a + u},
                                         List<RealAssignment>{let(a) = x, let(a) = x + 1},
                                         inputs));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
            DifferenceAlgebraicInclusion(List<PrimedRealAssignment>{next(x) = x + u},
                                         List<RealAssignment>{let(a) = y},
                                         inputs));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_difference_algebraic_inclusion");
    ARIADNE_TESTING_ADD_TEST(TestDifferenceAlgebraicInclusion, test_bounded_input_semantics);
    ARIADNE_TESTING_ADD_TEST(TestDifferenceAlgebraicInclusion, test_invalid_declarations);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
