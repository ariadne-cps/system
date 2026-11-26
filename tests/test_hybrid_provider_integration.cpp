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
 *  @brief Verify System state spaces against the decoupled Hybrid grid and paving APIs.
 */

#include <memory>

#include "hybrid/hybrid_grid.hpp"
#include "hybrid/hybrid_grid_cell.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "hybrid/hybrid_scalings.hpp"
#include "logging/test_runner.hpp"
#include "paving/grid.hpp"
#include "paving/robdd_paving.hpp"
#include "symbolic/assignment.hpp"
#include "system/hybrid_automaton.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestHybridProviderIntegration : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_system_spaces_and_ordered_grids() {
        const RealVariable x("x"), y("y");
        const DiscreteLocation first(1), second(2);
        HybridAutomaton automaton;
        automaton.new_mode(first, List<DottedRealAssignment>{dot(x) = -x});
        automaton.new_mode(second, List<DottedRealAssignment>{dot(y) = -y});
        const HybridSpace spaces = automaton.state_space();
        ARIADNE_TESTING_PROVE(spaces.has_location(first));
        ARIADNE_TESTING_PROVE(spaces.has_location(second));

        const Map<Identifier, ExactDouble> factors{
            {x.name(), 2.0_x}, {y.name(), 3.0_x}
        };
        HybridGrid grid{spaces, HybridScalings(factors)};
        ARIADNE_TESTING_PROVE(grid[first] == Grid(Vector<ExactDouble>{2.0_x}));
        ARIADNE_TESTING_PROVE(grid.grid(second) == Grid(Vector<ExactDouble>{3.0_x}));
        ARIADNE_TESTING_PROVE(HybridGrid::make_grid(
            grid.scalings(), first, RealSpace({y, x})) ==
            Grid(Vector<ExactDouble>{3.0_x, 2.0_x}));
    }

    Void test_lazy_locations_and_shared_robdd_backend() {
        const DiscreteLocation first(1), second(2), absent(3);
        const RealVariable x("x");
        MonolithicHybridSpace locations;
        locations.new_location(first, RealSpace({x}));
        locations.new_location(second, RealSpace({x}));
        const HybridGrid grid{HybridSpace(locations)};
        auto manager = std::make_shared<RobddManager>();
        HybridPaving paving(grid, manager);
        ARIADNE_TESTING_PROVE(paving.values().has_location(first));
        ARIADNE_TESTING_PROVE(paving.values().has_location(second));
        ARIADNE_TESTING_PROVE(not paving.values().has_location(absent));
        ARIADNE_TESTING_PROVE(not paving.values().has_value(first));
        ARIADNE_TESTING_PROVE(not paving.values().has_value(second));

        const GridCell cell(grid[first], 0u, BinaryWord{});
        paving.adjoin(HybridGridCell(first, RealSpace({x}), cell));
        ARIADNE_TESTING_PROVE(paving.values().has_value(first));
        ARIADNE_TESTING_PROVE(not paving.values().has_value(second));
        ARIADNE_TESTING_PROVE(paving[first].superset(cell));
        const PavingInterface& selected = static_cast<const PavingInterface&>(paving[first]);
        ARIADNE_TESTING_PROVE(dynamic_cast<const RobddPaving*>(&selected) != nullptr);

        const HybridPaving copy(paving);
        ARIADNE_TESTING_PROVE(copy[first].superset(cell));
        ARIADNE_TESTING_PROVE(not copy.values().has_value(second));
        ARIADNE_TESTING_EXPECT_EXCEPTION(paving[absent]);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_provider_integration");
    ARIADNE_TESTING_ADD_TEST(TestHybridProviderIntegration, test_system_spaces_and_ordered_grids);
    ARIADNE_TESTING_ADD_TEST(TestHybridProviderIntegration, test_lazy_locations_and_shared_robdd_backend);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
