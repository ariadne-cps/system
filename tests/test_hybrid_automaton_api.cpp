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

// Exercises the full public API of HybridAutomaton / DiscreteMode /
// DiscreteTransition: constructors, mode/transition building (including
// every error path), accessors, check_mode validation errors, and the
// verbose/compact writers plus EventKind streaming.

#include <sstream>

#include "ariadne-testing.hpp"

#include "symbolic/variable.hpp"
#include "symbolic/expression.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

#include "hybrid/hybrid_space.hpp"

#include "system/hybrid_automaton_interface.hpp"
#include "system/hybrid_automaton.hpp"

using namespace Ariadne;

class TestHybridAutomatonApi : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;
  public:
    Void test_constructors();
    Void test_modes_and_errors();
    Void test_invariants_guards_transitions();
    Void test_accessors();
    Void test_check_mode_errors();
    Void test_writers_and_streaming();

  private:
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
    ARIADNE_TESTING_EXPECT_EQUAL(ha1.name(), Identifier("named"));
    ARIADNE_TESTING_EXPECT_EQUAL(ha0.locations().size(), 0u);
    HybridAutomaton* copy=ha1.clone();
    ARIADNE_TESTING_PROVE(copy!=nullptr);
    ARIADNE_TESTING_EXPECT_EQUAL(copy->name(), Identifier("named"));
    delete copy;

    // DiscreteMode objects are only created through HybridAutomaton.
    HybridAutomaton ha;
    RealVariable x("x");
    DiscreteLocation q1({q|"1"});
    ha.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
    DiscreteMode dm=ha.mode(q1);
    ARIADNE_TESTING_EXPECT_EQUAL(dm.location(), q1);
}

