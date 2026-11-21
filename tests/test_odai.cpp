/***************************************************************************
 *            test_odai.cpp
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

/** @file tests/test_odai.cpp
 *  @brief Test differential inclusion behavior.
 */

// Exercises OrdinaryDifferentialAlgebraicInclusion: construction (with symmetric and
// asymmetric input bounds, additive/affine and non-additive inputs), all
// accessors, clone, streaming, and the input-validation errors.

#include <sstream>

#include "logging/test_runner.hpp"
#include "paving/grid.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"
#include "system/odai.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestOrdinaryDifferentialAlgebraicInclusion : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

  public:
    Void test_accessors();
    Void test_input_kinds();
    Void test_errors();
    Void test_time_and_constant_parameters();
    Void test_owned_interval_taylor_multifunction();
};

Void TestOrdinaryDifferentialAlgebraicInclusion::test_accessors() {
    RealVariable x("x"), y("y"), u("u");
    DottedRealAssignments dynamics = {dot(x) = -y, dot(y) = x + 2 * u};
    RealVariablesBox inputs = {1 <= u <= 2}; // asymmetric bounds

    OrdinaryDifferentialAlgebraicInclusion inclusion(dynamics, inputs);
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
    OrdinaryDifferentialAlgebraicInclusion* copy = inclusion.clone();
    ARIADNE_TESTING_PROVE(copy != nullptr);
    ARIADNE_TESTING_EXPECT_EQUAL(copy->dimension(), 2u);
    delete copy;
    std::ostringstream os;
    os << inclusion;
    ARIADNE_TESTING_PROVE(os.str().size() > 0u);
}

Void TestOrdinaryDifferentialAlgebraicInclusion::test_input_kinds() {
    RealVariable x("x"), u("u");
    // Symmetric input bounds take the symmetric centering branch.
    OrdinaryDifferentialAlgebraicInclusion symmetric({dot(x) = x + u}, RealVariablesBox({-1 <= u <= 1}));
    ARIADNE_TESTING_PROVE(symmetric.is_input_additive());
    // Unit input coefficient takes the constant-derivative branch.
    OrdinaryDifferentialAlgebraicInclusion unit({dot(x) = x + u}, RealVariablesBox({1 <= u <= 2}));
    ARIADNE_TESTING_PROVE(unit.is_input_affine());
    // A product with the input is not additive.
    OrdinaryDifferentialAlgebraicInclusion product({dot(x) = x * u}, RealVariablesBox({1 <= u <= 2}));
    ARIADNE_TESTING_PROVE(!product.is_input_additive());
    ARIADNE_TESTING_PROVE(product.is_input_affine());
}

Void TestOrdinaryDifferentialAlgebraicInclusion::test_errors() {
    RealVariable x("x"), z("z"), u("u");
    // An input that does not occur in the dynamics is unused.
    ARIADNE_TESTING_EXPECT_THROWS(UnusedInputException, OrdinaryDifferentialAlgebraicInclusion({dot(x) = x}, RealVariablesBox({1 <= u <= 2})));
    // A dynamics argument that is neither a state variable nor an input
    // (the declared input is used, so the unused-input check passes first).
    ARIADNE_TESTING_EXPECT_THROWS(MissingInputException, OrdinaryDifferentialAlgebraicInclusion({dot(x) = x + u + z}, RealVariablesBox({1 <= u <= 2})));
}

