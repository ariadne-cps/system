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

#include "macro/testing.hpp"

#include "symbolic/variable.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

#include "system/iterated_map.hpp"
#include "system/vector_field.hpp"

using namespace Ariadne;

Void test_iterated_map()
{
    RealVariable x("x"), y("y"), a("a");
    {
        IteratedMap map(List<PrimedRealAssignment>{prime(x)=x+1, prime(y)=y});
        ARIADNE_TEST_EQUALS(map.dimension(), 2u);
        ARIADNE_TEST_ASSERT(map.state_space() == RealSpace({x,y}));
        ARIADNE_TEST_EQUALS(map.auxiliary_space().size(), 0u);
        ARIADNE_TEST_EQUALS(map.state_auxiliary_space().size(), 2u);
        ARIADNE_TEST_ASSERT(&map.function() == &map.update_function());
        ARIADNE_TEST_ASSERT(&map.auxiliary_function() == &map.auxiliary_mapping());
        IteratedMap* copy = map.clone();
        ARIADNE_TEST_ASSERT(copy != nullptr);
        ARIADNE_TEST_EQUALS(copy->dimension(), 2u);
        delete copy;
        ARIADNE_TEST_PRINT(map);
    }
    {
        IteratedMap map(List<PrimedRealAssignment>{prime(x)=x+a},
                        List<RealAssignment>{let(a)=x+1});
        ARIADNE_TEST_EQUALS(map.dimension(), 1u);
        ARIADNE_TEST_ASSERT(map.auxiliary_space() == RealSpace({a}));
        ARIADNE_TEST_EQUALS(map.state_auxiliary_space().size(), 2u);
        ARIADNE_TEST_PRINT(map);
    }
    {
        // Chained auxiliary assignments exercise the sorting loop.
        RealVariable b("b");
        IteratedMap map(List<PrimedRealAssignment>{prime(x)=x+a},
                        List<RealAssignment>{let(a)=x, let(b)=a+1});
        ARIADNE_TEST_EQUALS(map.dimension(), 1u);
        ARIADNE_TEST_EQUALS(map.auxiliary_space().size(), 2u);
        ARIADNE_TEST_EQUALS(map.state_auxiliary_space().size(), 3u);
        ARIADNE_TEST_PRINT(map);
    }
}

Void test_vector_field()
{
    RealVariable x("x"), y("y"), a("a");
    {
        VectorField field(List<DottedRealAssignment>{dot(x)=-x, dot(y)=y});
        ARIADNE_TEST_EQUALS(field.dimension(), 2u);
        ARIADNE_TEST_ASSERT(field.state_space() == RealSpace({x,y}));
        ARIADNE_TEST_EQUALS(field.auxiliary_space().size(), 0u);
        ARIADNE_TEST_EQUALS(field.state_auxiliary_space().size(), 2u);
        ARIADNE_TEST_ASSERT(&field.function() == &field.dynamic_function());
        ARIADNE_TEST_ASSERT(&field.auxiliary_function() == &field.auxiliary_mapping());
        VectorField* copy = field.clone();
        ARIADNE_TEST_ASSERT(copy != nullptr);
        ARIADNE_TEST_EQUALS(copy->dimension(), 2u);
        delete copy;
        ARIADNE_TEST_PRINT(field);
    }
    {
        VectorField field(List<DottedRealAssignment>{dot(x)=-x},
                          List<RealAssignment>{let(a)=x});
        ARIADNE_TEST_ASSERT(field.auxiliary_space() == RealSpace({a}));
        ARIADNE_TEST_EQUALS(field.state_auxiliary_space().size(), 2u);
        ARIADNE_TEST_PRINT(field);
    }
    // A right-hand-side variable without a dynamics assignment is rejected.
    ARIADNE_TEST_THROWS(VectorField(List<DottedRealAssignment>{dot(x)=y}),
                        std::runtime_error);
}

Int main()
{
    ARIADNE_TEST_CALL(test_iterated_map());
    ARIADNE_TEST_CALL(test_vector_field());
    return ARIADNE_TEST_FAILURES;
}
