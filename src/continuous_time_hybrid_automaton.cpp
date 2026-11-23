/***************************************************************************
 *            composite_continuous_time_hybrid_automaton.cpp
 *
 *  Copyright  2004-20  Alberto Casagrande, Pieter Collins
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System.  If not, see <https://www.gnu.org/licenses/>.
 */

/** @file src/continuous_time_hybrid_automaton.cpp
 *  @brief Implement hybrid automata and discrete modes.
 */

#include "system/continuous_time_hybrid_automaton.hpp"

#include <map>

#include "function/function.hpp"
#include "function/functional.hpp"
#include "hybrid/hybrid_space.hpp"
#include "ordinary_dynamics.hpp"
#include "symbolic/expression.hpp"
#include "symbolic/space.hpp"
#include "system/odae.hpp"
#include "system/odai.hpp"
#include "system/system_function.hpp"
#include "utility/container.hpp"
#include "utility/macros.hpp"
#include "utility/stlio.hpp"

namespace Ariadne {

const OrdinaryDifferentialAlgebraicEquation* ContinuousTimeHybridAutomatonInterface::equation(DiscreteLocation) const { return nullptr; }
const OrdinaryDifferentialAlgebraicInclusion* ContinuousTimeHybridAutomatonInterface::inclusion(DiscreteLocation) const { return nullptr; }

std::optional<RealVariable> ContinuousTimeHybridAutomatonInterface::time_variable(DiscreteLocation location) const {
    if (const auto* model = equation(location))
        return model->time_variable();
    if (const auto* model = inclusion(location))
        return model->time_variable();
    return std::nullopt;
}

RealSpace ContinuousTimeHybridAutomatonInterface::continuous_argument_space(DiscreteLocation location) const {
    RealSpace parameters;
    if (const auto* model = equation(location))
        parameters = model->parameter_space();
    if (const auto* model = inclusion(location))
        parameters = model->parameter_space();
    return Detail::ordinary_argument_space(continuous_state_space(location), time_variable(location), parameters, RealSpace());
}

DiscreteMode::DiscreteMode() {
}

DiscreteMode::
    DiscreteMode(DiscreteLocation location,
                 const List<RealAssignment>& auxiliary,
                 const List<DottedRealAssignment>& dynamic)
    : HybridModeStructure(location), _auxiliary(auxiliary), _dynamic(dynamic) {
    _sorted_auxiliary = algebraic_sort(auxiliary);
}

template<class K, class T1, class T2>
Map<K, Pair<T1, T2>> merge(const Map<K, T1>& m1, const Map<K, T2>& m2) {
    ARIADNE_ASSERT(m1.keys() == m2.keys());
    Map<K, Pair<T1, T2>> r;
    Set<K> keys = m1.keys();
    for (typename Set<K>::ConstIterator iter = keys.begin(); iter != keys.end(); ++iter) {
        r.insert(*iter, make_pair(m1[*iter], m2[*iter]));
    }
    return r;
}

template<class X>
Set<Identifier> names(const Set<Variable<X>>& variables) {
    Set<Identifier> names;
    for (typename Set<Variable<X>>::ConstIterator variable_iter = variables.begin(); variable_iter != variables.end(); ++variable_iter) {
        names.insert(variable_iter->name());
    }
    return names;
}

template<class X>
Set<Identifier> names(const List<Variable<X>>& variables) {
    Set<Identifier> names;
    for (typename List<Variable<X>>::ConstIterator variable_iter = variables.begin(); variable_iter != variables.end(); ++variable_iter) {
        names.insert(variable_iter->name());
    }
    return names;
}

Writer<DiscreteMode> DiscreteMode::_default_writer(new VerboseDiscreteModeWriter());

OutputStream&
    DiscreteMode::_write(OutputStream& os) const {
    return os << _default_writer(*this);
}

OutputStream&
    VerboseDiscreteModeWriter::_write(OutputStream& os, const DiscreteMode& m) const {
    os << "DiscreteMode( "
       << "location=" << m._location;
    if (m._auxiliary.size() > 0) {
        os << ", algebraic_equations=" << m._auxiliary;
    }
    if (m._dynamic.size() > 0) {
        os << ", differential_equations=" << m._dynamic;
    }
    if (m._equation)
        os << ", continuous_dynamics=" << *m._equation;
    if (m._inclusion)
        os << ", continuous_dynamics=" << *m._inclusion;
    if (m._invariants.size() > 0) {
        os << ", invariants=" << m._invariants;
    }
    if (m._guards.size() > 0) {
        os << ", guards=" << m._guards;
    }
    if (m._targets.size() > 0) {
        os << ", targets=" << m._targets;
    }
    if (m._targets.size() > 0) {
        os << ", resets=" << m._resets;
    }
    return os << " )";
}

OutputStream&
    CompactDiscreteModeWriter::_write(OutputStream& os, const DiscreteMode& m) const {
    if (m._location.values().size() > 0)
        os << m._location << ": ";
    if (m._equation)
        os << "dynamics=" << *m._equation;
    if (m._inclusion)
        os << "dynamics=" << *m._inclusion;
    if (m._auxiliary.size() > 0 || m._dynamic.size() > 0)
        os << "eqt=";
    if (m._auxiliary.size() > 0) {
        os << m._auxiliary;
    }
    if (m._auxiliary.size() > 0 && m._dynamic.size() > 0)
        os << ",";
    if (m._dynamic.size() > 0) {
        os << m._dynamic;
    }
    if (m._invariants.size() > 0) {
        os << ", inv=" << m._invariants;
    }
    if (m._guards.size() > 0) {
        os << ", grd=" << m._guards;
    }
    if (m._targets.size() > 0) {
        os << ", trg=" << m._targets;
    }
    if (m._targets.size() > 0) {
        Map<DiscreteEvent, List<PrimedRealAssignment>> nonempty_resets;
        for (auto it = m._resets.begin(); it != m._resets.end(); ++it)
            if (it->second.size() != 0)
                nonempty_resets.insert(*it);
        if (not nonempty_resets.empty())
            os << ", rst=" << nonempty_resets;
    }
    return os;
}

ContinuousTimeHybridAutomaton::ContinuousTimeHybridAutomaton()
    : _name("automaton"), _modes() {
}

ContinuousTimeHybridAutomaton::ContinuousTimeHybridAutomaton(Identifier name)
    : _name(name), _modes() {
}

ContinuousTimeHybridAutomaton::ContinuousTimeHybridAutomaton(
    Identifier name,
    const List<StringVariable>&)
    : _name(name), _modes() {
}

Void ContinuousTimeHybridAutomaton::_new_mode(DiscreteLocation location,
                                const List<RealAssignment>& auxiliary,
                                const List<DottedRealAssignment>& dynamic) {
    for (Map<DiscreteLocation, DiscreteMode>::ConstIterator mode_iter = this->_modes.begin(); mode_iter != this->_modes.end(); ++mode_iter) {
        if (!are_distinguishable(location, mode_iter->first))
            ARIADNE_THROW(IndistinguishableModeError, "ContinuousTimeHybridAutomaton::new_mode",
                          "Location " << location << " is indistinguishable from location " << mode_iter->first << " in hybrid automaton with locations " << this->locations());
    }

    Set<UntypedVariable> defined_variables;
    Set<UntypedVariable> argument_variables;

    // Compute the auxiliary variables ordered by the given equations
    for (SizeType i = 0; i != auxiliary.size(); ++i) {
        if (defined_variables.contains(auxiliary[i].lhs))
            ARIADNE_THROW(SystemSpecificationError, "ContinuousTimeHybridAutomaton::new_mode",
                          "Variable " << auxiliary[i].lhs << " is defined twice by the algebraic equations " << auxiliary << " for mode " << location);

        defined_variables.insert(auxiliary[i].lhs);
        argument_variables.adjoin(auxiliary[i].rhs.arguments());
    }

    // Compute the state variables ordered by the given differential equations
    for (SizeType i = 0; i != dynamic.size(); ++i) {
        if (defined_variables.contains(dynamic[i].lhs.base()))
            ARIADNE_THROW(SystemSpecificationError, "ContinuousTimeHybridAutomaton::new_mode",
                          "Variable " << dynamic[i].lhs.base() << " defined by the differential equations " << dynamic << " for mode " << location << " is also defined in the algebraic equations " << auxiliary);

        defined_variables.insert(dynamic[i].lhs.base());
        argument_variables.adjoin(dynamic[i].rhs.arguments());
    }

    DiscreteMode new_mode = DiscreteMode(location, auxiliary, dynamic);

    this->_modes.insert(location, new_mode);
}

Void ContinuousTimeHybridAutomaton::new_mode(DiscreteLocation location, const OrdinaryDifferentialAlgebraicEquation& dynamics) {
    auto owned = std::shared_ptr<const OrdinaryDifferentialAlgebraicEquation>(dynamics.clone());
    this->_new_mode(location, dynamics._auxiliary, {});
    this->_modes.value(location)._equation = std::move(owned);
}

Void ContinuousTimeHybridAutomaton::new_mode(DiscreteLocation location, const OrdinaryDifferentialAlgebraicInclusion& dynamics) {
    auto owned = std::shared_ptr<const OrdinaryDifferentialAlgebraicInclusion>(dynamics.clone());
    this->_new_mode(location, {}, {});
    this->_modes.value(location)._inclusion = std::move(owned);
}

const OrdinaryDifferentialAlgebraicEquation* ContinuousTimeHybridAutomaton::equation(DiscreteLocation location) const {
    return this->mode(location)._equation.get();
}

const OrdinaryDifferentialAlgebraicInclusion* ContinuousTimeHybridAutomaton::inclusion(DiscreteLocation location) const {
    return this->mode(location)._inclusion.get();
}

std::optional<RealVariable> ContinuousTimeHybridAutomaton::time_variable(DiscreteLocation location) const {
    if (const auto declared = ContinuousTimeHybridAutomatonInterface::time_variable(location))
        return declared;
    const RealVariable clock = TimeVariable();
    if (continuous_state_space(location).contains(clock) || continuous_auxiliary_space(location).contains(clock))
        return std::nullopt;
    if (const auto* model = equation(location))
        if (model->parameter_space().contains(clock))
            return std::nullopt;
    if (const auto* model = inclusion(location))
        if (model->parameter_space().contains(clock) || model->input_space().contains(clock))
            return std::nullopt;
    const auto& data = mode(location);
    for (const auto& assignment: data._dynamic)
        if (assignment.rhs.arguments().contains(clock))
            return clock;
    for (const auto& assignment: data._auxiliary)
        if (assignment.rhs.arguments().contains(clock))
            return clock;
    for (const auto& item: data._guards)
        if (item.second.arguments().contains(clock))
            return clock;
    for (const auto& item: data._invariants)
        if (item.second.arguments().contains(clock))
            return clock;
    for (const auto& item: data._resets)
        for (const auto& assignment: item.second)
            if (assignment.rhs.arguments().contains(clock))
                return clock;
    return std::nullopt;
}

Void ContinuousTimeHybridAutomaton::_new_invariant(DiscreteLocation location, ContinuousPredicate invariant, DiscreteEvent event) {
    if (!this->has_location(location))
        ARIADNE_THROW(NonExistentModeError, "ContinuousTimeHybridAutomaton::new_invariant",
                      "mode " << location << " is not a location of the automaton with locations " << this->locations());

    DiscreteMode& mode = this->_modes.value(location);
    if (mode._kinds.has_key(event))
        ARIADNE_THROW(MultipleGuardError, "ContinuousTimeHybridAutomaton::new_invariant",
                      "Constraint for event " << event << " is already defined in mode " << mode);

    mode._invariants.insert(event, invariant);
    mode._kinds.insert(event, EventKind::INVARIANT);
}

Void ContinuousTimeHybridAutomaton::_new_guard(DiscreteLocation location, DiscreteEvent event, ContinuousPredicate guard, EventKind kind) {
    if (!this->has_location(location))
        ARIADNE_THROW(NonExistentModeError, "ContinuousTimeHybridAutomaton::new_guard",
                      "mode " << location << " is not a location of the automaton with locations " << this->locations());

    DiscreteMode& mode = this->_modes.value(location);
    if (mode._kinds.has_key(event))
        ARIADNE_THROW(MultipleGuardError, "ContinuousTimeHybridAutomaton::new_guard",
                      "Constraint for event " << event << " is already defined in mode " << mode);

    mode._guards.insert(event, guard);
    mode._kinds.insert(event, kind);
}

Void ContinuousTimeHybridAutomaton::_new_action(DiscreteLocation location,
                                  ContinuousPredicate invariant,
                                  DiscreteEvent event,
                                  ContinuousPredicate guard,
                                  EventKind kind) {
    if (!this->has_location(location))
        ARIADNE_THROW(NonExistentModeError, "ContinuousTimeHybridAutomaton::new_action",
                      "mode " << location << " is not a location of the automaton with locations " << this->locations());

    DiscreteMode& mode = this->_modes.value(location);
    if (mode._kinds.has_key(event))
        ARIADNE_THROW(MultipleGuardError, "ContinuousTimeHybridAutomaton::new_action",
                      "Constraint for event " << event << " is already defined in mode " << mode);

    mode._invariants.insert(event, invariant);
    mode._guards.insert(event, guard);
    mode._kinds.insert(event, kind);
}

Void ContinuousTimeHybridAutomaton::_new_update(DiscreteLocation source,
                                  DiscreteEvent event,
                                  DiscreteLocation target,
                                  const List<PrimedRealAssignment>& reset) {
    if (!this->has_location(source))
        ARIADNE_THROW(NonExistentModeError, "ContinuousTimeHybridAutomaton::new_reset",
                      "Source mode " << source << " is not a location of the automaton with locations " << this->locations());

    if (!this->has_location(target))
        ARIADNE_THROW(NonExistentModeError, "ContinuousTimeHybridAutomaton::new_reset",
                      "Target mode " << target << " is not a location of the automaton with locations " << this->locations());

    DiscreteMode& source_mode = this->mode(source); // Non-constant since we may wish to update input variables
    if (source_mode._targets.has_key(event))
        ARIADNE_THROW(MultipleTransitionError, "ContinuousTimeHybridAutomaton::new_update",
                      "Update for event " << event << " is already defined in mode " << source);

    Set<RealVariable> target_auxiliary_variable = make_set(this->auxiliary_variables(target));
    if (!disjoint(target_auxiliary_variable, left_hand_sides(reset)))
        ARIADNE_THROW(OverspecifiedResetError, "ContinuousTimeHybridAutomaton::new_update",
                      "reset " << reset << " for event " << event << " in source " << source << " specifies variables " << intersection(target_auxiliary_variable, left_hand_sides(reset)) << " which are auxiliary variables in target " << target);

    if (!unique_elements(left_hand_sides(reset)))
        ARIADNE_THROW(OverspecifiedResetError, "ContinuousTimeHybridAutomaton::new_update",
                      "reset " << reset << " for event " << event << " in source " << source << "overspecifies variables " << duplicate_elements(left_hand_sides(reset)));

    List<RealVariable> target_state_space = this->state_variables(target);
    Set<RealVariable> target_state_variables(target_state_space);
    Set<RealVariable> reset_variables(left_hand_sides(reset));

    source_mode.add_transition(event, target);
    source_mode._resets.insert(event, reset);
}

Void ContinuousTimeHybridAutomaton::_new_transition(DiscreteLocation source,
                                      DiscreteEvent event,
                                      DiscreteLocation target,
                                      const List<PrimedRealAssignment>& reset,
                                      ContinuousPredicate guard,
                                      EventKind kind) {
    if (kind == EventKind::URGENT || kind == EventKind::IMPACT) {
        this->_new_action(source, !guard, event, guard, kind);
    } else if (kind == EventKind::PERMISSIVE) {
        this->_new_guard(source, event, guard, kind);
    } else
        ARIADNE_FAIL_MSG("Unhandled event kind " << kind);

    this->_new_update(source, event, target, reset);
}

Set<DiscreteLocation>
    ContinuousTimeHybridAutomaton::locations() const {
    return this->_modes.keys();
}

Bool ContinuousTimeHybridAutomaton::has_location(DiscreteLocation location) const {
    return this->_modes.has_key(location);
}

Bool ContinuousTimeHybridAutomaton::has_mode(DiscreteLocation location) const {
    for (Map<DiscreteLocation, DiscreteMode>::ConstIterator mode_iter = this->_modes.begin();
         mode_iter != this->_modes.end(); ++mode_iter) {
        if (are_same(mode_iter->first, location)) {
            return true;
        }
    }
    return false;
}

Bool ContinuousTimeHybridAutomaton::has_partial_mode(DiscreteLocation location) const {
    for (Map<DiscreteLocation, DiscreteMode>::ConstIterator mode_iter = this->_modes.begin();
         mode_iter != this->_modes.end(); ++mode_iter) {
        if (is_restriction(mode_iter->first, location)) {
            return true;
        }
    }
    return false;
}

Bool ContinuousTimeHybridAutomaton::has_transition(DiscreteLocation source, DiscreteEvent event) const {
    return this->has_partial_mode(source) && this->mode(source).has_transition(event);
}

Bool ContinuousTimeHybridAutomaton::has_invariant(DiscreteLocation source, DiscreteEvent event) const {
    return this->has_partial_mode(source) && this->mode(source)._invariants.has_key(event);
}

Bool ContinuousTimeHybridAutomaton::has_guard(DiscreteLocation source, DiscreteEvent event) const {
    return this->has_partial_mode(source) && this->mode(source)._guards.has_key(event);
}

const Map<DiscreteLocation, DiscreteMode>&
    ContinuousTimeHybridAutomaton::modes() const {
    return this->_modes;
}

// Set<DiscreteMode>
// ContinuousTimeHybridAutomaton::modes() const
//{
//     return Set<DiscreteMode>(this->_modes.values());
// }

DiscreteMode&
    ContinuousTimeHybridAutomaton::mode(DiscreteLocation location) {
    const ContinuousTimeHybridAutomaton& self = *this;
    return const_cast<DiscreteMode&>(self.mode(location));
}

const DiscreteMode&
    ContinuousTimeHybridAutomaton::mode(DiscreteLocation location) const {
    const DiscreteMode* result = nullptr;
    for (Map<DiscreteLocation, DiscreteMode>::ConstIterator mode_iter = this->_modes.begin();
         mode_iter != this->_modes.end(); ++mode_iter) {
        if (is_restriction(mode_iter->first, location)) {
            result = &mode_iter->second;
            break;
        }
    }
    if (result == nullptr)
        ARIADNE_THROW(SystemSpecificationError, "ContinuousTimeHybridAutomaton::mode(DiscreteLocation)",
                      location << " does not define a mode of the automaton with locations " << this->locations());
    return *result;
}

Writer<ContinuousTimeHybridAutomaton> ContinuousTimeHybridAutomaton::_default_writer(new VerboseHybridAutomatonWriter());

OutputStream&
    ContinuousTimeHybridAutomaton::_write(OutputStream& os) const {
    return os << _default_writer(*this);
}

OutputStream&
    VerboseHybridAutomatonWriter::_write(OutputStream& os, const ContinuousTimeHybridAutomaton& ha) const {
    os << "\nHybridAutomaton( \n  modes=\n";
    Set<DiscreteMode> modes(ha.modes().values());
    for (Set<DiscreteMode>::ConstIterator mode_iter = modes.begin();
         mode_iter != modes.end(); ++mode_iter) {
        os << "    " << *mode_iter << ",\n";
    }
    return os << ")\n";
}

OutputStream&
    CompactHybridAutomatonWriter::_write(OutputStream& os, const ContinuousTimeHybridAutomaton& ha) const {
    os << ha.name() << ": ";
    Set<DiscreteMode> modes(ha.modes().values());
    Bool multiple_modes = (modes.size() > 1);
    Writer<DiscreteMode> previous_mode_writer = DiscreteMode::default_writer();
    DiscreteMode::set_default_writer(CompactDiscreteModeWriter());
    for (Set<DiscreteMode>::ConstIterator mode_iter = modes.begin();
         mode_iter != modes.end(); ++mode_iter) {
        if (multiple_modes)
            os << "\n - ";
        os << *mode_iter;
    }
    DiscreteMode::set_default_writer(previous_mode_writer);
    return os << "\n";
}

List<RealVariable>
    ContinuousTimeHybridAutomaton::state_variables(DiscreteLocation location) const {
    List<RealVariable> result;
    const DiscreteMode& mode = this->mode(location);
    if (mode._equation)
        return mode._equation->state_space().variables();
    if (mode._inclusion)
        return mode._inclusion->state_space().variables();
    for (SizeType i = 0; i != mode._dynamic.size(); ++i) {
        result.append(mode._dynamic[i].lhs.base());
    }
    return result;
}

List<RealVariable>
    ContinuousTimeHybridAutomaton::auxiliary_variables(DiscreteLocation location) const {
    List<RealVariable> result;
    const DiscreteMode& mode = this->mode(location);
    if (mode._equation)
        return mode._equation->auxiliary_space().variables();
    if (mode._inclusion)
        return {};
    for (SizeType i = 0; i != mode._auxiliary.size(); ++i) {
        result.append(mode._auxiliary[i].lhs);
    }
    return result;
}

Set<RealVariable>
    ContinuousTimeHybridAutomaton::input_variables(DiscreteLocation location) const {
    Set<UntypedVariable> used;
    Set<RealVariable> defined;
    const DiscreteMode& mode = this->mode(location);
    if (mode._inclusion)
        return make_set(mode._inclusion->input_space().variables());
    if (mode._equation)
        return {};
    for (List<RealAssignment>::ConstIterator iter = mode._auxiliary.begin(); iter != mode._auxiliary.end(); ++iter) {
        defined.insert(iter->lhs.base());
        used.adjoin(iter->rhs.arguments());
    }
    for (List<DottedRealAssignment>::ConstIterator iter = mode._dynamic.begin(); iter != mode._dynamic.end(); ++iter) {
        defined.insert(iter->lhs.base());
        used.adjoin(iter->rhs.arguments());
    }
    for (Map<DiscreteEvent, ContinuousPredicate>::ConstIterator iter = mode._invariants.begin(); iter != mode._invariants.end(); ++iter) {
        used.adjoin(iter->second.arguments());
    }
    for (Map<DiscreteEvent, ContinuousPredicate>::ConstIterator iter = mode._guards.begin(); iter != mode._guards.end(); ++iter) {
        used.adjoin(iter->second.arguments());
    }
    for (Map<DiscreteEvent, List<PrimedRealAssignment>>::ConstIterator iter = mode._resets.begin(); iter != mode._resets.end(); ++iter) {
        const List<PrimedRealAssignment>& reset = iter->second;
        for (List<PrimedRealAssignment>::ConstIterator rst_iter = reset.begin(); rst_iter != reset.end(); ++rst_iter) {
            used.adjoin(rst_iter->rhs.arguments());
        }
    }
    Set<RealVariable> result;
    for (Set<UntypedVariable>::ConstIterator iter = used.begin(); iter != used.end(); ++iter) {
        RealVariable var(iter->name());
        if (!defined.contains(var) && !(time_variable(location) && var == *time_variable(location))) {
            result.insert(var);
        }
    }
    return result;
}

DiscreteLocation
    ContinuousTimeHybridAutomaton::target(DiscreteLocation source, DiscreteEvent event) const {
    if (this->has_transition(source, event)) {
        return this->mode(source).transition_target(event, source);
    } else {
        return source;
    }
}

List<RealAssignment>
    ContinuousTimeHybridAutomaton::auxiliary_assignments(DiscreteLocation location) const {
    const auto& mode = this->mode(location);
    return mode._auxiliary;
}

/*
List<RealAssignment>
ContinuousTimeHybridAutomaton::sorted_auxiliary_assignments(DiscreteLocation location) const {
    DiscreteMode const& mode=this->mode(location);
    assert(mode._sorted_auxiliary.size()==mode._auxiliary.size());
    return mode._sorted_auxiliary;
}
*/

List<DottedRealAssignment>
    ContinuousTimeHybridAutomaton::dynamic_assignments(DiscreteLocation location) const {
    const auto& mode = this->mode(location);
    if (mode._equation || mode._inclusion)
        throw std::logic_error("Use the original typed dynamics declaration for this mode");
    return mode._dynamic;
}

List<PrimedRealAssignment>
    ContinuousTimeHybridAutomaton::reset_assignments(DiscreteLocation source, DiscreteEvent event) const {
    if (this->has_transition(source, event)) {
        return this->mode(source)._resets[event];
    } else {
        List<PrimedRealAssignment> nonjump_assignments;
        List<RealVariable> state_variables = this->state_variables(source);
        for (SizeType i = 0; i != state_variables.size(); ++i) {
            nonjump_assignments.append(next(state_variables[i]) = state_variables[i]);
        }
        return nonjump_assignments;
    }
}

ContinuousPredicate
    ContinuousTimeHybridAutomaton::invariant_predicate(DiscreteLocation location, DiscreteEvent action) const {
    const DiscreteMode& mode = this->mode(location);
    if (mode._invariants.has_key(action)) {
        return mode._invariants[action];
    } else {
        return ContinuousPredicate(true);
    }
}

ContinuousPredicate
    ContinuousTimeHybridAutomaton::guard_predicate(DiscreteLocation location, DiscreteEvent event) const {
    const DiscreteMode& mode = this->mode(location);
    if (mode._guards.has_key(event)) {
        return mode._guards[event];
    } else {
        return ContinuousPredicate(true);
    }
}

HybridSpace ContinuousTimeHybridAutomaton::state_space() const {
    MonolithicHybridSpace space;
    for (Map<DiscreteLocation, DiscreteMode>::ConstIterator iter = this->_modes.begin(); iter != this->_modes.end(); ++iter) {
        const DiscreteLocation& loc = iter->first;
        space.new_location(loc, this->continuous_state_space(loc));
    }
    return space;
}

HybridSpace ContinuousTimeHybridAutomaton::state_auxiliary_space() const {
    MonolithicHybridSpace space;
    for (Map<DiscreteLocation, DiscreteMode>::ConstIterator iter = this->_modes.begin(); iter != this->_modes.end(); ++iter) {
        const DiscreteLocation& loc = iter->first;
        space.new_location(loc, join(this->continuous_state_space(loc), this->continuous_auxiliary_space(loc)));
    }
    return space;
}

Set<DiscreteEvent> ContinuousTimeHybridAutomaton::events(DiscreteLocation location) const {
    const DiscreteMode& mode = this->mode(location);
    return join(join(mode._invariants.keys(), mode._guards.keys()), mode.transition_events());
}

DimensionType ContinuousTimeHybridAutomaton::dimension(DiscreteLocation location) const {
    return this->continuous_state_space(location).dimension();
}

RealSpace ContinuousTimeHybridAutomaton::continuous_state_space(DiscreteLocation location) const {
    return RealSpace(this->state_variables(location));
}

RealSpace ContinuousTimeHybridAutomaton::continuous_auxiliary_space(DiscreteLocation location) const {
    const auto& mode = this->mode(location);
    if (mode._equation)
        return mode._equation->auxiliary_space();
    if (mode._inclusion)
        return RealSpace();
    return RealSpace(left_hand_sides(mode._sorted_auxiliary));
}

EventKind ContinuousTimeHybridAutomaton::event_kind(DiscreteLocation location, DiscreteEvent event) const {
    return this->mode(location)._kinds[event];
}

EffectiveVectorMultivariateFunction ContinuousTimeHybridAutomaton::auxiliary_function(DiscreteLocation location) const {
    const DiscreteMode& mode = this->mode(location);
    return Ariadne::make_auxiliary_function(this->continuous_argument_space(location), mode._sorted_auxiliary);
}

EffectiveVectorMultivariateFunction ContinuousTimeHybridAutomaton::dynamic_function(DiscreteLocation location) const {
    const DiscreteMode& mode = this->mode(location);
    if (mode._inclusion)
        throw std::logic_error("An inclusion mode has no single-valued state-only dynamic function");
    return Ariadne::make_dynamic_function(this->continuous_argument_space(location), mode._sorted_auxiliary,
                                          mode._equation ? mode._equation->_dynamics : mode._dynamic);
}

EffectiveVectorMultivariateFunction ContinuousTimeHybridAutomaton::reset_function(DiscreteLocation location, DiscreteEvent event) const {
    const DiscreteMode& mode = this->mode(location);
    return Ariadne::make_reset_function(this->continuous_argument_space(location), mode._sorted_auxiliary, mode._resets[event]);
}

EffectiveScalarMultivariateFunction ContinuousTimeHybridAutomaton::invariant_function(DiscreteLocation location, DiscreteEvent event) const {
    const DiscreteMode& mode = this->mode(location);
    return Ariadne::make_constraint_function(this->continuous_argument_space(location), mode._auxiliary, mode._invariants[event], Sign::NEGATIVE);
}

EffectiveScalarMultivariateFunction ContinuousTimeHybridAutomaton::guard_function(DiscreteLocation location, DiscreteEvent event) const {
    const DiscreteMode& mode = this->mode(location);
    return Ariadne::make_constraint_function(this->continuous_argument_space(location), mode._sorted_auxiliary, mode._guards[event], Sign::POSITIVE);
}

Void ContinuousTimeHybridAutomaton::check_mode(DiscreteLocation location) const {
    const DiscreteMode& mode = this->mode(location);

    List<RealVariable> defined_real_variables(catenate(this->continuous_argument_space(location).variables(), this->auxiliary_variables(location)));
    Set<UntypedVariable> defined_variables(make_set(defined_real_variables));

    if (!mode._equation)
        for (List<RealAssignment>::ConstIterator aux_iter = mode._auxiliary.begin(); aux_iter != mode._auxiliary.end(); ++aux_iter) {
            if (!subset(aux_iter->rhs.arguments(), defined_variables))
                ARIADNE_THROW(UnderspecifiedDynamicError, "ContinuousTimeHybridAutomaton::check_mode(...)",
                              "Arguments " << difference(aux_iter->rhs.arguments(), defined_variables) << " of " << *aux_iter << " are not defined in location " << location << " with variables " << defined_variables);
        }

    for (List<DottedRealAssignment>::ConstIterator dyn_iter = mode._dynamic.begin(); dyn_iter != mode._dynamic.end(); ++dyn_iter) {
        if (!subset(dyn_iter->rhs.arguments(), defined_variables))
            ARIADNE_THROW(UnderspecifiedDynamicError, "ContinuousTimeHybridAutomaton::check_mode(...)",
                          "Arguments " << difference(dyn_iter->rhs.arguments(), defined_variables) << " of " << *dyn_iter << " are not defined in location " << location << " with variables " << defined_variables);
    }

    for (Map<DiscreteEvent, ContinuousPredicate>::ConstIterator inv_iter = mode._invariants.begin(); inv_iter != mode._invariants.end(); ++inv_iter) {
        if (!subset(inv_iter->second.arguments(), defined_variables))
            ARIADNE_THROW(UnderspecifiedConstraintError, "ContinuousTimeHybridAutomaton::check_mode(...)",
                          "Arguments " << difference(inv_iter->second.arguments(), defined_variables) << " of " << *inv_iter << " are not defined in location " << location << " with variables " << defined_variables);
    }

    for (Map<DiscreteEvent, ContinuousPredicate>::ConstIterator grd_iter = mode._guards.begin(); grd_iter != mode._guards.end(); ++grd_iter) {
        if (!subset(grd_iter->second.arguments(), defined_variables))
            ARIADNE_THROW(UnderspecifiedConstraintError, "ContinuousTimeHybridAutomaton::check_mode(...)",
                          "Arguments " << difference(grd_iter->second.arguments(), defined_variables) << " of " << *grd_iter << " are not defined in location " << location << " with variables " << defined_variables);
    }

    for (Map<DiscreteEvent, List<PrimedRealAssignment>>::ConstIterator rst_iter = mode._resets.begin(); rst_iter != mode._resets.end(); ++rst_iter) {
        const List<PrimedRealAssignment>& reset = rst_iter->second;
        for (List<PrimedRealAssignment>::ConstIterator pra_iter = reset.begin(); pra_iter != reset.end(); ++pra_iter) {
            if (!subset(pra_iter->rhs.arguments(), defined_variables))
                ARIADNE_THROW(UnderspecifiedResetError, "ContinuousTimeHybridAutomaton::check_mode(...)",
                              "Arguments " << difference(pra_iter->rhs.arguments(), defined_variables) << " of reset " << *pra_iter << " are not defined in location " << location << " with variables " << defined_variables);
        }
    }

    for (Map<DiscreteEvent, List<PrimedRealAssignment>>::ConstIterator rst_iter = mode._resets.begin(); rst_iter != mode._resets.end(); ++rst_iter) {
        DiscreteLocation target = mode._targets[rst_iter->first];
        Set<RealVariable> reset_variables(left_hand_sides(rst_iter->second));
        Set<RealVariable> target_state_variables(this->state_variables(target));
        if (!subset(reset_variables, target_state_variables))
            ARIADNE_THROW(UnderspecifiedDynamicError, "ContinuousTimeHybridAutomaton::check_mode(...)",
                          "Variables " << difference(reset_variables, target_state_variables) << " of reset" << *rst_iter << " are not defined in location " << target << " with state variables " << target_state_variables);

        if (!subset(target_state_variables, reset_variables))
            ARIADNE_THROW(UnderspecifiedResetError, "ContinuousTimeHybridAutomaton::check_mode(...)",
                          "Variables " << difference(target_state_variables, reset_variables) << " in target" << target << " are not defined reset " << *rst_iter);
    }
}

Void ContinuousTimeHybridAutomaton::check_reachable_modes(const Set<DiscreteLocation>& initial_locations) const {
    Set<DiscreteLocation> reachable_locations = this->discrete_reachability(initial_locations);
    for (Set<DiscreteLocation>::ConstIterator iter = reachable_locations.begin(); iter != reachable_locations.end(); ++iter) {
        this->check_mode(*iter);
    }
}

Set<DiscreteLocation> ContinuousTimeHybridAutomaton::discrete_reachability(const Set<DiscreteLocation>& initial_locations) const {
    const ContinuousTimeHybridAutomaton& automaton = *this;

    Set<DiscreteLocation> reached = initial_locations;
    Set<DiscreteLocation> working = initial_locations;
    Set<DiscreteLocation> found;

    Map<DiscreteLocation, Natural> steps;
    Natural step = 0u;

    for (Set<DiscreteLocation>::ConstIterator initial_iter = initial_locations.begin(); initial_iter != initial_locations.end(); ++initial_iter) {
        steps.insert(*initial_iter, step);
    }

    while (!working.empty()) {
        ++step;
        for (Set<DiscreteLocation>::ConstIterator source_iter = working.begin(); source_iter != working.end(); ++source_iter) {
            DiscreteLocation location = *source_iter;
            Set<DiscreteEvent> events = automaton.events(location);
            for (Set<DiscreteEvent>::ConstIterator event_iter = events.begin(); event_iter != events.end(); ++event_iter) {
                DiscreteEvent event = *event_iter;
                DiscreteLocation target = automaton.target(location, event);
                if (!reached.contains(target)) {
                    found.insert(target);
                    reached.insert(target);
                    steps.insert(target, step);
                }
            }
        }
        working.clear();
        working.swap(found);
    }

    return reached;
}

} // namespace Ariadne
