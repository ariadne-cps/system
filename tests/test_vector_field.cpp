/***************************************************************************
 *            tests/test_vector_field.cpp
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

/** @file tests/test_vector_field.cpp
 *  @brief Test vector field construction and auxiliary assignments.
 */

#include <stdexcept>

#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"
#include "system/vector_field.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestVectorField : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_vector_field() {
        RealVariable x("x"), y("y"), a("a");
        {
            VectorField field(List<DottedRealAssignment>{dot(x) = -x, dot(y) = y});
            ARIADNE_TESTING_EXPECT_EQUAL(field.dimension(), 2u);
            ARIADNE_TESTING_PROVE(field.state_space() == RealSpace({x, y}));
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
            VectorField field(List<DottedRealAssignment>{dot(x) = -x},
                              List<RealAssignment>{let(a) = x});
            ARIADNE_TESTING_PROVE(field.auxiliary_space() == RealSpace({a}));
            ARIADNE_TESTING_EXPECT_EQUAL(field.state_auxiliary_space().size(), 2u);
            ARIADNE_TESTING_NOTE(field);
        }
        // A right-hand-side variable without a dynamics assignment is rejected.
        ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, VectorField(List<DottedRealAssignment>{dot(x) = y}));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_vector_field");
    ARIADNE_TESTING_ADD_TEST(TestVectorField, test_vector_field);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
