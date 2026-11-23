/***************************************************************************
 *            tests/test_system_header.cpp
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

/** @file tests/test_system_header.cpp
 *  @brief Test system header behavior.
 */

#include <type_traits>

#include "logging/test_runner.hpp"

#include "ariadne-system.hpp"
#include "ariadne-testing.hpp"
static_assert(std::is_base_of_v<Ariadne::HybridAutomatonInterface, Ariadne::ContinuousTimeHybridAutomatonInterface>);
static_assert(std::is_base_of_v<Ariadne::HybridAutomatonInterface, Ariadne::ContinuousTimeHybridAutomaton>);
static_assert(std::is_base_of_v<Ariadne::HybridAutomatonInterface, Ariadne::CompositeContinuousTimeHybridAutomaton>);
static_assert(std::is_base_of_v<Ariadne::HybridAutomatonInterface, Ariadne::DiscreteTimeHybridAutomatonInterface>);
static_assert(std::is_base_of_v<Ariadne::DiscreteTimeHybridAutomatonInterface, Ariadne::DiscreteTimeHybridAutomaton>);
static_assert(sizeof(Ariadne::NeuralValidatedEvaluator) > 0);
static_assert(sizeof(Ariadne::OrdinaryDifferentialAlgebraicEquation) > 0);
static_assert(sizeof(Ariadne::DifferenceAlgebraicEquation) > 0);
static_assert(sizeof(Ariadne::OrdinaryDifferentialAlgebraicInclusion) > 0);
class TestSystemHeader : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    void test_public_header() {
        Ariadne::DiscreteEvent event("tick");
        ARIADNE_TESTING_PROVE(event.name() == "tick");
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_system_header");
    ARIADNE_TESTING_ADD_TEST(TestSystemHeader, test_public_header);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
