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

// Exercises DifferentialInclusion: construction (with symmetric and
// asymmetric input bounds, additive/affine and non-additive inputs), all
// accessors, clone, streaming, and the input-validation errors.

#include <sstream>

#include "macro/testing.hpp"

#include "symbolic/assignment.hpp"
#include "symbolic/variable.hpp"
#include "symbolic/space.hpp"

#include "paving/grid.hpp"

#include "system/differential_inclusion.hpp"

using namespace Ariadne;

class TestDifferentialInclusion {
  public:
    Void test();
  private:
    Void test_accessors();
    Void test_input_kinds();
    Void test_errors();
};

Void TestDifferentialInclusion::test_accessors()
{
    RealVariable x("x"), y("y"), u("u");
    DottedRealAssignments dynamics={dot(x)=-y, dot(y)=x+2*u};
    RealVariablesBox inputs={1<=u<=2};  // asymmetric bounds

    DifferentialInclusion inclusion(dynamics, inputs);
    ARIADNE_TEST_EQUALS(inclusion.dimension(), 2u);
    ARIADNE_TEST_EQUALS(inclusion.number_of_inputs(), 1u);
    ARIADNE_TEST_ASSERT(inclusion.state_space()==RealSpace({x,y}));
    ARIADNE_TEST_EQUALS(inclusion.grid().dimension(), 2u);
    ARIADNE_TEST_ASSERT(inclusion.is_input_affine());
    ARIADNE_TEST_ASSERT(inclusion.is_input_additive());
    ARIADNE_TEST_PRINT(inclusion.function());
    ARIADNE_TEST_PRINT(inclusion.inputs());
    ARIADNE_TEST_PRINT(inclusion.noise_independent_component());
    Vector<EffectiveVectorMultivariateFunction> derivatives=inclusion.input_derivatives();
    ARIADNE_TEST_EQUALS(derivatives.size(), 1u);
    DifferentialInclusion* copy=inclusion.clone();
    ARIADNE_TEST_ASSERT(copy!=nullptr);
    ARIADNE_TEST_EQUALS(copy->dimension(), 2u);
    delete copy;
    std::ostringstream os;
    os << inclusion;
    ARIADNE_TEST_ASSERT(os.str().size()>0u);
}

Void TestDifferentialInclusion::test_input_kinds()
{
    RealVariable x("x"), u("u");
    // Symmetric input bounds take the symmetric centering branch.
    DifferentialInclusion symmetric({dot(x)=x+u}, RealVariablesBox({-1<=u<=1}));
    ARIADNE_TEST_ASSERT(symmetric.is_input_additive());
    // Unit input coefficient takes the constant-derivative branch.
    DifferentialInclusion unit({dot(x)=x+u}, RealVariablesBox({1<=u<=2}));
    ARIADNE_TEST_ASSERT(unit.is_input_affine());
    // A product with the input is not additive.
    DifferentialInclusion product({dot(x)=x*u}, RealVariablesBox({1<=u<=2}));
    ARIADNE_TEST_ASSERT(!product.is_input_additive());
    ARIADNE_TEST_ASSERT(product.is_input_affine());
}

Void TestDifferentialInclusion::test_errors()
{
    RealVariable x("x"), z("z"), u("u");
    // An input that does not occur in the dynamics is unused.
    ARIADNE_TEST_THROWS(
        DifferentialInclusion({dot(x)=x}, RealVariablesBox({1<=u<=2})),
        UnusedInputException);
    // A dynamics argument that is neither a state variable nor an input
    // (the declared input is used, so the unused-input check passes first).
    ARIADNE_TEST_THROWS(
        DifferentialInclusion({dot(x)=x+u+z}, RealVariablesBox({1<=u<=2})),
        MissingInputException);
}

Void TestDifferentialInclusion::test()
{
    ARIADNE_TEST_CALL(test_accessors());
    ARIADNE_TEST_CALL(test_input_kinds());
    ARIADNE_TEST_CALL(test_errors());
}

Int main()
{
    ARIADNE_TEST_CALL(TestDifferentialInclusion().test());
    return ARIADNE_TEST_FAILURES;
}
