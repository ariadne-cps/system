/***************************************************************************
 *            test_hybrid_automaton_api.cpp
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

/** @file tests/test_hybrid_automaton_api.cpp
 *  @brief Test hybrid automaton api behavior.
 */

// Exercises the full public API of ContinuousTimeHybridAutomaton / ContinuousTimeHybridMode /
// DiscreteTransition: constructors, mode/transition building (including
// every error path), accessors, check_mode validation errors, and the
// verbose/compact writers plus EventKind streaming.

#include <sstream>

#include "hybrid/hybrid_space.hpp"
#include "logging/test_runner.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/expression.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"
#include "system/continuous_time_hybrid_automaton.hpp"
#include "system/continuous_time_hybrid_automaton_interface.hpp"
#include "system/odae.hpp"
#include "system/odai.hpp"

#include "ariadne-testing.hpp"
#include "system/composite_continuous_time_hybrid_automaton.hpp"

using namespace Ariadne;

class TestHybridAutomatonApi : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

  public:
    Void test_external_clock_without_a_differential_state() {
        const TimeVariable t;
        const RealVariable x("x"), u("u"), a("a");
        const OrdinaryDifferentialAlgebraicEquation equation({dot(x) = t}, {let(a) = x + t});
        const OrdinaryDifferentialAlgebraicInclusion inclusion({dot(x) = t + u}, RealVariablesBox({-1 <= u <= 1}));
        ARIADNE_TESTING_PROVE(equation.has_explicit_time() && inclusion.has_explicit_time());
        ARIADNE_TESTING_EXPECT_EQUAL(equation.dimension(), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(inclusion.dimension(), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(equation.argument_space(), RealSpace({x, t}));
        ARIADNE_TESTING_EXPECT_EQUAL(inclusion.argument_space(), RealSpace({x, t, u}));
        ARIADNE_TESTING_EXPECT_EQUAL(inclusion.parameter_space().size(), 0u);
        const DiscreteLocation source(1), target(2);
        const DiscreteEvent event("timer"), bound("horizon");
        ContinuousTimeHybridAutomaton system;
        system.new_mode(source, OrdinaryDifferentialAlgebraicEquation({dot(x) = 0}));
        system.new_mode(target, equation);
        system.new_invariant(source, t <= 2, bound);
        system.new_transition(source, event, target, {next(x) = t + x}, t >= 1, EventKind::URGENT);
        system.check_mode(source);
        system.check_mode(target);
        ARIADNE_TESTING_PROVE(system.time_variable(source).has_value());
        ARIADNE_TESTING_EXPECT_EQUAL(system.continuous_state_space(source), RealSpace({x}));
        ARIADNE_TESTING_EXPECT_EQUAL(system.continuous_argument_space(source), RealSpace({x, t}));
        ARIADNE_TESTING_PROVE(system.input_variables(source).empty());
        const Vector<FloatDPBounds> point{FloatDPBounds(3, dp), FloatDPBounds(1.5_x, dp)};
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(0.5_x, dp), system.guard_function(source, event)(point)));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(-0.5_x, dp), system.invariant_function(source, bound)(point)));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(4.5_x, dp), system.reset_function(source, event)(point)[0u]));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(1.5_x, dp), system.dynamic_function(target)(point)[0u]));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(4.5_x, dp), system.auxiliary_function(target)(point)[0u]));
        auto invalid = system;
        invalid.new_transition(source, DiscreteEvent("reset global time"), target,
                               {next(x) = x, next(t) = 0}, t >= 2, EventKind::URGENT);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedDynamicError, invalid.check_mode(source));
        // An explicitly declared clock-like state retains its physical role.
        const OrdinaryDifferentialAlgebraicEquation state_clock({dot(t) = 1});
        ARIADNE_TESTING_PROVE(!state_clock.has_explicit_time());
        ARIADNE_TESTING_EXPECT_EQUAL(state_clock.state_space(), RealSpace({t}));
    }

    Void test_typed_ordinary_modes() {
        const DiscreteLocation q(1), r(2), timed(3);
        const DiscreteEvent event("switch");
        const RealVariable x("x"), y("output"), u("u"), p("p"), t("t");
        auto system = [&]() {
            ContinuousTimeHybridAutomaton owned;
            const OrdinaryDifferentialAlgebraicEquation equation({dot(x) = 1}, {let(y) = 2 * x});
            const OrdinaryDifferentialAlgebraicInclusion inclusion({dot(x) = u}, RealVariablesBox({-1 <= u <= 1}));
            owned.new_mode(q, equation);
            owned.new_mode(r, inclusion);
            owned.new_mode(timed, OrdinaryDifferentialAlgebraicEquation(
                                      {dot(x) = t + p}, RealVariablesBox({0 <= p <= 1}), t, {let(y) = p + x}));
            return owned;
        }();
        ARIADNE_TESTING_REQUIRE(system.equation(q) != nullptr);
        ARIADNE_TESTING_REQUIRE(system.inclusion(r) != nullptr);
        ARIADNE_TESTING_PROVE(system.inclusion(q) == nullptr);
        ARIADNE_TESTING_PROVE(system.equation(r) == nullptr);
        ARIADNE_TESTING_EXPECT_EQUAL(system.dimension(q), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(system.continuous_state_space(r), RealSpace({x}));
        ARIADNE_TESTING_EXPECT_EQUAL(system.continuous_auxiliary_space(q), RealSpace({y}));
        ARIADNE_TESTING_EXPECT_EQUAL(system.input_variables(r), Set<RealVariable>({u}));
        ARIADNE_TESTING_PROVE(system.input_variables(timed).empty());
        ARIADNE_TESTING_PROVE(system.equation(timed)->has_explicit_time());
        ARIADNE_TESTING_EXPECT_EQUAL(system.equation(timed)->parameter_space(), RealSpace({p}));
        system.check_mode(timed);
        ARIADNE_TESTING_EXPECT_THROWS(std::logic_error, system.dynamic_function(r));
        ARIADNE_TESTING_EXPECT_EQUAL(system.dynamic_function(timed).argument_size(), 3u);
        ARIADNE_TESTING_EXPECT_EQUAL(system.continuous_argument_space(timed), RealSpace({x, t, p}));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(2.5_x, dp), system.dynamic_function(timed)(Vector<FloatDPBounds>{FloatDPBounds(0, dp), FloatDPBounds(2, dp), FloatDPBounds(0.5_x, dp)})[0u]));
        ARIADNE_TESTING_EXPECT_THROWS(std::logic_error, system.dynamic_assignments(r));
        system.new_transition(q, event, r, {next(x) = x + 1}, y >= 2, EventKind::URGENT);
        system.check_mode(q);
        system.check_mode(r);
        const Vector<FloatDPBounds> point{FloatDPBounds(1, dp)};
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(0, dp), system.guard_function(q, event)(point)));
        ARIADNE_TESTING_PROVE(refines(FloatDPBounds(2, dp), system.reset_function(q, event)(point)[0]));
        ARIADNE_TESTING_EXPECT_THROWS(IndistinguishableModeError, system.new_mode(q, *system.inclusion(r)));
        ContinuousTimeHybridAutomaton copied(system);
        system = ContinuousTimeHybridAutomaton();
        ARIADNE_TESTING_EXPECT_EQUAL(copied.inclusion(r)->state_space(), RealSpace({x}));
        std::ostringstream description;
        description << copied;
        ARIADNE_TESTING_PROVE(description.str().find("u") != std::string::npos);
        CompositeContinuousTimeHybridAutomaton composite(copied);
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, composite.dynamic_function(q));
    }

    Void test_constructors();
    Void test_modes_and_errors();
    Void test_invariants_guards_transitions();
    Void test_accessors();
    Void test_check_mode_errors();
    Void test_writers_and_streaming();

  private:
    ContinuousTimeHybridAutomaton _make_automaton() const;
};

