/***************************************************************************
 *            test_dynamics.cpp
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

/** @file tests/test_dynamics.cpp
 *  @brief Test dynamics behavior.
 */

#include "ariadne-testing.hpp"

#include "symbolic/variable.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

#include "system/iterated_map.hpp"
#include "system/vector_field.hpp"

using namespace Ariadne;

class TestDynamics : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;


Void test_iterated_map()
{
    RealVariable x("x"), y("y"), a("a");
    {
        IteratedMap map(List<PrimedRealAssignment>{prime(x)=x+1, prime(y)=y});
        ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 2u);
        ARIADNE_TESTING_PROVE(map.state_space() == RealSpace({x,y}));
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
        IteratedMap map(List<PrimedRealAssignment>{prime(x)=x+a},
                        List<RealAssignment>{let(a)=x+1});
        ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 1u);
        ARIADNE_TESTING_PROVE(map.auxiliary_space() == RealSpace({a}));
        ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 2u);
        ARIADNE_TESTING_NOTE(map);
    }
    {
        // Chained auxiliary assignments exercise the sorting loop.
        RealVariable b("b");
        IteratedMap map(List<PrimedRealAssignment>{prime(x)=x+a},
                        List<RealAssignment>{let(a)=x, let(b)=a+1});
        ARIADNE_TESTING_EXPECT_EQUAL(map.dimension(), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(map.auxiliary_space().size(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(map.state_auxiliary_space().size(), 3u);
        ARIADNE_TESTING_NOTE(map);
    }
}

Void test_vector_field()
{
    RealVariable x("x"), y("y"), a("a");
    {
        VectorField field(List<DottedRealAssignment>{dot(x)=-x, dot(y)=y});
        ARIADNE_TESTING_EXPECT_EQUAL(field.dimension(), 2u);
        ARIADNE_TESTING_PROVE(field.state_space() == RealSpace({x,y}));
        ARIADNE_TESTING_EXPECT_EQUAL(field.auxiliary_space().size(), 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(field.state_auxiliary_space().size(), 2u);
        ARIADNE_TESTING_PROVE(&field.function() == &field.dynamic_function());
        ARIADNE_TESTING_PROVE(&field.auxiliary_function() == &field.auxiliary_mapping());
        VectorField* copy = field.clone();
        ARIADNE_TESTING_PROVE(copy != nullptr);
        ARIADNE_TESTING_EXPECT_EQUAL(copy->dimension(), 2u);
        delete copy;
        ARIADNE_TESTING_NOTE(field);
    }
    {
        VectorField field(List<DottedRealAssignment>{dot(x)=-x},
                          List<RealAssignment>{let(a)=x});
        ARIADNE_TESTING_PROVE(field.auxiliary_space() == RealSpace({a}));
        ARIADNE_TESTING_EXPECT_EQUAL(field.state_auxiliary_space().size(), 2u);
        ARIADNE_TESTING_NOTE(field);
    }
    // A right-hand-side variable without a dynamics assignment is rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, VectorField(List<DottedRealAssignment>{dot(x)=y}));
}


};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_dynamics");
    ARIADNE_TESTING_ADD_TEST(TestDynamics, test_iterated_map);
    ARIADNE_TESTING_ADD_TEST(TestDynamics, test_vector_field);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}
