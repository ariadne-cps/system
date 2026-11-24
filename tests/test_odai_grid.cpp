/***************************************************************************
 *            tests/test_odai_grid.cpp
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

/** @file tests/test_odai_grid.cpp
 *  @brief Test differential inclusion grid behavior.
 */

// Regression for OrdinaryDifferentialAlgebraicInclusion::grid() in the standalone System layer.
// Compiles the method's return type and links its implementation with Paving.
#include "logging/test_runner.hpp"
#include "paving/grid.hpp"
#include "symbolic/assignment.hpp"
#include "system/odai.hpp"

#include "ariadne-testing.hpp"
#include "expression-set/expression_set.hpp"

using namespace Ariadne;

class TestOrdinaryDifferentialAlgebraicInclusionGrid : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;
    Void test_grid() {
        RealVariable x("x"), y("y"), u("u");
        DottedRealAssignments dynamics = {dot(x) = -y, dot(y) = x + 2 * u};
        RealVariablesBox inputs = {1 <= u <= 2};

        OrdinaryDifferentialAlgebraicInclusion inclusion(dynamics, inputs);
        ARIADNE_TESTING_EXPECT_EQUAL(inclusion.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(inclusion.grid().dimension(), 2u);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_odai_grid");
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicInclusionGrid, test_grid);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