Void TestOrdinaryDifferentialAlgebraicInclusion::test_time_and_constant_parameters() {
    RealVariable x("x"), t("t"), p("p"), u("u");
    const OrdinaryDifferentialAlgebraicInclusion inclusion(
        {dot(x) = t + p + 2 * u}, RealVariablesBox({1 <= u <= 2}),
        RealVariablesBox({Real(0.125_x) <= p <= Real(0.25_x)}), t);
    ARIADNE_TESTING_EXPECT_EQUAL(inclusion.dimension(), 1u);
    ARIADNE_TESTING_PROVE(inclusion.state_space() == RealSpace({x}));
    ARIADNE_TESTING_PROVE(inclusion.argument_space() == RealSpace({x, t, p, u}));
    ARIADNE_TESTING_PROVE(inclusion.has_explicit_time());
    ARIADNE_TESTING_PROVE(inclusion.is_input_additive());
    ARIADNE_TESTING_PROVE(inclusion.constant_parameter_domain() == BoxDomainType({
                                                                       {0.125_x, 0.25_x}
    }));
    // Centering u in [1,2] and absorbing its coefficient gives u' in [-1,1].
    // Time and p must not be centred, scaled or differentiated as inputs.
    ARIADNE_TESTING_PROVE(inclusion.inputs() == BoxDomainType({
                                                    {-1.0_x, 1.0_x}
    }));
    const Vector<FloatDPBounds> point({FloatDPBounds(0, dp), FloatDPBounds(0.25_x, dp),
                                       FloatDPBounds(0.125_x, dp), FloatDPBounds(0, dp)});
    ARIADNE_TESTING_PROVE(refines(FloatDPBounds(3.375_x, dp), inclusion.function()(point)[0u]));
    ARIADNE_TESTING_PROVE(refines(FloatDPBounds(3.375_x, dp), inclusion.noise_independent_component()(point)[0u]));
    const auto derivatives = inclusion.input_derivatives();
    ARIADNE_TESTING_EXPECT_EQUAL(derivatives.size(), 1u);
    ARIADNE_TESTING_PROVE(refines(FloatDPBounds(1, dp), derivatives[0u](point)[0u]));
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
                                  OrdinaryDifferentialAlgebraicInclusion({dot(x) = u}, RealVariablesBox({0 <= u <= 1}), RealVariablesBox(), x));
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
                                  OrdinaryDifferentialAlgebraicInclusion({dot(x) = u}, RealVariablesBox({0 <= u <= 1}), RealVariablesBox({0 <= u <= 1})));
}

Void TestOrdinaryDifferentialAlgebraicInclusion::test_owned_interval_taylor_multifunction() {
    RealVariable x("x"), y("y");
    const BoxDomainType domain({
        {-1.0_x, 1.0_x}
    });
    const auto inclusion = [&] {
        const ThresholdSweeper<FloatDP> sweeper(dp, 1e-10);
        const auto uncertain = ValidatedIntervalTaylorFunctionModel<FloatDP>::constant(
            domain, FloatDPUpperInterval(-1, 1, dp), sweeper);
        const ValidatedVectorIntervalTaylorFunctionModel<FloatDP> field({uncertain});
        return OrdinaryDifferentialAlgebraicInclusion(RealSpace({x}), field);
    }();
    ARIADNE_TESTING_EXPECT_EQUAL(inclusion.dimension(), 1u);
    ARIADNE_TESTING_PROVE(not inclusion.has_input_function());
    ARIADNE_TESTING_PROVE(not inclusion.has_explicit_time());
    ARIADNE_TESTING_PROVE(inclusion.interval_taylor_multifunction().domain() == domain);
    const Vector<FloatDPBounds> point({FloatDPBounds(0, dp)});
    ARIADNE_TESTING_PROVE(definitely(subset(BoxDomainType({
                                                {-1.0_x, 1.0_x}
    }),
                                            cast_exact_box(inclusion.interval_taylor_multifunction()(point)))));
    const auto view = inclusion.multifunction();
    const auto value = view(Vector<ValidatedNumber>{ValidatedNumber(0)});
    ARIADNE_TESTING_EXPECT_EQUAL(value.dimension(), 1u);
    ARIADNE_TESTING_EXPECT_THROWS(std::logic_error, inclusion.function());
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
                                  OrdinaryDifferentialAlgebraicInclusion(RealSpace({x, y}), inclusion.interval_taylor_multifunction()));
    ARIADNE_TESTING_EXPECT_THROWS(std::logic_error,
                                  OrdinaryDifferentialAlgebraicInclusion({dot(x) = y}, RealVariablesBox({-1 <= y <= 1})).interval_taylor_multifunction());
}

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_odai");
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicInclusion, test_accessors);
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicInclusion, test_input_kinds);
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicInclusion, test_errors);
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicInclusion, test_time_and_constant_parameters);
    ARIADNE_TESTING_ADD_TEST(TestOrdinaryDifferentialAlgebraicInclusion, test_owned_interval_taylor_multifunction);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