ContinuousTimeHybridAutomaton TestHybridAutomatonApi::_make_automaton() const {
    RealVariable x("x"), y("y"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q | "1"}), q2({q | "2"});
    DiscreteEvent e1("e1"), e2("e2");
    ContinuousTimeHybridAutomaton ha("test");
    ha.new_mode(q1, List<RealAssignment>{let(a) = x + 1},
                List<DottedRealAssignment>{dot(x) = -x, dot(y) = y});
    ha.new_mode(q2, List<DottedRealAssignment>{dot(x) = x});
    ha.new_invariant(q1, x <= 1, e1);
    ha.new_transition(q1, e2, q2,
                      List<PrimedRealAssignment>{prime(x) = 2 * x}, x >= 0,
                      EventKind::PERMISSIVE);
    return ha;
}

Void TestHybridAutomatonApi::test_constructors() {
    StringVariable q("q");
    ContinuousTimeHybridAutomaton ha0;
    ContinuousTimeHybridAutomaton ha1("named");
    ContinuousTimeHybridAutomaton ha2("named2", List<StringVariable>{q});
    ARIADNE_TESTING_EXPECT_EQUAL(ha1.name(), Identifier("named"));
    ARIADNE_TESTING_EXPECT_EQUAL(ha0.locations().size(), 0u);
    ContinuousTimeHybridAutomaton* copy = ha1.clone();
    ARIADNE_TESTING_PROVE(copy != nullptr);
    ARIADNE_TESTING_EXPECT_EQUAL(copy->name(), Identifier("named"));
    delete copy;

    // ContinuousTimeHybridMode objects are only created through ContinuousTimeHybridAutomaton.
    ContinuousTimeHybridAutomaton ha;
    RealVariable x("x");
    DiscreteLocation q1({q | "1"});
    ha.new_mode(q1, List<DottedRealAssignment>{dot(x) = x});
    ContinuousTimeHybridMode dm = ha.mode(q1);
    ARIADNE_TESTING_EXPECT_EQUAL(dm.location(), q1);
}