Void TestHybridAutomatonApi::test_modes_and_errors()
{
    RealVariable x("x"), y("y"), z("z"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"}), q9({q|"9"});

    HybridAutomaton ha;
    // All new_mode overloads.
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_mode(q1,
        List<RealAssignment>{let(a)=x+1},
        List<DottedRealAssignment>{dot(x)=-x, dot(y)=y}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_mode(q2,
        List<DottedRealAssignment>{dot(x)=x},
        List<RealAssignment>{}));
    ARIADNE_TESTING_EXPECT_THROWS(IndistinguishableModeError, ha.new_mode(q1));
    // Variable defined twice by the algebraic equations.
    ARIADNE_TESTING_EXPECT_THROWS(SystemSpecificationError, HybridAutomaton().new_mode(q2,
            List<RealAssignment>{let(a)=x, let(a)=y},
            List<DottedRealAssignment>{}));
    // Differential variable also defined algebraically.
    ARIADNE_TESTING_EXPECT_THROWS(SystemSpecificationError, HybridAutomaton().new_mode(q2,
            List<RealAssignment>{let(x)=1},
            List<DottedRealAssignment>{dot(x)=x}));
    // Overloads without a location all use the empty location, so each
    // needs a fresh automaton.
    ARIADNE_TESTING_EXPECT_NO_THROW(HybridAutomaton().new_mode(
        List<RealAssignment>{let(a)=x}, List<DottedRealAssignment>{dot(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(HybridAutomaton().new_mode(
        List<DottedRealAssignment>{dot(x)=x}, List<RealAssignment>{let(a)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(HybridAutomaton().new_mode(
        List<DottedRealAssignment>{dot(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(HybridAutomaton().new_mode(
        List<RealAssignment>{let(a)=x}));
    HybridAutomaton hb;
    ARIADNE_TESTING_EXPECT_NO_THROW(hb.new_mode(DiscreteLocation(1)));
    ARIADNE_TESTING_EXPECT_NO_THROW(hb.new_mode(DiscreteLocation(2),
                                     List<DottedRealAssignment>{dot(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(hb.new_mode(DiscreteLocation(3),
                                     List<RealAssignment>{let(a)=x}));
    ARIADNE_TESTING_EXPECT_THROWS(IndistinguishableModeError, hb.new_mode(DiscreteLocation(1)));
    ARIADNE_TESTING_EXPECT_EQUAL(ha.locations().size(), 2u);
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
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_invariant(q9, x<=1, e1));
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_guard(q9, e1, x>=0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_action(q9, x<=1, e1, x>=0));
    ARIADNE_TESTING_EXPECT_THROWS(NonExistentModeError, ha.new_action(q9, e1, x>=0));

    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_invariant(q1, x<=1, e1));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.set_invariant(q1, x<=2, e1));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_guard(q1, e2, x>=0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.set_guard(q1, e2, x>=1, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_action(q1, x<=1, e3, x>=0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_action(q1, e4, x>=0, EventKind::URGENT));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_action(q1, e5, x>=0, EventKind::IMPACT));
    // Wrong event kind for each new_action overload trips the assertion.
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, ha.new_action(q1, x<=1, DiscreteEvent("w1"), x>=0,
                                      EventKind::URGENT));
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, ha.new_action(q1, DiscreteEvent("w2"), x>=0,
                                      EventKind::PERMISSIVE));

    // Re-declaring a constraint for the same event is an error.
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_invariant(q1, x<=3, e1));
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_guard(q1, e2, x<=3, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_action(q1, x<=3, e3, x>=1));

    // Transitions of each kind.
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, e6, q2,
        List<PrimedRealAssignment>{prime(x)=2*x}, x>=0, EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q2, DiscreteEvent("u"), q1,
        List<PrimedRealAssignment>{prime(x)=x}, x>=0, EventKind::URGENT));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q2, DiscreteEvent("i"), q1, x>=0,
        EventKind::IMPACT));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("p"), q2, x>=0,
        EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("k"), x>=0, q2,
        EventKind::PERMISSIVE));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_update(q1, DiscreteEvent("w"), q2,
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_update(q1, DiscreteEvent("x"), q2));
    // Overloads without a source location need an automaton with the
    // empty location.
    HybridAutomaton he;
    he.new_mode(DiscreteLocation(), List<DottedRealAssignment>{dot(x)=x});
    ARIADNE_TESTING_EXPECT_NO_THROW(he.new_update(DiscreteEvent("y"),
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_reset(q1, DiscreteEvent("z"), q2,
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_reset(q1, DiscreteEvent("v"), q2));
    ARIADNE_TESTING_EXPECT_NO_THROW(he.new_reset(DiscreteEvent("u2"),
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("n"), q2,
        List<PrimedRealAssignment>{prime(x)=x}));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.new_transition(q1, DiscreteEvent("m"), q2));
    ARIADNE_TESTING_EXPECT_NO_THROW(he.new_transition(DiscreteEvent("g"),
        List<PrimedRealAssignment>{prime(x)=x}, x>=0, EventKind::PERMISSIVE));
    // An unhandled event kind for a transition is rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error, ha.new_transition(q1, DiscreteEvent("b"), q2,
        List<PrimedRealAssignment>{}, x>=0, EventKind::INVARIANT));
    // Multiple transitions for the same event are rejected.
    ARIADNE_TESTING_EXPECT_THROWS(MultipleGuardError, ha.new_transition(q1, e6, q1,
        List<PrimedRealAssignment>{}, x>=0, EventKind::PERMISSIVE));
}

Void TestHybridAutomatonApi::test_accessors()
{
    HybridAutomaton ha=_make_automaton();
    RealVariable x("x"), y("y"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"}), q9({q|"9"});
    DiscreteEvent e1("e1"), e2("e2"), e_none("none");

    ARIADNE_TESTING_EXPECT_EQUAL(ha.locations().size(), 2u);
    ARIADNE_TESTING_PROVE(ha.has_location(q1));
    ARIADNE_TESTING_PROVE(!ha.has_location(q9));
    ARIADNE_TESTING_PROVE(ha.has_mode(q1));
    ARIADNE_TESTING_PROVE(ha.has_partial_mode(q1));
    ARIADNE_TESTING_PROVE(!ha.has_mode(q9));
    ARIADNE_TESTING_PROVE(ha.modes().has_key(q1));
    ARIADNE_TESTING_PROVE(ha.mode(q1).location()==q1);
    DiscreteMode& mutable_mode=ha.mode(q1);
    ARIADNE_TESTING_PROVE(mutable_mode.location()==q1);
    ARIADNE_TESTING_EXPECT_THROWS(SystemSpecificationError, ha.mode(q9));

    ARIADNE_TESTING_EXPECT_EQUAL(ha.dimension(q1), 2u);
    ARIADNE_TESTING_PROVE(ha.continuous_state_space(q1)==RealSpace({x,y}));
    ARIADNE_TESTING_PROVE(ha.continuous_auxiliary_space(q1)==RealSpace({a}));
    ARIADNE_TESTING_PROVE(ha.state_space().has_location(q1));
    ARIADNE_TESTING_PROVE(ha.state_auxiliary_space().has_location(q1));
    ARIADNE_TESTING_EXPECT_EQUAL(ha.state_auxiliary_space()[q1].size(), 3u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.state_variables(q1).size(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.auxiliary_variables(q1).size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.auxiliary_assignments(q1).size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.dynamic_assignments(q1).size(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.input_variables(q1).size(), 0u);
    ARIADNE_TESTING_PROVE(ha.has_invariant(q1,e1));
    ARIADNE_TESTING_PROVE(!ha.has_invariant(q1,e2));
    ARIADNE_TESTING_PROVE(ha.has_guard(q1,e2));
    ARIADNE_TESTING_PROVE(ha.has_transition(q1,e2));
    ARIADNE_TESTING_PROVE(!ha.has_transition(q1,e1));
    // Short-circuit when the source location does not exist.
    ARIADNE_TESTING_PROVE(!ha.has_transition(q9,e2));
    ARIADNE_TESTING_PROVE(!ha.has_invariant(q9,e1));
    ARIADNE_TESTING_PROVE(!ha.has_guard(q9,e2));
    ARIADNE_TESTING_EXPECT_EQUAL(ha.event_kind(q1,e1), EventKind::INVARIANT);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.event_kind(q1,e2), EventKind::PERMISSIVE);
    ARIADNE_TESTING_PROVE(ha.events(q1).size()>=2u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.target(q1,e2), q2);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.target(q1,e_none), q1);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.reset_assignments(q1,e2).size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(ha.reset_assignments(q1,e_none).size(), 2u);
    ARIADNE_TESTING_NOTE(ha.invariant_predicate(q1,e1));
    ARIADNE_TESTING_NOTE(ha.guard_predicate(q1,e2));
    ARIADNE_TESTING_NOTE(ha.invariant_predicate(q1,e_none));
    ARIADNE_TESTING_NOTE(ha.guard_predicate(q1,e_none));
    ARIADNE_TESTING_NOTE(ha.auxiliary_function(q1));
    ARIADNE_TESTING_NOTE(ha.dynamic_function(q1));
    ARIADNE_TESTING_NOTE(ha.invariant_function(q1,e1));
    ARIADNE_TESTING_NOTE(ha.guard_function(q1,e2));
    ARIADNE_TESTING_NOTE(ha.reset_function(q1,e2));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.check_mode(q1));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.check_mode(q2));
    ARIADNE_TESTING_EXPECT_NO_THROW(ha.check_reachable_modes(Set<DiscreteLocation>{q1}));
    ARIADNE_TESTING_PROVE(ha.discrete_reachability(Set<DiscreteLocation>{q1}).contains(q2));
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
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedDynamicError, bad.check_mode(q1));
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<RealAssignment>{let(a)=x},
                        List<DottedRealAssignment>{dot(x)=x+z});
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedDynamicError, bad.check_mode(q1));
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_invariant(q1, z<=1, e1);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedConstraintError, bad.check_mode(q1));
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_guard(q1, e2, z>=0, EventKind::PERMISSIVE);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedConstraintError, bad.check_mode(q1));
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        bad.new_transition(q1, e3, q2,
            List<PrimedRealAssignment>{prime(x)=z}, x>=0, EventKind::PERMISSIVE);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedResetError, bad.check_mode(q1));
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x)=x, dot(a)=a});
        bad.new_transition(q1, e3, q2,
            List<PrimedRealAssignment>{prime(x)=x}, x>=0, EventKind::PERMISSIVE);
        // The reset does not define the target variable a.
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedResetError, bad.check_mode(q1));
    }
    {
        HybridAutomaton bad;
        bad.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        bad.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        // The reset defines y, which is not a state variable of the target.
        bad.new_transition(q1, e3, q2,
            List<PrimedRealAssignment>{prime(x)=x, prime(a)=x}, x>=0,
            EventKind::PERMISSIVE);
        ARIADNE_TESTING_EXPECT_THROWS(UnderspecifiedDynamicError, bad.check_mode(q1));
    }
}

