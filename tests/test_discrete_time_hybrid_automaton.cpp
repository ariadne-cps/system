/***************************************************************************
 *            test_discrete_time_hybrid_automaton.cpp
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

/** @file tests/test_discrete_time_hybrid_automaton.cpp
 *  @brief Structural and semantic declaration tests for integer-time hybrid modes.
 */

#include <memory>
#include <stdexcept>
#include <type_traits>

#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/variable.hpp"
#include "system/discrete_time_hybrid_automaton.hpp"
#include "system/difference_algebraic_equation.hpp"
#include "system/difference_algebraic_inclusion.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

static_assert(std::is_base_of_v<HybridModeStructure, DiscreteTimeHybridMode>);
static_assert(std::is_base_of_v<HybridAutomatonInterface, DiscreteTimeHybridAutomatonInterface>);
static_assert(std::is_base_of_v<DiscreteTimeHybridAutomatonInterface, DiscreteTimeHybridAutomaton>);

class TestDiscreteTimeHybridAutomaton : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_modes_and_structure() {
        RealVariable x("x"), y("y");
        DiscreteLocation first(1), second(2);
        DifferenceAlgebraicEquation f(List<PrimedRealAssignment>{next(x) = x + 1});
        DifferenceAlgebraicEquation g(List<PrimedRealAssignment>{next(y) = y + 1});
        DiscreteTimeHybridAutomaton system("discrete");
        system.new_mode(first, f);
        system.new_mode(second, g);
        ARIADNE_TESTING_PROVE(system.has_mode(first));
        ARIADNE_TESTING_PROVE(system.has_mode(second));
        ARIADNE_TESTING_EXPECT_EQUAL(system.dimension(first), 1u);
        ARIADNE_TESTING_PROVE(system.equation(first).state_space() == RealSpace({x}));
        ARIADNE_TESTING_PROVE(&system.update_function(first) == &system.equation(first).update_function());
        ARIADNE_TESTING_PROVE(system.state_space(first) == RealSpace({x}));
        ARIADNE_TESTING_PROVE(system.events(first).empty());
        ARIADNE_TESTING_PROVE(!system.has_transition(first, DiscreteEvent("absent")));
        ARIADNE_TESTING_EXPECT_EQUAL(system.target(first, DiscreteEvent("absent")), first);
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, system.new_mode(first, f));
        ARIADNE_TESTING_EXPECT_THROWS(std::out_of_range, system.mode(DiscreteLocation(9)));
        std::unique_ptr<DiscreteTimeHybridAutomaton> copy(system.clone());
        ARIADNE_TESTING_PROVE(copy->has_mode(second));
        ARIADNE_TESTING_PROVE(copy->equation(second).state_space() == RealSpace({y}));
    }

    Void test_explicit_algebraic_outputs() {
        const RealVariable x("x"), a("a");
        const DiscreteLocation location(1);
        const DifferenceAlgebraicEquation declared(
            List<PrimedRealAssignment>{next(x) = x + a},
            List<RealAssignment>{let(a) = x + 1});

        DiscreteTimeHybridAutomaton system("algebraic discrete");
        system.new_mode(location, declared);
        ARIADNE_TESTING_EXPECT_EQUAL(system.equation(location).dimension(), 1u);
        ARIADNE_TESTING_PROVE(system.equation(location).state_space() == RealSpace({x}));
        ARIADNE_TESTING_PROVE(system.equation(location).auxiliary_space() == RealSpace({a}));
        ARIADNE_TESTING_EXPECT_EQUAL(system.equation(location).updates().size(), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(system.equation(location).auxiliary_assignments().size(), 1u);
        ARIADNE_TESTING_PROVE(&system.equation(location).update_function() ==
                             &system.update_function(location));
        ARIADNE_TESTING_PROVE(&system.equation(location).auxiliary_function() ==
                             &system.auxiliary_function(location));

        std::unique_ptr<DiscreteTimeHybridAutomaton> copied(system.clone());
        ARIADNE_TESTING_PROVE(copied->equation(location).auxiliary_space() == RealSpace({a}));
    }

    Void test_mixed_equation_and_inclusion_modes() {
        const RealVariable x("x"), a("a"), u("u");
        const DiscreteLocation deterministic(1), set_valued(2);
        const DiscreteEvent jump("jump"), return_event("return");
        const DifferenceAlgebraicEquation equation(
            List<PrimedRealAssignment>{next(x) = x + 1});
        const DifferenceAlgebraicInclusion inclusion(
            List<PrimedRealAssignment>{next(x) = x + a + u},
            List<RealAssignment>{let(a) = x + u},
            RealVariablesBox({-1 <= u <= 1}));

        DiscreteTimeHybridAutomaton automaton("mixed dynamics");
        automaton.new_mode(deterministic, equation);
        automaton.new_mode(set_valued, inclusion);

        ARIADNE_TESTING_PROVE(automaton.has_equation(deterministic));
        ARIADNE_TESTING_PROVE(!automaton.has_inclusion(deterministic));
        ARIADNE_TESTING_PROVE(!automaton.has_equation(set_valued));
        ARIADNE_TESTING_PROVE(automaton.has_inclusion(set_valued));
        ARIADNE_TESTING_EXPECT_EQUAL(automaton.dimension(deterministic), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(automaton.dimension(set_valued), 1u);
        ARIADNE_TESTING_PROVE(automaton.state_space(set_valued) == RealSpace({x}));
        ARIADNE_TESTING_PROVE(automaton.inclusion(set_valued).input_space() == RealSpace({u}));
        ARIADNE_TESTING_PROVE(automaton.inclusion(set_valued).inputs() ==
                             BoxDomainType({{-1.0_x, 1.0_x}}));
        ARIADNE_TESTING_EXPECT_EQUAL(automaton.update_function(deterministic).argument_size(), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(automaton.update_function(set_valued).argument_size(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(automaton.auxiliary_function(set_valued).result_size(), 1u);
        ARIADNE_TESTING_PROVE(&automaton.update_function(deterministic) ==
                             &automaton.equation(deterministic).update_function());
        ARIADNE_TESTING_PROVE(&automaton.update_function(set_valued) ==
                             &automaton.inclusion(set_valued).update_function());

        // Wrong-family access must fail rather than select or freeze an input.
        ARIADNE_TESTING_EXPECT_THROWS(std::logic_error, automaton.equation(set_valued));
        ARIADNE_TESTING_EXPECT_THROWS(std::logic_error, automaton.inclusion(deterministic));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, automaton.new_mode(deterministic, inclusion));

        // Both directions of a mixed transition graph have unchanged
        // guard/invariant semantics and validate reset variables against the
        // target mode's physical state space.
        automaton.new_permissive_transition(set_valued, jump, deterministic,
                                            {next(x) = x + a}, x >= 0, x <= 2);
        automaton.new_urgent_transition(deterministic, return_event, set_valued,
                                        {next(x) = x}, x >= 1);
        ARIADNE_TESTING_PROVE(automaton.has_transition(set_valued, jump));
        ARIADNE_TESTING_PROVE(automaton.has_transition(deterministic, return_event));
        ARIADNE_TESTING_EXPECT_EQUAL(automaton.target(set_valued, jump), deterministic);
        ARIADNE_TESTING_EXPECT_EQUAL(automaton.target(deterministic, return_event), set_valued);
        ARIADNE_TESTING_PROVE(automaton.transition_kind(set_valued, jump) ==
                             DiscreteTimeTransitionKind::PERMISSIVE);
        ARIADNE_TESTING_PROVE(automaton.transition_kind(deterministic, return_event) ==
                             DiscreteTimeTransitionKind::URGENT);

        // Direct free-input references in event data are not yet admitted:
        // their step-level correlation needs its own evolver contract.
        const DiscreteEvent unsupported("unsupported");
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, automaton.new_permissive_transition(
            set_valued, unsupported, deterministic, {next(x) = x + u}, x >= 0, x <= 1));
        ARIADNE_TESTING_PROVE(!automaton.has_transition(set_valued, unsupported));

        const DiscreteEvent input_guard("input_guard");
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, automaton.new_urgent_transition(
            set_valued, input_guard, deterministic, {next(x) = x}, u >= 0));
        ARIADNE_TESTING_PROVE(!automaton.has_transition(set_valued, input_guard));

        const DiscreteEvent input_invariant("input_invariant");
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, automaton.new_permissive_transition(
            set_valued, input_invariant, deterministic, {next(x) = x}, x >= 0, u >= 0));
        ARIADNE_TESTING_PROVE(!automaton.has_transition(set_valued, input_invariant));

        std::unique_ptr<DiscreteTimeHybridAutomaton> cloned(automaton.clone());
        ARIADNE_TESTING_PROVE(cloned->has_inclusion(set_valued));
        ARIADNE_TESTING_PROVE(cloned->has_equation(deterministic));
        ARIADNE_TESTING_PROVE(cloned->inclusion(set_valued).inputs() == inclusion.inputs());
        ARIADNE_TESTING_PROVE(cloned->has_transition(set_valued, jump));
    }

    Void test_urgent_and_permissive_declarations() {
        RealVariable x("x");
        DiscreteLocation source(1), destination(2);
        DiscreteEvent urgent("urgent"), permissive("permissive");
        DiscreteTimeHybridAutomaton system("time discrete");
        system.new_mode(source, DifferenceAlgebraicEquation(List<PrimedRealAssignment>{next(x) = x + 1}));
        system.new_mode(destination, DifferenceAlgebraicEquation(List<PrimedRealAssignment>{next(x) = x - 1}));

        system.new_urgent_transition(source, urgent, destination,
                                      {next(x) = x}, x >= 1);
        ARIADNE_TESTING_PROVE(system.has_transition(source, urgent));
        ARIADNE_TESTING_EXPECT_EQUAL(system.target(source, urgent), destination);
        ARIADNE_TESTING_PROVE(system.transition_kind(source, urgent) == DiscreteTimeTransitionKind::URGENT);
        ARIADNE_TESTING_EXPECT_EQUAL(system.reset_assignments(source, urgent).size(), 1u);
        ARIADNE_TESTING_NOTE(system.invariant_predicate(source, urgent));

        // The permissive guard and the progression invariant deliberately overlap.
        // This declares two possible branches; an evolver must preserve both.
        system.new_permissive_transition(source, permissive, destination,
                                          {next(x) = x + 2}, x >= 0, x <= 2);
        ARIADNE_TESTING_PROVE(system.transition_kind(source, permissive) == DiscreteTimeTransitionKind::PERMISSIVE);
        ARIADNE_TESTING_PROVE(system.events(source).contains(urgent));
        ARIADNE_TESTING_PROVE(system.events(source).contains(permissive));
        ARIADNE_TESTING_NOTE(system.guard_predicate(source, permissive));
        ARIADNE_TESTING_NOTE(system.invariant_predicate(source, permissive));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, system.new_urgent_transition(
            source, permissive, destination, {next(x) = x}, x >= 0));
    }

    Void test_transition_errors() {
        RealVariable x("x"), y("y");
        DiscreteLocation source(1), destination(2), missing(3);
        const DiscreteEvent edge("edge");
        DiscreteTimeHybridAutomaton system;
        system.new_mode(source, DifferenceAlgebraicEquation(List<PrimedRealAssignment>{next(x) = x}));
        system.new_mode(destination, DifferenceAlgebraicEquation(List<PrimedRealAssignment>{next(y) = y}));

        ARIADNE_TESTING_EXPECT_THROWS(std::out_of_range, system.new_urgent_transition(
            missing, edge, destination, {next(y) = x}, x >= 0));
        ARIADNE_TESTING_EXPECT_THROWS(std::out_of_range, system.new_urgent_transition(
            source, edge, missing, {next(y) = x}, x >= 0));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, system.new_urgent_transition(
            source, edge, destination, {next(x) = x}, x >= 0));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, system.new_urgent_transition(
            source, edge, destination, {next(y) = x, next(y) = x}, x >= 0));

        system.new_permissive_transition(source, edge, destination,
                                          {next(y) = x}, x >= 0, x <= 1);
        ARIADNE_TESTING_PROVE(system.has_transition(source, edge));
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, system.new_permissive_transition(
            source, edge, destination, {next(y) = x}, x >= 0, x <= 1));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_discrete_time_hybrid_automaton");
    ARIADNE_TESTING_ADD_TEST(TestDiscreteTimeHybridAutomaton, test_modes_and_structure);
    ARIADNE_TESTING_ADD_TEST(TestDiscreteTimeHybridAutomaton, test_explicit_algebraic_outputs);
    ARIADNE_TESTING_ADD_TEST(TestDiscreteTimeHybridAutomaton, test_mixed_equation_and_inclusion_modes);
    ARIADNE_TESTING_ADD_TEST(TestDiscreteTimeHybridAutomaton, test_urgent_and_permissive_declarations);
    ARIADNE_TESTING_ADD_TEST(TestDiscreteTimeHybridAutomaton, test_transition_errors);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