Void TestHybridAutomatonApi::test_modes_and_errors() {
    RealVariable x("x"), y("y"), z("z"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q | "1"}), q2({q | "2"}), q9({q | "9"});

    ContinuousTimeHybridAutomaton ha;
    // All new_mode overloads.
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_mode(q1,
                                                List<RealAssignment>{let(a) = x + 1},
                                                List<DottedRealAssignment>{dot(x) = -x, dot(y) = y}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_mode(q2,
                                                List<DottedRealAssignment>{dot(x) = x},
                                                List<RealAssignment>{}));
    ARIADNE_TESTING_EXPECT_THROWS(IndistinguishableModeError, ha.new_mode(q1));
    // Variable defined twice by the algebraic equations.
    ARIADNE_TESTING_EXPECT_THROWS(SystemSpecificationError, ContinuousTimeHybridAutomaton().new_mode(q2,
                                                                                       List<RealAssignment>{let(a) = x, let(a) = y},
                                                                                       List<DottedRealAssignment>{}));
    // Differential variable also defined algebraically.
    ARIADNE_TESTING_EXPECT_THROWS(SystemSpecificationError, ContinuousTimeHybridAutomaton().new_mode(q2,
                                                                                       List<RealAssignment>{let(x) = 1},
                                                                                       List<DottedRealAssignment>{dot(x) = x}));
    // Overloads without a location all use the empty location, so each
    // needs a fresh automaton.
    ARIADNE_TESTING_EXPECT_NO_THROW(ContinuousTimeHybridAutomaton().new_mode(
        List<RealAssignment>{let(a) = x}, List<DottedRealAssignment>{dot(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ContinuousTimeHybridAutomaton().new_mode(
        List<DottedRealAssignment>{dot(x) = x}, List<RealAssignment>{let(a) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ContinuousTimeHybridAutomaton().new_mode(
        List<DottedRealAssignment>{dot(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ContinuousTimeHybridAutomaton().new_mode(
        List<RealAssignment>{let(a) = x}));
    ContinuousTimeHybridAutomaton hb;
    ARIADNE_TESTING_EXPECT_NO_THROW(hb.new_mode(DiscreteLocation(1)));
    ARIADNE_TESTING_EXPECT_NO_THROW(hb.new_mode(DiscreteLocation(2),
                                                List<DottedRealAssignment>{dot(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(hb.new_mode(DiscreteLocation(3),
                                                List<RealAssignment>{let(a) = x}));
    ARIADNE_TESTING_EXPECT_THROWS(IndistinguishableModeError, hb.new_mode(DiscreteLocation(1)));
    ARIADNE_TESTING_EXPECT_EQUAL(ha.locations().size(), 2u);
}

Void TestHybridAutomatonApi::test_invariants_guards_transitions() {
    RealVariable x("x");
    StringVariable q("q");
    DiscreteLocation q1({q | "1"}), q2({q | "2"}), q9({q | "9"});
    DiscreteEvent e1("e1"), e2("e2"), e3("e3"), e4("e4"), e5("e5"), e6("e6");

    ContinuousTimeHybridAutomaton ha;
    ha.new_mode(q1, List<DottedRealAssignment>{dot(x) = x});
    ha.new_mode(q2, List<DottedRealAssignment>{dot(x) = -x});

    // Error paths for a nonexistent location.
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_invariant(q9, x <= 1, e1));
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_guard(q9, e1, x >= 0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_action(q9, x <= 1, e1, x >= 0));
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_action(q9, e1, x >= 0));

    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_invariant(q1, x <= 1, e1));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.set_invariant(q1, x <= 2, e1));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_guard(q1, e2, x >= 0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.set_guard(q1, e2, x >= 1, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_action(q1, x <= 1, e3, x >= 0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_action(q1, e4, x >= 0, EventKind::URGENT));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_action(q1, e5, x >= 0, EventKind::IMPACT));
    // Wrong event kind for each new_action overload trips the assertion.
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, ha.new_action(q1, x <= 1, DiscreteEvent("w1"), x >= 0,
                                                                    EventKind::URGENT));
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, ha.new_action(q1, DiscreteEvent("w2"), x >= 0,
                                                                    EventKind::PERMISSIVE));

    // Re-declaring a constraint for the same event is an error.
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_invariant(q1, x <= 3, e1));
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_guard(q1, e2, x <= 3, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_action(q1, x <= 3, e3, x >= 1));

    // Transitions of each kind.
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, e6, q2,
                                                      List<PrimedRealAssignment>{prime(x) = 2 * x}, x >= 0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q2, DiscreteEvent("u"), q1,
                                                      List<PrimedRealAssignment>{prime(x) = x}, x >= 0, EventKind::URGENT));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q2, DiscreteEvent("i"), q1, x >= 0,
                                                      EventKind::IMPACT));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("p"), q2, x >= 0,
                                                      EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("k"), x >= 0, q2,
                                                      EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_update(q1, DiscreteEvent("w"), q2,
                                                  List<PrimedRealAssignment>{prime(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_update(q1, DiscreteEvent("x"), q2));
    // Overloads without a source location need an automaton with the
    // empty location.
    ContinuousTimeHybridAutomaton he;
    he.new_mode(DiscreteLocation(), List<DottedRealAssignment>{dot(x) = x});
    ARIADNE_TESTING_EXPECT_NO_THROW(he.new_update(DiscreteEvent("y"),
                                                  List<PrimedRealAssignment>{prime(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_reset(q1, DiscreteEvent("z"), q2,
                                                 List<PrimedRealAssignment>{prime(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_reset(q1, DiscreteEvent("v"), q2));
    ARIADNE_TESTING_EXPECT_NO_THROW(he.new_reset(DiscreteEvent("u2"),
                                                 List<PrimedRealAssignment>{prime(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("n"), q2,
                                                      List<PrimedRealAssignment>{prime(x) = x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("m"), q2));
    ARIADNE_TESTING_EXPECT_NO_THROW(he.new_transition(DiscreteEvent("g"),
                                                      List<PrimedRealAssignment>{prime(x) = x}, x >= 0, EventKind::PERMISSIVE));
    // An unhandled event kind for a transition is rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, ha.new_transition(q1, DiscreteEvent("b"), q2,
                                                                        List<PrimedRealAssignment>{}, x >= 0, EventKind::INVARIANT));
    // Multiple transitions for the same event are rejected.
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_transition(q1, e6, q1,
                                                                        List<PrimedRealAssignment>{}, x >= 0, EventKind::PERMISSIVE));
}

Void TestHybridAutomatonApi::test_accessors() {
    ContinuousTimeHybridAutomaton ha = _make_automaton();
    RealVariable x("x"), y("y"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q | "1"}), q2({q | "2"}), q9({q | "9"});
    DiscreteEvent e1("e1"), e2("e2"), e_none("none");

    ARIADNE_TESTING_EXPECT_EQUAL(ha.locations().size(), 2u);
    ARIADNE_TESTING_PROVE(ha.has_location(q1));
    ARIADNE_TESTING_PROVE(!ha.has_location(q9));
    ARIADNE_TESTING_PROVE(ha.has_mode(q1));
    ARIADNE_TESTING_PROVE(ha.has_partial_mode(q1));
    ARIADNE_TESTING_PROVE(!ha.has_mode(q9));
    ARIADNE_TESTING_PROVE(ha.modes().has_key(q1));
    ARIADNE_TESTING_PROVE(ha.mode(q1).location() == q1);
    ContinuousTimeHybridMode& mutable_mode = ha.mode(q1);
    ARIADNE_TESTING_PROVE(mutable_mode.location() == q1);
    ARIADNE_TESTING_EXPECT_THROWS(SystemSpecificationError, ha.mode(q9));

    ARIADNE_TESTING_EXPECT_EQUAL(ha.dimension(q1), 2u);
    ARIADNE_TESTING_PROVE(ha.continuous_state_space(q1) == RealSpace({x, y}));
    ARIADNE_TESTING_PROVE(ha.continuous_auxiliary_space(q1) == RealSpace({a}));
    ARIADNE_TESTING_PROVE(ha.state_space().has_location(q1));
    ARIADNE_TESTING_PROVE(ha.state_auxiliary_space().has_location(q1));
    ARIADNE_TESTING_EXPECT_EQUAL(ha.state_auxiliary_space()[q1].size(), 3u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.state_variables(q1).size(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.auxiliary_variables(q1).size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.auxiliary_assignments(q1).size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.dynamic_assignments(q1).size(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.input_variables(q1).size(), 0u);
    ARIADNE_TESTING_PROVE(ha.has_invariant(q1, e1));
    ARIADNE_TESTING_PROVE(!ha.has_invariant(q1, e2));
    ARIADNE_TESTING_PROVE(ha.has_guard(q1, e2));
    ARIADNE_TESTING_PROVE(ha.has_transition(q1, e2));
    ARIADNE_TESTING_PROVE(!ha.has_transition(q1, e1));
    // Short-circuit when the source location does not exist.
    ARIADNE_TESTING_PROVE(!ha.has_transition(q9, e2));
    ARIADNE_TESTING_PROVE(!ha.has_invariant(q9, e1));
    ARIADNE_TESTING_PROVE(!ha.has_guard(q9, e2));
    ARIADNE_TESTING_EXPECT_EQUAL(ha.event_kind(q1, e1), EventKind::INVARIANT);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.event_kind(q1, e2), EventKind::PERMISSIVE);
    ARIADNE_TESTING_PROVE(ha.events(q1).size() >= 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.target(q1, e2), q2);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.target(q1, e_none), q1);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.reset_assignments(q1, e2).size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.reset_assignments(q1, e_none).size(), 2u);
    ARIADNE_TESTING_NOTE(ha.invariant_predicate(q1, e1));
    ARIADNE_TESTING_NOTE(ha.guard_predicate(q1, e2));
    ARIADNE_TESTING_NOTE(ha.invariant_predicate(q1, e_none));
    ARIADNE_TESTING_NOTE(ha.guard_predicate(q1, e_none));
    ARIADNE_TESTING_NOTE(ha.auxiliary_function(q1));
    ARIADNE_TESTING_NOTE(ha.dynamic_function(q1));
    ARIADNE_TESTING_NOTE(ha.invariant_function(q1, e1));
    ARIADNE_TESTING_NOTE(ha.guard_function(q1, e2));
    ARIADNE_TESTING_NOTE(ha.reset_function(q1, e2));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.check_mode(q1));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.check_mode(q2));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.check_reachable_modes(Set<DiscreteLocation>{q1}));
    ARIADNE_TESTING_PROVE(ha.discrete_reachability(Set<DiscreteLocation>{q1}).contains(q2));
}

Void TestHybridAutomatonApi::test_check_mode_errors() {
    RealVariable x("x"), z("z"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q | "1"}), q2({q | "2"});
    DiscreteEvent e1("e1"), e2("e2"), e3("e3");

    {
        ContinuousTimeHybridAutomaton bad;
        bad.new_mode(q1, List<RealAssignment>{let(a) = z},
                     List<DottedRealAssignment>{dot(x) = x});
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedDynamicError, bad.check_mode(q1));
    }
    {
        ContinuousTimeHybridAutomaton bad;
        bad.new_mode(q1, List<RealAssignment>{let(a) = x},
                     List<DottedRealAssignment>{dot(x) = x + z});
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedDynamicError, bad.check_mode(q1));
    }
    {
        ContinuousTimeHybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x) = x});
        bad.new_invariant(q1, z <= 1, e1);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedConstraintError, bad.check_mode(q1));
    }
    {
        ContinuousTimeHybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x) = x});
        bad.new_guard(q1, e2, z >= 0, EventKind::PERMISSIVE);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedConstraintError, bad.check_mode(q1));
    }
    {
        ContinuousTimeHybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x) = x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x) = x});
        bad.new_transition(q1, e3, q2,
                           List<PrimedRealAssignment>{prime(x) = z}, x >= 0, EventKind::PERMISSIVE);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedResetError, bad.check_mode(q1));
    }
    {
        ContinuousTimeHybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x) = x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x) = x, dot(a) = a});
        bad.new_transition(q1, e3, q2,
                           List<PrimedRealAssignment>{prime(x) = x}, x >= 0, EventKind::PERMISSIVE);
        // The reset does not define the target variable a.
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedResetError, bad.check_mode(q1));
    }
    {
        ContinuousTimeHybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x) = x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x) = x});
        // The reset defines y, which is not a state variable of the target.
        bad.new_transition(q1, e3, q2,
                           List<PrimedRealAssignment>{prime(x) = x, prime(a) = x}, x >= 0,
                           EventKind::PERMISSIVE);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedDynamicError, bad.check_mode(q1));
    }
}

