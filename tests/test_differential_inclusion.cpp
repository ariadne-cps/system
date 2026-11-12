/***************************************************************************
 *            test_differential_inclusion.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Threading.
 *
 *  Threading is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Threading is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Threading.  If not, see <https://www.gnu.org/licenses/>.
 */

/** @file tests/test_differential_inclusion.cpp
 *  @brief Test differential inclusion behavior.
 */

// Exercises DifferentialInclusion: construction (with symmetric and
// asymmetric input bounds, additive/affine and non-additive inputs), all
// accessors, clone, streaming, and the input-validation errors.

#include <sstream>

#include "paving/grid.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"
#include "system/differential_inclusion.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestDifferentialInclusion : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

  public:
    Void test_accessors();
    Void test_input_kinds();
    Void test_errors();
};

Void TestDifferentialInclusion::test_accessors() {
    RealVariable x("x"), y("y"), u("u");
    DottedRealAssignments dynamics = {dot(x) = -y, dot(y) = x + 2 * u};
    RealVariablesBox inputs = {1 <= u <= 2}; // asymmetric bounds

    DifferentialInclusion inclusion(dynamics, inputs);
    ARIADNE_TESTING_EXPECT_EQUAL(inclusion.dimension(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(inclusion.number_of_inputs(), 1u);
    ARIADNE_TESTING_PROVE(inclusion.state_space() == RealSpace({x, y}));
    ARIADNE_TESTING_EXPECT_EQUAL(inclusion.grid().dimension(), 2u);
    ARIADNE_TESTING_PROVE(inclusion.is_input_affine());
    ARIADNE_TESTING_PROVE(inclusion.is_input_additive());
    ARIADNE_TESTING_NOTE(inclusion.function());
    ARIADNE_TESTING_NOTE(inclusion.inputs());
    ARIADNE_TESTING_NOTE(inclusion.noise_independent_component());
    Vector<EffectiveVectorMultivariateFunction> derivatives = inclusion.input_derivatives();
    ARIADNE_TESTING_EXPECT_EQUAL(derivatives.size(), 1u);
    DifferentialInclusion* copy = inclusion.clone();
    ARIADNE_TESTING_PROVE(copy != nullptr);
    ARIADNE_TESTING_EXPECT_EQUAL(copy->dimension(), 2u);
    delete copy;
    std::ostringstream os;
    os << inclusion;
    ARIADNE_TESTING_PROVE(os.str().size() > 0u);
}

Void TestDifferentialInclusion::test_input_kinds() {
    RealVariable x("x"), u("u");
    // Symmetric input bounds take the symmetric centering branch.
    DifferentialInclusion symmetric({dot(x) = x + u}, RealVariablesBox({-1 <= u <= 1}));
    ARIADNE_TESTING_PROVE(symmetric.is_input_additive());
    // Unit input coefficient takes the constant-derivative branch.
    DifferentialInclusion unit({dot(x) = x + u}, RealVariablesBox({1 <= u <= 2}));
    ARIADNE_TESTING_PROVE(unit.is_input_affine());
    // A product with the input is not additive.
    DifferentialInclusion product({dot(x) = x * u}, RealVariablesBox({1 <= u <= 2}));
    ARIADNE_TESTING_PROVE(!product.is_input_additive());
    ARIADNE_TESTING_PROVE(product.is_input_affine());
}

Void TestDifferentialInclusion::test_errors() {
    RealVariable x("x"), z("z"), u("u");
    // An input that does not occur in the dynamics is unused.
    ARIADNE_TESTING_EXPECT_THROWS(UnusedInputException, DifferentialInclusion({dot(x) = x}, RealVariablesBox({1 <= u <= 2})));
    // A dynamics argument that is neither a state variable nor an input
    // (the declared input is used, so the unused-input check passes first).
    ARIADNE_TESTING_EXPECT_THROWS(MissingInputException, DifferentialInclusion({dot(x) = x + u + z}, RealVariablesBox({1 <= u <= 2})));
}

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_differential_inclusion");
    ARIADNE_TESTING_ADD_TEST(TestDifferentialInclusion, test_accessors);
    ARIADNE_TESTING_ADD_TEST(TestDifferentialInclusion, test_input_kinds);
    ARIADNE_TESTING_ADD_TEST(TestDifferentialInclusion, test_errors);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}
