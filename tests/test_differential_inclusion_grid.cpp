// Regression for DifferentialInclusion::grid() in the standalone System layer.
// Compiles the method's return type and links its implementation with Paving.
#include "system/differential_inclusion.hpp"
#include "paving/grid.hpp"
#include "symbolic/assignment.hpp"
#include "expression-set/expression_set.hpp"
#include "macro/testing.hpp"

using namespace Ariadne;

int main() {
    RealVariable x("x"), y("y"), u("u");
    DottedRealAssignments dynamics={dot(x)=-y,dot(y)=x+2*u};
    RealVariablesBox inputs={1<=u<=2};

    DifferentialInclusion inclusion(dynamics,inputs);
    ARIADNE_TEST_EQUALS(inclusion.dimension(),2u);
    ARIADNE_TEST_EQUALS(inclusion.grid().dimension(),2u);
    return ARIADNE_TEST_FAILURES;
}