Void TestHybridAutomatonApi::test_writers_and_streaming() {
    ContinuousTimeHybridAutomaton ha = _make_automaton();
    RealVariable x("x"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q | "1"}), q2({q | "2"});
    DiscreteEvent e1("e1");

    std::ostringstream verbose;
    verbose << ha;
    ARIADNE_TESTING_PROVE(verbose.str().size() > 0u);

    Writer<ContinuousTimeHybridAutomaton> previous = ContinuousTimeHybridAutomaton::default_writer();
    ContinuousTimeHybridAutomaton::set_default_writer(CompactHybridAutomatonWriter());
    std::ostringstream compact;
    compact << ha;
    ARIADNE_TESTING_PROVE(compact.str().size() > 0u);
    // A single-mode automaton takes the non-multiple branch.
    ContinuousTimeHybridAutomaton single_mode;
    single_mode.new_mode(DiscreteLocation(1), List<DottedRealAssignment>{dot(x) = x});
    std::ostringstream compact_single;
    compact_single << single_mode;
    ARIADNE_TESTING_PROVE(compact_single.str().size() > 0u);
    ContinuousTimeHybridAutomaton::set_default_writer(previous);

    ContinuousTimeHybridMode mode = ha.mode(q1);
    std::ostringstream mode_verbose;
    mode_verbose << mode;
    Writer<ContinuousTimeHybridMode> previous_mode = ContinuousTimeHybridMode::default_writer();
    ContinuousTimeHybridMode::set_default_writer(CompactContinuousTimeHybridModeWriter());
    std::ostringstream mode_compact;
    mode_compact << mode;
    ARIADNE_TESTING_PROVE(mode_compact.str().size() > 0u);
    // Empty, auxiliary-only, dynamic-only and reset-less modes cover the
    // remaining compact-writer branches.
    {
        ContinuousTimeHybridAutomaton bare;
        bare.new_mode(DiscreteLocation(1), List<RealAssignment>{let(a) = 1});
        bare.new_mode(DiscreteLocation(2), List<DottedRealAssignment>{dot(x) = x});
        bare.new_mode(DiscreteLocation(3));
        bare.new_transition(DiscreteLocation(3), DiscreteEvent("t"),
                            DiscreteLocation(3));
        for (const auto& m: bare.modes().values()) {
            std::ostringstream os;
            os << m;
            ARIADNE_TESTING_PROVE(os.str().size() > 0u);
        }
        ContinuousTimeHybridAutomaton bare_empty;
        bare_empty.new_mode(DiscreteLocation());
        {
            // A completely empty mode prints as the empty string in compact form.
            std::ostringstream os;
            os << bare_empty.mode(DiscreteLocation());
        }
    }
    ContinuousTimeHybridMode::set_default_writer(previous_mode);

    std::ostringstream kinds;
    kinds << EventKind::INVARIANT << " " << EventKind::PROGRESS << " "
          << EventKind::PERMISSIVE << " " << EventKind::URGENT << " "
          << EventKind::IMPACT;
    ARIADNE_TESTING_NOTE(kinds.str());
}

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_automaton_api");
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_typed_ordinary_modes);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_external_clock_without_a_differential_state);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_constructors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_modes_and_errors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_invariants_guards_transitions);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_accessors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_check_mode_errors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_writers_and_streaming);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}