Void TestHybridAutomatonApi::test_writers_and_streaming()
{
    HybridAutomaton ha=_make_automaton();
    RealVariable x("x"), a("a");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"});
    DiscreteEvent e1("e1");

    std::ostringstream verbose;
    verbose << ha;
    ARIADNE_TESTING_PROVE(verbose.str().size()>0u);

    Writer<HybridAutomaton> previous=HybridAutomaton::default_writer();
    HybridAutomaton::set_default_writer(CompactHybridAutomatonWriter());
    std::ostringstream compact;
    compact << ha;
    ARIADNE_TESTING_PROVE(compact.str().size()>0u);
    // A single-mode automaton takes the non-multiple branch.
    HybridAutomaton single_mode;
    single_mode.new_mode(DiscreteLocation(1), List<DottedRealAssignment>{dot(x)=x});
    std::ostringstream compact_single;
    compact_single << single_mode;
    ARIADNE_TESTING_PROVE(compact_single.str().size()>0u);
    HybridAutomaton::set_default_writer(previous);

    DiscreteMode mode=ha.mode(q1);
    std::ostringstream mode_verbose;
    mode_verbose << mode;
    Writer<DiscreteMode> previous_mode=DiscreteMode::default_writer();
    DiscreteMode::set_default_writer(CompactDiscreteModeWriter());
    std::ostringstream mode_compact;
    mode_compact << mode;
    ARIADNE_TESTING_PROVE(mode_compact.str().size()>0u);
    // Empty, auxiliary-only, dynamic-only and reset-less modes cover the
    // remaining compact-writer branches.
    {
        HybridAutomaton bare;
        bare.new_mode(DiscreteLocation(1), List<RealAssignment>{let(a)=1});
        bare.new_mode(DiscreteLocation(2), List<DottedRealAssignment>{dot(x)=x});
        bare.new_mode(DiscreteLocation(3));
        bare.new_transition(DiscreteLocation(3), DiscreteEvent("t"),
                            DiscreteLocation(3));
        for (auto const& m : bare.modes().values()) {
            std::ostringstream os;
            os << m;
            ARIADNE_TESTING_PROVE(os.str().size()>0u);
        }
        HybridAutomaton bare_empty;
        bare_empty.new_mode(DiscreteLocation());
        {
            // A completely empty mode prints as the empty string in compact form.
            std::ostringstream os;
            os << bare_empty.mode(DiscreteLocation());
        }
    }
    DiscreteMode::set_default_writer(previous_mode);

    std::ostringstream kinds;
    kinds << EventKind::INVARIANT << " " << EventKind::PROGRESS << " "
          << EventKind::PERMISSIVE << " " << EventKind::URGENT << " "
          << EventKind::IMPACT;
    ARIADNE_TESTING_NOTE(kinds.str());
}




int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_automaton_api");
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_constructors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_modes_and_errors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_invariants_guards_transitions);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_accessors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_check_mode_errors);
    ARIADNE_TESTING_ADD_TEST(TestHybridAutomatonApi, test_writers_and_streaming);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}
