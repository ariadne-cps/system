/***************************************************************************
 *            test_hybrid_automaton_composite.cpp
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

// Exercises CompositeHybridAutomaton: constructors, component access,
// mode caching, all delegating accessors, the composite hybrid spaces,
// check_mode validation (success and every error), reachability analysis,
// the MissingGuardError path, and the verbose/compact writers.

#include <sstream>

#include "macro/testing.hpp"

#include "symbolic/variable.hpp"
#include "symbolic/expression.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

#include "hybrid/hybrid_space.hpp"
#include "logging/logging.hpp"

#include "system/hybrid_automaton_interface.hpp"
#include "system/hybrid_automaton.hpp"
#include "system/hybrid_automaton-composite.hpp"

using namespace Ariadne;

class TestCompositeHybridAutomaton {
  public:
    Void test();
  private:
    HybridAutomaton _component(Identifier name, Identifier variable) const;
    CompositeHybridAutomaton _composite() const;

    Void test_constructors_and_components();
    Void test_accessors();
    Void test_spaces();
    Void test_writers();
    Void test_check_mode_success();
    Void test_check_mode_errors();
    Void test_missing_guard_error();
    Void test_predicate_combination();
    Void test_reachability();
};

HybridAutomaton TestCompositeHybridAutomaton::_component(Identifier name, Identifier variable) const
{
    RealVariable x(variable);
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"});
    DiscreteEvent e("e");
    HybridAutomaton automaton(name);
    automaton.new_mode(q1, List<DottedRealAssignment>{dot(x)=-x});
    automaton.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
    automaton.new_transition(q1, e, q2,
        List<PrimedRealAssignment>{prime(x)=x}, x>=0, EventKind::PERMISSIVE);
    return automaton;
}

CompositeHybridAutomaton TestCompositeHybridAutomaton::_composite() const
{
    return CompositeHybridAutomaton(List<HybridAutomaton>{
        _component("a","x"), _component("b","y")});
}

Void TestCompositeHybridAutomaton::test_constructors_and_components()
{
    CompositeHybridAutomaton empty;
    ARIADNE_TEST_EQUALS(empty.number_of_components(), 0u);
    ARIADNE_TEST_EQUALS(empty.name(), Identifier("system"));

    CompositeHybridAutomaton named("custom");
    ARIADNE_TEST_EQUALS(named.name(), Identifier("custom"));

    HybridAutomaton solo=_component("solo","x");
    CompositeHybridAutomaton from_single(solo);
    ARIADNE_TEST_EQUALS(from_single.number_of_components(), 1u);
    ARIADNE_TEST_EQUALS(from_single.name(), Identifier("solo"));

    CompositeHybridAutomaton pair(List<HybridAutomaton>{_component("a","x"), _component("b","y")});
    ARIADNE_TEST_EQUALS(pair.number_of_components(), 2u);
    ARIADNE_TEST_EQUALS(pair.name(), Identifier("a&b"));
    ARIADNE_TEST_EQUALS(pair.component(0u).name(), Identifier("a"));
    ARIADNE_TEST_EQUALS(pair.component(1u).name(), Identifier("b"));

    CompositeHybridAutomaton named_two("named", List<HybridAutomaton>{_component("a","x")});
    ARIADNE_TEST_EQUALS(named_two.name(), Identifier("named"));
    ARIADNE_TEST_EQUALS(named_two.number_of_components(), 1u);

    CompositeHybridAutomaton composed=parallel_composition(
        List<HybridAutomaton>{_component("a","x"), _component("b","y")});
    ARIADNE_TEST_EQUALS(composed.name(), Identifier("a&b"));
    ARIADNE_TEST_EQUALS(composed.number_of_components(), 2u);

    CompositeHybridAutomaton* copy=pair.clone();
    ARIADNE_TEST_ASSERT(copy!=nullptr);
    ARIADNE_TEST_EQUALS(copy->number_of_components(), 2u);
    delete copy;
    SizeType counted=0u;
    for (auto it=pair.begin(); it!=pair.end(); ++it) ++counted;
    ARIADNE_TEST_EQUALS(counted, 2u);
}

Void TestCompositeHybridAutomaton::test_accessors()
{
    CompositeHybridAutomaton system=_composite();
    RealVariable x("x"), y("y");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"}), q9({q|"9"});
    DiscreteEvent e("e"), e_none("none");

    ARIADNE_TEST_ASSERT(system.has_mode(q1));
    ARIADNE_TEST_ASSERT(system.has_mode(q2));
    ARIADNE_TEST_ASSERT(!system.has_mode(q9));
    ARIADNE_TEST_ASSERT(system.has_partial_mode(q1));
    ARIADNE_TEST_ASSERT(!system.has_partial_mode(q9));
    ARIADNE_TEST_ASSERT(system.has_transition(q1,e));
    ARIADNE_TEST_ASSERT(!system.has_transition(q1,e_none));
    ARIADNE_TEST_ASSERT(system.has_guard(q1,e));
    ARIADNE_TEST_ASSERT(!system.has_guard(q1,e_none));
    ARIADNE_TEST_ASSERT(!system.has_invariant(q1,e));
    // A component with an invariant makes has_invariant true.
    {
        HybridAutomaton a=_component("a","x");
        DiscreteEvent inv_event("inv");
        a.new_invariant(q1, x<=1, inv_event);
        CompositeHybridAutomaton with_inv(a);
        ARIADNE_TEST_ASSERT(with_inv.has_invariant(q1,inv_event));
    }

    ARIADNE_TEST_EQUALS(system.mode(q1).location(), q1);
    ARIADNE_TEST_EQUALS(system.mode(q1).location(), q1); // cached
    ARIADNE_TEST_EQUALS(system.dimension(q1), 2u);
    ARIADNE_TEST_EQUALS(system.state_variables(q1).size(), 2u);
    ARIADNE_TEST_EQUALS(system.auxiliary_variables(q1).size(), 0u);
    ARIADNE_TEST_EQUALS(system.variables(q1).size(), 2u);
    ARIADNE_TEST_EQUALS(system.auxiliary_assignments(q1).size(), 0u);
    ARIADNE_TEST_EQUALS(system.sorted_auxiliary_assignments(q1).size(), 0u);
    ARIADNE_TEST_EQUALS(system.dynamic_assignments(q1).size(), 2u);
    ARIADNE_TEST_EQUALS(system.reset_assignments(q1,e).size(), 2u);
    ARIADNE_TEST_EQUALS(system.reset_assignments(q1,e_none).size(), 2u);
    ARIADNE_TEST_ASSERT(system.continuous_state_space(q1)==RealSpace({x,y}));
    ARIADNE_TEST_EQUALS(system.continuous_auxiliary_space(q1).size(), 0u);
    ARIADNE_TEST_EQUALS(system.continuous_state_auxiliary_space(q1).size(), 2u);
    ARIADNE_TEST_ASSERT(system.events(q1).contains(e));
    ARIADNE_TEST_EQUALS(system.event_kind(q1,e), EventKind::PERMISSIVE);
    ARIADNE_TEST_EQUALS(system.target(q1,e), q2);
    ARIADNE_TEST_PRINT(system.invariant_predicate(q1,e));
    ARIADNE_TEST_PRINT(system.guard_predicate(q1,e));
    ARIADNE_TEST_PRINT(system.auxiliary_function(q1));
    ARIADNE_TEST_PRINT(system.dynamic_function(q1));
    ARIADNE_TEST_PRINT(system.reset_function(q1,e));
    ARIADNE_TEST_PRINT(system.invariant_function(q1,e));
    ARIADNE_TEST_PRINT(system.guard_function(q1,e));
}

Void TestCompositeHybridAutomaton::test_spaces()
{
    CompositeHybridAutomaton system=_composite();
    StringVariable q("q");
    DiscreteLocation q1({q|"1"});

    HybridSpace state_space=system.state_space();
    ARIADNE_TEST_ASSERT(state_space.has_location(q1));
    ARIADNE_TEST_ASSERT(state_space[q1]==RealSpace({RealVariable("x"),RealVariable("y")}));
    HybridSpace copy=state_space;
    ARIADNE_TEST_ASSERT(definitely(copy==state_space));
    // Cloning through the interface copies the composite space view.
    HybridSpace cloned(static_cast<const HybridSpaceInterface&>(state_space));
    ARIADNE_TEST_ASSERT(definitely(cloned==state_space));
    MonolithicHybridSpace other;
    ARIADNE_TEST_ASSERT(!definitely(copy==HybridSpace(other)));
    // Same view type but different automaton: indeterminate.
    ARIADNE_TEST_ASSERT(!definitely(state_space==_composite().state_space()));
    std::ostringstream os;
    os << state_space;
    ARIADNE_TEST_ASSERT(os.str().size()>0u);

    HybridSpace aux_space=system.state_auxiliary_space();
    ARIADNE_TEST_ASSERT(aux_space.has_location(q1));
    ARIADNE_TEST_EQUALS(aux_space[q1].size(), 2u);
    HybridSpace aux_cloned(static_cast<const HybridSpaceInterface&>(aux_space));
    ARIADNE_TEST_ASSERT(definitely(aux_space==aux_space));
    ARIADNE_TEST_ASSERT(!definitely(aux_space==_composite().state_auxiliary_space()));
    MonolithicHybridSpace other_aux;
    ARIADNE_TEST_ASSERT(!definitely(aux_space==HybridSpace(other_aux)));
    std::ostringstream os2;
    os2 << aux_space;
    ARIADNE_TEST_ASSERT(os2.str().size()>0u);
    (void)aux_cloned;
}

Void TestCompositeHybridAutomaton::test_writers()
{
    CompositeHybridAutomaton system=_composite();
    std::ostringstream verbose;
    verbose << system;
    ARIADNE_TEST_ASSERT(verbose.str().size()>0u);

    Writer<CompositeHybridAutomaton> previous=CompositeHybridAutomaton::default_writer();
    CompositeHybridAutomaton::set_default_writer(CompactCompositeHybridAutomatonWriter());
    std::ostringstream compact;
    compact << system;
    ARIADNE_TEST_ASSERT(compact.str().size()>0u);
    CompositeHybridAutomaton::set_default_writer(previous);

    // A single-component composite takes the non-multiple branch.
    CompositeHybridAutomaton single_comp(_component("solo","x"));
    CompositeHybridAutomaton::set_default_writer(CompactCompositeHybridAutomatonWriter());
    std::ostringstream compact_single;
    compact_single << single_comp;
    ARIADNE_TEST_ASSERT(compact_single.str().size()>0u);
    CompositeHybridAutomaton::set_default_writer(previous);

    // Stream through the interface base class as well.
    HybridAutomatonInterface const& interface=system;
    std::ostringstream via_interface;
    via_interface << interface;
    ARIADNE_TEST_ASSERT(via_interface.str().size()>0u);
}

Void TestCompositeHybridAutomaton::test_check_mode_success()
{
    CompositeHybridAutomaton system=_composite();
    StringVariable q("q");
    DiscreteLocation q1({q|"1"});
    ARIADNE_TEST_EXECUTE(system.check_mode(q1));
    ARIADNE_TEST_EXECUTE(system.check_reachable_modes(Set<DiscreteLocation>{q1}));
    // Chained auxiliary assignments exercise the topological ordering.
    {
        RealVariable x("x"), a("a"), b("b");
        HybridAutomaton c("c");
        c.new_mode(q1, List<RealAssignment>{let(a)=x, let(b)=a+1},
                      List<DottedRealAssignment>{dot(x)=a});
        CompositeHybridAutomaton with_aux(c);
        ARIADNE_TEST_EXECUTE(with_aux.check_mode(q1));
    }
}

Void TestCompositeHybridAutomaton::test_check_mode_errors()
{
    RealVariable x("x"), y("y"), z("z");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"});
    DiscreteEvent e("e");

    // Same variable defined by two components: overspecified.
    {
        HybridAutomaton a("a"), b("b");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=-x});
        b.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        CompositeHybridAutomaton system(List<HybridAutomaton>{a,b});
        ARIADNE_TEST_THROWS(system.check_mode(q1), OverspecifiedDynamicError);
    }
    // Algebraic equation uses an undefined variable.
    {
        HybridAutomaton a("a");
        a.new_mode(q1, List<RealAssignment>{let(y)=z},
                      List<DottedRealAssignment>{dot(x)=x});
        CompositeHybridAutomaton system(a);
        ARIADNE_TEST_THROWS(system.check_mode(q1), UnderspecifiedDynamicError);
    }
    // Dynamics use an undefined variable.
    {
        HybridAutomaton a("a");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x+z});
        CompositeHybridAutomaton system(a);
        ARIADNE_TEST_THROWS(system.check_mode(q1), UnderspecifiedDynamicError);
    }
    // Invariant uses an undefined variable.
    {
        HybridAutomaton a("a");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_invariant(q1, z<=1, e);
        CompositeHybridAutomaton system(a);
        ARIADNE_TEST_THROWS(system.check_mode(q1), UnderspecifiedConstraintError);
    }
    // Guard uses an undefined variable.
    {
        HybridAutomaton a("a");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        a.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(x)=x},
                         z>=0, EventKind::PERMISSIVE);
        CompositeHybridAutomaton system(a);
        ARIADNE_TEST_THROWS(system.check_mode(q1), UnderspecifiedConstraintError);
    }
    // Reset uses an undefined variable.
    {
        HybridAutomaton a("a");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        a.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(x)=z},
                         x>=0, EventKind::PERMISSIVE);
        CompositeHybridAutomaton system(a);
        ARIADNE_TEST_THROWS(system.check_mode(q1), UnderspecifiedResetError);
    }
    // Two components reset the same variable: overspecified combined reset.
    {
        HybridAutomaton a("a"), b("b");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        a.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(x)=x},
                         x>=0, EventKind::PERMISSIVE);
        b.new_mode(q1, List<DottedRealAssignment>{dot(y)=y});
        b.new_mode(q2, List<DottedRealAssignment>{dot(y)=y});
        b.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(x)=y},
                         y>=0, EventKind::PERMISSIVE);
        CompositeHybridAutomaton system(List<HybridAutomaton>{a,b});
        ARIADNE_TEST_THROWS(system.check_mode(q1), OverspecifiedResetError);
    }
    // Reset defines a variable that is not a state variable of the target.
    {
        HybridAutomaton a("a");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        a.new_transition(q1, e, q2,
            List<PrimedRealAssignment>{prime(x)=x, prime(y)=x},
            x>=0, EventKind::PERMISSIVE);
        CompositeHybridAutomaton system(a);
        ARIADNE_TEST_THROWS(system.check_mode(q1), SystemSpecificationError);
    }
    // Target state variable missing from the reset.
    {
        HybridAutomaton a("a");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x, dot(y)=y});
        a.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(x)=x},
                         x>=0, EventKind::PERMISSIVE);
        CompositeHybridAutomaton system(a);
        ARIADNE_TEST_THROWS(system.check_mode(q1), UnderspecifiedResetError);
    }
    // Two components declare the event urgent.
    {
        HybridAutomaton a("a"), b("b");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        a.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(x)=x},
                         x>=0, EventKind::URGENT);
        b.new_mode(q1, List<DottedRealAssignment>{dot(y)=y});
        b.new_mode(q2, List<DottedRealAssignment>{dot(y)=y});
        b.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(y)=y},
                         y>=0, EventKind::URGENT);
        // A third component without any declaration for the event.
        HybridAutomaton c("c");
        c.new_mode(q1, List<DottedRealAssignment>{dot(z)=-z});
        c.new_mode(q2, List<DottedRealAssignment>{dot(z)=z});
        CompositeHybridAutomaton system(List<HybridAutomaton>{a,b,c});
        ARIADNE_TEST_THROWS(system.check_mode(q1), MultipleUrgentDeclarationError);
    }
    // One component urgent, another with a nontrivial permissive guard.
    {
        HybridAutomaton a("a"), b("b");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
        a.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(x)=x},
                         x>=0, EventKind::URGENT);
        b.new_mode(q1, List<DottedRealAssignment>{dot(y)=y});
        b.new_mode(q2, List<DottedRealAssignment>{dot(y)=y});
        b.new_transition(q1, e, q2, List<PrimedRealAssignment>{prime(y)=y},
                         y>=0, EventKind::PERMISSIVE);
        CompositeHybridAutomaton system(List<HybridAutomaton>{a,b});
        ARIADNE_TEST_THROWS(system.check_mode(q1),
                            UrgentDeclarationWithMultipleGuardsException);
    }
}

Void TestCompositeHybridAutomaton::test_missing_guard_error()
{
    // A transition (target+reset) without any guard is rejected when the
    // composite mode is materialized.
    RealVariable x("x");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"});
    DiscreteEvent e("e");
    HybridAutomaton a("a");
    a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
    a.new_mode(q2, List<DottedRealAssignment>{dot(x)=x});
    a.new_update(q1, e, q2, List<PrimedRealAssignment>{prime(x)=x});
    // Another mode of the same component declares the event kind, so the
    // diagnostic loop reports it before the error is thrown.
    a.new_guard(q2, e, x>=0, EventKind::PERMISSIVE);
    CompositeHybridAutomaton system(a);
    ARIADNE_TEST_THROWS(system.mode(q1), MissingGuardError);
}

Void TestCompositeHybridAutomaton::test_predicate_combination()
{
    // invariant_predicate / guard_predicate combine component predicates:
    // constant-true is skipped, constant-false dominates, two nonconstant
    // predicates are conjoined.
    RealVariable x("x"), y("y");
    StringVariable q("q");
    DiscreteLocation q1({q|"1"});
    DiscreteEvent inv_event("inv"), guard_event("grd");
    auto make=[&](ContinuousPredicate pa, ContinuousPredicate pb) {
        HybridAutomaton a("a"), b("b");
        a.new_mode(q1, List<DottedRealAssignment>{dot(x)=x});
        b.new_mode(q1, List<DottedRealAssignment>{dot(y)=y});
        a.new_invariant(q1, pa, inv_event);
        b.new_invariant(q1, pb, inv_event);
        a.new_guard(q1, guard_event, pa, EventKind::PERMISSIVE);
        b.new_guard(q1, guard_event, pb, EventKind::PERMISSIVE);
        return CompositeHybridAutomaton(List<HybridAutomaton>{a,b});
    };
    CompositeHybridAutomaton both_nonconstant=make(x<=1, y<=2);
    ARIADNE_TEST_PRINT(both_nonconstant.invariant_predicate(q1,inv_event));
    ARIADNE_TEST_PRINT(both_nonconstant.guard_predicate(q1,guard_event));
    CompositeHybridAutomaton one_true=make(x<=1, ContinuousPredicate(true));
    ARIADNE_TEST_PRINT(one_true.invariant_predicate(q1,inv_event));
    CompositeHybridAutomaton one_false=make(x<=1, ContinuousPredicate(false));
    ARIADNE_TEST_PRINT(one_false.invariant_predicate(q1,inv_event));
    ARIADNE_TEST_PRINT(one_false.guard_predicate(q1,guard_event));
}

Void TestCompositeHybridAutomaton::test_reachability()
{
    CompositeHybridAutomaton system=_composite();
    StringVariable q("q");
    DiscreteLocation q1({q|"1"}), q2({q|"2"}), q9({q|"9"});
    // Raise verbosity so the LOGGING_PRINTLN_AT branches in the reachability
    // loop take the printing side.
    const unsigned old_verbosity=Logger::instance().configuration().verbosity();
    Logger::instance().configuration().set_verbosity(10u);
    Set<DiscreteLocation> reached=system.discrete_reachability(Set<DiscreteLocation>{q1});
    Logger::instance().configuration().set_verbosity(old_verbosity);
    ARIADNE_TEST_ASSERT(reached.contains(q1));
    ARIADNE_TEST_ASSERT(reached.contains(q2));
    ARIADNE_TEST_ASSERT(!reached.contains(q9));
    // A self-loop reaches an already-reached target.
    {
        RealVariable x("x");
        DiscreteLocation l1({q|"1"}), l2({q|"2"});
        DiscreteEvent e("e"), self("self");
        HybridAutomaton a("a");
        a.new_mode(l1, List<DottedRealAssignment>{dot(x)=x});
        a.new_mode(l2, List<DottedRealAssignment>{dot(x)=-x});
        a.new_transition(l1, e, l2, List<PrimedRealAssignment>{prime(x)=x},
                         x>=0, EventKind::PERMISSIVE);
        a.new_transition(l2, self, l2, List<PrimedRealAssignment>{prime(x)=x},
                         x>=0, EventKind::PERMISSIVE);
        CompositeHybridAutomaton loop(a);
        Set<DiscreteLocation> reached2=loop.discrete_reachability(Set<DiscreteLocation>{l1});
        ARIADNE_TEST_ASSERT(reached2.contains(l1));
        ARIADNE_TEST_ASSERT(reached2.contains(l2));
    }
}

Void TestCompositeHybridAutomaton::test()
{
    ARIADNE_TEST_CALL(test_constructors_and_components());
    ARIADNE_TEST_CALL(test_accessors());
    ARIADNE_TEST_CALL(test_spaces());
    ARIADNE_TEST_CALL(test_writers());
    ARIADNE_TEST_CALL(test_check_mode_success());
    ARIADNE_TEST_CALL(test_check_mode_errors());
    ARIADNE_TEST_CALL(test_missing_guard_error());
    ARIADNE_TEST_CALL(test_predicate_combination());
    ARIADNE_TEST_CALL(test_reachability());
}

Int main()
{
    ARIADNE_TEST_CALL(TestCompositeHybridAutomaton().test());
    return ARIADNE_TEST_FAILURES;
}
