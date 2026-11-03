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

// Exercises the full public API of HybridAutomaton / DiscreteMode /
// DiscreteTransition: constructors, mode/transition building (including
// every error path), accessors, check_mode validation errors, and the
// verbose/compact writers plus EventKind streaming.

#include <sstream>

#include "macro/testing.hpp"

#include "symbolic/variable.hpp"
#include "symbolic/expression.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

#include "hybrid/hybrid_space.hpp"

#include "system/hybrid_automaton_interface.hpp"
#include "system/hybrid_automaton.hpp"

using namespace Ariadne;

class TestHybridAutomatonApi {
  public:
    Void test();
  private:
    Void test_constructors();
    Void test_modes_and_errors();
    Void test_invariants_guards_transitions();
    Void test_accessors();
    Void test_check_mode_errors();
    Void test_writers_and_streaming();

    HybridAutomaton _make_automaton() const;
};

HybridAutomaton TestHybridAutomatonApi::_make_automaton() const
{
    RealVariable x("x"), y("y"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"});
    DiscreteEvent e1("e1"), e2("e2");
    HybridAutomaton ha("test");
    ha.new_mode(q1, List<RealAssignment>{let(a)=x+1},
                   List<DottedRealAssignment>{dot(x)=-x, dot(y)=y});
    ha.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
    ha.new_invariant(q1, x<=1, e1);
    ha.new_transition(q1, e2, q2,
                      List<PrimedRealAssignment>{prime(x)=2*x}, x>=0,
                      EventKind::PERMISSIVE);
    return ha;
}

Void TestHybridAutomatonApi::test_constructors()
{
    StringVariable q("q");
    HybridAutomaton ha0;
    HybridAutomaton ha1("named");
    HybridAutomaton ha2("named2", List<StringVariable>{q});
    ARIADNE_TEST_EQUALS(ha1.name(), Identifier("named"));
    ARIADNE_TEST_EQUALS(ha0.locations().size(), 0u);
    HybridAutomaton* copy=ha1.clone();
    ARIADNE_TEST_ASSERT(copy!=nullptr);
    ARIADNE_TEST_EQUALS(copy->name(), Identifier("named"));
    delete copy;

    // DiscreteMode objects are only created through HybridAutomaton.
    HybridAutomaton ha;
    RealVariable x("x");
    DiscreteLocation q1({q|"1"});
    ha.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
    DiscreteMode dm=ha.mode(q1);
    ARIADNE_TEST_EQUALS(dm.location(), q1);
}

Void TestHybridAutomatonApi::test_modes_and_errors()
{
    RealVariable x("x"), y("y"), z("z"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"}), q9({q|"9"});

    HybridAutomaton ha;
    // All new_mode overloads.
    ARIADNE_TEST_EXECUTE(ha.new_mode(q1,
        List<RealAssignment>{let(a)=x+1},
        List<DottedRealAssignment>{dot(x)=-x, dot(y)=y}));
    ARIADNE_TEST_EXECUTE(ha.new_mode(q2,
        List<DottedRealAssignment>{dot(x)=x},
        List<RealAssignment>{}));
    ARIADNE_TEST_THROWS(ha.new_mode(q1), IndistinguishableModeError);
    // Variable defined twice by the algebraic equations.
    ARIADNE_TEST_THROWS(
        HybridAutomaton().new_mode(q2,
            List<RealAssignment>{let(a)=x, let(a)=y},
            List<DottedRealAssignment>{}),
        SystemSpecificationError);
    // Differential variable also defined algebraically.
    ARIADNE_TEST_THROWS(
        HybridAutomaton().new_mode(q2,
            List<RealAssignment>{let(x)=1},
            List<DottedRealAssignment>{dot(x)=x}),
        SystemSpecificationError);
    // Overloads without a location all use the empty location, so each
    // needs a fresh automaton.
    ARIADNE_TEST_EXECUTE(HybridAutomaton().new_mode(
        List<RealAssignment>{let(a)=x}, List<DottedRealAssignment>{dot(x)=x}));
    ARIADNE_TEST_EXECUTE(HybridAutomaton().new_mode(
        List<DottedRealAssignment>{dot(x)=x}, List<RealAssignment>{let(a)=x}));
    ARIADNE_TEST_EXECUTE(HybridAutomaton().new_mode(
        List<DottedRealAssignment>{dot(x)=x}));
    ARIADNE_TEST_EXECUTE(HybridAutomaton().new_mode(
        List<RealAssignment>{let(a)=x}));
    HybridAutomaton hb;
    ARIADNE_TEST_EXECUTE(hb.new_mode(DiscreteLocation(1)));
    ARIADNE_TEST_EXECUTE(hb.new_mode(DiscreteLocation(2),
                                     List<DottedRealAssignment>{dot(x)=x}));
    ARIADNE_TEST_EXECUTE(hb.new_mode(DiscreteLocation(3),
                                     List<RealAssignment>{let(a)=x}));
    ARIADNE_TEST_THROWS(hb.new_mode(DiscreteLocation(1)),
                        IndistinguishableModeError);
    ARIADNE_TEST_EQUALS(ha.locations().size(), 2u);
}

Void TestHybridAutomatonApi::test_invariants_guards_transitions()
{
    RealVariable x("x");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"}), q9({q|"9"});
    DiscreteEvent e1("e1"), e2("e2"), e3("e3"), e4("e4"), e5("e5"), e6("e6");

    HybridAutomaton ha;
    ha.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
    ha.new_mode(q2, List<DottedRealAssignment>{dot(x)=-x});

    // Error paths for a nonexistent location.
    ARIADNE_TEST_THROWS(ha.new_invariant(q9, x<=1, e1), NonExistentModeError);
    ARIADNE_TEST_THROWS(ha.new_guard(q9, e1, x>=0, EventKind::PERMISSIVE),
                        NonExistentModeError);
    ARIADNE_TEST_THROWS(ha.new_action(q9, x<=1, e1, x>=0), NonExistentModeError);
    ARIADNE_TEST_THROWS(ha.new_action(q9, e1, x>=0), NonExistentModeError);

    ARIADNE_TEST_EXECUTE(ha.new_invariant(q1, x<=1, e1));
    ARIADNE_TEST_EXECUTE(ha.set_invariant(q1, x<=2, e1));
    ARIADNE_TEST_EXECUTE(ha.new_guard(q1, e2, x>=0, EventKind::PERMISSIVE));
    ARIADNE_TEST_EXECUTE(ha.set_guard(q1, e2, x>=1, EventKind::PERMISSIVE));
    ARIADNE_TEST_EXECUTE(ha.new_action(q1, x<=1, e3, x>=0, EventKind::PERMISSIVE));
    ARIADNE_TEST_EXECUTE(ha.new_action(q1, e4, x>=0, EventKind::URGENT));
    ARIADNE_TEST_EXECUTE(ha.new_action(q1, e5, x>=0, EventKind::IMPACT));
    // Wrong event kind for each new_action overload trips the assertion.
    ARIADNE_TEST_THROWS(ha.new_action(q1, x<=1, DiscreteEvent("w1"), x>=0,
                                      EventKind::URGENT),
                        std::runtime_error);
    ARIADNE_TEST_THROWS(ha.new_action(q1, DiscreteEvent("w2"), x>=0,
                                      EventKind::PERMISSIVE),
                        std::runtime_error);

    // Re-declaring a constraint for the same event is an error.
    ARIADNE_TEST_THROWS(ha.new_invariant(q1, x<=3, e1), MultipleGuardError);
    ARIADNE_TEST_THROWS(ha.new_guard(q1, e2, x<=3, EventKind::PERMISSIVE),
                        MultipleGuardError);
    ARIADNE_TEST_THROWS(ha.new_action(q1, x<=3, e3, x>=1), MultipleGuardError);

    // Transitions of each kind.
    ARIADNE_TEST_EXECUTE(ha.new_transition(q1, e6, q2,
        List<PrimedRealAssignment>{prime(x)=2*x}, x>=0, EventKind::PERMISSIVE));
    ARIADNE_TEST_EXECUTE(ha.new_transition(q2, DiscreteEvent("u"), q1,
        List<PrimedRealAssignment>{prime(x)=x}, x>=0, EventKind::URGENT));
    ARIADNE_TEST_EXECUTE(ha.new_transition(q2, DiscreteEvent("i"), q1, x>=0,
        EventKind::IMPACT));
    ARIADNE_TEST_EXECUTE(ha.new_transition(q1, DiscreteEvent("p"), q2, x>=0,
        EventKind::PERMISSIVE));
    ARIADNE_TEST_EXECUTE(ha.new_transition(q1, DiscreteEvent("k"), x>=0, q2,
        EventKind::PERMISSIVE));
    ARIADNE_TEST_EXECUTE(ha.new_update(q1, DiscreteEvent("w"), q2,
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TEST_EXECUTE(ha.new_update(q1, DiscreteEvent("x"), q2));
    // Overloads without a source location need an automaton with the
    // empty location.
    HybridAutomaton he;
    he.new_mode(DiscreteLocation(), List<DottedRealAssignment>{dot(x)=x});
    ARIADNE_TEST_EXECUTE(he.new_update(DiscreteEvent("y"),
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TEST_EXECUTE(ha.new_reset(q1, DiscreteEvent("z"), q2,
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TEST_EXECUTE(ha.new_reset(q1, DiscreteEvent("v"), q2));
    ARIADNE_TEST_EXECUTE(he.new_reset(DiscreteEvent("u2"),
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TEST_EXECUTE(ha.new_transition(q1, DiscreteEvent("n"), q2,
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TEST_EXECUTE(ha.new_transition(q1, DiscreteEvent("m"), q2));
    ARIADNE_TEST_EXECUTE(he.new_transition(DiscreteEvent("g"),
        List<PrimedRealAssignment>{prime(x)=x}, x>=0, EventKind::PERMISSIVE));
    // An unhandled event kind for a transition is rejected.
    ARIADNE_TEST_THROWS(ha.new_transition(q1, DiscreteEvent("b"), q2,
        List<PrimedRealAssignment>{}, x>=0, EventKind::INVARIANT),
        std::runtime_error);
    // Multiple transitions for the same event are rejected.
    ARIADNE_TEST_THROWS(ha.new_transition(q1, e6, q1,
        List<PrimedRealAssignment>{}, x>=0, EventKind::PERMISSIVE),
        MultipleGuardError);
}

Void TestHybridAutomatonApi::test_accessors()
{
    HybridAutomaton ha=_make_automaton();
    RealVariable x("x"), y("y"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"}), q9({q|"9"});
    DiscreteEvent e1("e1"), e2("e2"), e_none("none");

    ARIADNE_TEST_EQUALS(ha.locations().size(), 2u);
    ARIADNE_TEST_ASSERT(ha.has_location(q1));
    ARIADNE_TEST_ASSERT(!ha.has_location(q9));
    ARIADNE_TEST_ASSERT(ha.has_mode(q1));
    ARIADNE_TEST_ASSERT(ha.has_partial_mode(q1));
    ARIADNE_TEST_ASSERT(!ha.has_mode(q9));
    ARIADNE_TEST_ASSERT(ha.modes().has_key(q1));
    ARIADNE_TEST_ASSERT(ha.mode(q1).location()==q1);
    DiscreteMode& mutable_mode=ha.mode(q1);
    ARIADNE_TEST_ASSERT(mutable_mode.location()==q1);
    ARIADNE_TEST_THROWS(ha.mode(q9), SystemSpecificationError);

    ARIADNE_TEST_EQUALS(ha.dimension(q1), 2u);
    ARIADNE_TEST_ASSERT(ha.continuous_state_space(q1)==RealSpace({x,y}));
    ARIADNE_TEST_ASSERT(ha.continuous_auxiliary_space(q1)==RealSpace({a}));
    ARIADNE_TEST_ASSERT(ha.state_space().has_location(q1));
    ARIADNE_TEST_ASSERT(ha.state_auxiliary_space().has_location(q1));
    ARIADNE_TEST_EQUALS(ha.state_auxiliary_space()[q1].size(), 3u);
    ARIADNE_TEST_EQUALS(ha.state_variables(q1).size(), 2u);
    ARIADNE_TEST_EQUALS(ha.auxiliary_variables(q1).size(), 1u);
    ARIADNE_TEST_EQUALS(ha.auxiliary_assignments(q1).size(), 1u);
    ARIADNE_TEST_EQUALS(ha.dynamic_assignments(q1).size(), 2u);
    ARIADNE_TEST_ASSERT(ha.input_variables(q1).size()>=0u);
    ARIADNE_TEST_ASSERT(ha.has_invariant(q1,e1));
    ARIADNE_TEST_ASSERT(!ha.has_invariant(q1,e2));
    ARIADNE_TEST_ASSERT(ha.has_guard(q1,e2));
    ARIADNE_TEST_ASSERT(ha.has_transition(q1,e2));
    ARIADNE_TEST_ASSERT(!ha.has_transition(q1,e1));
    // Short-circuit when the source location does not exist.
    ARIADNE_TEST_ASSERT(!ha.has_transition(q9,e2));
    ARIADNE_TEST_ASSERT(!ha.has_invariant(q9,e1));
    ARIADNE_TEST_ASSERT(!ha.has_guard(q9,e2));
    ARIADNE_TEST_EQUALS(ha.event_kind(q1,e1), EventKind::INVARIANT);
    ARIADNE_TEST_EQUALS(ha.event_kind(q1,e2), EventKind::PERMISSIVE);
    ARIADNE_TEST_ASSERT(ha.events(q1).size()>=2u);
    ARIADNE_TEST_EQUALS(ha.target(q1,e2), q2);
    ARIADNE_TEST_EQUALS(ha.target(q1,e_none), q1);
    ARIADNE_TEST_EQUALS(ha.reset_assignments(q1,e2).size(), 1u);
    ARIADNE_TEST_EQUALS(ha.reset_assignments(q1,e_none).size(), 2u);
    ARIADNE_TEST_PRINT(ha.invariant_predicate(q1,e1));
    ARIADNE_TEST_PRINT(ha.guard_predicate(q1,e2));
    ARIADNE_TEST_PRINT(ha.invariant_predicate(q1,e_none));
    ARIADNE_TEST_PRINT(ha.guard_predicate(q1,e_none));
    ARIADNE_TEST_PRINT(ha.auxiliary_function(q1));
    ARIADNE_TEST_PRINT(ha.dynamic_function(q1));
    ARIADNE_TEST_PRINT(ha.invariant_function(q1,e1));
    ARIADNE_TEST_PRINT(ha.guard_function(q1,e2));
    ARIADNE_TEST_PRINT(ha.reset_function(q1,e2));
    ARIADNE_TEST_EXECUTE(ha.check_mode(q1));
    ARIADNE_TEST_EXECUTE(ha.check_mode(q2));
    ARIADNE_TEST_EXECUTE(ha.check_reachable_modes(Set<DiscreteLocation>{q1}));
    ARIADNE_TEST_ASSERT(ha.discrete_reachability(Set<DiscreteLocation>{q1}).contains(q2));
    // The interface default for auxiliary_expressions is an unconditional
    // assertion failure (HybridAutomaton does not override it).
    ARIADNE_TEST_THROWS(ha.auxiliary_expressions(q1), std::runtime_error);
}

Void TestHybridAutomatonApi::test_check_mode_errors()
{
    RealVariable x("x"), z("z"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"});
    DiscreteEvent e1("e1"), e2("e2"), e3("e3");

    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<RealAssignment>{let(a)=z},
                        List<DottedRealAssignment>{dot(x)=x});
        ARIADNE_TEST_THROWS(bad.check_mode(q1), UnderspecifiedDynamicError);
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<RealAssignment>{let(a)=x},
                        List<DottedRealAssignment>{dot(x)=x+z});
        ARIADNE_TEST_THROWS(bad.check_mode(q1), UnderspecifiedDynamicError);
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_invariant(q1, z<=1, e1);
        ARIADNE_TEST_THROWS(bad.check_mode(q1), UnderspecifiedConstraintError);
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_guard(q1, e2, z>=0, EventKind::PERMISSIVE);
        ARIADNE_TEST_THROWS(bad.check_mode(q1), UnderspecifiedConstraintError);
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        bad.new_transition(q1, e3, q2,
            List<PrimedRealAssignment>{prime(x)=z}, x>=0, EventKind::PERMISSIVE);
        ARIADNE_TEST_THROWS(bad.check_mode(q1), UnderspecifiedResetError);
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x)=x, dot(a)=a});
        bad.new_transition(q1, e3, q2,
            List<PrimedRealAssignment>{prime(x)=x}, x>=0, EventKind::PERMISSIVE);
        // The reset does not define the target variable a.
        ARIADNE_TEST_THROWS(bad.check_mode(q1), UnderspecifiedResetError);
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        // The reset defines y, which is not a state variable of the target.
        bad.new_transition(q1, e3, q2,
            List<PrimedRealAssignment>{prime(x)=x, prime(a)=x}, x>=0,
            EventKind::PERMISSIVE);
        ARIADNE_TEST_THROWS(bad.check_mode(q1), UnderspecifiedDynamicError);
    }
}

Void TestHybridAutomatonApi::test_writers_and_streaming()
{
    HybridAutomaton ha=_make_automaton();
    RealVariable x("x");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"});
    DiscreteEvent e1("e1");

    std::ostringstream verbose;
    verbose << ha;
    ARIADNE_TEST_ASSERT(verbose.str().size()>0u);

    Writer<HybridAutomaton> previous=HybridAutomaton::default_writer();
    HybridAutomaton::set_default_writer(CompactHybridAutomatonWriter());
    std::ostringstream compact;
    compact << ha;
    ARIADNE_TEST_ASSERT(compact.str().size()>0u);
    // A single-mode automaton takes the non-multiple branch.
    HybridAutomaton single_mode;
    single_mode.new_mode(DiscreteLocation(1), List<DottedRealAssignment>{dot(x)=x});
    std::ostringstream compact_single;
    compact_single << single_mode;
    ARIADNE_TEST_ASSERT(compact_single.str().size()>0u);
    HybridAutomaton::set_default_writer(previous);

    DiscreteMode mode=ha.mode(q1);
    std::ostringstream mode_verbose;
    mode_verbose << mode;
    Writer<DiscreteMode> previous_mode=DiscreteMode::default_writer();
    DiscreteMode::set_default_writer(CompactDiscreteModeWriter());
    std::ostringstream mode_compact;
    mode_compact << mode;
    ARIADNE_TEST_ASSERT(mode_compact.str().size()>0u);
    DiscreteMode::set_default_writer(previous_mode);

    std::ostringstream kinds;
    kinds << EventKind::INVARIANT << " " << EventKind::PROGRESS << " "
          << EventKind::PERMISSIVE << " " << EventKind::URGENT << " "
          << EventKind::IMPACT;
    ARIADNE_TEST_PRINT(kinds.str());
}

Void TestHybridAutomatonApi::test()
{
    ARIADNE_TEST_CALL(test_constructors());
    ARIADNE_TEST_CALL(test_modes_and_errors());
    ARIADNE_TEST_CALL(test_invariants_guards_transitions());
    ARIADNE_TEST_CALL(test_accessors());
    ARIADNE_TEST_CALL(test_check_mode_errors());
    ARIADNE_TEST_CALL(test_writers_and_streaming());
}

Int main()
{
    ARIADNE_TEST_CALL(TestHybridAutomatonApi().test());
    return ARIADNE_TEST_FAILURES;
}
