/***************************************************************************
 *            test_hybrid_mode_structure.cpp
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

/** @file tests/test_hybrid_mode_structure.cpp
 *  @brief Test time-independent hybrid mode location and transition topology.
 */

#include "logging/test_runner.hpp"
#include "system/hybrid_mode_structure.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestHybridModeStructure : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_transition_topology() {
        const DiscreteLocation source(1);
        const DiscreteLocation target(2);
        const DiscreteEvent event("edge");
        const DiscreteEvent absent("absent");

        HybridModeStructure mode(source);
        ARIADNE_TESTING_EXPECT_EQUAL(mode.location(), source);
        ARIADNE_TESTING_PROVE(!mode.has_transition(event));
        ARIADNE_TESTING_EXPECT_EQUAL(mode.transition_target(absent, source), source);

        mode.add_transition(event, target);
        ARIADNE_TESTING_PROVE(mode.has_transition(event));
        ARIADNE_TESTING_PROVE(mode.transition_events().contains(event));
        ARIADNE_TESTING_EXPECT_EQUAL(mode.transition_target(event, source), target);
        ARIADNE_TESTING_EXPECT_EQUAL(mode.transition_target(absent, source), source);

        HybridModeStructure combined;
        combined.adjoin_structure(mode);
        ARIADNE_TESTING_EXPECT_EQUAL(combined.location(), source);
        ARIADNE_TESTING_EXPECT_EQUAL(combined.transition_target(event, source), target);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_mode_structure");
    ARIADNE_TESTING_ADD_TEST(TestHybridModeStructure, test_transition_topology);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
