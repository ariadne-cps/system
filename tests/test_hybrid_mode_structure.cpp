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
#include "system/hybrid_mode_registry.hpp"
#include "symbolic/variable.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestHybridModeRegistry : public HybridModeRegistry<HybridModeStructure> {
  public:
    Void add_mode(DiscreteLocation location) {
        this->_modes.insert(location, HybridModeStructure(location));
    }

    std::optional<DiscreteLocation> first_indistinguishable(DiscreteLocation location) const {
        return this->_first_indistinguishable_mode(location);
    }

    const HybridModeStructure* find_mode(DiscreteLocation location) const {
        return this->_find_mode(location);
    }
};

class TestHybridModeStructure : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_registry_location_matching() {
        StringVariable q("q");
        StringVariable r("r");
        const DiscreteLocation first(q | "first");
        const DiscreteLocation second(q | "second");
        const DiscreteLocation extended({q | "first", r | "other"});
        const DiscreteLocation conflicting(r | "other");

        TestHybridModeRegistry registry;
        ARIADNE_TESTING_PROVE(registry.locations().empty());
        ARIADNE_TESTING_PROVE(!registry.has_mode(first));
        ARIADNE_TESTING_PROVE(!registry.has_partial_mode(first));
        ARIADNE_TESTING_PROVE(!registry.first_indistinguishable(first).has_value());

        registry.add_mode(first);
        registry.add_mode(second);
        ARIADNE_TESTING_PROVE(registry.has_location(first));
        ARIADNE_TESTING_PROVE(registry.has_mode(first));
        ARIADNE_TESTING_PROVE(registry.has_mode(second));
        ARIADNE_TESTING_PROVE(!registry.has_mode(extended));
        ARIADNE_TESTING_PROVE(registry.has_partial_mode(extended));
        ARIADNE_TESTING_PROVE(registry.find_mode(extended) != nullptr);
        ARIADNE_TESTING_EXPECT_EQUAL(registry.find_mode(extended)->location(), first);
        ARIADNE_TESTING_PROVE(!registry.has_partial_mode(conflicting));
        ARIADNE_TESTING_PROVE(registry.first_indistinguishable(conflicting).has_value());
        ARIADNE_TESTING_PROVE(!registry.first_indistinguishable(q | "third").has_value());
    }

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
    ARIADNE_TESTING_ADD_TEST(TestHybridModeStructure, test_registry_location_matching);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
