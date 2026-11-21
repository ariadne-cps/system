/***************************************************************************
 *            tests/test_odae.cpp
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

/** @file tests/test_odae.cpp
 *  @brief Test vector field construction and auxiliary assignments.
 */

#include <stdexcept>

#include "function/taylor_model.hpp"
#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"
#include "system/odae.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestOrdinaryDifferentialAlgebraicEquation : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_odae() {
        RealVariable x("x"), y("y"), a("a");
        {
            OrdinaryDifferentialAlgebraicEquation field(List<DottedRealAssignment>{dot(x) = -x, dot(y) = y});
            ARIADNE_TESTING_EXPECT_EQUAL(field.dimension(), 2u);
            ARIADNE_TESTING_PROVE(field.state_space() == RealSpace({x, y}));
            ARIADNE_TESTING_EXPECT_EQUAL(field.auxiliary_space().size(), 0u);
            ARIADNE_TESTING_EXPECT_EQUAL(field.state_auxiliary_space().size(), 2u);
            ARIADNE_TESTING_EXPECT_EQUAL(field.function().argument_size(), 2u);
            ARIADNE_TESTING_EXPECT_EQUAL(field.auxiliary_function().result_size(), 0u);
            OrdinaryDifferentialAlgebraicEquation* copy = field.clone();
            ARIADNE_TESTING_PROVE(copy != nullptr);
            ARIADNE_TESTING_EXPECT_EQUAL(copy->dimension(), 2u);
            delete copy;
            ARIADNE_TESTING_NOTE(field);
        }
        {
            OrdinaryDifferentialAlgebraicEquation field(List<DottedRealAssignment>{dot(x) = -x},
                                                        List<RealAssignment>{let(a) = x});
            ARIADNE_TESTING_PROVE(field.auxiliary_space() == RealSpace({a}));
            ARIADNE_TESTING_EXPECT_EQUAL(field.state_auxiliary_space().size(), 2u);
            ARIADNE_TESTING_NOTE(field);
        }
        // A right-hand-side variable without a dynamics assignment is rejected.
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, OrdinaryDifferentialAlgebraicEquation(List<DottedRealAssignment>{dot(x) = y}));
    }
    Void test_time_parameters_and_explicit_auxiliary_output() {
        RealVariable x("x"), t("t"), p("p"), a("a");
        const RealVariablesBox parameters({Real(0.125_x) <= p <= Real(0.25_x)});
        const OrdinaryDifferentialAlgebraicEquation equation(
            {dot(x) = a}, parameters, t, {let(a) = x + t + p});
        ARIADNE_TESTING_EXPECT_EQUAL(equation.dimension(), 1u);
        ARIADNE_TESTING_PROVE(equation.state_space() == RealSpace({x}));
        ARIADNE_TESTING_PROVE(equation.argument_space() == RealSpace({x, t, p}));
        ARIADNE_TESTING_PROVE(equation.parameter_space() == RealSpace({p}));
        ARIADNE_TESTING_PROVE(equation.has_explicit_time());
        ARIADNE_TESTING_PROVE(equation.auxiliary_space() == RealSpace({a}));
        const Vector<FloatDPBounds> point({FloatDPBounds(0.5_x, dp), FloatDPBounds(0.25_x, dp), FloatDPBounds(0.125_x, dp)});
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(0.875_x, dp), equation.function()(point)[0u]));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(0.875_x, dp), equation.auxiliary_function()(point)[0u]));
        RealVariable b("b");
        const OrdinaryDifferentialAlgebraicEquation chained(
            {dot(x) = b}, parameters, t, {let(b) = a + p, let(a) = x + t});
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(0.875_x, dp), chained.function()(point)[0u]));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(0.875_x, dp), chained.auxiliary_function()(point)[0u]));
        const OrdinaryDifferentialAlgebraicEquation autonomous({dot(x) = x + p}, parameters);
        ARIADNE_TESTING_PROVE(not autonomous.has_explicit_time());
        ARIADNE_TESTING_PROVE(autonomous.argument_space() == RealSpace({x, p}));
        ARIADNE_TESTING_EXPECT_EQUAL(autonomous.dimension(), 1u);
    }

    Void test_reject_conflicting_variable_roles() {
        RealVariable x("x"), p("p"), a("a");
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
                                      OrdinaryDifferentialAlgebraicEquation({dot(x) = x}, RealVariablesBox(), x));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
                                      OrdinaryDifferentialAlgebraicEquation({dot(x) = x}, RealVariablesBox({0 <= x <= 1})));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
                                      OrdinaryDifferentialAlgebraicEquation({dot(x) = a}, RealVariablesBox(), std::nullopt, {let(a) = p}));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_odae");
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicEquation, test_odae);
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicEquation, test_time_parameters_and_explicit_auxiliary_output);
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicEquation, test_reject_conflicting_variable_roles);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
