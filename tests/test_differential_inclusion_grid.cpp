// Regression for DifferentialInclusion::grid() in the standalone System layer.
// Compiles the method's return type and links its implementation with Paving.
#include "system/differential_inclusion.hpp"
#include "paving/grid.hpp"
#include "symbolic/assignment.hpp"
#include "expression-set/expression_set.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;


class TestDifferentialInclusionGrid : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;
    Void test_grid() {
    RealVariable x("x"), y("y"), u("u");
    DottedRealAssignments dynamics={dot(x)=-y,dot(y)=x+2*u};
    RealVariablesBox inputs={1<=u<=2};

    DifferentialInclusion inclusion(dynamics,inputs);
    ARIADNE_TESTING_EXPECT_EQUAL(inclusion.dimension(),2u);
    ARIADNE_TESTING_EXPECT_EQUAL(inclusion.grid().dimension(),2u);
    
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_differential_inclusion_grid");
    ARIADNE_TESTING_ADD_TEST(TestDifferentialInclusionGrid, test_grid);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}
