/***************************************************************************
 *            tests/test_hybrid_provider_integration.cpp
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

/** @file tests/test_hybrid_provider_integration.cpp
 *  @brief Verify System state spaces against the decoupled generic Hybrid value families.
 */

#include "hybrid/hybrid_space.hpp"
#include "hybrid/value.hpp"
#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "system/hybrid_automaton.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestHybridProviderIntegration : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_system_spaces_and_lazy_values() {
        const RealVariable x("x"), y("y");
        const DiscreteLocation first(1), second(2);
        HybridAutomaton automaton;
        automaton.new_mode(first, List<DottedRealAssignment>{dot(x) = -x});
        automaton.new_mode(second, List<DottedRealAssignment>{dot(y) = -y});
        const HybridSpace spaces = automaton.state_space();
        ARIADNE_TESTING_PROVE(spaces.has_location(first));
        ARIADNE_TESTING_PROVE(spaces.has_location(second));

        Hybrid<RealSpace> family(spaces,
                                 [](const DiscreteLocation&, const RealSpace& space) { return space; });
        ARIADNE_TESTING_PROVE(family.has_location(first));
        ARIADNE_TESTING_PROVE(not family.has_value(first));
        ARIADNE_TESTING_PROVE(not family.has_value(second));
        ARIADNE_TESTING_PROVE(family[first].dimension() == 1u);
        ARIADNE_TESTING_PROVE(family.has_value(first));
        ARIADNE_TESTING_PROVE(family[second].dimension() == 1u);
    }

    Void test_lazy_locations_copy_and_absent_access() {
        const DiscreteLocation first(1), second(2), absent(3);
        const RealVariable x("x");
        MonolithicHybridSpace locations;
        locations.new_location(first, RealSpace({x}));
        locations.new_location(second, RealSpace({x}));
        Hybrid<int> family(HybridSpace(locations),
                           [](const DiscreteLocation&, const RealSpace&) { return 42; });
        ARIADNE_TESTING_PROVE(family.has_location(first));
        ARIADNE_TESTING_PROVE(family.has_location(second));
        ARIADNE_TESTING_PROVE(not family.has_location(absent));
        ARIADNE_TESTING_PROVE(not family.has_value(first));
        ARIADNE_TESTING_PROVE(not family.has_value(second));

        ARIADNE_TESTING_PROVE(family[first] == 42);
        ARIADNE_TESTING_PROVE(family.has_value(first));
        ARIADNE_TESTING_PROVE(not family.has_value(second));

        const Hybrid<int> copy(family);
        ARIADNE_TESTING_PROVE(copy[first] == 42);
        ARIADNE_TESTING_PROVE(not copy.has_value(second));

        const Hybrid<int>& const_family = family;
        ARIADNE_TESTING_EXPECT_EXCEPTION(const_family[second]);
        ARIADNE_TESTING_EXPECT_EXCEPTION(family[absent]);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_provider_integration");
    ARIADNE_TESTING_ADD_TEST(TestHybridProviderIntegration, test_system_spaces_and_lazy_values);
    ARIADNE_TESTING_ADD_TEST(TestHybridProviderIntegration, test_lazy_locations_copy_and_absent_access);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
